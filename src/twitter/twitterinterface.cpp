/*
 * Copyright (C) 2013 Jolla Ltd. <chris.adams@jollamobile.com>
 *
 * You may use this file under the terms of the BSD license as follows:
 *
 * "Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in
 *     the documentation and/or other materials provided with the
 *     distribution.
 *   * Neither the name of Nemo Mobile nor the names of its contributors
 *     may be used to endorse or promote products derived from this
 *     software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
 */

#include "twitterinterface.h"
#include "twitterinterface_p.h"
#include "twitterontology_p.h"
#include "socialnetworkinterface_p.h"

#include "contentiteminterface.h"
#include "contentiteminterface_p.h"
#include "identifiablecontentiteminterface.h"
#include "contentitemtypefilterinterface.h"

#include "twitteruserinterface.h"
//#include "twittertweetinterface.h"
//#include "twitterplaceinterface.h"

#include "util_p.h"

#include <QtCore/QVariantMap>
#include <QtCore/QStringList>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QByteArray>
#include <QtNetwork/QNetworkReply>

#include <QtDebug>

TwitterInterfacePrivate::TwitterInterfacePrivate(TwitterInterface *q)
    : SocialNetworkInterfacePrivate(q)
    , populatePending(false)
    , populateDataForUnseenPending(false)
    , continuationRequestActive(false)
    , outOfBandConnectionsLimit(-1)
    , internalStatus(TwitterInterfacePrivate::Idle)
    , currentReply(0)
{
}

/*!
    \internal

    Attempt to detect which type of object the given \a data represents.
    This function makes use of some fairly inexact heuristics, and only
    supports tweets, places and users.
*/
int TwitterInterfacePrivate::detectTypeFromData(const QVariantMap &data) const
{
    // it's possible that we've already tagged the type already
    if (data.contains(NEMOQMLPLUGINS_SOCIAL_CONTENTITEMTYPE)) {
        TwitterInterface::ContentItemType taggedType = static_cast<TwitterInterface::ContentItemType>(data.value(NEMOQMLPLUGINS_SOCIAL_CONTENTITEMTYPE).toInt());
        if (taggedType > TwitterInterface::Unknown) {
            return taggedType;
        }
    }

    qWarning() << Q_FUNC_INFO << "No type information available in object metadata - attempting to heuristically detect type";
    if (data.value(TWITTER_ONTOLOGY_PROPERTY_TWEET_TEXT).isValid() && data.value(TWITTER_ONTOLOGY_PROPERTY_TWEET_RETWEETED).isValid())
        return TwitterInterface::Tweet;
    else if (data.value(TWITTER_ONTOLOGY_PROPERTY_PLACE_PLACETYPE).isValid() && data.value(TWITTER_ONTOLOGY_PROPERTY_COUNTRY).isValid())
        return TwitterInterface::Place;
    else if (data.value(TWITTER_ONTOLOGY_PROPERTY_USER_SCREENNAME).isValid() || data.value(TWITTER_ONTOLOGY_PROPERTY_USER_FOLLOWREQUESTSENT).isValid())
        return TwitterInterface::User;

    qWarning() << Q_FUNC_INFO << "Unable to heuristically detect type!";
    return TwitterInterface::Unknown;
}

