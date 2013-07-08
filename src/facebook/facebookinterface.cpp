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

#include "contentiteminterface.h"
#include "contentiteminterface_p.h"
#include "identifiablecontentiteminterface.h"
#include "contentitemtypefilterinterface.h"

#include "facebookobjectreferenceinterface.h"
#include "facebookalbuminterface.h"
#include "facebookcommentinterface.h"
#include "facebooknotificationinterface.h"
#include "facebookphotointerface.h"
#include "facebookpostinterface.h"
#include "facebookuserinterface.h"

#include "facebooklikeinterface.h"

#include "util_p.h"

#include <QtCore/QVariantMap>
#include <QtCore/QStringList>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QByteArray>
#include <QtNetwork/QNetworkReply>

#include <QtDebug>

#define FACEBOOK_ME QLatin1String("me")

// TODO XXX We need to implement a "metadata" filter, that is used to detect type
// of elements that we don't know. That metadata filter will simply enable &metadata=1
// so that the type detection won't fail.
// Or we can load the data, and if we don't know the type, we could load the metadata=1
// field and then get the info on the entity

QVariant FACEBOOK_DEBUG_VALUE_STRING_FROM_DATA(const QString &key, const QVariantMap &data)
{
    QVariant variant = data.value(key);
    if (variant.type() == QVariant::List)
        return variant.toStringList();
    else if (variant.type() != QVariant::Map)
        return variant;
    QVariant dataVal = variant.toMap().value(FACEBOOK_ONTOLOGY_CONNECTIONS_DATA);
    if (dataVal.type() == QVariant::List)
        return QVariant(QString(QLatin1String("%1 data entries")).arg(dataVal.toList().count()));
    if (dataVal.type() == QVariant::Map)
        return QVariant(QString(QLatin1String("... some other map")));
    return dataVal;
}

FacebookInterfacePrivate::FacebookInterfacePrivate(FacebookInterface *q)
    : SocialNetworkInterfacePrivate(q)
    , currentUserIdentifier(FACEBOOK_ME)
    , prependingRelatedData(false)
    , appendingRelatedData(false)
    , internalStatus(Idle)
    , currentReply(0)
{
}

/*!
    \internal

    Attempt to detect which type of object the given \a data represents.
    This function makes use of some fairly inexact heuristics, and only
    supports comments, albums, photos, and users.
*/
int FacebookInterfacePrivate::detectTypeFromData(const QVariantMap &data) const
{
    // attempt to detect the type directly from the Facebook metadata field
    if (data.contains(FACEBOOK_ONTOLOGY_METADATA)
            && data.value(FACEBOOK_ONTOLOGY_METADATA).type() == QVariant::Map) {

        QVariantMap metadata = data.value(FACEBOOK_ONTOLOGY_METADATA).toMap();
        if (!metadata.value(FACEBOOK_ONTOLOGY_METADATA_TYPE).toString().isEmpty()) {

            QString typeStr = metadata.value(FACEBOOK_ONTOLOGY_METADATA_TYPE).toString();
            if (typeStr == FACEBOOK_ONTOLOGY_USER)
                return FacebookInterface::User;
            else if (typeStr == FACEBOOK_ONTOLOGY_ALBUM)
                return FacebookInterface::Album;
            else if (typeStr == FACEBOOK_ONTOLOGY_COMMENT)
                return FacebookInterface::Comment;
            else if (typeStr == FACEBOOK_ONTOLOGY_PHOTO)
                return FacebookInterface::Photo;
            else if (typeStr == FACEBOOK_ONTOLOGY_POST)
                return FacebookInterface::Post;
            qWarning() << Q_FUNC_INFO << "Unsupported type:" << typeStr;
            return FacebookInterface::Unknown;
        }
    }

    // it's possible that we've already tagged the type already
    if (data.contains(NEMOQMLPLUGINS_SOCIAL_CONTENTITEMTYPE)) {
        FacebookInterface::ContentItemType taggedType = static_cast<FacebookInterface::ContentItemType>(data.value(NEMOQMLPLUGINS_SOCIAL_CONTENTITEMTYPE).toInt());
        if (taggedType > FacebookInterface::Unknown) {
            return taggedType;
        }
    }

    qWarning() << Q_FUNC_INFO << "No type information available in object metadata - "\
                                 "attempting to heuristically detect type";
    if (data.value(FACEBOOK_ONTOLOGY_COMMENT_MESSAGE).isValid()
        && data.value(FACEBOOK_ONTOLOGY_COMMENT_LIKECOUNT).isValid())
        return FacebookInterface::Comment;
    else if (data.value(FACEBOOK_ONTOLOGY_ALBUM_PRIVACY).isValid()
             && data.value(FACEBOOK_ONTOLOGY_ALBUM_CANUPLOAD).isValid())
        return FacebookInterface::Album;
    else if (data.value(FACEBOOK_ONTOLOGY_PHOTO_WIDTH).isValid()
             && data.value(FACEBOOK_ONTOLOGY_PHOTO_SOURCE).isValid())
        return FacebookInterface::Photo;
    else if (data.value(FACEBOOK_ONTOLOGY_USER_FIRSTNAME).isValid()
             || data.value(FACEBOOK_ONTOLOGY_USER_GENDER).isValid())
        return FacebookInterface::User;
    else if (data.value(FACEBOOK_ONTOLOGY_POST_ACTIONS).isValid() && data.value(FACEBOOK_ONTOLOGY_POST_POSTTYPE).isValid())
        return FacebookInterface::Post;

    qWarning() << Q_FUNC_INFO << "Unable to heuristically detect type!";
    foreach (const QString &datakey, data.keys())
        qWarning() << "        " << datakey << " = "
                   << FACEBOOK_DEBUG_VALUE_STRING_FROM_DATA(datakey, data);
    return FacebookInterface::Unknown;
}

