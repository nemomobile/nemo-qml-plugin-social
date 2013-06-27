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

#include "twitterdatautil_p.h"

#include <QtDebug>

static const char *TRUE_STRING = "true";
static const char *FALSE_STRING = "false";
static const char *HEADER_AUTHORIZATION_KEY = "Authorization";

inline QString boolToString(bool boolean)
{
    if (boolean) {
        return QLatin1String(TRUE_STRING);
    } else {
        return QLatin1String(FALSE_STRING);
    }
}

TwitterInterfacePrivate::TwitterInterfacePrivate(TwitterInterface *q)
    : SocialNetworkInterfacePrivate(q)
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

//    qWarning() << Q_FUNC_INFO << "No type information available in object metadata - attempting to heuristically detect type";
//    if (data.value(TWITTER_ONTOLOGY_PROPERTY_TWEET_TEXT).isValid() && data.value(TWITTER_ONTOLOGY_PROPERTY_TWEET_RETWEETED).isValid())
//        return TwitterInterface::Tweet;
//    else if (data.value(TWITTER_ONTOLOGY_PROPERTY_PLACE_PLACETYPE).isValid() && data.value(TWITTER_ONTOLOGY_PROPERTY_COUNTRY).isValid())
//        return TwitterInterface::Place;
//    else if (data.value(TWITTER_ONTOLOGY_PROPERTY_USER_SCREENNAME).isValid() || data.value(TWITTER_ONTOLOGY_PROPERTY_USER_FOLLOWREQUESTSENT).isValid())
//        return TwitterInterface::User;

//    qWarning() << Q_FUNC_INFO << "Unable to heuristically detect type!";


    return TwitterInterface::Unknown;
}

/*! \internal */
QNetworkRequest TwitterInterfacePrivate::networkRequest(const QString &extraPath,
                                                        const QVariantMap &extraData,
                                                        const QByteArray &requestMethod,
                                                        const QVariantMap &postData)
{
    // Handling the URL cannot be managed in an easy way from a single method
    // So it is left to the different classs to handle the API call
    // TwitterInterfacePrivate offers some conveinent methods to get the
    // good arguments for some known calls though

    // We will also sign the request in this method
    QList<QPair<QString, QString> > queryItems;
    QList<QPair<QByteArray, QByteArray> > parameters;

    const QStringList &extraDataKeys = extraData.keys();
    foreach (const QString &key, extraDataKeys) {
        queryItems.append(qMakePair<QString, QString>(key, extraData.value(key).toString()));
        parameters.append(qMakePair<QByteArray, QByteArray>(key.toLocal8Bit(),
                                                            extraData.value(key).toByteArray()));
    }

    const QStringList &postDataKeys = postData.keys();
    foreach (const QString &key, postDataKeys) {
        parameters.append(qMakePair<QByteArray, QByteArray>(key.toLocal8Bit(),
                                                            postData.value(key).toByteArray()));
    }


    QByteArray urlByteArray = "https://api.twitter.com/1.1/" + extraPath.toLocal8Bit();
    QByteArray header = TwitterDataUtil::authorizationHeader(consumerKey.toLocal8Bit(),
                                                             consumerSecret.toLocal8Bit(),
                                                             requestMethod, urlByteArray,
                                                             parameters,
                                                             oauthToken.toLocal8Bit(),
                                                             oauthTokenSecret.toLocal8Bit());

    QUrl url (urlByteArray);
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    QUrlQuery query;
    query.setQueryItems(queryItems);
    url.setQuery(query);
#else
    url.setQueryItems(queryItems);
#endif

    QNetworkRequest request (url);
    request.setRawHeader(HEADER_AUTHORIZATION_KEY, header);
    return request;
}