/*! \internal */
QUrl TwitterInterfacePrivate::requestUrl(const QString &objectId, const QString &extraPath, const QStringList &whichFields, const QVariantMap &extraData)
{
    Q_UNUSED(whichFields); // XXX TODO: trim fields not included in whichFields ?

    // In Twitter, the extraPath actually comes _before_ the id, to identify the type of object and the operation.
    // eg: https://api.twitter.com/1.1/statuses/show.json?id=12345
    // https              -> scheme
    // api.twitter.com    -> host
    // /1.1/              -> api version, constant
    // statuses/show.json -> extraPath
    // id=12345           -> object id.

    // XXX TODO: check the objectId, determine if it's a numeric id or a screen name.
    // Build up a hash of screen name to numeric id, for use in the cache, later.

    QList<QPair<QString, QString> > queryItems;
    QStringList extraDataKeys = extraData.keys();
    foreach (const QString &edk, extraDataKeys)
        queryItems.append(qMakePair<QString, QString>(edk, extraData.value(edk).toString()));

    QString modifiedExtraPath = extraPath;
    if (extraPath.isEmpty()) {
        // the extra path will only be empty if the request url is being generated
        // for a "setNodeIdentifier" operation.  In this case, we need to do some
        // tricky stuff to figure out what the correct request will be, since in
        // the Twitter API, you can't just specify an id: you also need to specify
        // an operation / API call, appropriate to the object type...
        if (objectId == TWITTER_ONTOLOGY_TYPE_USER_ME) {
            // if objectId is the special "me" id, we get the current user information.
            modifiedExtraPath = TWITTER_ONTOLOGY_QUERY_USER_ME;
        } else if (TwitterDataUtil::idIsScreenName(objectId)) {
            // if objectId is a screen name, we assume that the client wants to show a User.
            queryItems.append(qMakePair<QString, QString(TWITTER_ONTOLOGY_PROPERTY_USER_SCREENNAME, objectId));
            modifiedExtraPath = TWITTER_ONTOLOGY_QUERY_USER;
        } else {
            // could be a tweet, a direct message, or a user.
            TwitterInterface::ContentItemType objectType = m_idsToTypes.value(objectId, TwitterInterface::Unknown);
            if (objectType == TwitterInterface::Unknown) {
                // XXX TODO: try/fail loop: status, directmessage, place, user.
                qWarning() << Q_FUNC_INFO << "TODO: exploratory requests for unknown id";
            } else if (objectType == TwitterInterface::User) {
                // user object queries require "user_id" specified
                queryItems.append(qMakePair<QString, QString(TWITTER_ONTOLOGY_PROPERTY_USER_USERID, objectId));
                modifiedExtraPath = TWITTER_ONTOLOGY_QUERY_USER;
            } else if (objectType == TwitterInterface::Tweet) {
                // tweet object queries require "id" specified
                queryItems.append(qMakePair<QString, QString(TWITTER_ONTOLOGY_PROPERTY_TWEET_ID, objectId));
                modifiedExtraPath = TWITTER_ONTOLOGY_QUERY_TWEET;
            } else if (objectType == TwitterInterface::DirectMessage) {
                // direct message object queries require "id" specified
                queryItems.append(qMakePair<QString, QString(TWITTER_ONTOLOGY_PROPERTY_DIRECTMESSAGE_ID, objectId));
                modifiedExtraPath = TWITTER_ONTOLOGY_QUERY_DIRECTMESSAGE;
            } else if (objectType == TwitterInterface::Place) {
                // place object queries require modifying the path instead of adding a query item
                modifiedExtraPath = QString(TWITTER_ONTOLOGY_QUERY_PLACE).arg(objectId);
            } else {
                qWarning() << Q_FUNC_INFO << "unsupported node type:" << objectType << "for" << objectId;
            }
        }
    } else {
        // if an extra path is supplied, it's being generated via a filter
        // and we should therefore know what the node object type is (and
        // we can check that the filter is appropriate for that type).
        // So, we can only get to this code if it is appropriate, and thus
        // we can determine from the operation type, what sort of object
        // is being represented by the id.
        if (objectId.isEmpty()) {
            // the api call is something for the "me" user (eg, user_timeline etc)
            // we shouldn't need to modify the extra path or set an id, as the ontology string should be fine.
        } else if (modifiedExtraPath == TWITTER_ONTOLOGY_CONNECTIONS_TWEET_RETWEETS) {
            // the given object id must be a tweet id.  The retweets query requires modifying the path.
            modifiedExtraPath = modifiedExtraPath.arg(objectId);
        } else if (modifiedExtraPath == TWITTER_ONTOLOGY_CONNECTIONS_PLACE_TRENDS) {
            // the given object id must be a WOE id
            queryItems.append(qMakePair<QString, QString(TWITTER_ONTOLOGY_PROPERTY_PLACE_ID, objectId));
        } else {
            // need to implement all supported operations.
            qWarning() << Q_FUNC_INFO << "TODO: extraPath:" << modifiedExtraPath;
        }
    }

    QUrl retn;
    retn.setScheme("https");
    retn.setHost("api.twitter.com");
    retn.setPath(QLatin1String("1.1/") + modifiedExtraPath);
    retn.setQueryItems(queryItems);
    return retn;
}

void TwitterInterfacePrivate::setCurrentReply(QNetworkReply *newCurrentReply, const QString &whichNodeIdentifier)
{
    deleteReply();
    newCurrentReply->setProperty("whichNodeIdentifier", whichNodeIdentifier);
    currentReply = newCurrentReply;
}

void TwitterInterfacePrivate::connectFinishedAndErrors()
{
    Q_Q(TwitterInterface);
    QObject::connect(currentReply, SIGNAL(finished()), q, SLOT(finishedHandler()));
    QObject::connect(currentReply, SIGNAL(error(QNetworkReply::NetworkError)), q, SLOT(errorHandler(QNetworkReply::NetworkError)));
    QObject::connect(currentReply, SIGNAL(sslErrors(QList<QSslError>)), q, SLOT(sslErrorsHandler(QList<QSslError>)));
}

