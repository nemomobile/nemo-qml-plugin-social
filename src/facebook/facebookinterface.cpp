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

#include "facebookinterface.h"
#include "facebookinterface_p.h"
#include "facebookontology_p.h"
#include "socialnetworkinterface_p.h"
#include "socialnetworkmodelinterface.h"
#include "socialnetworkmodelinterface_p.h"

#include "contentiteminterface.h"
#include "contentiteminterface_p.h"
#include "identifiablecontentiteminterface.h"
#include "contentitemtypefilterinterface.h"
#include "facebookcommentfilterinterface.h"

#include "facebookobjectreferenceinterface.h"
#include "facebookalbuminterface.h"
#include "facebookcommentinterface.h"
#include "facebooknotificationinterface.h"
#include "facebookphotointerface.h"
#include "facebookpostinterface.h"
#include "facebookuserinterface.h"

#include "facebooklikeinterface.h"

#include "util_p.h"

#include <QtCore/QMetaObject>
#include <QtCore/QMetaProperty>
#include <QtCore/QVariantMap>
#include <QtCore/QStringList>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QByteArray>
#include <QtNetwork/QNetworkReply>

#include <QtDebug>

#define FACEBOOK_ME QLatin1String("me")
#define GETTING_ME_KEY QLatin1String("getting_me")
#define PERFORM_ADDITIONAL_LOADING QLatin1String("perform_additional_loading")
#define PERFORM_TYPE_LOADING QLatin1String("perform_type_loading")
#define PAGING_HAVE_KEY QLatin1String("have")

FacebookInterfacePrivate::FacebookInterfacePrivate(FacebookInterface *q)
    : SocialNetworkInterfacePrivate(q)
    , currentUserIdentifier(FACEBOOK_ME)
{
}

void FacebookInterfacePrivate::populateDataForNode(CacheNode::Ptr node)
{
    // We are trying to get the current user identifier at the same time
    // Of cause, we do that if we are not already getting the "me" as a node
    if (currentUserIdentifier == FACEBOOK_ME && node->identifier() != FACEBOOK_ME) {
        QVariantMap extraInfo;
        extraInfo.insert(GETTING_ME_KEY, QVariant::fromValue(true));
        node->setExtraInfo(extraInfo);
        QVariantMap extra;
        extra.insert(QLatin1String("ids"), QString("%1,%2").arg(FACEBOOK_ME, node->identifier()));
        setReply(node, getRequest(QString(), QString(), QStringList(), extra));
        return;
    }

    setReply(node, getRequest(node->identifier(), QString(), QStringList(), QVariantMap()));
}