void TwitterInterfacePrivate::handlePopulateRelatedData(Node &node,
                                                        const QVariantMap &relatedData,
                                                        const QUrl &requestUrl)
{
    // We receive the related data and transform it into ContentItems.
    // Finally, we populate the cache for the node and update the internal model data.
    QList<CacheEntry> relatedContent;
    QString requestPath = requestUrl.path();

    TwitterInterface::ContentItemType type = TwitterInterface::Unknown;
    // We strip the 1.1 from the path, and then, we are able to know what is retrieved
    // by simply using the ontology
    requestPath.remove(0, 5);
    if (requestPath == TWITTER_ONTOLOGY_CONNECTION_FRIENDS_LIST
        || requestPath == TWITTER_ONTOLOGY_CONNECTION_FOLLOWERS_LIST) {
        type = TwitterInterface::User;
    }


    switch (type) {
        case TwitterInterface::User:
        {
            QVariantList userList = relatedData.value(TWITTER_ONTOLOGY_CONNECTION_USERS_KEY).toList();
            foreach (const QVariant &variant, userList) {
                QVariantMap variantMap = variant.toMap();
                QString identifier = variantMap.value(TWITTER_ONTOLOGY_METADATA_ID).toString();
                variantMap.insert(NEMOQMLPLUGINS_SOCIAL_CONTENTITEMTYPE, TwitterInterface::User);
                variantMap.insert(NEMOQMLPLUGINS_SOCIAL_CONTENTITEMID, identifier);
                relatedContent.append(createCacheEntry(variantMap, identifier));
            }
        }
        break;
        case TwitterInterface::Tweet: break;
        default: {
            qWarning() << Q_FUNC_INFO << "Unsupported data retrieved";
            qWarning() << Q_FUNC_INFO << "Request url:" << requestUrl;
            qWarning() << Q_FUNC_INFO << "List of keys: " << relatedData.keys();
        }
        break;
    }

    // Do the cursoring
    QVariantMap nodeExtra;
    nodeExtra.insert(TWITTER_ONTOLOGY_METADATA_PREVIOUS_CURSOR,
                     relatedData.value(TWITTER_ONTOLOGY_METADATA_PREVIOUS_CURSOR,
                                       TWITTER_ONTOLOGY_METADATA_NULL_CURSOR).toString());
    nodeExtra.insert(TWITTER_ONTOLOGY_METADATA_NEXT_CURSOR,
                     relatedData.value(TWITTER_ONTOLOGY_METADATA_NEXT_CURSOR,
                                       TWITTER_ONTOLOGY_METADATA_NULL_CURSOR).toString());

    // Do the cursoring
    node.setExtraInfo(nodeExtra);

    bool havePreviousRelatedData = nodeExtra.value(TWITTER_ONTOLOGY_METADATA_PREVIOUS_CURSOR).toString()
                                   != TWITTER_ONTOLOGY_METADATA_NULL_CURSOR;
    bool haveNextRelatedData = nodeExtra.value(TWITTER_ONTOLOGY_METADATA_NEXT_CURSOR).toString()
                               != TWITTER_ONTOLOGY_METADATA_NULL_CURSOR;

    node.setHavePreviousAndNext(havePreviousRelatedData, haveNextRelatedData);
    updateModelRelatedData(node, relatedContent);

    setStatus(node, NodePrivate::Idle);
}

RequestInfo TwitterInterfacePrivate::requestUserInfo(const QString &identifier)
{
    RequestInfo info;
    info.extraPath = TWITTER_ONTOLOGY_CONNECTION_USERS_SHOW;
    info.extraData.insert(TWITTER_ONTOLOGY_CONNECTION_USER_ID_KEY, identifier);
    return info;
}

RequestInfo  TwitterInterfacePrivate::requestTweetInfo(const QString &identifier, bool trimUser,
                                                       bool includeMyRetweet, bool includeEntities)
{
    RequestInfo info;
    info.extraPath = QString(TWITTER_ONTOLOGY_CONNECTION_STATUSES_SHOW).arg(identifier);
    info.extraData.insert(TWITTER_ONTOLOGY_CONNECTION_STATUSES_SHOW_TRIM_USER_KEY,
                          boolToString(trimUser));
    info.extraData.insert(TWITTER_ONTOLOGY_CONNECTION_STATUSES_SHOW_INCLUDE_MY_RETWEET_KEY,
                          boolToString(includeMyRetweet));
    info.extraData.insert(TWITTER_ONTOLOGY_CONNECTION_INCLUDE_ENTITIES_KEY,
                          boolToString(includeEntities));
    return info;
}