/*! \internal */
void TwitterInterfacePrivate::finishedHandler()
{
    Q_Q(TwitterInterface);
    if (!currentReply) {
        // if an error occurred, it might have been deleted by the error handler.
        qWarning() << Q_FUNC_INFO << "network request finished but no reply!";
        return;
    }

    if (currentReply->property("whichNodeIdentifier").toString() != q->nodeIdentifier()) {
        // the data we've received is not for the current node.
        // The client must have changed the node while the request was in process.
        // Note that this should NEVER happen, as we should always use SNIP::setCurrentReply()
        // which should delete the old reply (and thus the disconnect it from the finished handler).
        qWarning() << Q_FUNC_INFO
                   << "error: network request finished for non-current node:"
                   << currentReply->property("whichNodeIdentifier").toString()
                   << ", expected:" << q->nodeIdentifier();
        return; // ignore this data, as otherwise we will corrupt the cache
    }

    QByteArray replyData = currentReply->readAll();
    QUrl requestUrl = currentReply->request().url();
    QVariant specialLimitVar = currentReply->property("specialLimit");
    deleteReply();
    bool ok = false;
    QVariant responseData = TwitterDataUtil::parseReplyData(replyData, &ok);
    if (!ok) {
        responseData.insert("response", replyData);
        error = SocialNetworkInterface::RequestError;
        errorMessage = QLatin1String("Error populating node: response is invalid.  Perhaps the requested object id was incorrect?  Response: ") + QString::fromLatin1(replyData.constData());
        status = SocialNetworkInterface::Error;
        emit q->statusChanged();
        emit q->errorChanged();
        emit q->errorMessageChanged();
        return;
    }

    if (responseData.type() == QVariant::Map && responseData.toMap().contains(QLatin1String("error"))) {
        responseMap = responseData.toMap();
        QString errorResponse = QLatin1String("\n    error:");
        QVariantMap errMap = responseMap.value(QLatin1String("error")).toMap();
        QStringList keys = errMap.keys();
        foreach (const QString &key, keys) {
            errorResponse += QLatin1String("\n        ") + key + QLatin1String("=") + errMap.value(key).toString();
        }
        qWarning() << Q_FUNC_INFO << "error response:" << errorResponse << "while getting:" << requestUrl.toString();

        error = SocialNetworkInterface::RequestError;
        errorMessage = QLatin1String("Error populating node: response is error.  Response: ") + QString::fromLatin1(replyData.constData());
        status = SocialNetworkInterface::Error;
        emit q->statusChanged();
        emit q->errorChanged();
        emit q->errorMessageChanged();
        return;
    }

    // some forms of requests require manual limit checking.
    int specialLimit = -1;
    if (specialLimitVar.isValid()) {
        specialLimit = specialLimitVar.toInt();
    }

    if (internalStatus == TwitterInterfacePrivate::PopulatingUnseenNode) {
        // This one is tricky, because we don't know the type of the current node.
        q->continuePopulateDataForUnseenNode(responseData);
    } else if (internalStatus == TwitterInterfacePrivate::PopulatingSeenNode) {
        // This one should be simpler because each of the requested fields/connections is a property.
        outOfBandConnectionsLimit = specialLimit;
        q->continuePopulateDataForSeenNode(responseData, requestUrl);
    } else {
        qWarning() << Q_FUNC_INFO << "Error: network reply finished while in unexpectant state!  Received:" << responseData;
    }
}

/*! \internal */
void TwitterInterfacePrivate::errorHandler(QNetworkReply::NetworkError err)
{
    Q_Q(TwitterInterface);
    if (err == QNetworkReply::UnknownContentError) {
        // ignore this.  It's not actually an error, Twitter just formats some responses strangely.
        return;
    }
    errorMessage = networkErrorString(err);

    qWarning() << Q_FUNC_INFO << "Error: network error occurred:" << err << ":" << errorMessage;

    error = SocialNetworkInterface::RequestError;
    status = SocialNetworkInterface::Error;

    if ((internalStatus == TwitterInterfacePrivate::PopulatingUnseenNode
            || internalStatus == TwitterInterfacePrivate::PopulatingSeenNode)
            && repopulatingCurrentNode) {
        // failed repopulating, either at "get node" step, or at "get related data" step.
        repopulatingCurrentNode = false;
    }

    if (continuationRequestActive) {
        // failed during a continuation request.  This shouldn't be a huge deal,
        // since we have been populating the cache as we received more data anyway
        continuationRequestActive = false;
    }

    emit q->statusChanged();
    emit q->errorChanged();
    emit q->errorMessageChanged();
}

/*! \internal */
void TwitterInterfacePrivate::sslErrorsHandler(const QList<QSslError> &errs)
{
    Q_Q(TwitterInterface);
    errorMessage = QLatin1String("SSL error: ");
    if (errs.isEmpty()) {
        errorMessage += QLatin1String("unknown SSL error");
    } else {
        foreach (const QSslError &sslE, errs)
            errorMessage += sslE.errorString() + QLatin1String("; ");
        errorMessage.chop(2);
    }

    error = SocialNetworkInterface::RequestError;
    status = SocialNetworkInterface::Error;

    emit q->statusChanged();
    emit q->errorChanged();
    emit q->errorMessageChanged();
}

/*! \internal */
void TwitterInterfacePrivate::deleteReply()
{
    if (currentReply) {
        currentReply->disconnect();
        currentReply->deleteLater();
        currentReply = 0;
    }
}

//----------------------------------------------------------