void FacebookInterfacePrivate::populateRelatedDataForLastNode(const QVariantMap &relatedData,
                                                              const QUrl &requestUrl)
{
    // We receive the related data and transform it into ContentItems.
    // Finally, we populate the cache for the node and update the internal model data.
    QList<CacheEntry> relatedContent;

    QString requestPath = requestUrl.path();
    bool ok = false;
    QVariantMap nodeExtra;
    ok = ok || tryAddCacheEntryFromData(relatedData, requestPath, FacebookInterface::Like,
                                        FACEBOOK_ONTOLOGY_CONNECTIONS_LIKES,
                                        relatedContent, nodeExtra);
    ok = ok || tryAddCacheEntryFromData(relatedData, requestPath, FacebookInterface::Comment,
                                        FACEBOOK_ONTOLOGY_CONNECTIONS_COMMENTS,
                                        relatedContent, nodeExtra);
    ok = ok || tryAddCacheEntryFromData(relatedData, requestPath, FacebookInterface::PhotoTag,
                                        FACEBOOK_ONTOLOGY_CONNECTIONS_TAGS,
                                        relatedContent, nodeExtra);
    ok = ok || tryAddCacheEntryFromData(relatedData, requestPath, FacebookInterface::Photo,
                                        FACEBOOK_ONTOLOGY_CONNECTIONS_PHOTOS,
                                        relatedContent, nodeExtra);
    ok = ok || tryAddCacheEntryFromData(relatedData, requestPath, FacebookInterface::Album,
                                        FACEBOOK_ONTOLOGY_CONNECTIONS_ALBUMS,
                                        relatedContent, nodeExtra);
    ok = ok || tryAddCacheEntryFromData(relatedData, requestPath, FacebookInterface::User,
                                        FACEBOOK_ONTOLOGY_CONNECTIONS_FRIENDS,
                                        relatedContent, nodeExtra);
    ok = ok || tryAddCacheEntryFromData(relatedData, requestPath, FacebookInterface::Notification,
                                        FACEBOOK_ONTOLOGY_CONNECTIONS_NOTIFICATIONS,
                                        relatedContent, nodeExtra);
    ok = ok || tryAddCacheEntryFromData(relatedData, requestPath, FacebookInterface::Post,
                                        FACEBOOK_ONTOLOGY_CONNECTIONS_FEED,
                                        relatedContent, nodeExtra);
    if (!ok) {
        // If we have an empty list, we will only obtain a related data with "id"
        // so that's ok. If the count is > 1, we should be more careful.
        if (relatedData.keys().count() > 1) {
            qWarning() << Q_FUNC_INFO << "Unsupported data retrieved";
            qWarning() << Q_FUNC_INFO << "Request url:" << requestUrl;
            qWarning() << Q_FUNC_INFO << "List of keys: " << relatedData.keys();
        }
    }

    lastNode().setExtraInfo(nodeExtra);

    bool havePreviousRelatedData = false;
    bool haveNextRelatedData = false;

    foreach (QString key, nodeExtra.keys()) {
        QVariantMap paging = nodeExtra.value(key).toMap().value(FACEBOOK_ONTOLOGY_METADATA_PAGING).toMap();
        havePreviousRelatedData = havePreviousRelatedData
                                  || paging.value(FACEBOOK_ONTOLOGY_METADATA_PAGING_PREVIOUS).toBool();
        haveNextRelatedData = haveNextRelatedData
                              || paging.value(FACEBOOK_ONTOLOGY_METADATA_PAGING_NEXT).toBool();
    }

    RelatedDataPagingFlags relatedDataPagingFlags = RelatedDataPagingFlags();
    bool replacing = !prependingRelatedData && !appendingRelatedData;

    if (havePreviousRelatedData && (prependingRelatedData || replacing)) {
        relatedDataPagingFlags = relatedDataPagingFlags | HavePreviousRelatedDataFlag;
    }
    if (haveNextRelatedData && (appendingRelatedData || replacing)) {
        relatedDataPagingFlags = relatedDataPagingFlags | HaveNextRelatedDataFlag;
    }


    FacebookInterfacePrivate::UpdateMode updateMode = FacebookInterfacePrivate::Replace;
    if (prependingRelatedData) {
        updateMode = FacebookInterfacePrivate::Prepend;
    }
    if (appendingRelatedData) {
        updateMode = FacebookInterfacePrivate::Append;
    }

    prependingRelatedData = false;
    appendingRelatedData = false;

    insertContent(relatedContent, relatedDataPagingFlags, updateMode);
    internalStatus = Idle;
    setStatus(SocialNetworkInterface::Idle);

    if (atLastNode()) {
        updateRelatedData();
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
    foreach (const QString &key, extraDataKeys)
        queryItems.append(qMakePair<QString, QString>(key, extraData.value(key).toString()));

    QUrl returnedUrl;
    returnedUrl.setScheme("https");
    returnedUrl.setHost("graph.facebook.com");
    if (extraPath.isEmpty())
        returnedUrl.setPath(QLatin1String("/") + objectId);
    else
        returnedUrl.setPath(QLatin1String("/") + objectId + QLatin1String("/") + extraPath);
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
QNetworkReply *FacebookInterfacePrivate::uploadImage(const QString &objectId,
                                                     const QString &extraPath,
                                                     const QVariantMap &data,
                                                     const QVariantMap &extraData)
{
    Q_Q(FacebookInterface);
    Q_UNUSED(extraData); // XXX TODO: privacy passed via extraData?

    // the implementation code for this function is taken from the transfer engine
    QNetworkRequest request;
    QUrl url("https://graph.facebook.com");
    QString path = objectId;
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

void FacebookInterfacePrivate::setCurrentReply(QNetworkReply *newCurrentReply, const QString &whichNodeIdentifier)
{
    deleteReply();
    newCurrentReply->setProperty("whichNodeIdentifier", whichNodeIdentifier);
    currentReply = newCurrentReply;
}

void FacebookInterfacePrivate::connectFinishedAndErrors()
{
    Q_Q(FacebookInterface);
    QObject::connect(currentReply, SIGNAL(finished()), q, SLOT(finishedHandler()));
    QObject::connect(currentReply, SIGNAL(error(QNetworkReply::NetworkError)),
                     q, SLOT(errorHandler(QNetworkReply::NetworkError)));
    QObject::connect(currentReply, SIGNAL(sslErrors(QList<QSslError>)),
                     q, SLOT(sslErrorsHandler(QList<QSslError>)));
}

/*! \internal */
void FacebookInterfacePrivate::updateCurrentUserIdentifierHandler(bool isError,
                                                                  const QVariantMap &data)
{
    Q_Q(FacebookInterface);
    QObject::disconnect(q, SIGNAL(arbitraryRequestResponseReceived(bool,QVariantMap)),
                        q, SLOT(updateCurrentUserIdentifierHandler(bool,QVariantMap)));
    if (isError) {
        return;
    }

    if (data.contains(FACEBOOK_ONTOLOGY_OBJECTREFERENCE_OBJECTIDENTIFIER)) {
        QVariant id = data.value(FACEBOOK_ONTOLOGY_OBJECTREFERENCE_OBJECTIDENTIFIER);
        if (currentUserIdentifier != id.toString()) {
            currentUserIdentifier = id.toString();
            emit q->currentUserIdentifierChanged();
        }
    }
}

/*! \internal */
void FacebookInterfacePrivate::finishedHandler()
{
    Q_Q(FacebookInterface);
    if (!currentReply) {
        // if an error occurred, it might have been deleted by the error handler.
        qWarning() << Q_FUNC_INFO << "network request finished but no reply!";
        return;
    }
    // TODO: remove the use of property, use the stack instead, it should contain all the info
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
    deleteReply();
    bool ok = false;
    QVariantMap responseData = ContentItemInterfacePrivate::parseReplyData(replyData, &ok);
    if (!ok) {
        responseData.insert("response", replyData);
        setError(SocialNetworkInterface::RequestError,
                 QLatin1String("Error populating node: response is invalid. "\
                               "Perhaps the requested object id was incorrect?  Response: ")
                 + QString::fromLatin1(replyData.constData()));
        setStatus(SocialNetworkInterface::Error);
        internalStatus = Idle;
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

        setError(SocialNetworkInterface::RequestError,
                 QLatin1String("Error populating node: response is error.  Response: ")
                 + QString::fromLatin1(replyData.constData()));
        setStatus(SocialNetworkInterface::Error);
        internalStatus = Idle;
        return;
    }

    switch (internalStatus) {
    case PopulatingNodeData:
        {
            // TODO: some objects might need a second populating step
            // Create a cache entry associated to the retrieved data
            QString identifier;
            if (responseData.contains(FACEBOOK_ONTOLOGY_OBJECTREFERENCE_OBJECTIDENTIFIER)) {
                identifier = responseData.value(FACEBOOK_ONTOLOGY_OBJECTREFERENCE_OBJECTIDENTIFIER).toString();
            }
            CacheEntry cacheEntry = createCacheEntry(responseData, identifier);
            setLastNodeCacheEntry(cacheEntry);
            if (atLastNode()) {
                updateNode();
            }
            q->populateRelatedDataforLastNode();
        }
        break;
    case PopulatingNodeRelatedData:
        populateRelatedDataForLastNode(responseData, requestUrl);
        if (atLastNode()) {
            updateRelatedData();
        }
        break;

    default:
        break;
    }
}

/*! \internal */
void FacebookInterfacePrivate::errorHandler(QNetworkReply::NetworkError networkError)
{
    Q_Q(FacebookInterface);
    if (networkError == QNetworkReply::UnknownContentError) {
        // ignore this.  It's not actually an error, Facebook just formats some responses strangely.
        return;
    }
    setError(SocialNetworkInterface::RequestError, networkErrorString(networkError));
    setStatus(SocialNetworkInterface::Error);
    internalStatus = Idle;

    qWarning() << Q_FUNC_INFO << "Error: network error occurred:"
               << networkError << ":" << q->errorMessage();

}

/*! \internal */
void FacebookInterfacePrivate::sslErrorsHandler(const QList<QSslError> &sslErrors)
{
    QString newErrorMessage = QLatin1String("SSL error: ");
    if (sslErrors.isEmpty()) {
        newErrorMessage += QLatin1String("unknown SSL error");
    } else {
        foreach (const QSslError &error, sslErrors)
            newErrorMessage += error.errorString() + QLatin1String("; ");
        newErrorMessage.chop(2);
    }

    setStatus(SocialNetworkInterface::Error);
    setError(SocialNetworkInterface::RequestError, newErrorMessage);
}

/*! \internal */
void FacebookInterfacePrivate::deleteReply()
{
    if (currentReply) {
        currentReply->disconnect();
        currentReply->deleteLater();
        currentReply = 0;
    }
}

bool FacebookInterfacePrivate::tryAddCacheEntryFromData(const QVariantMap &relatedData,
                                                        const QString &requestPath,
                                                        int type,
                                                        const QString &typeName,
                                                        QList<CacheEntry> &list,
                                                        QVariantMap &nodeExtra)
{

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
        return false;
    }

    QVariantList itemsList = data.value(FACEBOOK_ONTOLOGY_CONNECTIONS_DATA).toList();
    foreach (const QVariant &variant, itemsList) {
        QVariantMap variantMap = variant.toMap();
        QString identifier = variantMap.value(FACEBOOK_ONTOLOGY_METADATA_ID).toString();
        // If the type is a "Like", it is not identifiable:
        if (type == FacebookInterface::Like) {
            identifier.clear();
        }
        variantMap.insert(NEMOQMLPLUGINS_SOCIAL_CONTENTITEMTYPE, type);
        variantMap.insert(NEMOQMLPLUGINS_SOCIAL_CONTENTITEMID, identifier);
        list.append(createCacheEntry(variantMap, identifier));
    }


    // Extract paging informations
    // We deliver paging informations to the node extra using the following scheme
    // {
    //     "next": {
    //         "key1": "value1", "key2": "value2"
    //     },
    //     "previous": {...},
    //     "paging": {
    //         "next": true,
    //         "previous": false
    //     }
    // }
    // The key-values pairs in "next" and "previous" are used to store the query items
    // used in the URL for next and previous.
    // The paging field is used to indicate if there is a next or previous page.
    // If they are not present, they are guessed using the following pattern:
    // If the next or previous URL that Facebook API provide do not exist then
    // there is no next or previous. If it is provided, there is a next (resp. previous)
    // if and only if we are not appending (resp. prepending) or if we are appending
    // (resp. prepending) and that the data that is retrieved is not empty.
    if (!nodeExtra.contains(QString::number(type))) {
        QVariantMap pagingMap = data.value(FACEBOOK_ONTOLOGY_METADATA_PAGING).toMap();
        if (pagingMap.contains(FACEBOOK_ONTOLOGY_METADATA_PAGING_CURSORS)) {
            QVariantMap cursorsMap
                    = pagingMap.value(FACEBOOK_ONTOLOGY_METADATA_PAGING_CURSORS).toMap();

            // If it is using cursors, we should build a nice cursor query
            QVariantMap extraData;
            QVariantMap paging;
            paging.insert(FACEBOOK_ONTOLOGY_METADATA_PAGING_PREVIOUS,
                          pagingMap.contains(FACEBOOK_ONTOLOGY_METADATA_PAGING_PREVIOUS));
            paging.insert(FACEBOOK_ONTOLOGY_METADATA_PAGING_NEXT,
                          pagingMap.contains(FACEBOOK_ONTOLOGY_METADATA_PAGING_NEXT));
            extraData.insert(FACEBOOK_ONTOLOGY_METADATA_PAGING, paging);

            if (cursorsMap.contains(FACEBOOK_ONTOLOGY_METADATA_PAGING_CURSORS_AFTER)) {
                QVariantMap next;
                next.insert(FACEBOOK_ONTOLOGY_METADATA_PAGING_CURSORS_AFTER,
                            cursorsMap.value(FACEBOOK_ONTOLOGY_METADATA_PAGING_CURSORS_AFTER).toString());
                extraData.insert(FACEBOOK_ONTOLOGY_METADATA_PAGING_NEXT, next);
            }

            if (cursorsMap.contains(FACEBOOK_ONTOLOGY_METADATA_PAGING_CURSORS_BEFORE)) {
                QVariantMap previous;
                previous.insert(FACEBOOK_ONTOLOGY_METADATA_PAGING_CURSORS_BEFORE,
                                cursorsMap.value(FACEBOOK_ONTOLOGY_METADATA_PAGING_CURSORS_BEFORE).toString());
                extraData.insert(FACEBOOK_ONTOLOGY_METADATA_PAGING_PREVIOUS, previous);
            }
            nodeExtra.insert(QString::number(type), extraData);
        } else {
            // If not, we should parse the next and previous url and extract relevant data
            QVariantMap extraData;
            QUrl previousUrl = QUrl(pagingMap.value(FACEBOOK_ONTOLOGY_METADATA_PAGING_PREVIOUS).toString());

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
            QList<QPair<QString, QString> > previousQueries = previousUrl.queryItems();
#else
            QUrlQuery query (previousUrl);
            QList<QPair<QString, QString> > previousQueries = query.queryItems();
#endif
            QList<QPair<QString, QString> >::const_iterator i;

            QVariantMap previous;
            for (i = previousQueries.begin(); i != previousQueries.end(); ++ i) {
                if (i->first == FACEBOOK_ONTOLOGY_METADATA_PAGING_OFFSET
                    || i->first == FACEBOOK_ONTOLOGY_METADATA_PAGING_SINCE) {
                    previous.insert(i->first, i->second);
                }
            }
            if (!previous.isEmpty()) {
                extraData.insert(FACEBOOK_ONTOLOGY_METADATA_PAGING_PREVIOUS, previous);
            }


            QUrl nextUrl = QUrl(pagingMap.value(FACEBOOK_ONTOLOGY_METADATA_PAGING_NEXT).toString());
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
            QList<QPair<QString, QString> > nextQueries = nextUrl.queryItems();
#else
            query = QUrlQuery(nextUrl);
            QList<QPair<QString, QString> > nextQueries = query.queryItems();
#endif

            QVariantMap next;
            for (i = nextQueries.begin(); i != nextQueries.end(); ++ i) {
                if (i->first == FACEBOOK_ONTOLOGY_METADATA_PAGING_OFFSET
                    || i->first == FACEBOOK_ONTOLOGY_METADATA_PAGING_UNTIL) {
                    next.insert(i->first, i->second);
                }
            }
            if (!next.isEmpty()) {
                extraData.insert(FACEBOOK_ONTOLOGY_METADATA_PAGING_NEXT, next);
            }

            // Let's guess previous and next in paging
            bool hasPrevious = false;
            bool hasNext = false;

            if (pagingMap.contains(FACEBOOK_ONTOLOGY_METADATA_PAGING_PREVIOUS)) {
                hasPrevious = true;
                if (prependingRelatedData && list.isEmpty()) {
                    hasPrevious = false;
                }
            }

            if (pagingMap.contains(FACEBOOK_ONTOLOGY_METADATA_PAGING_NEXT)) {
                hasNext = true;
                if (prependingRelatedData && list.isEmpty()) {
                    hasNext = false;
                }
            }

            QVariantMap paging;
            paging.insert(FACEBOOK_ONTOLOGY_METADATA_PAGING_PREVIOUS, hasPrevious);
            paging.insert(FACEBOOK_ONTOLOGY_METADATA_PAGING_NEXT, hasNext);

            extraData.insert(FACEBOOK_ONTOLOGY_METADATA_PAGING, paging);


            nodeExtra.insert(QString::number(type), extraData);
        }
    }

    return true;
}

bool FacebookInterfacePrivate::performRelatedDataRequest(const QString &identifier,
                                                         const QList<FilterInterface *> &filters,
                                                         const RequestFieldsMap &extra)
{

    Q_Q(FacebookInterface);
    if (filters.isEmpty()) {
        // An empty filter should not create an error
        // Actually, nothing should be loaded in the node and
        // the request should be terminated
        internalStatus = Idle;
        setStatus(SocialNetworkInterface::Idle);
        return true;
    }

    // Notifications is a tough one. It cannot be queried via fields expansion
    // so if notifications is passed in the list of filters, it is ignored if
    // there are other filters, and queried only if it is the only one.

    // Extra should map the type to some extra fields to add. These fields will
    // be added to field expansion (or, in case of notifications, to normal
    // graph query)

    QList<int> connectionTypes;
    RequestFieldsMap connectionFields;


    foreach (FilterInterface *filter, filters) {
        ContentItemTypeFilterInterface *contentItemTypeFilter
                = qobject_cast<ContentItemTypeFilterInterface*>(filter);
        if (!contentItemTypeFilter) {
            qWarning() << Q_FUNC_INFO
                       << "The Facebook adapter only supports ContentItemType filters";
            continue;
        }

        // We ignore also all redundant filters
        int type = contentItemTypeFilter->type();
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

            if (extra.contains(type)) {
                QList<QPair<QString, QString> > extraValues = extra.values(type);
                QList<QPair<QString, QString> >::Iterator i;
                for (i = extraValues.begin(); i != extraValues.end(); i++) {
                    connectionFields.insert(type, *i);
                }
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

            setCurrentReply(q->getRequest(identifier, FACEBOOK_ONTOLOGY_CONNECTIONS_NOTIFICATIONS,
                                          QStringList(), extraData), identifier);
            connectFinishedAndErrors();
            internalStatus = PopulatingNodeRelatedData;
            return true;
        } else {
            qWarning() << Q_FUNC_INFO << "Notifications should not be requested with other filters";
            connectionTypes.removeAll(FacebookInterface::Notification);
            connectionFields.remove(FacebookInterface::Notification);
        }
    }

    // generate appropriate query string, using the Field Expansion query syntax of the Facebook OpenGraph API
    // eg: with currentNode = Photo; connectionTypes == comments,likes,tags; whichFields = id,name; limit = 10:
    // GET https://graph.facebook.com/<photo_id>/fields=comments.limit(10).fields(id,name),likes.limit(10).fields(id,name),tags.fields(id,name).limit(10)

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
        case FacebookInterface::Like:
            fields.append(createField(typeInteger, FACEBOOK_ONTOLOGY_CONNECTIONS_LIKES,
                                      connectionFields));
            break;
        case FacebookInterface::PhotoTag:
            fields.append(createField(typeInteger, FACEBOOK_ONTOLOGY_CONNECTIONS_TAGS,
                                      connectionFields));
            break;
        case FacebookInterface::UserPicture:
            fields.append(createField(typeInteger, FACEBOOK_ONTOLOGY_CONNECTIONS_PICTURE,
                                      connectionFields));
            break;
        case FacebookInterface::Location:
            fields.append(createField(typeInteger, FACEBOOK_ONTOLOGY_CONNECTIONS_LOCATIONS,
                                      connectionFields));
            break; // not supported?
        case FacebookInterface::Comment:
            fields.append(createField(typeInteger, FACEBOOK_ONTOLOGY_CONNECTIONS_COMMENTS,
                                      connectionFields));
            break;
        case FacebookInterface::User:
            fields.append(createField(typeInteger, FACEBOOK_ONTOLOGY_CONNECTIONS_FRIENDS,
                                      connectionFields));
            break; // subscriptions etc?
        case FacebookInterface::Album:
            fields.append(createField(typeInteger, FACEBOOK_ONTOLOGY_CONNECTIONS_ALBUMS,
                                      connectionFields));
            break;
        case FacebookInterface::Photo:
            fields.append(createField(typeInteger, FACEBOOK_ONTOLOGY_CONNECTIONS_PHOTOS,
                                      connectionFields));
            break;
        case FacebookInterface::Event:
            fields.append(createField(typeInteger, FACEBOOK_ONTOLOGY_CONNECTIONS_EVENTS,
                                      connectionFields));
            break;
        case FacebookInterface::Post:
            fields.append(createField(typeInteger, FACEBOOK_ONTOLOGY_CONNECTIONS_FEED,
                                      connectionFields));
            break;
        default:
            qWarning() << Q_FUNC_INFO << "Invalid content item type specified in filter";
            break;
        }
    }

    setCurrentReply(q->getRequest(identifier, QString(), fields, QVariantMap()), identifier);
    connectFinishedAndErrors();
    internalStatus = PopulatingNodeRelatedData;
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
    return field;
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
                delegate: Label { text: "id: " + contentItemIdentifier }
            }
        }

        Facebook {
            id: fb
            accessToken: "your access token"    // you must supply a valid access token
            nodeIdentifier: "10150146071791729" // some valid Facebook album id.
            filters: [ ContentItemTypeFilter { type: Facebook.Photo } ]
        }

        FacebookPhoto {
            id: fbph
            socialNetwork: fb
            identifier: "10150146071966729"     // some valid Facebook Photo fbid
        }

        Image {
            id: img
            anchors.top: parent.top
            anchors.bottom: parent.verticalCenter
            anchors.left: parent.left
            anchors.right: parent.right
            source: fbph.source
            onStatusChanged: {
                if (status == Image.Ready) {
                    // could upload a comment with:
                    fbph.uploadComment("I really like this photo!")
                    // could like the photo with:
                    fbph.like()
                    // or unlike it with:
                    fbph.unlike()
                }
            }
        }

        Component.onCompleted: {
            fb.populate()
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

/*! \reimp */
QNetworkReply *FacebookInterface::getRequest(const QString &objectIdentifier,
                                             const QString &extraPath,
                                             const QStringList &whichFields,
                                             const QVariantMap &extraData)
{
    Q_D(FacebookInterface);
    if (!isInitialized()) {
        qWarning() << Q_FUNC_INFO << "cannot complete get request: not initialized";
        return 0;
    }

    QVariantMap modifiedExtraData = extraData;
    QUrl geturl = d->requestUrl(objectIdentifier, extraPath, whichFields, modifiedExtraData);
    return d->networkAccessManager->get(QNetworkRequest(geturl));
}

/*! \reimp */
QNetworkReply *FacebookInterface::postRequest(const QString &objectIdentifier,
                                              const QString &extraPath, const QVariantMap &data,
                                              const QVariantMap &extraData)
{
    Q_D(FacebookInterface);
    if (!isInitialized()) {
        qWarning() << Q_FUNC_INFO << "cannot complete post request: not initialized";
        return 0;
    }

    // image upload is handled specially.
    if (extraData.value("isImageUpload").toBool())
        return d->uploadImage(objectIdentifier, extraPath, data, extraData);
    
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
    request.setRawHeader("Content-Type",QString("multipart/form-data; boundary="+multipartBoundary).toLatin1());
    request.setHeader(QNetworkRequest::ContentLengthHeader, postData.size());

    // perform POST request
    return d->networkAccessManager->post(request, postData);
}

/*! \reimp */
QNetworkReply *FacebookInterface::deleteRequest(const QString &objectIdentifier,
                                                const QString &extraPath,
                                                const QVariantMap &extraData)
{
    Q_D(FacebookInterface);
    if (!isInitialized()) {
        qWarning() << Q_FUNC_INFO << "cannot complete delete request: not initialized";
        return 0;
    }

    return d->networkAccessManager->deleteResource(QNetworkRequest(d->requestUrl(objectIdentifier,
                                                                                 extraPath,
                                                                                 QStringList(),
                                                                                 extraData)));
}

/*! \reimp */
QString FacebookInterface::dataSection(int type, const QVariantMap &data) const
{
    switch (type) {
    case User:
        return data.value(FACEBOOK_ONTOLOGY_USER_NAME).toString();
    default:
        break;
    }
    return SocialNetworkInterface::dataSection(type, data);
}

/*! \internal */
QString FacebookInterface::currentUserIdentifier() const
{
    Q_D(const FacebookInterface);
    // returns the object identifier associated with the "me" node, if loaded.
    return d->currentUserIdentifier;
}

void FacebookInterface::loadNextRelatedData()
{
    Q_D(FacebookInterface);
    switch (status()) {
    case Initializing:
        qWarning() << Q_FUNC_INFO << "Cannot load next: not initialized";
        return;
    case Busy:
        qWarning() << Q_FUNC_INFO << "Cannot load next: another action is already running";
        return;
    case Invalid:
        qWarning() << Q_FUNC_INFO
                   << "The social network is not in a valid state, no operation can be complete";
        return;
    default:
        break;
    }

    if (!hasNextRelatedData()) {
        return;
    }

    // Build the request field map
    RequestFieldsMap requestFieldsMap;
    QVariantMap extraMap = d->lastNode().extraInfo();

    foreach (QString key, extraMap.keys()) {
        int keyInteger = key.toInt();
        QVariantMap entries
                = extraMap.value(key).toMap().value(FACEBOOK_ONTOLOGY_METADATA_PAGING_NEXT).toMap();

        foreach(QString entryKey, entries.keys()) {
            StringPair pair (entryKey, entries.value(entryKey).toString());
            requestFieldsMap.insert(keyInteger, pair);
        }
    }

    d->appendingRelatedData = true;
    if (!d->performRelatedDataRequest(d->lastNode().identifier(), d->lastNode().filters().toList(),
                                      requestFieldsMap)) {
        d->setError(SocialNetworkInterface::DataUpdateError,
                    QLatin1String("Cannot perform related data request"));
    }
}

void FacebookInterface::loadPreviousRelatedData()
{
    Q_D(FacebookInterface);
    switch (status()) {
    case Initializing:
        qWarning() << Q_FUNC_INFO << "Cannot load next: not initialized";
        return;
    case Busy:
        qWarning() << Q_FUNC_INFO << "Cannot load next: another action is already running";
        return;
    case Invalid:
        qWarning() << Q_FUNC_INFO
                   << "The social network is not in a valid state, no operation can be complete";
        return;
    default:
        break;
    }

    if (!hasPreviousRelatedData()) {
        return;
    }

    // Build the request field map
    RequestFieldsMap requestFieldsMap;
    QVariantMap extraMap = d->lastNode().extraInfo();

    foreach (QString key, extraMap.keys()) {
        int keyInteger = key.toInt();
        QVariantMap entries
                = extraMap.value(key).toMap().value(FACEBOOK_ONTOLOGY_METADATA_PAGING_PREVIOUS).toMap();

        foreach(QString entryKey, entries.keys()) {
            StringPair pair (entryKey, entries.value(entryKey).toString());
            requestFieldsMap.insert(keyInteger, pair);
        }
    }

    d->prependingRelatedData = true;
    if (!d->performRelatedDataRequest(d->lastNode().identifier(), d->lastNode().filters().toList(),
                                      requestFieldsMap)) {
        d->setError(SocialNetworkInterface::DataUpdateError,
                    QLatin1String("Cannot perform related data request"));
    }
}

/*! \reimp */
ContentItemInterface *FacebookInterface::contentItemFromData(QObject *parent, const QVariantMap &data) const
{
    Q_D(const FacebookInterface);
    // Construct the appropriate FacebookWhateverInterface for the given data.
    FacebookInterface::ContentItemType detectedType = static_cast<FacebookInterface::ContentItemType>(d->detectTypeFromData(data));
    switch (detectedType) {
        case FacebookInterface::Like: {
            FacebookLikeInterface *returnedInterface = new FacebookLikeInterface(parent);
            returnedInterface->classBegin();
            returnedInterface->setSocialNetwork(const_cast<FacebookInterface*>(this));
            setContentItemData(returnedInterface, data);
            returnedInterface->componentComplete();
            return returnedInterface;
        }
        break;

        case FacebookInterface::Comment: {
            FacebookCommentInterface *returnedInterface = new FacebookCommentInterface(parent);
            returnedInterface->classBegin();
            returnedInterface->setSocialNetwork(const_cast<FacebookInterface*>(this));
            setContentItemData(returnedInterface, data);
            returnedInterface->componentComplete();
            return returnedInterface;
        }
        break;

        case FacebookInterface::Photo: {
            FacebookPhotoInterface *returnedInterface = new FacebookPhotoInterface(parent);
            returnedInterface->classBegin();
            returnedInterface->setSocialNetwork(const_cast<FacebookInterface*>(this));
            setContentItemData(returnedInterface, data);
            returnedInterface->componentComplete();
            return returnedInterface;
        }
        break;

        case FacebookInterface::Album: {
            FacebookAlbumInterface *returnedInterface = new FacebookAlbumInterface(parent);
            returnedInterface->classBegin();
            returnedInterface->setSocialNetwork(const_cast<FacebookInterface*>(this));
            setContentItemData(returnedInterface, data);
            returnedInterface->componentComplete();
            return returnedInterface;
        }
        break;

        case FacebookInterface::User: {
            FacebookUserInterface *returnedInterface = new FacebookUserInterface(parent);
            returnedInterface->classBegin();
            returnedInterface->setSocialNetwork(const_cast<FacebookInterface*>(this));
            setContentItemData(returnedInterface, data);
            returnedInterface->componentComplete();
            return returnedInterface;
        }
        break;

        case FacebookInterface::Notification: {
            FacebookNotificationInterface *returnedInterface
                    = new FacebookNotificationInterface(parent);
            returnedInterface->classBegin();
            returnedInterface->setSocialNetwork(const_cast<FacebookInterface*>(this));
            setContentItemData(returnedInterface, data);
            returnedInterface->componentComplete();
            return returnedInterface;
        }
        break;

        case FacebookInterface::Post: {
            FacebookPostInterface *retn = new FacebookPostInterface(parent);
            retn->classBegin();
            retn->setSocialNetwork(const_cast<FacebookInterface*>(this));
            setContentItemData(retn, data);
            retn->componentComplete();
            return retn;
        }
        break;

        case FacebookInterface::Unknown: {
            qWarning() << Q_FUNC_INFO << "Unable to detect the type of the content item";
            IdentifiableContentItemInterface *returnedInterface
                    = new IdentifiableContentItemInterface(parent);
            returnedInterface->classBegin();
            returnedInterface->setSocialNetwork(const_cast<FacebookInterface*>(this));
            setContentItemData(returnedInterface, data);
            returnedInterface->componentComplete();
            return returnedInterface;
        }
        break;

        default: qWarning() << Q_FUNC_INFO << "unsupported type:" << detectedType; break;
    }

    return 0;
}

void FacebookInterface::populateDataForLastNode()
{
    Q_D(FacebookInterface);
    const Node &node = d->lastNode();
    d->internalStatus = FacebookInterfacePrivate::PopulatingNodeData;

    if (node.isNull()) {
        qWarning() << Q_FUNC_INFO << "Cannot retrieve related content for null node!";
        return;
    }

    d->setCurrentReply(getRequest(node.identifier(), QString(), QStringList(), QVariantMap()),
                       node.identifier());
    d->connectFinishedAndErrors();
}

void FacebookInterface::populateRelatedDataforLastNode()
{
    Q_D(FacebookInterface);
    if (!d->performRelatedDataRequest(d->lastNode().identifier(), d->lastNode().filters().toList(),
                           QMultiMap<int, QPair<QString, QString> >())) {
        d->setError(SocialNetworkInterface::DataUpdateError,
                    QLatin1String("Cannot perform related data request"));
        d->internalStatus = FacebookInterfacePrivate::Idle;
    }
}

bool FacebookInterface::validateCacheEntryForLastNode(const QVariantMap &cacheEntryData)
{
    Q_UNUSED(cacheEntryData)
    // Case of friends loaded from the friendlist
    // TODO XXX ^
    return true;
}

/*! \internal */
void FacebookInterface::updateCurrentUserIdentifier()
{
    Q_D(FacebookInterface);
    if (d->accessToken.isEmpty()) {
        return;
    }

    QVariantMap queryItems;
    queryItems.insert(QLatin1String("access_token"), d->accessToken);
    queryItems.insert(QLatin1String("fields"), QLatin1String("id"));
    arbitraryRequest(SocialNetworkInterface::GetRequest, QLatin1String("https://graph.facebook.com/me"),
                     queryItems);
    connect(this, SIGNAL(arbitraryRequestResponseReceived(bool,QVariantMap)),
            this, SLOT(updateCurrentUserIdentifierHandler(bool,QVariantMap)));
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