/*! \reimp */
void TwitterInterfacePrivate::populateDataForNode(Node &node)
{
    RequestInfo requestInfo;
    switch (node.type()) {
        case TwitterInterface::User: {
            requestInfo = TwitterInterfacePrivate::requestUserInfo(node.identifier());
        }
        break;
        case TwitterInterface::Tweet: {
            requestInfo = TwitterInterfacePrivate::requestTweetInfo(node.identifier());
        }
        break;
        default: {
            qWarning() << Q_FUNC_INFO << "Please set the type of the node";
            setError(node, SocialNetworkInterface::RequestError,
                     QLatin1String("Please set the type of the node"));
        }
        return;
    }

    setReply(node, getRequest(requestInfo.objectIdentifier, requestInfo.extraPath,
                              requestInfo.whichFields, requestInfo.extraData));
}

/*! \reimp */
void TwitterInterfacePrivate::populateRelatedDataforNode(Node &node)
{
    QString cursor;
    switch (node.status()) {
        case NodePrivate::LoadingRelatedDataAppending: {
            cursor = node.extraInfo().value(TWITTER_ONTOLOGY_METADATA_NEXT_CURSOR).toString();
        }
        break;
        case NodePrivate::LoadingRelatedDataPrepending: {
            cursor = node.extraInfo().value(TWITTER_ONTOLOGY_METADATA_PREVIOUS_CURSOR).toString();
        }
        break;
        default: break;
    }



    if (!performRelatedDataRequest(node, node.identifier(), node.filters().toList(), cursor)) {
        setError(node, SocialNetworkInterface::DataUpdateError,
                 QLatin1String("Cannot perform related data request"));
    }
}

/*! \reimp */
bool TwitterInterfacePrivate::validateCacheEntryForNode(const CacheEntry &cacheEntry)
{
    Q_UNUSED(cacheEntry)
    return true;
}

/*! \reimp */
QString TwitterInterfacePrivate::dataSection(int type, const QVariantMap &data) const
{
    Q_UNUSED(type)
    Q_UNUSED(data)
    return QString();
}

/*! \reimp */
ContentItemInterface *TwitterInterfacePrivate::contentItemFromData(const QVariantMap &data,
                                                                   QObject *parent) const
{
    Q_Q(const TwitterInterface);
    // Construct the appropriate TwitterWhateverInterface for the given data.
    TwitterInterface::ContentItemType detectedType
            = static_cast<TwitterInterface::ContentItemType>(detectTypeFromData(data));
    switch (detectedType) {
        case TwitterInterface::User: {
            TwitterUserInterface *returnedInterface = new TwitterUserInterface(parent);
            returnedInterface->classBegin();
            returnedInterface->setSocialNetwork(const_cast<TwitterInterface*>(q));
            q->setContentItemData(returnedInterface, data);
            returnedInterface->componentComplete();
            return returnedInterface;
        }
        break;

        // TODO: other types incl. me user, tweets, places, directmessages.
        case TwitterInterface::Unknown: {
            qWarning() << Q_FUNC_INFO << "Unable to detect the type of the content item";
            IdentifiableContentItemInterface *returnedInterface
                    = new IdentifiableContentItemInterface(parent);
            returnedInterface->classBegin();
            returnedInterface->setSocialNetwork(const_cast<TwitterInterface*>(q));
            q->setContentItemData(returnedInterface, data);
            returnedInterface->componentComplete();
            return returnedInterface;
        }
        break;

        default: qWarning() << Q_FUNC_INFO << "unsupported type:" << detectedType; break;
    }

    return 0;
}

/*! \reimp */
QNetworkReply * TwitterInterfacePrivate::getRequest(const QString &objectIdentifier,
                                                    const QString &extraPath,
                                                    const QStringList &whichFields,
                                                    const QVariantMap &extraData)
{
    Q_Q(TwitterInterface);
    Q_UNUSED(objectIdentifier)
    Q_UNUSED(whichFields)
    if (!q->isInitialized()) {
        qWarning() << Q_FUNC_INFO << "cannot complete get request: not initialized";
        return 0;
    }

    return networkAccessManager->get(networkRequest(extraPath, extraData, "GET"));
}