/*!
    \qmltype Twitter
    \instantiates TwitterInterface
    \inqmlmodule org.nemomobile.social 1
    \brief Implements the SocialNetwork interface for the Twitter service.

    The Twitter type is an implementation of the SocialNetwork interface
    specifically for the Twitter social network service.
    It provides access to graph objects such as users, tweets and places,
    and allows operations such as "retweet" and "follow".

    Clients should provide an \c oauthToken, \c oauthTokenSecret,
    \c oauthConsumerKey and \c oauthConsumerSecret to use the adapter.
    These values may be retrieved via the org.nemomobile.signon adapters,
    or from another OAuth2 implementation.

    An example of usage is as follows:

    \qml
    import QtQuick 1.1
    import org.nemomobile.social 1.0

    Item {
        id: root
        width: 400
        height: 800

        Flickable {
            anchors.top: parent.verticalCenter
            anchors.bottom: parent.bottom
            anchors.left: parent.left
            anchors.right: parent.right

            ListView {
                model: fb
                anchors.fill: parent
                delegate: Label { text: contentItem.user.screenName } // display the screen name of the retweeter
            }
        }

        Twitter {
            id: tw
            oauthToken: "your oauth token"              // you must supply a valid oauth token
            oauthTokenSecret: "your oauth secret"       // you must supply a valid oauth token secret
            oauthConsumerKey: "your consumer key"       // you must supply a valid oauth consumer key
            oauthConsumerSecret: "your consumer secret" // you must supply a valid oauth consumer secret
            nodeIdentifier: "332842978072735744"        // some valid Tweet/DirectMessage/Place/User id or screen name.
            filters: [ RetweetFilter { } ]              // the model data will contain retweets of the above Tweet.
        }

        TwitterUser {
            id: twus
            socialNetwork: tw
            identifier: "MarcDillonDotFi"               // some valid Twitter user identifier or screen name
            onStatusChanged: {
                if (status == SocialNetwork.Idle) {
                    twus.follow()                       // will add Marc as a friend on Twitter
                }
            }
        }

        Component.onCompleted: {
            tw.populate()                               // populate retweets of specified node Tweet
        }
    }
    \endqml

    Note: the Twitter adapter currently only supports the
    \c ContentItemTypeFilter filter type, and does not support
    any form of sorting.
*/

TwitterInterface::TwitterInterface(QObject *parent)
    : SocialNetworkInterface(*(new TwitterInterfacePrivate(this)), parent)
{
}


/*!
    \qmlproperty QString Twitter::oauthToken
    The oauth token to use when accessing the Twitter REST API.
*/
QString TwitterInterface::oauthToken() const
{
    Q_D(const TwitterInterface);
    return d->oauthToken;
}

void TwitterInterface::setOAuthToken(const QString &token)
{
    Q_D(TwitterInterface);
    if (d->oauthToken != token) {
        d->oauthToken = token;
        emit oauthTokenChanged();
    }
}


/*!
    \qmlproperty QString Twitter::oauthTokenSecret
    The oauth token secret to use when accessing the Twitter REST API.
*/
QString TwitterInterface::oauthTokenSecret() const
{
    Q_D(const TwitterInterface);
    return d->oauthTokenSecret;
}

void TwitterInterface::setOAuthTokenSecret(const QString &secret)
{
    Q_D(TwitterInterface);
    if (d->oauthTokenSecret != secret) {
        d->oauthTokenSecret = secret;
        emit oauthTokenSecretChanged();
    }
}


/*!
    \qmlproperty QString Twitter::oauthConsumerKey
    The oauth consumer key to use when accessing the Twitter REST API.
*/
QString TwitterInterface::oauthConsumerKey() const
{
    Q_D(const TwitterInterface);
    return d->oauthConsumerKey;
}

void TwitterInterface::setOAuthConsumerKey(const QString &key)
{
    Q_D(TwitterInterface);
    if (d->oauthConsumerKey != key) {
        d->oauthConsumerKey = key;
        emit oauthConsumerKeyChanged();
    }
}


/*!
    \qmlproperty QString Twitter::oauthConsumerSecret
    The oauth consumer secret to use when accessing the Twitter REST API.
*/
QString TwitterInterface::oauthConsumerSecret() const
{
    Q_D(const TwitterInterface);
    return d->oauthConsumerSecret;
}

void TwitterInterface::setOAuthConsumerSecret(const QString &secret)
{
    Q_D(TwitterInterface);
    if (d->oauthConsumerSecret != secret) {
        d->oauthConsumerSecret = secret;
        emit oauthConsumerSecretChanged();
    }
}

/*! \reimp */
void TwitterInterface::componentComplete()
{
    Q_D(TwitterInterface);
    // must set d->initialized to true.
    d->initialized = true;

    // now that we're initialized, perform any pending operations.
    if (d->populatePending) {
        populate();
    } else if (d->populateDataForUnseenPending) {
        populateDataForNode(d->pendingCurrentNodeIdentifier);
    }
}

/*! \reimp */
void TwitterInterface::populate()
{
    Q_D(TwitterInterface);
    // if no central node identifier is set by the client,
    // we load the "me" node by default.

    if (!d->initialized) {
        d->populatePending = true;
        return;
    }

    if (!node()) {
        if (nodeIdentifier().isEmpty()) {
            setNodeIdentifier(TWITTER_ONTOLOGY_TYPE_USER_ME);
        } else {
            populateDataForNode(nodeIdentifier());
        }
    } else {
        populateDataForNode(node());
    }
}