void FacebookInterfacePrivate::populateRelatedDataforNode(CacheNode::Ptr node)
{
    const QString &identifier = node->identifier();
    const QSet<FilterInterface *> &filters = node->filters();


    QList<FilterInterface *> realFilters;
    foreach (FilterInterface *filter, filters) {
        ContentItemTypeFilterInterface *contentItemTypeFilter
                = qobject_cast<ContentItemTypeFilterInterface*>(filter);
        FacebookCommentFilterInterface *facebookCommentFilterInterface
                = qobject_cast<FacebookCommentFilterInterface*>(filter);
        if (!contentItemTypeFilter && !facebookCommentFilterInterface) {
            qWarning() << Q_FUNC_INFO
                       << "The Facebook adapter only supports ContentItemType and FacebookCommentFilter filters";
            continue;
        }

        realFilters.append(filter);
    }

    if (realFilters.isEmpty()) {
        // An empty filter should not create an error
        // Actually, nothing should be loaded in the node and
        // the request should be terminated
        setStatus(node, CacheNodePrivate::Idle);
        return;
    }

    // Notifications is a tough one as well. It cannot be queried via fields expansion
    // so if notifications is passed in the list of filters, it is ignored if
    // there are other filters, and queried only if it is the only one.

    // Extra should map the type to some extra fields to add. These fields will
    // be added to field expansion (or, in case of notifications, to normal
    // graph query)

    QList<int> connectionTypes;
    RequestFieldsMap connectionFields;

    // Create paging request
    RequestFieldsMap requestFieldsMap;
    switch (node->status()) {
        case CacheNodePrivate::LoadingRelatedDataAppending: {
            QVariantMap extraMap = node->extraInfo().value(NODE_EXTRA_PAGING_NEXT_KEY).toMap();

            foreach (QString key, extraMap.keys()) {
                int keyInteger = key.toInt();
                QVariantMap entries = extraMap.value(key).toMap();

                foreach(QString entryKey, entries.keys()) {
                    if (entryKey != PAGING_HAVE_KEY) {
                        StringPair pair (entryKey, entries.value(entryKey).toString());
                        requestFieldsMap.insert(keyInteger, pair);
                    }
                }
            }
        }
        break;
        case CacheNodePrivate::LoadingRelatedDataPrepending: {
            QVariantMap extraMap = node->extraInfo().value(NODE_EXTRA_PAGING_PREVIOUS_KEY).toMap();

            foreach (QString key, extraMap.keys()) {
                int keyInteger = key.toInt();
                QVariantMap entries = extraMap.value(key).toMap();

                foreach(QString entryKey, entries.keys()) {
                    if (entryKey != PAGING_HAVE_KEY) {
                        StringPair pair (entryKey, entries.value(entryKey).toString());
                        requestFieldsMap.insert(keyInteger, pair);
                    }
                }
            }
        }
        break;
        default: break;
    }

    foreach (FilterInterface *filter, realFilters) {
        ContentItemTypeFilterInterface *contentItemTypeFilter
                = qobject_cast<ContentItemTypeFilterInterface*>(filter);
        FacebookCommentFilterInterface *facebookCommentFilterInterface
                = qobject_cast<FacebookCommentFilterInterface*>(filter);

        int type = -1;
        if (contentItemTypeFilter) {
            // We only use the first filter of a given type to
            // build the request. So we ignore all the following filters
            // of the same type
            type = contentItemTypeFilter->type();
            if (!connectionTypes.contains(type)) {
                connectionTypes.append(type);
                if (!contentItemTypeFilter->whichFields().isEmpty()) {
                    StringPair fieldsEntry (QLatin1String("fields"),
                                            contentItemTypeFilter->whichFields().join(","));
                    connectionFields.insert(type, fieldsEntry);
                }
                if (contentItemTypeFilter->limit() > 0) {
                    StringPair limitEntry (QLatin1String("limit"),
                                           QString::number(contentItemTypeFilter->limit()));
                    connectionFields.insert(type, limitEntry);
                }

            }
        }

        if (facebookCommentFilterInterface) {
            type = FacebookInterface::Comment;
            // Just like before, we ignore redundant filter
            // especially a "Comment" type ContentItemTypeFilterInterface
            if (!connectionTypes.contains(type)) {
                connectionTypes.append(type);

                int limit = facebookCommentFilterInterface->limit();
                int offset = 0;
                StringPair limitEntry (FACEBOOK_ONTOLOGY_METADATA_PAGING_LIMIT,
                                       QString::number(limit));
                connectionFields.insert(type, limitEntry);

                // If we are prepending or appending, we don't need offset
                if (node->status() != CacheNodePrivate::LoadingRelatedDataAppending
                    && node->status() != CacheNodePrivate::LoadingRelatedDataPrepending) {

                    switch (facebookCommentFilterInterface->retrieveMode()) {
                        case FacebookCommentFilterInterface::RetrieveOffset:
                            offset = facebookCommentFilterInterface->offset() * limit;
                        break;
                        case FacebookCommentFilterInterface::RetrieveLatest: {

                            // Slightly hacky, we retrieve the commentsCount property
                            // from the CacheEntry item. If we can't we fallback to 0.
                            const ContentItemInterface *item = node->cacheEntry()->item();
                            if (!item) {
                                break;
                            }

                            const QMetaObject *metaObject = item->metaObject();
                            int propertyIndex = metaObject->indexOfProperty("commentsCount");
                            if (propertyIndex == -1) {
                                break;
                            }

                            QMetaProperty property = metaObject->property(propertyIndex);
                            QVariant countVariant = property.read(item);

                            if (!countVariant.isValid()) {
                                break;
                            }

                            int count = countVariant.toInt();
                            offset = qMax(count - limit, 0);
                        }
                        break;
                    default:
                        break;
                    }

                    StringPair offsetEntry (FACEBOOK_ONTOLOGY_METADATA_PAGING_OFFSET,
                                            QString::number(offset));
                    connectionFields.insert(type, offsetEntry);
                }
            }

        }

        // Add paging entries first
        if (requestFieldsMap.contains(type)) {
            QList<QPair<QString, QString> > extraValues = requestFieldsMap.values(type);
            QList<QPair<QString, QString> >::Iterator i;
            for (i = extraValues.begin(); i != extraValues.end(); i++) {
                connectionFields.insert(type, *i);
            }
        }
    }

    // Check if we have notifications
    if (connectionTypes.contains(FacebookInterface::Notification)) {
        if (connectionTypes.count() == 1) {
            QVariantMap extraData;
            extraData.insert(QLatin1String("include_read"), 1);
            QList<QPair<QString, QString> > extraValues
                    = connectionFields.values(FacebookInterface::Notification);
            QList<QPair<QString, QString> >::Iterator i;
            for (i = extraValues.begin(); i != extraValues.end(); i++) {
                extraData.insert(i->first, i->second);
            }

            setReply(node, getRequest(identifier, FACEBOOK_ONTOLOGY_CONNECTIONS_NOTIFICATIONS,
                                      QStringList(), extraData));
            return;
        } else {
            qWarning() << Q_FUNC_INFO << "Notifications should not be requested with other filters";
            connectionTypes.removeAll(FacebookInterface::Notification);
            connectionFields.remove(FacebookInterface::Notification);
        }
    }

    // generate appropriate query string, using the Field Expansion query syntax
    // of the Facebook OpenGraph API. eg: with currentNode = Photo;
    // connectionTypes == comments,likes,tags; whichFields = id,name; limit = 10, we have
    // GET https://graph.facebook.com/<photo_id>?fields=comments.limit(10).fields(id,name),
    //                                                  likes.limit(10).fields(id,name),
    //                                                  tags.fields(id,name).limit(10)

    // XXX TODO: in the future, each of the Facebook-specific IdentifiableContentItemType classes should
    // provide private helper functions for the FacebookInterface to build the appropriate query.
    // e.g: QString FacebookAlbumInterface::relatedDataQuery(types, limits, whichfields);
    // That way we can provide "special case" code for every type in a neat, modular fashion.
    // generic query
    QStringList fields;

    foreach (int typeInteger, connectionTypes) {
        FacebookInterface::ContentItemType type
                = static_cast<FacebookInterface::ContentItemType>(typeInteger);
        switch (type) {
            case FacebookInterface::Like: {
                fields.append(createField(typeInteger, FACEBOOK_ONTOLOGY_CONNECTIONS_LIKES,
                                          connectionFields));
            }
            break;
            case FacebookInterface::PhotoTag: {
                fields.append(createField(typeInteger, FACEBOOK_ONTOLOGY_CONNECTIONS_TAGS,
                                          connectionFields));
            }
            break;
            case FacebookInterface::UserPicture: {
                fields.append(createField(typeInteger, FACEBOOK_ONTOLOGY_CONNECTIONS_PICTURE,
                                          connectionFields));
            }
            break;
            case FacebookInterface::Location: {
                fields.append(createField(typeInteger, FACEBOOK_ONTOLOGY_CONNECTIONS_LOCATIONS,
                                          connectionFields));
            }
            break; // not supported?
            case FacebookInterface::Comment: {
                fields.append(createField(typeInteger, FACEBOOK_ONTOLOGY_CONNECTIONS_COMMENTS,
                                          connectionFields));
            }
            break;
            case FacebookInterface::User: {
                fields.append(createField(typeInteger, FACEBOOK_ONTOLOGY_CONNECTIONS_FRIENDS,
                                          connectionFields));
            }
            break; // subscriptions etc?
            case FacebookInterface::Album: {
                fields.append(createField(typeInteger, FACEBOOK_ONTOLOGY_CONNECTIONS_ALBUMS,
                                          connectionFields));
            }
            break;
            case FacebookInterface::Photo: {
                fields.append(createField(typeInteger, FACEBOOK_ONTOLOGY_CONNECTIONS_PHOTOS,
                                          connectionFields));
            }
            break;
            case FacebookInterface::Event: {
                fields.append(createField(typeInteger, FACEBOOK_ONTOLOGY_CONNECTIONS_EVENTS,
                                          connectionFields));
            }
            break;
            case FacebookInterface::Post: {
                fields.append(createField(typeInteger, FACEBOOK_ONTOLOGY_CONNECTIONS_FEED,
                                          connectionFields));
            }
            break;
            case FacebookInterface::Home: {
                fields.append(createField(typeInteger, FACEBOOK_ONTOLOGY_CONNECTIONS_HOME,
                                          connectionFields));
            }
            break;
            default: {
                qWarning() << Q_FUNC_INFO << "Invalid content item type specified in filter";
            }
            break;
        }
    }

    setReply(node, getRequest(identifier, QString(), fields, QVariantMap()));
}

bool FacebookInterfacePrivate::validateCacheEntryForNode(CacheEntry::ConstPtr cacheEntry)
{
    // If the item has not been created yet, we might
    // need to be sure to have it created by reloading it
    const IdentifiableContentItemInterface *item = cacheEntry->identifiableItem();
    if (!item) {
        return false;
    }

    switch (cacheEntry->identifiableItem()->type()) {
        case FacebookInterface::Album:
        //case FacebookInterface::Comment: // TODO Please check
        case FacebookInterface::Photo:
        case FacebookInterface::Post: {
            return cacheEntry->data().contains(FACEBOOK_ONTOLOGY_METADATA_SECONDPHASE);
        }
        break;
        // TODO Case of friends loaded from the friendlist
        default: break;
    }
    return true;
}

/*! \reimp */
QString FacebookInterfacePrivate::dataSection(int type, const QVariantMap &data) const
{
    switch (type) {
        case FacebookInterface::User: {
            return data.value(FACEBOOK_ONTOLOGY_USER_NAME).toString();
        }
        default: {
            break;
        }
    }
    return SocialNetworkInterfacePrivate::dataSection(type, data);
}