/*! \reimp */
QNetworkReply * TwitterInterfacePrivate::postRequest(const QString &objectIdentifier,
                                                     const QString &extraPath,
                                                     const QVariantMap &data,
                                                     const QVariantMap &extraData)
{
//    Q_D(TwitterInterface);
//    if (!d->initialized) {
//        qWarning() << Q_FUNC_INFO << "cannot complete post request: not initialized";
//        return 0;
//    }

//    // create post data
//    QString multipartBoundary = QLatin1String("-------Sska2129ifcalksmqq3");
//    QByteArray postData;
//    foreach (const QString &key, data.keys()) {
//        postData.append("--"+multipartBoundary+"\r\n");
//        postData.append("Content-Disposition: form-data; name=\"");
//        postData.append(key);
//        postData.append("\"\r\n\r\n");
//        postData.append(data.value(key).toString());
//        postData.append("\r\n");
//    }
//    postData.append("--"+multipartBoundary+"\r\n");

//    // create request
//    QNetworkRequest request(d->requestUrl(objectIdentifier, extraPath, QStringList(), extraData));
//    request.setRawHeader("Accept", "text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8");
//    request.setRawHeader("Accept-Language", "en-us,en;q=0.5");
//    request.setRawHeader("Accept-Encoding", "gzip,deflate");
//    request.setRawHeader("Accept-Charset", "ISO-8859-1,utf-8;q=0.7,*;q=0.7");
//    request.setRawHeader("Keep-Alive", "300");
//    request.setRawHeader("Connection", "keep-alive");
//    request.setRawHeader("Content-Type",QString("multipart/form-data; boundary="+multipartBoundary).toAscii());
//    request.setHeader(QNetworkRequest::ContentLengthHeader, postData.size());

//    // perform POST request
//    return d->networkAccessManager->post(request, postData);
    return 0;
}

/*! \reimp */
QNetworkReply * TwitterInterfacePrivate::deleteRequest(const QString &objectIdentifier,
                                                       const QString &extraPath,
                                                       const QVariantMap &extraData)
{
//    Q_D(TwitterInterface);
//    if (!d->initialized) {
//        qWarning() << Q_FUNC_INFO << "cannot complete delete request: not initialized";
//        return 0;
//    }

//    return d->networkAccessManager->deleteResource(QNetworkRequest(d->requestUrl(objectIdentifier, extraPath, QStringList(), extraData)));
    return 0;
}

/*! \internal */
void TwitterInterfacePrivate::handleFinished(Node &node, QNetworkReply *reply)
{
    Q_Q(TwitterInterface);

    QByteArray replyData = reply->readAll();
    QUrl requestUrl = reply->request().url();
    deleteReply(reply);
    bool ok = false;
    QVariantMap responseData = ContentItemInterfacePrivate::parseReplyData(replyData, &ok);
    if (!ok) {
        responseData.insert("response", replyData);
        setError(node, SocialNetworkInterface::RequestError,
                 QLatin1String("Error populating node: response is invalid. "\
                               "Perhaps the requested object id was incorrect?  Response: ")
                 + QString::fromLatin1(replyData.constData()));
        return;
    }

    if (responseData.contains(QLatin1String("error"))) {
        QString errorResponse = QLatin1String("\n    error:");
        QVariantMap errMap = responseData.value(QLatin1String("error")).toMap();
        QStringList keys = errMap.keys();
        foreach (const QString &key, keys) {
            errorResponse += QLatin1String("\n        ")
                             + key + QLatin1String("=") + errMap.value(key).toString();
        }
        qWarning() << Q_FUNC_INFO << "error response:" << errorResponse
                   << "while getting:" << requestUrl.toString();

        setError(node, SocialNetworkInterface::RequestError,
                 QLatin1String("Error populating node: response is error.  Response: ")
                 + QString::fromLatin1(replyData.constData()));
        return;
    };

    switch (node.status()) {
        case NodePrivate::LoadingNodeData:{
            // Create a cache entry associated to the retrieved data
            QString identifier;
            if (responseData.contains(TWITTER_ONTOLOGY_METADATA_ID)) {
                identifier = responseData.value(TWITTER_ONTOLOGY_METADATA_ID).toString();
            }
            responseData.insert(NEMOQMLPLUGINS_SOCIAL_CONTENTITEMTYPE, node.type());

            CacheEntry cacheEntry = createCacheEntry(responseData, identifier);
            node.setCacheEntry(cacheEntry);
            updateModelNode(node);

            // Performing a replace
            node.setStatus(NodePrivate::LoadingRelatedDataReplacing);
            populateRelatedDataforNode(node);
        }
        break;
        case NodePrivate::LoadingRelatedDataReplacing:
        case NodePrivate::LoadingRelatedDataPrepending:
        case NodePrivate::LoadingRelatedDataAppending: {
            handlePopulateRelatedData(node, responseData, requestUrl);
        }
        break;

        default: break;
    }
}