/*! \reimp */
QNetworkReply *TwitterInterface::getRequest(const QString &objectIdentifier, const QString &extraPath, const QStringList &whichFields, const QVariantMap &extraData)
{
    Q_D(TwitterInterface);
    if (!d->initialized) {
        qWarning() << Q_FUNC_INFO << "cannot complete get request: not initialized";
        return 0;
    }

    QVariantMap modifiedExtraData = extraData;
    QUrl geturl = d->requestUrl(objectIdentifier, extraPath, whichFields, modifiedExtraData);
    QString baseUrl = geturl.toString(QUrl::RemoveQuery);
    QList<QPair<QString, QString> > queryItems = geturl.queryItems();
    QNetworkRequest nreq;
    nreq.setRawHeader("Authorization", TwitterDataUtil::authorizationHeader(
            d->oauthToken, d->oauthTokenSecret,
            QLatin1String("GET"), baseUrl, queryItems).toLatin1());
    return d->networkAccessManager->get(QNetworkRequest(geturl));
}

/*! \reimp */
QNetworkReply *TwitterInterface::postRequest(const QString &objectIdentifier, const QString &extraPath, const QVariantMap &data, const QVariantMap &extraData)
{
    Q_D(TwitterInterface);
    if (!d->initialized) {
        qWarning() << Q_FUNC_INFO << "cannot complete post request: not initialized";
        return 0;
    }
    
    // create post data
    QString multipartBoundary = QLatin1String("-------Sska2129ifcalksmqq3");
    QByteArray postData;
    foreach (const QString &key, data.keys()) {
        postData.append("--"+multipartBoundary+"\r\n");
        postData.append("Content-Disposition: form-data; name=\"");
        postData.append(key);
        postData.append("\"\r\n\r\n");
        postData.append(data.value(key).toString());
        postData.append("\r\n");
    }
    postData.append("--"+multipartBoundary+"\r\n");

    // create request
    QNetworkRequest request(d->requestUrl(objectIdentifier, extraPath, QStringList(), extraData));
    request.setRawHeader("Accept", "text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8");
    request.setRawHeader("Accept-Language", "en-us,en;q=0.5");
    request.setRawHeader("Accept-Encoding", "gzip,deflate");
    request.setRawHeader("Accept-Charset", "ISO-8859-1,utf-8;q=0.7,*;q=0.7");
    request.setRawHeader("Keep-Alive", "300");
    request.setRawHeader("Connection", "keep-alive");
    request.setRawHeader("Content-Type",QString("multipart/form-data; boundary="+multipartBoundary).toAscii());
    request.setHeader(QNetworkRequest::ContentLengthHeader, postData.size());

    // perform POST request
    return d->networkAccessManager->post(request, postData);
}

/*! \reimp */
QNetworkReply *TwitterInterface::deleteRequest(const QString &objectIdentifier, const QString &extraPath, const QVariantMap &extraData)
{
    Q_D(TwitterInterface);
    if (!d->initialized) {
        qWarning() << Q_FUNC_INFO << "cannot complete delete request: not initialized";
        return 0;
    }

    return d->networkAccessManager->deleteResource(QNetworkRequest(d->requestUrl(objectIdentifier, extraPath, QStringList(), extraData)));
}

/*! \reimp */
QString TwitterInterface::dataSection(int type, const QVariantMap &data) const
{
    switch (type) {
    case TwitterInterface::User:
        return data.value(FACEBOOK_ONTOLOGY_PROPERTY_USER_NAME).toString();
    default:
        break;
    }
    return SocialNetworkInterface::dataSection(type, data);
}

/*! \reimp */
void TwitterInterface::updateInternalData(QList<CacheEntry*> data)
{
    Q_D(TwitterInterface);
    qWarning() << Q_FUNC_INFO << "filtering/sorting not implemented.  TODO!";

    // XXX TODO: filter the data in a better manner than linear searches...
    QList<CacheEntry*> filteredData;
    for (int i = 0; i < d->filters.size(); ++i) {
        FilterInterface *currFilter = d->filters.at(i);
        for (int j = 0; j < data.size(); ++j) {
            CacheEntry *currEntry = data.at(j);
            if ((!currEntry->item && currFilter->matches(currEntry->data))
                    || (currEntry->item && currFilter->matches(currEntry->item))) {
                filteredData.append(currEntry);
            }
        }
    }

    // XXX TODO: sort the filtered data
    QList<CacheEntry*> sortedData = filteredData;

    // clear the internal data
    QModelIndex parent;
    if (d->internalData.count()) {
        beginRemoveRows(parent, 0, d->internalData.count());
        d->internalData = QList<CacheEntry*>();
        endRemoveRows();
    }

    // update the internal data
    beginInsertRows(parent, 0, sortedData.count());
    d->internalData = sortedData;
    endInsertRows();
    emit countChanged();
}