ContentItemInterface * FacebookInterfacePrivate::contentItemFromData(const QVariantMap &data,
                                                                     QObject *parent) const
{
    Q_Q(const FacebookInterface);
    // Construct the appropriate FacebookWhateverInterface for the given data.
    int typeInt = data.value(NEMOQMLPLUGINS_SOCIAL_CONTENTITEMTYPE).toInt();
    FacebookInterface::ContentItemType type
            = static_cast<FacebookInterface::ContentItemType>(typeInt);

    ContentItemInterface *interface = 0;


    switch (type) {
        case FacebookInterface::Album: interface = new FacebookAlbumInterface(parent); break;
        case FacebookInterface::Comment: interface = new FacebookCommentInterface(parent); break;
        case FacebookInterface::Notification: {
            interface = new FacebookNotificationInterface(parent);
        }
        break;
        case FacebookInterface::Photo: interface = new FacebookPhotoInterface(parent); break;
        case FacebookInterface::Post: interface = new FacebookPostInterface(parent); break;
        case FacebookInterface::User: interface = new FacebookUserInterface(parent); break;

        case FacebookInterface::Like: interface = new FacebookLikeInterface(parent); break;

        case FacebookInterface::Unknown: {
            qWarning() << Q_FUNC_INFO << "Unable to detect the type of the content item";
            interface = new IdentifiableContentItemInterface(parent);
        }
        break;
        default: qWarning() << Q_FUNC_INFO << "unsupported type:" << type; break;
    }

    if (interface) {
        interface->classBegin();
        interface->setSocialNetwork(const_cast<FacebookInterface*>(q));
        q->setContentItemData(interface, data);
        interface->componentComplete();
    }

    return interface;
}

/*! \reimp */
QNetworkReply * FacebookInterfacePrivate::getRequest(const QString &objectIdentifier,
                                                     const QString &extraPath,
                                                     const QStringList &whichFields,
                                                     const QVariantMap &extraData)
{
    Q_Q(FacebookInterface);
    if (!q->isInitialized()) {
        qWarning() << Q_FUNC_INFO << "cannot complete get request: not initialized";
        return 0;
    }

    QNetworkRequest request = QNetworkRequest(requestUrl(objectIdentifier, extraPath,
                                                         whichFields, extraData));
    return networkAccessManager->get(request);
}