bool TwitterInterfacePrivate::performRelatedDataRequest(Node &node, const QString &identifier,
                                                        const QList<FilterInterface *> &filters,
                                                        const QString &cursor)
{
    Q_Q(TwitterInterface);

    // We first cast filters to known filters
    QList<FilterInterface *> castedFilters;

    foreach (FilterInterface *filter, filters) {
        ContentItemTypeFilterInterface *contentItemTypeFilter
                = qobject_cast<ContentItemTypeFilterInterface*>(filter);
        if (!contentItemTypeFilter) {
            qWarning() << "The Twitter adapter only supports ContentItemType filters";
        } else {
            castedFilters.append(contentItemTypeFilter);
        }
    }

    if (castedFilters.isEmpty()) {
        setStatus(node, NodePrivate::Idle);
        return true;
    }

    // Twitter only support one filter at each time
    // If we have more set, we will throw a warning, while only
    // using the first filter.

    if (castedFilters.count() > 1) {
        qWarning() << Q_FUNC_INFO << "Twitter interface only supports one filter at each time. " \
                      "Others filters will be discarded";
    }

    ContentItemTypeFilterInterface *filter
            = qobject_cast<ContentItemTypeFilterInterface*>(castedFilters.first());

    // Build the query
    QString extraPath;
    QVariantMap extraData;
    if (!cursor.isEmpty()) {
        extraData.insert(TWITTER_ONTOLOGY_METADATA_CURSOR, cursor);
    }

    switch (filter->type()) {
    case TwitterInterface::Friends:
        extraPath = TWITTER_ONTOLOGY_CONNECTION_FRIENDS_LIST;
        extraData.insert(TWITTER_ONTOLOGY_CONNECTION_USER_ID_KEY, identifier);
        extraData.insert(TWITTER_ONTOLOGY_CONNECTION_USER_ID_KEY, identifier);
        extraData.insert(TWITTER_ONTOLOGY_CONNECTION_SKIP_STATUS_KEY, TRUE_STRING);
        extraData.insert(TWITTER_ONTOLOGY_CONNECTION_INCLUDE_USER_ENTITIES_KEY, TRUE_STRING);
        break;
    case TwitterInterface::Followers:
        extraPath = TWITTER_ONTOLOGY_CONNECTION_FOLLOWERS_LIST;
        extraData.insert(TWITTER_ONTOLOGY_CONNECTION_USER_ID_KEY, identifier);
        extraData.insert(TWITTER_ONTOLOGY_CONNECTION_USER_ID_KEY, identifier);
        extraData.insert(TWITTER_ONTOLOGY_CONNECTION_SKIP_STATUS_KEY, TRUE_STRING);
        extraData.insert(TWITTER_ONTOLOGY_CONNECTION_INCLUDE_USER_ENTITIES_KEY, TRUE_STRING);
        break;

    default:
        qWarning() << Q_FUNC_INFO << "Type" << filter->type() << "is invalid";
        return false;
        break;
    }

    setReply(node, getRequest(QString(), extraPath, QStringList(), extraData));

    return true;


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
QString TwitterInterface::consumerKey() const
{
    Q_D(const TwitterInterface);
    return d->consumerKey;
}

void TwitterInterface::setConsumerKey(const QString &consumerKey)
{
    Q_D(TwitterInterface);
    if (d->consumerKey != consumerKey) {
        d->consumerKey = consumerKey;
        emit consumerKeyChanged();
    }
}


/*!
    \qmlproperty QString Twitter::oauthConsumerSecret
    The oauth consumer secret to use when accessing the Twitter REST API.
*/
QString TwitterInterface::consumerSecret() const
{
    Q_D(const TwitterInterface);
    return d->consumerSecret;
}

void TwitterInterface::setConsumerSecret(const QString &secret)
{
    Q_D(TwitterInterface);
    if (d->consumerSecret != secret) {
        d->consumerSecret = secret;
        emit consumerSecretChanged();
    }
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
