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
#include "twittertweetinterface.h"
#include "twitterconversationfilterinterface.h"
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
#define PAGING_HAVE_KEY QLatin1String("have")

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

void TwitterInterfacePrivate::handlePopulateRelatedData(Node::Ptr node,
                                                        const QVariant &relatedData,
                                                        const QUrl &requestUrl)
{
    // We receive the related data and transform it into ContentItems.
    // Finally, we populate the cache for the node and update the internal model data.
    CacheEntry::List relatedContent;


    if (node->extraInfo().contains(TWITTER_ONTOLOGY_CONNECTION_CONVERSATION_KEY)) {
        // We handle this part differently
        // We build a map "twitter_id => CacheEntry" to store the interesting
        // tweets and a map "parent_twitter_id" => list_of_tweet_ids"
        // as the tree of replies.
        QMap<QString, CacheEntry::Ptr> tweets;
        QMap<QString, QList<QString> > tweetsTree;

        // We might need to take in account the tree for old tweets as well
        // TODO ^

        QVariantList relatedDataList = relatedData.toList();
        QString firstId;
        if (!relatedDataList.isEmpty()) {
            QVariantMap lastMap = relatedDataList.first().toMap();
            firstId = lastMap.value(TWITTER_ONTOLOGY_METADATA_ID).toString();
        }

        foreach (const QVariant &variant, relatedDataList) {
            QVariantMap variantMap = variant.toMap();
            QString identifier = variantMap.value(TWITTER_ONTOLOGY_METADATA_ID).toString();
            variantMap.insert(NEMOQMLPLUGINS_SOCIAL_CONTENTITEMTYPE, TwitterInterface::Tweet);
            variantMap.insert(NEMOQMLPLUGINS_SOCIAL_CONTENTITEMID, identifier);

            CacheEntry::Ptr tweet = createCacheEntry(variantMap, identifier);
            QString parentTweetId = tweet->data().value(TWITTER_ONTOLOGY_TWEET_INREPLYTOSTATUSIDENTIFIER).toString();
            if (!parentTweetId.isEmpty()) {
                tweets.insert(identifier, tweet);

                if (!tweetsTree.contains(parentTweetId)) {
                    tweetsTree.insert(parentTweetId, QList<QString>());
                }

                tweetsTree[parentTweetId].append(identifier);
            }
        }
        makeConversationList(tweets, tweetsTree, node->identifier(), relatedContent);
        updateModelRelatedData(node, relatedContent);
        updateModelHavePreviousAndNext(node, false, true);
        if (!firstId.isEmpty()) {
            QVariantMap nodeExtra = node->extraInfo();
            nodeExtra.insert(TWITTER_ONTOLOGY_METADATA_CONVERSATIONNEXT_CURSOR, firstId);
            node->setExtraInfo(nodeExtra);
        }
        setStatus(node, NodePrivate::Idle);
        return;
    }

    QString requestPath = requestUrl.path();

    TwitterInterface::ContentItemType type = TwitterInterface::Unknown;
    // We strip the 1.1 from the path, and then, we are able to know what is retrieved
    // by simply using the ontology
    requestPath.remove(0, 5);
    if (requestPath == TWITTER_ONTOLOGY_CONNECTION_FRIENDS_LIST
        || requestPath == TWITTER_ONTOLOGY_CONNECTION_FOLLOWERS_LIST) {
        type = TwitterInterface::User;
    }

    if (requestPath == TWITTER_ONTOLOGY_CONNECTION_STATUSES_USER_TIMELINE
        || requestPath == TWITTER_ONTOLOGY_CONNECTION_STATUSES_HOME_TIMELINE) {
        type = TwitterInterface::Tweet;
    }

    // Just like in Facebook, we create two QVariantMap to hold page properties.
    // They will use PAGING_HAVE_KEY to store if there is a next (resp. previous)
    // page that can be loaded.
    bool havePreviousRelatedData = false;
    bool haveNextRelatedData = false;

    switch (type) {
        case TwitterInterface::User:
        {
            QVariantMap relatedDataMap = relatedData.toMap();
            QVariantList userList = relatedDataMap.value(TWITTER_ONTOLOGY_CONNECTION_USERS_KEY).toList();
            foreach (const QVariant &variant, userList) {
                QVariantMap variantMap = variant.toMap();
                QString identifier = variantMap.value(TWITTER_ONTOLOGY_METADATA_ID).toString();
                variantMap.insert(NEMOQMLPLUGINS_SOCIAL_CONTENTITEMTYPE, TwitterInterface::User);
                variantMap.insert(NEMOQMLPLUGINS_SOCIAL_CONTENTITEMID, identifier);
                relatedContent.append(createCacheEntry(variantMap, identifier));
            }

            // Do the cursoring
            QString previousCursor = relatedDataMap.value(TWITTER_ONTOLOGY_METADATA_PREVIOUS_CURSOR,
                                                          TWITTER_ONTOLOGY_METADATA_NULL_CURSOR).toString();
            QString nextCursor = relatedDataMap.value(TWITTER_ONTOLOGY_METADATA_NEXT_CURSOR,
                                                      TWITTER_ONTOLOGY_METADATA_NULL_CURSOR).toString();

            QVariantMap previousExtra;
            QVariantMap nextExtra;
            QVariantMap nodeExtra = node->extraInfo();
            previousExtra.insert(TWITTER_ONTOLOGY_METADATA_CURSOR, previousCursor);
            nextExtra.insert(TWITTER_ONTOLOGY_METADATA_CURSOR, nextCursor);

            previousExtra.insert(PAGING_HAVE_KEY, previousCursor != TWITTER_ONTOLOGY_METADATA_NULL_CURSOR);
            nextExtra.insert(PAGING_HAVE_KEY, nextCursor != TWITTER_ONTOLOGY_METADATA_NULL_CURSOR);
            SocialNetworkInterfacePrivate::setNodeExtraPaging(nodeExtra, previousExtra, nextExtra,
                                                              node->status());
            node->setExtraInfo(nodeExtra);
            previousExtra = nodeExtra.value(NODE_EXTRA_PAGING_PREVIOUS_KEY).toMap();
            nextExtra = nodeExtra.value(NODE_EXTRA_PAGING_NEXT_KEY).toMap();

            havePreviousRelatedData = previousExtra.value(PAGING_HAVE_KEY).toBool();
            haveNextRelatedData = nextExtra.value(PAGING_HAVE_KEY).toBool();
        }
        break;
        case TwitterInterface::Tweet:
        {
            QVariantList relatedDataList = relatedData.toList();
            foreach (const QVariant &variant, relatedDataList) {
                QVariantMap variantMap = variant.toMap();
                QString identifier = variantMap.value(TWITTER_ONTOLOGY_METADATA_ID).toString();
                variantMap.insert(NEMOQMLPLUGINS_SOCIAL_CONTENTITEMTYPE, TwitterInterface::Tweet);
                variantMap.insert(NEMOQMLPLUGINS_SOCIAL_CONTENTITEMID, identifier);
                relatedContent.append(createCacheEntry(variantMap, identifier));
            }
        }
        break;
        default: {
            qWarning() << Q_FUNC_INFO << "Unsupported data retrieved";
            qWarning() << Q_FUNC_INFO << "Request url:" << requestUrl;
            qWarning() << Q_FUNC_INFO << "Data: " << relatedData;
        }
        break;
    }

    updateModelRelatedData(node, relatedContent);

    // We should get the cursors for tweets with all the data we have loaded
    if (type == TwitterInterface::Tweet) {
        havePreviousRelatedData = true;
        haveNextRelatedData = true;

        CacheEntry::List relatedData = node->relatedData();
        if (!relatedData.isEmpty()) {
            // Create the token pairs for tweets
            // For newer tweets, (that are "previous") we should ask for since_id = first tweet id
            // And it might need several calls, but we TODO this
            // For older tweets, we should ask for max_id = last tweet id - 1
            QString firstTweetId = relatedData.first()->data().value(NEMOQMLPLUGINS_SOCIAL_CONTENTITEMID).toString();
            QString lastTweetId = relatedData.last()->data().value(NEMOQMLPLUGINS_SOCIAL_CONTENTITEMID).toString();
            qulonglong lastTweetIdInt = lastTweetId.toULongLong() - 1;

            QVariantMap previousExtra;
            previousExtra.insert(TWITTER_ONTOLOGY_CONNECTION_SINCE_ID_KEY, firstTweetId);

            QVariantMap nextExtra;
            nextExtra.insert(TWITTER_ONTOLOGY_CONNECTION_MAX_ID_KEY, QString::number(lastTweetIdInt));

            QVariantMap nodeExtra = node->extraInfo();
            SocialNetworkInterfacePrivate::setNodeExtraPaging(nodeExtra, previousExtra, nextExtra,
                                                              node->status());
            node->setExtraInfo(nodeExtra);

        }
    }

    updateModelHavePreviousAndNext(node, havePreviousRelatedData, haveNextRelatedData);
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
    info.extraData.insert(TWITTER_ONTOLOGY_CONNECTION_TRIM_USER_KEY,
                          boolToString(trimUser));
    info.extraData.insert(TWITTER_ONTOLOGY_CONNECTION_STATUSES_SHOW_INCLUDE_MY_RETWEET_KEY,
                          boolToString(includeMyRetweet));
    info.extraData.insert(TWITTER_ONTOLOGY_CONNECTION_INCLUDE_ENTITIES_KEY,
                          boolToString(includeEntities));
    return info;
}