/*! \reimp */
QNetworkReply * FacebookInterfacePrivate::postRequest(const QString &objectIdentifier,
                                                      const QString &extraPath,
                                                      const QVariantMap &data,
                                                      const QVariantMap &extraData)
{
    Q_Q(FacebookInterface);
    if (!q->isInitialized()) {
        qWarning() << Q_FUNC_INFO << "cannot complete post request: not initialized";
        return 0;
    }

    // image upload is handled specially.
    if (extraData.value("isImageUpload").toBool()) {
        return uploadImage(objectIdentifier, extraPath, data, extraData);
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
    QNetworkRequest request(requestUrl(objectIdentifier, extraPath, QStringList(), extraData));
    request.setRawHeader("Accept",
                         "text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8");
    request.setRawHeader("Accept-Language", "en-us,en;q=0.5");
    request.setRawHeader("Accept-Encoding", "gzip,deflate");
    request.setRawHeader("Accept-Charset", "ISO-8859-1,utf-8;q=0.7,*;q=0.7");
    request.setRawHeader("Keep-Alive", "300");
    request.setRawHeader("Connection", "keep-alive");
    request.setRawHeader("Content-Type",
                         QString("multipart/form-data; boundary="+multipartBoundary).toLatin1());
    request.setHeader(QNetworkRequest::ContentLengthHeader, postData.size());

    // perform POST request
    return networkAccessManager->post(request, postData);
}

/*! \reimp */
QNetworkReply * FacebookInterfacePrivate::deleteRequest(const QString &objectIdentifier,
                                                        const QString &extraPath,
                                                        const QVariantMap &extraData)
{
    Q_Q(FacebookInterface);
    if (!q->isInitialized()) {
        qWarning() << Q_FUNC_INFO << "cannot complete delete request: not initialized";
        return 0;
    }

    QNetworkRequest request (requestUrl(objectIdentifier, extraPath, QStringList(), extraData));
    return networkAccessManager->deleteResource(request);
}

void FacebookInterfacePrivate::handleFinished(CacheNode::Ptr node, QNetworkReply *reply)
{
    QByteArray replyData = reply->readAll();
    QUrl requestUrl = reply->request().url();
    deleteReply(reply);
    bool ok = false;
    QVariantMap responseData = ContentItemInterfacePrivate::parseReplyData(replyData, &ok);
    if (!ok) {
        setError(node, SocialNetworkInterface::RequestError,
                 QLatin1String("Error populating node: response is invalid. "\
                               "Perhaps the requested object id was incorrect?  Response: ")
                 + QString::fromLatin1(replyData.constData()));
        return;
    }

    if (responseData.contains(QLatin1String("error"))) {
        QString errorResponse = QLatin1String("\n    error:");
        QVariantMap errorMap = responseData.value(QLatin1String("error")).toMap();
        QStringList keys = errorMap.keys();
        foreach (const QString &key, keys) {
            errorResponse += QLatin1String("\n        ")
                             + key + QLatin1String("=") + errorMap.value(key).toString();
        }
        qWarning() << Q_FUNC_INFO << "error response:" << errorResponse
                   << "while getting:" << requestUrl.toString();

        setError(node, SocialNetworkInterface::RequestError,
                 QLatin1String("Error populating node: response is error.  Response: ")
                 + QString::fromLatin1(replyData.constData()));
        return;
    }

    switch (node->status()) {
        case CacheNodePrivate::LoadingNodeData:{
            handlePopulateNode(node, responseData);
        }
        break;
        case CacheNodePrivate::LoadingRelatedDataReplacing:
        case CacheNodePrivate::LoadingRelatedDataPrepending:
        case CacheNodePrivate::LoadingRelatedDataAppending: {
            handlePopulateRelatedData(node, responseData, requestUrl);
        }
        break;

        default: break;
    }
}

/*! \internal */
QUrl FacebookInterfacePrivate::requestUrl(const QString &objectId, const QString &extraPath,
                                          const QStringList &whichFields,
                                          const QVariantMap &extraData)
{
    QString joinedFields = whichFields.join(QLatin1String(","));
    QList<QPair<QString, QString> > queryItems;
    if (!accessToken.isEmpty())
        queryItems.append(qMakePair<QString, QString>(QLatin1String("access_token"), accessToken));
    if (!whichFields.isEmpty())
        queryItems.append(qMakePair<QString, QString>(QLatin1String("fields"), joinedFields));
    QStringList extraDataKeys = extraData.keys();
    foreach (const QString &key, extraDataKeys) {
        queryItems.append(qMakePair<QString, QString>(key, extraData.value(key).toString()));
    }

    QUrl returnedUrl;
    returnedUrl.setScheme("https");
    returnedUrl.setHost("graph.facebook.com");
    if (extraPath.isEmpty())
        returnedUrl.setPath(QLatin1String("/v2.2/") + objectId);
    else
        returnedUrl.setPath(QLatin1String("/v2.2/") + objectId + QLatin1String("/") + extraPath);
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    QUrlQuery query;
    query.setQueryItems(queryItems);
    returnedUrl.setQuery(query);
#else
    returnedUrl.setQueryItems(queryItems);
#endif
    return returnedUrl;
}

/*! \internal */
QNetworkReply * FacebookInterfacePrivate::uploadImage(const QString &objectId,
                                                      const QString &extraPath,
                                                      const QVariantMap &data,
                                                      const QVariantMap &extraData)
{
    Q_Q(FacebookInterface);
    Q_UNUSED(extraData); // XXX TODO: privacy passed via extraData?

    // the implementation code for this function is taken from the transfer engine
    QNetworkRequest request;
    QUrl url("https://graph.facebook.com");
    QString path = "v2.2/" + objectId;
    if (!extraPath.isEmpty()) {
        path += QLatin1String("/") + extraPath;
    }
    url.setPath(path);
    request.setUrl(url);

    QString multipartBoundary = QLatin1String("-------Sska2129ifcalksmqq3");
    QString filePath = data.value("source").toUrl().toLocalFile();
    QString mimeType = QLatin1String("image/jpeg");
    if (filePath.endsWith("png")) {
        mimeType = QLatin1String("image/png"); // XXX TODO: more mimetypes?  better way to do this?
    }

    QFile f(filePath, q);
    if(!f.open(QIODevice::ReadOnly)) {
        qWarning() << Q_FUNC_INFO << "Error opening image file:" << filePath;
        return 0;
    }

    QByteArray imageData(f.readAll());
    f.close();

    QFileInfo info(filePath);

    // Fill in the image data first
    QByteArray postData;
    postData.append("--"+multipartBoundary+"\r\n");
    postData.append("Content-Disposition: form-data; name=\"access_token\"\r\n\r\n");
    postData.append(accessToken);
    postData.append("\r\n");

    // Actually the title isn't used
    postData.append("--"+multipartBoundary+"\r\n");
    postData.append("Content-Disposition: form-data; name=\"message\"\r\n\r\n");
    postData.append(data.value("message").toString());
    postData.append("\r\n");

    postData.append("--"+multipartBoundary+"\r\n");
    postData.append("Content-Disposition: form-data; name=\"name\"; filename=\""+info.fileName()+"\"\r\n");
    postData.append("Content-Type:"+mimeType+"\r\n\r\n");
    postData.append(imageData);
    postData.append("\r\n");

    postData.append("--"+multipartBoundary+"\r\n");
    postData.append("Content-Disposition: form-data; name=\"privacy\"\r\n\r\n");
    postData.append(QString("{\'value\':\'ALL_FRIENDS\'}"));
    postData.append("\r\n");
    postData.append("--"+multipartBoundary+"\r\n");

    // Header required
    request.setRawHeader("Accept", "text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8");
    request.setRawHeader("Accept-Language", "en-us,en;q=0.5");
    request.setRawHeader("Accept-Encoding", "gzip,deflate");
    request.setRawHeader("Accept-Charset", "ISO-8859-1,utf-8;q=0.7,*;q=0.7");
    request.setRawHeader("Keep-Alive", "300");
    request.setRawHeader("Connection", "keep-alive");
    request.setRawHeader("Content-Type",QString("multipart/form-data; boundary="+multipartBoundary).toLatin1());
    request.setHeader(QNetworkRequest::ContentLengthHeader, postData.size());

    return networkAccessManager->post(request, postData);
}

void FacebookInterfacePrivate::handlePopulateNode(CacheNode::Ptr node,
                                                  const QVariantMap &responseData)
{
    Q_Q(FacebookInterface);
    if (node->extraInfo().contains(PERFORM_ADDITIONAL_LOADING)) {
        // Merge the response data that we got to the old
        // one and recreate a newer item
        QVariantMap data = node->cacheEntry()->data();
        // We add this marker because we have to make the item know that we
        // are performing a second population part, and that the data
        // that is retrieved is correct
        data.insert(FACEBOOK_ONTOLOGY_METADATA_SECONDPHASE, QVariant::fromValue(true));

        foreach (const QString &key, responseData.keys()) {
            data.insert(key, responseData.value(key));
        }

        node->cacheEntry()->setData(data);
        updateModelNode(node);
    } else if (node->extraInfo().contains(PERFORM_TYPE_LOADING)) {
        // Get the type based on the metadata field
        QVariantMap data = node->cacheEntry()->data();

        QVariantMap metadata = responseData.value(FACEBOOK_ONTOLOGY_METADATA).toMap();
        QString typeString = metadata.value(FACEBOOK_ONTOLOGY_METADATA_TYPE).toString();
        FacebookInterface::ContentItemType type = FacebookInterface::Unknown;


        if (typeString == FACEBOOK_ONTOLOGY_ALBUM) {
            type = FacebookInterface::Album;
        } else if (typeString == FACEBOOK_ONTOLOGY_COMMENT) {
            type = FacebookInterface::Comment;
        } else if (typeString == FACEBOOK_ONTOLOGY_NOTIFICATION) {
            type = FacebookInterface::Notification;
        } else if (typeString == FACEBOOK_ONTOLOGY_PHOTO) {
            type = FacebookInterface::Photo;
        } else if (typeString == FACEBOOK_ONTOLOGY_POST) {
            type = FacebookInterface::Post;
        } else if (typeString == FACEBOOK_ONTOLOGY_USER) {
            type = FacebookInterface::User;
        } else {
            qWarning() << "Unable to identify an object of type" << typeString;
            setError(node, SocialNetworkInterface::RequestError,
                     QString("Unable to identify an object of type %1").arg(typeString));
            return;
        }

        data.insert(NEMOQMLPLUGINS_SOCIAL_CONTENTITEMTYPE, type);
        node->cacheEntry()->setData(data);

        QVariantMap extraInfo = node->extraInfo();
        extraInfo.remove(PERFORM_TYPE_LOADING);
        node->setExtraInfo(extraInfo);
    } else {
        QVariantMap writableResponseData = responseData;
        if (node->extraInfo().contains(GETTING_ME_KEY)) {
            // Remove the getting_me_key
            QVariantMap nodeExtra = node->extraInfo();
            nodeExtra.remove(GETTING_ME_KEY);
            node->setExtraInfo(nodeExtra);

            // In the case of getting me, we should split into two different objects
            QVariantMap objectData = writableResponseData.value(node->identifier()).toMap();
            QVariantMap meData = writableResponseData.value(FACEBOOK_ME).toMap();

            // Change the identifier if needed
            QString meId = meData.value(FACEBOOK_ONTOLOGY_OBJECTREFERENCE_OBJECTIDENTIFIER).toString();
            setCurrentUserIdentifier(meId);
            writableResponseData = objectData;
        }

        // Create a cache entry associated to the retrieved data
        QString identifier;
        if (writableResponseData.contains(FACEBOOK_ONTOLOGY_OBJECTREFERENCE_OBJECTIDENTIFIER)) {
            identifier = writableResponseData.value(FACEBOOK_ONTOLOGY_OBJECTREFERENCE_OBJECTIDENTIFIER).toString();
        }

        if (node->identifier() == FACEBOOK_ME) {
            setCurrentUserIdentifier(identifier);
        }

        CacheEntry::Ptr cacheEntry = createCacheEntry(writableResponseData, identifier);
        node->setCacheEntry(cacheEntry);
    }

    // Set type
    if (checkNodeType(node)) {
        return;
    }

    // Manage specific cases (likes / comments count for example)
    if (checkIfNeedAdditionalLoading(node)) {
        return;
    }

    updateModelNode(node);

    // Performing a replace
    node->setStatus(CacheNodePrivate::LoadingRelatedDataReplacing);
    populateRelatedDataforNode(node);
}

void FacebookInterfacePrivate::handlePopulateRelatedData(CacheNode::Ptr node,
                                                         const QVariantMap &relatedData,
                                                         const QUrl &requestUrl)
{
    // We first grab an update of the current user (if UserPicture is requested)
    if (relatedData.keys().contains(FACEBOOK_ONTOLOGY_USER_PICTURE)) {
        QVariantMap pictureData = relatedData.value(FACEBOOK_ONTOLOGY_USER_PICTURE).toMap();
        // Merge with current object
        QVariantMap data = node->cacheEntry()->data();
        data.insert(FACEBOOK_ONTOLOGY_USER_PICTURE, pictureData);
        node->cacheEntry()->setData(data);
        updateModelNode(node);
    }

    // We receive the related data and transform it into ContentItems.
    // Finally, we populate the cache for the node and update the internal model data.
    CacheEntry::List relatedContent;

    QString requestPath = requestUrl.path();
    bool ok = false;
    QVariantMap extraInfo = node->extraInfo();

    ok = ok || tryAddCacheEntryFromData(node->status(), relatedData, requestPath,
                                        FacebookInterface::Like,
                                        FACEBOOK_ONTOLOGY_CONNECTIONS_LIKES,
                                        relatedContent, extraInfo);
    ok = ok || tryAddCacheEntryFromData(node->status(), relatedData, requestPath,
                                        FacebookInterface::Comment,
                                        FACEBOOK_ONTOLOGY_CONNECTIONS_COMMENTS,
                                        relatedContent, extraInfo);
    ok = ok || tryAddCacheEntryFromData(node->status(), relatedData, requestPath,
                                        FacebookInterface::PhotoTag,
                                        FACEBOOK_ONTOLOGY_CONNECTIONS_TAGS,
                                        relatedContent, extraInfo);
    ok = ok || tryAddCacheEntryFromData(node->status(), relatedData, requestPath,
                                        FacebookInterface::Photo,
                                        FACEBOOK_ONTOLOGY_CONNECTIONS_PHOTOS,
                                        relatedContent, extraInfo);
    ok = ok || tryAddCacheEntryFromData(node->status(), relatedData, requestPath,
                                        FacebookInterface::Album,
                                        FACEBOOK_ONTOLOGY_CONNECTIONS_ALBUMS,
                                        relatedContent, extraInfo);
    ok = ok || tryAddCacheEntryFromData(node->status(), relatedData, requestPath,
                                        FacebookInterface::User,
                                        FACEBOOK_ONTOLOGY_CONNECTIONS_FRIENDS,
                                        relatedContent, extraInfo);
    ok = ok || tryAddCacheEntryFromData(node->status(), relatedData, requestPath,
                                        FacebookInterface::Notification,
                                        FACEBOOK_ONTOLOGY_CONNECTIONS_NOTIFICATIONS,
                                        relatedContent, extraInfo);
    ok = ok || tryAddCacheEntryFromData(node->status(), relatedData, requestPath,
                                        FacebookInterface::Post,
                                        FACEBOOK_ONTOLOGY_CONNECTIONS_FEED,
                                        relatedContent, extraInfo);
    ok = ok || tryAddCacheEntryFromData(node->status(), relatedData, requestPath,
                                        FacebookInterface::Home,
                                        FACEBOOK_ONTOLOGY_CONNECTIONS_HOME,
                                        relatedContent, extraInfo);
    if (!ok && relatedData.keys().count() == 2
            && relatedData.keys().contains(FACEBOOK_ONTOLOGY_USER_PICTURE)) {
        // Force the fact that user pictures can get through
        ok = true;
    }

    if (!ok) {
        QStringList relatedDataKeys = relatedData.keys();
        // If the list only contains id and/or created_time, it is ok
        if (relatedDataKeys.contains(FACEBOOK_ONTOLOGY_METADATA_ID)) {
            relatedDataKeys.removeAll(FACEBOOK_ONTOLOGY_METADATA_ID);
        }

        if (relatedDataKeys.contains(FACEBOOK_ONTOLOGY_POST_CREATEDTIME)) {
            relatedDataKeys.removeAll(FACEBOOK_ONTOLOGY_POST_CREATEDTIME);
        }

        // If we have more entries, we should be careful
        if (!relatedDataKeys.isEmpty()) {
            qWarning() << Q_FUNC_INFO << "Unsupported or empty data retrieved";
            qWarning() << Q_FUNC_INFO << "Request url:" << requestUrl;
            qWarning() << Q_FUNC_INFO << "List of keys: " << relatedData.keys();
        }

        // If we are loading previous or next, we should not disable completely paging
        // but instead, keep extra information, and just remove any way to go
        // next or previous. It is because we might get empty data because we reached
        // the end of a list.
        QVariantMap previousExtra;
        QVariantMap nextExtra;
        SocialNetworkInterfacePrivate::setNodeExtraPaging(extraInfo, previousExtra, nextExtra,
                                                          node->status());
    }

    node->setExtraInfo(extraInfo);

    bool havePreviousRelatedData = false;
    bool haveNextRelatedData = false;

    QVariantMap previousExtra = extraInfo.value(NODE_EXTRA_PAGING_PREVIOUS_KEY).toMap();
    QVariantMap nextExtra = extraInfo.value(NODE_EXTRA_PAGING_NEXT_KEY).toMap();

    foreach (QString key, previousExtra.keys()) {
        bool have = previousExtra.value(key).toMap().value(PAGING_HAVE_KEY).toBool();
        havePreviousRelatedData = havePreviousRelatedData || have;
    }

    foreach (QString key, nextExtra.keys()) {
        bool have = nextExtra.value(key).toMap().value(PAGING_HAVE_KEY).toBool();
        haveNextRelatedData = haveNextRelatedData || have;
    }

    updateModelHavePreviousAndNext(node, havePreviousRelatedData, haveNextRelatedData);
    updateModelRelatedData(node, relatedContent);

    setStatus(node, CacheNodePrivate::Idle);
}

void FacebookInterfacePrivate::setCurrentUserIdentifier(const QString &meId)
{
    Q_Q(FacebookInterface);
    if (currentUserIdentifier != meId) {
        currentUserIdentifier = meId;
        aliases.insert(FACEBOOK_ME, meId);
        aliases.insert(meId, FACEBOOK_ME);
        emit q->currentUserIdentifierChanged();
    }
}

// Returns if a loading should be performed
bool FacebookInterfacePrivate::checkNodeType(CacheNode::Ptr node)
{
    QVariantMap data = node->cacheEntry()->data();
    if (!data.contains(NEMOQMLPLUGINS_SOCIAL_CONTENTITEMTYPE)) {
        // Use some heuristics to detect the type of the data
        // it is not really exactly precise, and is only used
        // to detect the most common types, without performing
        // another network operation
        FacebookInterface::ContentItemType detectedType = FacebookInterface::Unknown;
        if (data.contains(FACEBOOK_ONTOLOGY_COMMENT_MESSAGE)
            && data.contains(FACEBOOK_ONTOLOGY_COMMENT_LIKECOUNT)) {
            detectedType = FacebookInterface::Comment;
        } else if (data.contains(FACEBOOK_ONTOLOGY_ALBUM_PRIVACY)
                 && data.contains(FACEBOOK_ONTOLOGY_ALBUM_CANUPLOAD)) {
            detectedType = FacebookInterface::Album;
        } else if (data.contains(FACEBOOK_ONTOLOGY_PHOTO_WIDTH)
                 && data.contains(FACEBOOK_ONTOLOGY_PHOTO_SOURCE)) {
            detectedType = FacebookInterface::Photo;
        } else if (data.contains(FACEBOOK_ONTOLOGY_USER_FIRSTNAME)
                 || data.contains(FACEBOOK_ONTOLOGY_USER_GENDER)) {
            detectedType = FacebookInterface::User;
        } else if ((data.contains(FACEBOOK_ONTOLOGY_POST_ACTIONS)
                  && data.contains(FACEBOOK_ONTOLOGY_POST_POSTTYPE))
                 || data.contains(FACEBOOK_ONTOLOGY_POST_STORY)) {
            detectedType = FacebookInterface::Post;
        }

        if (detectedType == FacebookInterface::Unknown) {
            qWarning() << Q_FUNC_INFO << "Performing metadata request to detect type";

            QVariantMap extraInfo = node->extraInfo();
            extraInfo.insert(PERFORM_TYPE_LOADING, QVariant::fromValue(true));
            node->setExtraInfo(extraInfo);

            QVariantMap extra;
            extra.insert(FACEBOOK_ONTOLOGY_METADATA, 1);
            setReply(node, getRequest(node->identifier(), QString(), QStringList(), extra));

            return true;
        } else {
            data.insert(NEMOQMLPLUGINS_SOCIAL_CONTENTITEMTYPE, detectedType);
            node->cacheEntry()->setData(data);
        }
    }

    return false;
}

bool FacebookInterfacePrivate::checkIfNeedAdditionalLoading(CacheNode::Ptr node)
{
    if (node->extraInfo().contains(PERFORM_ADDITIONAL_LOADING)) {
        QVariantMap extraInfo = node->extraInfo();
        extraInfo.remove(PERFORM_ADDITIONAL_LOADING);
        node->setExtraInfo(extraInfo);
        return false;
    }


    QVariantMap data = node->cacheEntry()->data();
    int typeInt = data.value(NEMOQMLPLUGINS_SOCIAL_CONTENTITEMTYPE).toInt();

    FacebookInterface::ContentItemType type
            = static_cast<FacebookInterface::ContentItemType>(typeInt);
    switch (type) {
        case FacebookInterface::Album:
        //case FacebookInterface::Comment: // TODO Please check
        case FacebookInterface::Photo:
        case FacebookInterface::Post: {
            QString field = QLatin1String("%1.%3(1),%2.%3(1)");
            field = field.arg(FACEBOOK_ONTOLOGY_CONNECTIONS_LIKES,
                              FACEBOOK_ONTOLOGY_CONNECTIONS_COMMENTS,
                              FACEBOOK_ONTOLOGY_CONNECTIONS_SUMMARY);
            QStringList whichFields;
            whichFields.append(field);

            QVariantMap extraInfo = node->extraInfo();
            extraInfo.insert(PERFORM_ADDITIONAL_LOADING, QVariant::fromValue(true));
            node->setExtraInfo(extraInfo);

            setReply(node, getRequest(node->identifier(), QString(), whichFields, QVariantMap()));
            return true;
        }
        break;
        default: break;
    }

    return false;
}

bool FacebookInterfacePrivate::tryAddCacheEntryFromData(CacheNodePrivate::Status nodeStatus,
                                                        const QVariantMap &relatedData,
                                                        const QString &requestPath,
                                                        int type,
                                                        const QString &typeName,
                                                        CacheEntry::List &list,
                                                        QVariantMap &extraInfo)
{

    QVariantMap globalPreviousExtra = extraInfo.value(NODE_EXTRA_PAGING_PREVIOUS_KEY).toMap();
    QVariantMap globalNextExtra = extraInfo.value(NODE_EXTRA_PAGING_NEXT_KEY).toMap();

    QVariantMap data;
    // First we try if the provided related data contains the "data" field
    if (relatedData.contains(FACEBOOK_ONTOLOGY_CONNECTIONS_DATA)) {
        if (requestPath.endsWith(typeName)) {
            data = relatedData;
        }
    }

    // Then we check if the provided related data contains the type name. And if
    // it contains a subfield called "data"
    if (relatedData.contains(typeName)) {
        QVariantMap subData = relatedData.value(typeName).toMap();
        if (subData.contains(FACEBOOK_ONTOLOGY_CONNECTIONS_DATA)) {
            data = subData;
        }
    }

    if (data.isEmpty()) {
        // Workaround: if there is nothing, it is likely that
        // we don't have either next or previous data
        QVariantMap nullPagingMap;
        nullPagingMap.insert(PAGING_HAVE_KEY, false);

        if (globalPreviousExtra.contains(QString::number(type))) {
            globalPreviousExtra.insert(QString::number(type), nullPagingMap);
        }

        if (globalNextExtra.contains(QString::number(type))) {
            globalNextExtra.insert(QString::number(type), nullPagingMap);
        }

        SocialNetworkInterfacePrivate::setNodeExtraPaging(extraInfo, globalPreviousExtra,
                                                          globalNextExtra, nodeStatus);

        return false;
    }

    QVariantList itemsList = data.value(FACEBOOK_ONTOLOGY_CONNECTIONS_DATA).toList();
    foreach (const QVariant &variant, itemsList) {
        QVariantMap variantMap = variant.toMap();
        QString identifier = variantMap.value(FACEBOOK_ONTOLOGY_METADATA_ID).toString();
        // If the type is a "Like", it is not identifiable:
        int trueType = type;
        if (type == FacebookInterface::Like) {
            identifier.clear();
        }
        if (type == FacebookInterface::Home) {
            trueType = FacebookInterface::Post;
        }
        // Manage threaded comments
        // TODO: maybe make it an option (not to append
        // threaded comments in the main flow of comments)
        QVariantList threadedComments;
        if (type == FacebookInterface::Comment) {
            QVariantMap threadedCommentsObject = variantMap.value(FACEBOOK_ONTOLOGY_CONNECTIONS_COMMENTS).toMap();
            threadedComments = threadedCommentsObject.value(FACEBOOK_ONTOLOGY_CONNECTIONS_DATA).toList();
            variantMap.remove(FACEBOOK_ONTOLOGY_CONNECTIONS_COMMENTS);
        }

        variantMap.insert(NEMOQMLPLUGINS_SOCIAL_CONTENTITEMTYPE, trueType);
        variantMap.insert(NEMOQMLPLUGINS_SOCIAL_CONTENTITEMID, identifier);
        list.append(createCacheEntry(variantMap, identifier));

        // Manage threaded comments (continued)
        if (!threadedComments.isEmpty()) {
            foreach (const QVariant &comment, threadedComments) {
                QVariantMap commentVariantMap = comment.toMap();
                QString commentIdentifier = commentVariantMap.value(FACEBOOK_ONTOLOGY_METADATA_ID).toString();
                commentVariantMap.insert(NEMOQMLPLUGINS_SOCIAL_CONTENTITEMTYPE, FacebookInterface::Comment);
                commentVariantMap.insert(NEMOQMLPLUGINS_SOCIAL_CONTENTITEMID, commentIdentifier);
                list.append(createCacheEntry(commentVariantMap, commentIdentifier));
            }
        }
    }

    QVariantMap previousExtra;
    QVariantMap nextExtra;
    // Extract paging informations
    // We deliver paging informations to the node extra using the following scheme
    // {
    //     "next": {
    //         "key1": {
    //             "some_value": 12
    //             "have": true
    //         },
    //         "key2": {...}
    //     },
    //     "previous": {...}
    // }
    // Next and previous are the conventionnal way to handle paging cursors,
    // but we need to know if, for a specific type, we need to display the
    // next / previous button. The "have" key is used to display this button.
    //
    // It is usually simple to detect if there is a need of having a next / previous
    // button if Facebook uses cursors. But if it don't use cursors, it is harder.
    //
    // We guess the presence of this button with the following idea:
    // if the next or previous URL that Facebook API provide do not exist then
    // there is no next or previous. If it is provided, there is a next (resp. previous)
    // if and only if we are not appending (resp. prepending) or if we are appending
    // (resp. prepending) and that the data that is retrieved is not empty.
    QVariantMap pagingMap = data.value(FACEBOOK_ONTOLOGY_METADATA_PAGING).toMap();

    // Manage cursors: easy
    if (pagingMap.contains(FACEBOOK_ONTOLOGY_METADATA_PAGING_CURSORS)) {
        QVariantMap cursorsMap = pagingMap.value(FACEBOOK_ONTOLOGY_METADATA_PAGING_CURSORS).toMap();

        // Data for cursors
        if (cursorsMap.contains(FACEBOOK_ONTOLOGY_METADATA_PAGING_CURSORS_AFTER)) {
            nextExtra.insert(FACEBOOK_ONTOLOGY_METADATA_PAGING_CURSORS_AFTER,
                        cursorsMap.value(FACEBOOK_ONTOLOGY_METADATA_PAGING_CURSORS_AFTER).toString());
        }

        if (cursorsMap.contains(FACEBOOK_ONTOLOGY_METADATA_PAGING_CURSORS_BEFORE)) {
            previousExtra.insert(FACEBOOK_ONTOLOGY_METADATA_PAGING_CURSORS_BEFORE,
                                 cursorsMap.value(FACEBOOK_ONTOLOGY_METADATA_PAGING_CURSORS_BEFORE).toString());
        }
    } else {
        // If not, we should parse the next and previous url and extract relevant data
        QUrl previousUrl = QUrl(pagingMap.value(FACEBOOK_ONTOLOGY_METADATA_PAGING_PREVIOUS).toString());

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
        QList<QPair<QString, QString> > previousQueries = previousUrl.queryItems();
#else
        QUrlQuery query (previousUrl);
        QList<QPair<QString, QString> > previousQueries = query.queryItems();
#endif
        QList<QPair<QString, QString> >::const_iterator i;

        for (i = previousQueries.begin(); i != previousQueries.end(); ++ i) {
            if (i->first == FACEBOOK_ONTOLOGY_METADATA_PAGING_OFFSET
                || i->first == FACEBOOK_ONTOLOGY_METADATA_PAGING_SINCE) {
                previousExtra.insert(i->first, i->second);
            }
        }

        QUrl nextUrl = QUrl(pagingMap.value(FACEBOOK_ONTOLOGY_METADATA_PAGING_NEXT).toString());
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
        QList<QPair<QString, QString> > nextQueries = nextUrl.queryItems();
#else
        query = QUrlQuery(nextUrl);
        QList<QPair<QString, QString> > nextQueries = query.queryItems();
#endif

        for (i = nextQueries.begin(); i != nextQueries.end(); ++ i) {
            if (i->first == FACEBOOK_ONTOLOGY_METADATA_PAGING_OFFSET
                || i->first == FACEBOOK_ONTOLOGY_METADATA_PAGING_UNTIL) {
                nextExtra.insert(i->first, i->second);
            }
        }

    }

    // If we have cursors, next and previous are easy, except if the
    // type is comments. We need to workaround Facebook comments,
    // since the previous field won't exist in returned replies (sadly)
    bool hasPrevious = false;
    bool hasNext = false;

    if (pagingMap.contains(FACEBOOK_ONTOLOGY_METADATA_PAGING_CURSORS)
        && type != FacebookInterface::Comment) {
        hasPrevious = pagingMap.contains(FACEBOOK_ONTOLOGY_METADATA_PAGING_PREVIOUS);
        hasNext = pagingMap.contains(FACEBOOK_ONTOLOGY_METADATA_PAGING_NEXT);
    } else if (pagingMap.contains(FACEBOOK_ONTOLOGY_METADATA_PAGING_CURSORS)
               && type == FacebookInterface::Comment) {
        // If we have comments, we hack and say that we always have previous and next
        // This is really hacking and TODO
        hasPrevious = true;
        hasNext = true;
    } else {
        // Otherwise, let's guess previous and next in paging
        if (pagingMap.contains(FACEBOOK_ONTOLOGY_METADATA_PAGING_PREVIOUS)) {
            hasPrevious = true;
            if (nodeStatus == CacheNodePrivate::LoadingRelatedDataPrepending && list.isEmpty()) {
                hasPrevious = false;
            }
        }

        if (pagingMap.contains(FACEBOOK_ONTOLOGY_METADATA_PAGING_NEXT)) {
            hasNext = true;
            if (nodeStatus == CacheNodePrivate::LoadingRelatedDataAppending && list.isEmpty()) {
                hasNext = false;
            }
        }
    }

    previousExtra.insert(PAGING_HAVE_KEY, hasPrevious);
    nextExtra.insert(PAGING_HAVE_KEY, hasNext);

    globalPreviousExtra.insert(QString::number(type), previousExtra);
    globalNextExtra.insert(QString::number(type), nextExtra);

    SocialNetworkInterfacePrivate::setNodeExtraPaging(extraInfo, globalPreviousExtra,
                                                      globalNextExtra, nodeStatus);

    return true;
}

QString FacebookInterfacePrivate::createField(int type, const QString &connection,
                                              const RequestFieldsMap &requestFiledsMap)
{
    QString field = connection;
    if (requestFiledsMap.contains(type)) {
        QList<QPair<QString, QString> > fieldValues = requestFiledsMap.values(type);
        QList<QPair<QString, QString> >::Iterator i;
        for (i = fieldValues.begin(); i != fieldValues.end(); i++) {
            QString fieldsExtraEntry = QString(".%1(%2)").arg(i->first, i->second);
            field.append(fieldsExtraEntry);
        }
    }

    switch(type) {
        case FacebookInterface::Album: {
            field.append(".fields(likes.summary(1),comments.summary(1),from,name,description,link,"\
                         "cover_photo,privacy,count,type,created_time,updated_time,can_upload)");
        }
        break;
        case FacebookInterface::Photo: {
            field.append(".fields(from,tags,name,name_tags,icon,picture,source,height,width,"\
                         "images,link,place,created_time,updated_time)");
        }
        break;
        case FacebookInterface::Post:
        case FacebookInterface::Home: {
            field.append(".fields(likes.summary(1),comments.summary(1),from,to,message,"\
                         "message_tags,picture,link,name,caption,description,source,properties,"\
                         "icon,actions,story,story_tags,with_tags,application,created_time,"\
                         "updated_time,shares,status_type)");
        }
        break;
        case FacebookInterface::Comment: {
            // There is a hack to load a very large batch
            // subcomments, in order not to load them using cursors.
            // It should be quite unlikely that we have ~1000 subcomments
            field.append(".fields(from,message,created_time,like_count,user_likes,parent,"\
                         "can_comment,comment_count,comments.count(1000))");
        }
        break;
        default: break;
    }

    return field;
}

QVariantMap FacebookInterfacePrivate::makeCursorExtraData(CacheNodePrivate::Status insertionMode,
                                                          const QVariantMap &oldExtraData,
                                                          const QVariantMap &cursorsMap)
{
    if (insertionMode != CacheNodePrivate::LoadingRelatedDataReplacing
        && insertionMode != CacheNodePrivate::LoadingRelatedDataPrepending
        && insertionMode != CacheNodePrivate::LoadingRelatedDataAppending) {
        return QVariantMap();
    }

    QVariantMap extraData = oldExtraData;
    QVariantMap paging = oldExtraData.value(FACEBOOK_ONTOLOGY_METADATA_PAGING).toMap();

    // In case we are prepending, we should keep the old "next" entry
    if (insertionMode == CacheNodePrivate::LoadingRelatedDataReplacing
        || insertionMode == CacheNodePrivate::LoadingRelatedDataPrepending) {

        QVariantMap previous;
        previous.insert(FACEBOOK_ONTOLOGY_METADATA_PAGING_CURSORS_BEFORE,
                        cursorsMap.value(FACEBOOK_ONTOLOGY_METADATA_PAGING_CURSORS_BEFORE).toString());
        extraData.insert(FACEBOOK_ONTOLOGY_METADATA_PAGING_PREVIOUS, previous);

        paging.insert(FACEBOOK_ONTOLOGY_METADATA_PAGING_PREVIOUS,
                      cursorsMap.contains(FACEBOOK_ONTOLOGY_METADATA_PAGING_CURSORS_BEFORE));
    }

    // In case we are appending, we should keep the old "previous" entry
    if (insertionMode == CacheNodePrivate::LoadingRelatedDataReplacing
        || insertionMode == CacheNodePrivate::LoadingRelatedDataAppending) {

        QVariantMap next;
        next.insert(FACEBOOK_ONTOLOGY_METADATA_PAGING_CURSORS_AFTER,
                        cursorsMap.value(FACEBOOK_ONTOLOGY_METADATA_PAGING_CURSORS_AFTER).toString());
        extraData.insert(FACEBOOK_ONTOLOGY_METADATA_PAGING_NEXT, next);

        paging.insert(FACEBOOK_ONTOLOGY_METADATA_PAGING_NEXT,
                      cursorsMap.contains(FACEBOOK_ONTOLOGY_METADATA_PAGING_CURSORS_AFTER));
    }
    extraData.insert(FACEBOOK_ONTOLOGY_METADATA_PAGING, paging);

    return extraData;
}


//----------------------------------------------------------

/*!
    \qmltype Facebook
    \instantiates FacebookInterface
    \inqmlmodule org.nemomobile.social 1
    \brief Implements the SocialNetwork interface for the Facebook service.

    The Facebook type is an implementation of the SocialNetwork interface
    specifically for the Facebook social network service.
    It provides access to graph objects such as users, albums and photographs,
    and allows operations such as "like" and "comment".

    Clients should provide an \c accessToken to use the adapter.  The access
    token may be retrieved via the org.nemomobile.signon adapters, or from
    another OAuth2 implementation.

    An example of usage is as follows:

    \qml
    import QtQuick 1.1
    import org.nemomobile.social 1.0

    Item {
        id: root

        Facebook {
            id: facebook
            accessToken: "your access token"    // You must supply a valid access token.
        }

        SocialNetworkInterface {
            id: facebookModel
            socialNetwork: facebook
            nodeIdentifier: "10150146071791729" // Some valid Facebook album id.
            filters: [ ContentItemTypeFilter { type: Facebook.Photo } ]
        }

        ListView {
            model: facebookModel
            anchors.fill: parent
            delegate: Label { text: "id: " + contentItemIdentifier }
        }

        Component.onCompleted: {
            facebookModel.populate()
        }
    }
    \endqml

    Another example, showing the manipulation of a single photo

    \qml
    import QtQuick 1.1
    import org.nemomobile.social 1.0

    Item {
        id: root
        Facebook {
            id: facebook
            accessToken: "your access token"    // You must supply a valid access token.
        }

        FacebookPhoto {
            id: facebookPhoto
            socialNetwork: facebook
            identifier: "1234567890"     // Some valid Facebook Photo Facebook id
        }

        Image {
            id: image
            anchors.fill: parent
            source: facebookPhoto.source
            onStatusChanged: {
                if (status == Image.Ready) {
                    // Could upload a comment with:
                    facebookPhoto.uploadComment("I really like this photo!")
                    // Could like the photo with:
                    facebookPhoto.like()
                    // Or unlike it with:
                    facebookPhoto.unlike()
                }
            }
        }
    }
    \endqml

    Note: the Facebook adapter currently only supports the
    \c ContentItemTypeFilter filter type, and does not support
    any form of sorting.
*/

FacebookInterface::FacebookInterface(QObject *parent)
    : SocialNetworkInterface(*(new FacebookInterfacePrivate(this)), parent)
{
}


/*!
    \qmlproperty QString Facebook::accessToken
    The access token to use when accessing the Facebook OpenGraph API.
*/
QString FacebookInterface::accessToken() const
{
    Q_D(const FacebookInterface);
    return d->accessToken;
}

void FacebookInterface::setAccessToken(const QString &token)
{
    Q_D(FacebookInterface);
    if (d->accessToken != token) {
        d->accessToken = token;
        emit accessTokenChanged();
    }
}

/*! \internal */
QString FacebookInterface::currentUserIdentifier() const
{
    Q_D(const FacebookInterface);
    // returns the object identifier associated with the "me" node, if loaded.
    return d->currentUserIdentifier;
}

/*! \internal */
FacebookObjectReferenceInterface *FacebookInterface::objectReference(QObject *parent, int type,
                                                                     QString identifier,
                                                                     QString name)
{
    // constructs a FacebookObjectReference which will be exposed as a Q_PROPERTY
    // of some content item which requires it (eg, significantOther, etc).

    QVariantMap data;
    data.insert(FACEBOOK_ONTOLOGY_OBJECTREFERENCE_OBJECTTYPE, type);
    data.insert(FACEBOOK_ONTOLOGY_OBJECTREFERENCE_OBJECTIDENTIFIER, identifier);
    data.insert(FACEBOOK_ONTOLOGY_OBJECTREFERENCE_OBJECTNAME, name);

    FacebookObjectReferenceInterface *facebookObjectReference
            = new FacebookObjectReferenceInterface(parent);
    facebookObjectReference->classBegin();
    setContentItemData(facebookObjectReference, data);
    facebookObjectReference->componentComplete();
    return facebookObjectReference;
}

/*! \internal */
QVariantMap FacebookInterface::facebookContentItemData(ContentItemInterface *contentItem)
{
    return contentItemData(contentItem);
}

/*! \internal */
void FacebookInterface::setFacebookContentItemData(ContentItemInterface *contentItem,
                                                   const QVariantMap &data)
{
    // TODO: Using FacebookInterface, that have many friends, to call a private method
    // seems to be wrong. There should be a pretty way to change an object, and
    // the best way should be to make the setter public.
    setContentItemData(contentItem, data);
}

#include "moc_facebookinterface.cpp"
