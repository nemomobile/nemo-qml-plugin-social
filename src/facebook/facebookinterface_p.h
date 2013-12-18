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

#ifndef FACEBOOKINTERFACE_P_H
#define FACEBOOKINTERFACE_P_H

#include "socialnetworkinterface.h"
#include "socialnetworkinterface_p.h"
#include "facebookinterface.h"

#include <QtCore/QObject>
#include <QtCore/QVariantMap>
#include <QtCore/QStringList>
#include <QtCore/QString>
#include <QtCore/QUrl>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QSslError>

typedef QPair<QString, QString> StringPair;
typedef QMultiMap<int, StringPair> RequestFieldsMap;

class ContentItemInterface;

class FacebookInterfacePrivate : public SocialNetworkInterfacePrivate
{
public:
    // the following is for identifiable content item "actions"
    enum FacebookAction {
        NoAction = 0,
        RemoveAction,
        ReloadAction,
        LikeAction,
        DeleteLikeAction,
        TagAction,
        DeleteTagAction,
        UploadCommentAction,
        DeleteCommentAction,
        UploadPhotoAction,
        DeletePhotoAction,
        UploadAlbumAction,
        DeleteAlbumAction
    };

    explicit FacebookInterfacePrivate(FacebookInterface *q);
    QNetworkReply * performRequest(const QString &identifier, const QString &graph = QString(),
                                   const QString &fields = QString(),
                                   const QMap<QString, QString> &arguments = QMap<QString, QString>());

    QString accessToken;
    QString currentUserIdentifier;

    static QString makeFields(const QString &path, const QString &fields);


protected:
    QByteArray preprocessData(const QByteArray &data);
    // Reimplemented
//    void populateDataForNode(Node::Ptr node);
//    void populateRelatedDataforNode(Node::Ptr node);
//    bool validateCacheEntryForNode(CacheEntry::ConstPtr cacheEntry);
//    QString dataSection(int type, const QVariantMap &data) const;
//    ContentItemInterface * contentItemFromData(const QVariantMap &data, QObject *parent = 0) const;
//    QNetworkReply * getRequest(const QString &objectIdentifier, const QString &extraPath,
//                               const QStringList &whichFields, const QVariantMap &extraData);
//    QNetworkReply * postRequest(const QString &objectIdentifier, const QString &extraPath,
//                                const QVariantMap &data, const QVariantMap &extraData);
//    QNetworkReply * deleteRequest(const QString &objectIdentifier, const QString &extraPath,
//                                  const QVariantMap &extraData);
//    void handleFinished(Node::Ptr node, QNetworkReply *reply);

//private:
//    QUrl requestUrl(const QString &objectId, const QString &extraPath,
//                    const QStringList &whichFields, const QVariantMap &extraData);
//    QNetworkReply * uploadImage(const QString &objectId, const QString &extraPath,
//                                const QVariantMap &data, const QVariantMap &extraData);
//    void handlePopulateNode(Node::Ptr node, const QVariantMap &responseData);
//    void handlePopulateRelatedData(Node::Ptr node, const QVariantMap &relatedData,
//                                   const QUrl &requestUrl);
//    void setCurrentUserIdentifier(const QString &meId);
//    bool checkNodeType(Node::Ptr node);
//    bool checkIfNeedAdditionalLoading(Node::Ptr node);
//    inline bool tryAddCacheEntryFromData(NodePrivate::Status nodeStatus,
//                                         const QVariantMap &relatedData,
//                                         const QString &requestPath, int type,
//                                         const QString &typeName, CacheEntry::List &list,
//                                         QVariantMap &extraInfo);
//    inline QString createField(int type, const QString &connection,
//                               const RequestFieldsMap &requestFiledsMap);
//    inline QVariantMap makeCursorExtraData(NodePrivate::Status insertionMode,
//                                           const QVariantMap &oldExtraData,
//                                           const QVariantMap &cursorsMap);
    Q_DECLARE_PUBLIC(FacebookInterface)
};

#endif // FACEBOOKINTERFACE_P_H