/*! \reimp */
void TwitterInterfacePrivate::populateDataForNode(Node::Ptr node)
{
    RequestInfo requestInfo;
    switch (node->type()) {
        case TwitterInterface::User: {
            requestInfo = TwitterInterfacePrivate::requestUserInfo(node->identifier());
        }
        break;
        case TwitterInterface::Tweet: {
            requestInfo = TwitterInterfacePrivate::requestTweetInfo(node->identifier());
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
void TwitterInterfacePrivate::populateRelatedDataforNode(Node::Ptr node)
{
    if (!performRelatedDataRequest(node, node->identifier(), node->filters().toList())) {
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

    ContentItemInterface *interface = 0;

    switch (detectedType) {
        case TwitterInterface::User: interface = new TwitterUserInterface(parent); break;
        case TwitterInterface::Tweet: interface = new TwitterTweetInterface(parent); break;

        // TODO: other types incl. me user, tweets, places, directmessages.
        case TwitterInterface::Unknown: {
            qWarning() << Q_FUNC_INFO << "Unable to detect the type of the content item";
            interface = new IdentifiableContentItemInterface(parent);
        }
        break;
        default: qWarning() << Q_FUNC_INFO << "unsupported type:" << detectedType; break;
    }

    if (interface) {
        interface->classBegin();
        interface->setSocialNetwork(const_cast<TwitterInterface*>(q));
        q->setContentItemData(interface, data);
        interface->componentComplete();
    }

    return interface;
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
    Q_Q(TwitterInterface);
    Q_UNUSED(objectIdentifier)
    if (!q->isInitialized()) {
        qWarning() << Q_FUNC_INFO << "cannot complete post request: not initialized";
        return 0;
    }

    // create post data
    QByteArray postData;
    if (!data.isEmpty()) {
        foreach (const QString &key, data.keys()) {
            postData.append(key.toLocal8Bit());
            postData.append("=");
            postData.append(QUrl::toPercentEncoding(data.value(key).toString()));
            postData.append("&");
        }
        postData.chop(1);
    }

    QNetworkRequest request = networkRequest(extraPath, extraData, "POST", data);
    // Set type for request otherwise there is this warning

    return networkAccessManager->post(request, postData);
}

/*! \reimp */
QNetworkReply * TwitterInterfacePrivate::deleteRequest(const QString &objectIdentifier,
                                                       const QString &extraPath,
                                                       const QVariantMap &extraData)
{
    return SocialNetworkInterfacePrivate::deleteRequest(objectIdentifier, extraPath, extraData);
}

int TwitterInterfacePrivate::guessType(const QString &identifier, int type,
                                       const QSet<FilterInterface *> &filters)
{
    Q_UNUSED(identifier)
    Q_UNUSED(type)
    if (filters.isEmpty()) {
        return -1;
    }

    // If we have filters, we are able to identify the type of node
    // We first cast filters to known filters
    QList<ContentItemTypeFilterInterface *> contentTypeFilters;
    QList<TwitterConversationFilterInterface *> conversationFilters;

    foreach (FilterInterface *filter, filters) {
        ContentItemTypeFilterInterface *contentItemTypeFilter
                = qobject_cast<ContentItemTypeFilterInterface*>(filter);
        TwitterConversationFilterInterface *conversationFilter
                = qobject_cast<TwitterConversationFilterInterface *>(filter);
        if (!contentItemTypeFilter && !conversationFilter) {
            qWarning() << "The Twitter adapter only supports ContentItemType and TwitterConversationFilter filters";
        } else {
            if (contentItemTypeFilter) {
                contentTypeFilters.append(contentItemTypeFilter);
            }
            if (conversationFilter) {
                conversationFilters.append(conversationFilter);
            }
        }
    }

    // Get the type from the filter
    if (contentTypeFilters.count() + conversationFilters.count() != 1) {
        return -1;
    }

    int guessedType = -1;

    if (!contentTypeFilters.isEmpty()) {
        int filterType = contentTypeFilters.first()->type();
        switch (filterType) {
            case TwitterInterface::Tweet:
            case TwitterInterface::Home:
            case TwitterInterface::Friends:
            case TwitterInterface::Followers: {
                guessedType = TwitterInterface::User;
            }
            break;
            default: break;
        }
    }
    if (!conversationFilters.isEmpty()) {
        guessedType = TwitterInterface::Tweet;
    }

    return guessedType;
}

/*! \internal */
void TwitterInterfacePrivate::handleFinished(Node::Ptr node, QNetworkReply *reply)
{
    QByteArray replyData = reply->readAll();
    QUrl requestUrl = reply->request().url();
    deleteReply(reply);
    bool ok = false;
    QVariant responseData = ContentItemInterfacePrivate::parseReplyDataVariant(replyData, &ok);
    if (!ok) {
        setError(node, SocialNetworkInterface::RequestError,
                 QLatin1String("Error populating node: response is invalid. "\
                               "Perhaps the requested object id was incorrect?  Response: ")
                 + QString::fromLatin1(replyData.constData()));
        return;
    }


    switch (node->status()) {
        case NodePrivate::LoadingNodeData:{
            QVariantMap responseDataMap = responseData.toMap();
            // Create a cache entry associated to the retrieved data
            QString identifier;
            if (responseDataMap.contains(TWITTER_ONTOLOGY_METADATA_ID)) {
                identifier = responseDataMap.value(TWITTER_ONTOLOGY_METADATA_ID).toString();
            }
            responseDataMap.insert(NEMOQMLPLUGINS_SOCIAL_CONTENTITEMTYPE, node->type());

            CacheEntry::Ptr cacheEntry = createCacheEntry(responseDataMap, identifier);
            node->setCacheEntry(cacheEntry);
            updateModelNode(node);

            // Performing a replace
            node->setStatus(NodePrivate::LoadingRelatedDataReplacing);
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

bool TwitterInterfacePrivate::performRelatedDataRequest(Node::Ptr node, const QString &identifier,
                                                        const QList<FilterInterface *> &filters)
{
    // We first cast filters to known filters
    QList<FilterInterface *> castedFilters;

    foreach (FilterInterface *filter, filters) {
        ContentItemTypeFilterInterface *contentItemTypeFilter
                = qobject_cast<ContentItemTypeFilterInterface*>(filter);
        TwitterConversationFilterInterface *conversationFilter
                = qobject_cast<TwitterConversationFilterInterface*>(filter);
        if (!contentItemTypeFilter && !conversationFilter) {
            qWarning() << "The Twitter adapter only supports ContentItemType and TwitterConversationFilter filters";
        } else {
            castedFilters.append(filter);
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

    // Build the query
    QString extraPath;
    QVariantMap extraData;

    QVariantMap nodeExtraInfo = node->extraInfo();

    ContentItemTypeFilterInterface *filter
            = qobject_cast<ContentItemTypeFilterInterface*>(castedFilters.first());

    if (!filter) {
        // TODO implement upstream support
        QVariantMap extraInfo = node->extraInfo();
        extraInfo.insert(TWITTER_ONTOLOGY_CONNECTION_CONVERSATION_KEY, true);
        node->setExtraInfo(extraInfo);

        extraPath = TWITTER_ONTOLOGY_CONNECTION_STATUSES_HOME_TIMELINE;
        QString limitingIdentifier = identifier;
        if (nodeExtraInfo.contains(TWITTER_ONTOLOGY_METADATA_CONVERSATIONNEXT_CURSOR)
            && node->status() == NodePrivate::LoadingRelatedDataAppending) {
            limitingIdentifier = nodeExtraInfo.value(TWITTER_ONTOLOGY_METADATA_CONVERSATIONNEXT_CURSOR).toString();
        }
        extraData.insert(TWITTER_ONTOLOGY_CONNECTION_SINCE_ID_KEY, limitingIdentifier);
        extraData.insert(TWITTER_ONTOLOGY_CONNECTION_COUNT_KEY, 200);
        setReply(node, getRequest(QString(), extraPath, QStringList(), extraData));
        return true;
    }

    // If we have cursors, we add them
    switch (node->status()) {
        case NodePrivate::LoadingRelatedDataAppending: {
            QVariantMap extraMap = node->extraInfo().value(NODE_EXTRA_PAGING_NEXT_KEY).toMap();
            qDebug() << extraMap;

            foreach(QString key, extraMap.keys()) {
                if (key != PAGING_HAVE_KEY) {
                    extraData.insert(key, extraMap.value(key));
                }
            }
        }
        break;
        case NodePrivate::LoadingRelatedDataPrepending: {
            QVariantMap extraMap = node->extraInfo().value(NODE_EXTRA_PAGING_PREVIOUS_KEY).toMap();

            qDebug() << extraMap;

            foreach(QString key, extraMap.keys()) {
                if (key != PAGING_HAVE_KEY) {
                    extraData.insert(key, extraMap.value(key));
                }
            }
        }
        break;
        default: break;
    }

    switch (filter->type()) {
    case TwitterInterface::Friends:
        extraPath = TWITTER_ONTOLOGY_CONNECTION_FRIENDS_LIST;
        extraData.insert(TWITTER_ONTOLOGY_CONNECTION_USER_ID_KEY, identifier);
        extraData.insert(TWITTER_ONTOLOGY_CONNECTION_SKIP_STATUS_KEY, boolToString(true));
        extraData.insert(TWITTER_ONTOLOGY_CONNECTION_INCLUDE_USER_ENTITIES_KEY,
                         boolToString(true));
        break;
    case TwitterInterface::Followers:
        extraPath = TWITTER_ONTOLOGY_CONNECTION_FOLLOWERS_LIST;
        extraData.insert(TWITTER_ONTOLOGY_CONNECTION_USER_ID_KEY, identifier);
        extraData.insert(TWITTER_ONTOLOGY_CONNECTION_SKIP_STATUS_KEY, boolToString(true));
        extraData.insert(TWITTER_ONTOLOGY_CONNECTION_INCLUDE_USER_ENTITIES_KEY,
                         boolToString(true));
        break;
    case TwitterInterface::Tweet:
        extraPath = TWITTER_ONTOLOGY_CONNECTION_STATUSES_USER_TIMELINE;
        extraData.insert(TWITTER_ONTOLOGY_CONNECTION_USER_ID_KEY, identifier);
        extraData.insert(TWITTER_ONTOLOGY_CONNECTION_INCLUDE_USER_ENTITIES_KEY,
                         boolToString(true));
        break;
    case TwitterInterface::Home:
        extraPath = TWITTER_ONTOLOGY_CONNECTION_STATUSES_HOME_TIMELINE;
        extraData.insert(TWITTER_ONTOLOGY_CONNECTION_INCLUDE_USER_ENTITIES_KEY,
                         boolToString(true));
        extraData.insert(TWITTER_ONTOLOGY_CONNECTION_COUNT_KEY, 200);
        extraData.insert(TWITTER_ONTOLOGY_CONNECTION_TRIM_USER_KEY, boolToString(false));
        extraData.insert(TWITTER_ONTOLOGY_CONNECTION_EXCLUDE_REPLIES_KEY, boolToString(false));
        extraData.insert(TWITTER_ONTOLOGY_CONNECTION_CONTRIBUTOR_DETAILS_KEY, boolToString(true));
        extraData.insert(TWITTER_ONTOLOGY_CONNECTION_INCLUDE_USER_ENTITIES_KEY,
                         boolToString(true));
        break;
    default:
        qWarning() << Q_FUNC_INFO << "Type" << filter->type() << "is invalid";
        return false;
        break;
    }

    setReply(node, getRequest(QString(), extraPath, QStringList(), extraData));
    return true;
}

void TwitterInterfacePrivate::makeConversationList(const QMap<QString, CacheEntry::Ptr> &tweets,
                                                   const QMap<QString, QList<QString> > &tweetsTree,
                                                   const QString &parent, CacheEntry::List &list)
{
    const QList<QString> children = tweetsTree.value(parent);
    foreach (const QString &child, children) {
        makeConversationList(tweets, tweetsTree, child, list);
        list.prepend(tweets.value(child));
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

void TwitterInterface::setOAuthToken(const QString &oauthToken)
{
    Q_D(TwitterInterface);
    if (d->oauthToken != oauthToken) {
        d->oauthToken = oauthToken;
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

void TwitterInterface::setOAuthTokenSecret(const QString &oauthTokenSecret)
{
    Q_D(TwitterInterface);
    if (d->oauthTokenSecret != oauthTokenSecret) {
        d->oauthTokenSecret = oauthTokenSecret;
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

void TwitterInterface::setConsumerSecret(const QString &consumerSecret)
{
    Q_D(TwitterInterface);
    if (d->consumerSecret != consumerSecret) {
        d->consumerSecret = consumerSecret;
        emit consumerSecretChanged();
    }
}

QString TwitterInterface::currentUserIdentifier() const
{
    Q_D(const TwitterInterface);
    return d->currentUserIdentifier;
}

void TwitterInterface::setCurrentUserIdentifier(const QString &currentUserIdentifier)
{
    Q_D(TwitterInterface);
    if (d->currentUserIdentifier != currentUserIdentifier) {
        d->currentUserIdentifier = currentUserIdentifier;
        emit currentUserIdentifierChanged();
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