/*! \internal */
void TwitterInterface::retrieveRelatedContent(IdentifiableContentItemInterface *whichNode)
{
    Q_D(TwitterInterface);
    if (!whichNode) {
        qWarning() << Q_FUNC_INFO << "Cannot retrieve related content for null node!";
        return;
    }

    bool needsRequest = false;
    QVariantMap extraData;
    QString extraPath;

    QList<FilterInterface*> allFilters = d->filters;
    int nodeType = whichNode->type();
    if (allFilters.isEmpty()) {
        // if none are specified, fetch the most likely connection for each type.
        switch (nodeType) {
            case TwitterInterface::Tweet: {
                extraData.insert(TWITTER_ONTOLOGY_CONNECTION_TWEET_COUNT, QLatin1String("20"));
                extraPath = TWITTER_ONTOLOGY_CONNECTION_TWEET_RETWEETS;
                needsRequest = true;
                break;
            }

            case TwitterInterface::MeUser: {
                extraData.insert(TWITTER_ONTOLOGY_CONNECTION_USER_ME_COUNT, QLatin1String("20"));
                extraPath = TWITTER_ONTOLOGY_CONNECTION_USER_ME_TIMELINE_HOME;
                needsRequest = true;
                break;
            }

            // by default, don't fetch the connections for these types.
            case TwitterInterface::Place:
            case TwitterInterface::DirectMessage:
            case TwitterInterface::User:
            default: break;
        }
    } else if (allFilters.size() == 1) {
        bool filterValid = false;
        const FilterInterface *fi = allFilters[0];
        if (nodeType == TwitterInterface::Tweet) {
            const RetweetsFilter *rf = qobject_cast<const RetweetsFilter *>(fi);
            if (rf) {
                filterValid = true;
                extraData.insert(TWITTER_ONTOLOGY_CONNECTION_TWEET_COUNT, QString::number(rf->count()));
                extraPath = TWITTER_ONTOLOGY_CONNECTION_TWEET_RETWEETS;
                needsRequest = true;
            }
        } else if (nodeType == TwitterInterface::Place) {
            const NearbyPlacesFilter *nf = qobject_cast<const NearbyPlacesFilter *>(fi);
            if (sf) {
                filterValid = true;
                extraData.insert(TWITTER_ONTOLOGY_CONNECTION_PLACE_SIMILARPLACES_COUNT, QString::number(nf->count()));
                extraData.insert(TWITTER_ONTOLOGY_CONNECTION_PLACE_SIMILARPLACES_NAME, QString::number(nf->name()));
                extraPath = TWITTER_ONTOLOGY_CONNECTION_PLACE_SIMILARPLACES;
                needsRequest = true;
            }
        } else if (nodeType == TwitterInterface::MeUser) {
            const UserTimelineTweetsFilter *ut = qobject_cast<const UserTimelineTweetsFilter *>(fi);
            const MentionsTimelineTweetsFilter *mt = qobject_cast<const MentionsTimelineTweetsFilter *>(fi);
            const HomeTimelineTweetsFilter *ht = qobject_cast<const HomeTimelineTweetsFilter *>(fi);
            const RetweetsTimelineTweetsFilter *rt = qobject_cast<const RetweetsTimelineTweetsFilter *>(fi);
            const DirectMessagesFilter *dmf = qobject_cast<const DirectMessagesFilter *>(fi);
            if (ut) {
                filterValid = true;
                extraPath = TWITTER_ONTOLOGY_CONNECTION_USER_ME_TIMELINE_USER;
            } else if (mt) {
                filterValid = true;
                extraPath = TWITTER_ONTOLOGY_CONNECTION_USER_ME_TIMELINE_MENTIONS;
            } else if (ht) {
                filterValid = true;
                extraPath = TWITTER_ONTOLOGY_CONNECTION_USER_ME_TIMELINE_HOME;
            } else if (rt) {
                filterValid = true;
                extraPath = TWITTER_ONTOLOGY_CONNECTION_USER_ME_TIMELINE_RETWEETS;
            } else if (dmf) {
                filterValid = true;
                if (dmf->sentMessages()) {
                    extraPath = TWITTER_ONTOLOGY_CONNECTION_USER_ME_DIRECTMESSAGES_FROM;
                } else {
                    extraPath = TWITTER_ONTOLOGY_CONNECTION_USER_ME_DIRECTMESSAGES_TO;
                }
            }

            // XXX TODO: friends/followers/etc connections.
        }

        if (!filterValid) {
            qWarning() << Q_FUNC_INFO << "invalid filter specified for node" << whichNode->identifier();
            return;
        }
    } else {
        qWarning() << Q_FUNC_INFO << "Twitter only supports one filter to be specified at a time";
        return;
    }

    if (needsRequest) {
        d->setCurrentReply(getRequest(whichNode->identifier(),
                                      extraPath,
                                      QStringList(),
                                      extraData),
                           whichNode->identifier());
        d->connectFinishedAndErrors();
    } else {
        // this might not be an error (eg, in the case of no filter specified and place node)
        qWarning() << Q_FUNC_INFO << "Not retrieving any related data...";
    }
}

/*! \reimp */
void TwitterInterface::populateDataForNode(IdentifiableContentItemInterface *currentNode)
{
    Q_D(TwitterInterface);
    if (currentNode == d->placeHolderNode) {
        // actually populating an unseen / pending node.
        return; // should have been queued as pending anyway.
    }

    d->status = SocialNetworkInterface::Busy;
    d->internalStatus = TwitterInterfacePrivate::PopulatingSeenNode;
    emit statusChanged();

    // clear the internal data
    if (d->internalData.count()) {
        QModelIndex parent;
        beginRemoveRows(parent, 0, d->internalData.count());
        d->internalData = QList<CacheEntry*>();
        endRemoveRows();
        emit countChanged();
    }

    // retrieve the related content
    retrieveRelatedContent(currentNode);

    // continued in continuePopulateDataForSeenNode().
}

#define TWITTER_CREATE_UNCACHED_ENTRY_FROM_DATA(data, type)                                 \
    do {                                                                                    \
        QVariantList itemsList = data.toList();                                             \
        foreach (const QVariant &currVar, itemsList) {                                      \
            QVariantMap currMap = currVar.toMap();                                          \
            currMap.insert(NEMOQMLPLUGINS_SOCIAL_CONTENTITEMTYPE, type);                    \
            currMap.insert(NEMOQMLPLUGINS_SOCIAL_CONTENTITEMID,                             \
                           currVar.value(TWITTER_ONTOLOGY_PROPERTY_GENERIC_IDSTR)));        \
            relatedContent.append(d->createUncachedEntry(currMap));                         \
        }                                                                                   \
    } while (0)

static bool matchRequestPathFragment(const QString &requestPath, const QString &connection)
{
    // need to use this function, since some requests (like retweets of a tweet)
    // don't use the same format (instead they embed id within the path).

    if (connection == TWITTER_ONTOLOGY_CONNECTION_USER_ME_TIMELINE_HOME) {
    } else {
    }

    return false;
}

/*! \internal */
void TwitterInterface::continuePopulateDataForSeenNode(const QVariant &relatedData, const QUrl &requestUrl)
{
    Q_D(TwitterInterface);
    // We receive the related data and transform it into ContentItems.
    // Finally, we populate the cache for the node and update the internal model data.

    int currentCount = 0;
    QString continuationRequestUri;
    QList<CacheEntry *> relatedContent;
    if (d->continuationRequestActive) {
        // we are continuing a request, and thus don't overwrite the existing
        // cache entries, but instead append to them.
        bool ok = true;
        relatedContent = d->cachedContent(d->currentNode(), &ok);
        currentCount = relatedContent.size();
        if (!ok) {
            qWarning() << Q_FUNC_INFO << "Clobbering cached content in continuation request for node:" << d->currentNode()->identifier();
        }
    }

    // construct related content items from the request results.
    // Twitter doesn't include any metadata, and the results are always returned
    // in a JSON array.  So, we inspect the request url path for hints about what
    // types of objects are going to be returned.
    QString reqPath = requestUrl.path();
    if (matchRequestPathFragment(reqPath, TWITTER_ONTOLOGY_CONNECTION_USER_ME_TIMELINE_HOME)) {
        TWITTER_CREATE_UNCACHED_ENTRY_FROM_DATA(relatedData, TwitterInterface::Tweet);
    } else if (matchRequestPathFragment(reqPath, TWITTER_ONTOLOGY_CONNECTION_USER_ME_TIMELINE_USER)) {
        TWITTER_CREATE_UNCACHED_ENTRY_FROM_DATA(relatedData, TwitterInterface::Tweet);
    } else if (matchRequestPathFragment(reqPath, TWITTER_ONTOLOGY_CONNECTION_USER_ME_TIMELINE_MENTIONS)) {
        TWITTER_CREATE_UNCACHED_ENTRY_FROM_DATA(relatedData, TwitterInterface::Tweet);
    } else if (matchRequestPathFragment(reqPath, TWITTER_ONTOLOGY_CONNECTION_USER_ME_TIMELINE_RETWEETS)) {
        TWITTER_CREATE_UNCACHED_ENTRY_FROM_DATA(relatedData, TwitterInterface::Tweet);
    } else if (matchRequestPathFragment(reqPath, TWITTER_ONTOLOGY_CONNECTION_TWEET_RETWEETS)) {
        TWITTER_CREATE_UNCACHED_ENTRY_FROM_DATA(relatedData, TwitterInterface::Tweet);
    } else if (...) {
        // TODO: other related data types.
    } else {
        qWarning() << Q_FUNC_INFO << "Informative: Unsupported data retrieved via edge:" << reqPath;
    }

    // We don't need to sort it here, as sorting is done in-memory during updateInternalData().
    // XXX TODO: make the entire filter/sort codepath more efficient, by guaranteeing that whatever
    // comes out of the cache must be sorted/filtered already?  Requires invalidating the entire
    // cache on filter/sorter change, however... hrm...

    bool ok = false;
    d->populateCache(d->currentNode(), relatedContent, &ok);
    if (!ok) {
        qWarning() << Q_FUNC_INFO << "Error: Unable to populate the cache for the current node:" << d->currentNode()->identifier();
    }

    // Update the model data.
    updateInternalData(relatedContent);

    // If we need to request more (paged) data, do so.
    if (continuationRequestUri.isEmpty()) {
        // there are no more results / result pages to retrieve.
        d->continuationRequestActive = false;
        d->status = SocialNetworkInterface::Idle;
        emit statusChanged();
    } else {
        // there are more results to retrieve.  Start a continuation request.
        d->continuationRequestActive = true;
        // grab the relevant parts of the continuation uri to create a new request.
        QUrl continuationUrl(continuationRequestUri);
        QNetworkRequest continuationRequest(continuationUrl);
        continuationRequest.setRawHeader(...); // TODO: handle continuations via pages.
        d->setCurrentReply(d->networkAccessManager->get(continuationRequest, d->currentNode()->identifier());
        if (d->outOfBandConnectionsLimit != -1) {
            d->currentReply->setProperty("specialLimit", d->outOfBandConnectionsLimit);
        }
        d->connectFinishedAndErrors();
    }
}

/*! \reimp */
void TwitterInterface::populateDataForNode(const QString &unseenNodeIdentifier)
{
    Q_D(TwitterInterface);
    // This function should be implemented so that:
    // 0) the current model data should be set to empty
    // 1) the given node is requested from the service, with the given fields loaded
    // 2) when received, the node should be pushed to the nodeStack via d->pushNode(n)
    // 3) the related content data should be requested from the service, according to the filters
    // 4) when received, the related content data should be used to populate the cache via d->populateCache()
    // 5) finally, updateInternalData() should be called, passing in the new cache data.

    if (unseenNodeIdentifier != d->pendingCurrentNodeIdentifier) {
        qWarning() << Q_FUNC_INFO << "populating data for unseen node which isn't the pending current node!";
        return; // this is an error in the implementation of SocialNetworkInterface.
    }

    if (!d->initialized) {
        // we should delay this until we are initialized
        d->populateDataForUnseenPending = true;
        return;
    }

    d->status = SocialNetworkInterface::Busy;
    d->internalStatus = TwitterInterfacePrivate::PopulatingUnseenNode;
    emit statusChanged();

    // clear the internal data
    if (d->internalData.count()) {
        QModelIndex parent;
        beginRemoveRows(parent, 0, d->internalData.count());
        d->internalData = QList<CacheEntry*>();
        endRemoveRows();
        emit countChanged();
    }

    // get the unseen node data.
    d->setCurrentReply(getRequest(unseenNodeIdentifier, QString(), QStringList(), QVariantMap()), unseenNodeIdentifier);
    d->connectFinishedAndErrors();

    // continued in continuePopulateDataForUnseenNode().
}

/*! \internal */
void TwitterInterface::continuePopulateDataForUnseenNode(const QVariantMap &nodeData)
{
    Q_D(TwitterInterface);

    // having retrieved the node data, we construct the node, push it, and request
    // the related data required according to the filters.
    ContentItemInterface *convertedNode = contentItemFromData(this, nodeData);
    IdentifiableContentItemInterface *newCurrentNode = qobject_cast<IdentifiableContentItemInterface*>(convertedNode);
    if (!newCurrentNode) {
        if (convertedNode)
            convertedNode->deleteLater();
        d->status = SocialNetworkInterface::Error;
        d->error = SocialNetworkInterface::DataUpdateError;
        d->errorMessage = QLatin1String("Error retrieving node with identifier: ") + d->pendingCurrentNodeIdentifier;
        emit statusChanged();
        emit errorChanged();
        emit errorMessageChanged();
        return;
    }

    // push the retrieved node to the nodeStack.
    d->pushNode(newCurrentNode);
    d->pendingCurrentNodeIdentifier = QString();
    emit nodeChanged();

    // now that we have retrieved the node, now retrieve the related content.
    d->internalStatus = TwitterInterfacePrivate::PopulatingSeenNode;
    retrieveRelatedContent(newCurrentNode);
}

/*! \internal */
QString TwitterInterface::currentUserIdentifier() const
{
    Q_D(const TwitterInterface);
    // returns the object identifier associated with the "me" node, if loaded.
    return d->currentUserIdentifier;
}

/*! \reimp */
ContentItemInterface *TwitterInterface::contentItemFromData(QObject *parent, const QVariantMap &data) const
{
    Q_D(const TwitterInterface);
    // Construct the appropriate TwitterWhateverInterface for the given data.
    TwitterInterface::ContentItemType detectedType = static_cast<TwitterInterface::ContentItemType>(d->detectTypeFromData(data));
    switch (detectedType) {
        case TwitterInterface::User: {
            TwitterUserInterface *retn = new TwitterUserInterface(parent);
            retn->classBegin();
            retn->setSocialNetwork(const_cast<TwitterInterface*>(this));
            setContentItemData(retn, data);
            retn->componentComplete();
            return retn;
        }
        break;

// TODO: other types incl. me user, tweets, places, directmessages.

        case TwitterInterface::Unknown: {
            qWarning() << Q_FUNC_INFO << "Unable to detect the type of the content item";
            IdentifiableContentItemInterface *retn = new IdentifiableContentItemInterface(parent);
            retn->classBegin();
            retn->setSocialNetwork(const_cast<TwitterInterface*>(this));
            setContentItemData(retn, data);
            retn->componentComplete();
            return retn;
        }
        break;

        default: qWarning() << Q_FUNC_INFO << "unsupported type:" << detectedType; break;
    }

    return 0;
}

/*! \internal */
QVariantMap TwitterInterface::twitterContentItemData(ContentItemInterface *contentItem)
{
    return contentItemData(contentItem);
}

/*! \internal */
void TwitterInterface::setTwitterContentItemData(ContentItemInterface *contentItem, const QVariantMap &data)
{
    // TODO: Using TwitterInterface, that have many friends, to call a private method
    // seems to be wrong. There should be a pretty way to change an object, and
    // the best way should be to make the setter public.
    setContentItemData(contentItem, data);
}

#include "moc_twitterinterface.cpp"
