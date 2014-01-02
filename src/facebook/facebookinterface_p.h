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

#include "facebookobjectreferenceinterface.h"
#include "facebookalbuminterface.h"
#include "facebookcommentinterface.h"
#include "facebooklikeinterface.h"
#include "facebooknotificationinterface.h"
#include "facebookphotointerface.h"
#include "facebookpostinterface.h"
#include "facebookuserinterface.h"

typedef QPair<QString, QString> StringPair;
typedef QMultiMap<int, StringPair> RequestFieldsMap;

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
        UploadCommentAction,
        DeleteCommentAction,
        UploadAlbumAction,
        DeleteAlbumAction,
        UploadPhotoAction,
        DeletePhotoAction,
        TagAction,
        DeleteTagAction
    };

    explicit FacebookInterfacePrivate(FacebookInterface *q);
    QNetworkReply * performRequest(const QString &identifier, const QString &graph = QString(),
                                   const QString &fields = QString(),
                                   const QMap<QString, QString> &arguments = QMap<QString, QString>(),
                                   const QMap<QString, QString> &genericArguments = QMap<QString, QString>());
    QNetworkReply * performPostRequest(const QString &identifier, const QString &graph,
                                       const QMap<QString, QString> &arguments = QMap<QString, QString>(),
                                       const QByteArray &postData = QByteArray());
    QNetworkReply * performPhotoPostRequest(const QString &identifier, const QString &graph,
                                            const QString &source,
                                            const QMap<QString, QString> &arguments = QMap<QString, QString>());
    QNetworkReply * performDeleteRequest(const QString &identifier, const QString &graph = QString(),
                                         const QMap<QString, QString> &arguments = QMap<QString, QString>());

    QString accessToken;
    QString currentUserIdentifier;

    // Build "fields" argument
    static QString makeFields(const QString &path, const QString &fields,
                              const QMap<QString, QString> &arguments);

    // Get the type of an item associated to a connection type
    // and also get the path in the API.
    static inline void typeAndPath(FacebookInterface::ConnectionType connection,
                                   FacebookInterface::ContentItemType &contentType,
                                   QString &path)
    {
        switch (connection) {
            case FacebookInterface::Albums:
                path = QLatin1String("albums");
                contentType = FacebookInterface::Album;
                break;
            case FacebookInterface::Comments:
                path = QLatin1String("comments");
                contentType = FacebookInterface::Comment;
                break;
            case FacebookInterface::Feed:
                path = QLatin1String("feed");
                contentType = FacebookInterface::Post;
                break;
            case FacebookInterface::Friends:
                path = QLatin1String("friends");
                contentType = FacebookInterface::User;
                break;
            case FacebookInterface::Home:
                path = QLatin1String("home");
                contentType = FacebookInterface::Post;
                break;
            case FacebookInterface::Likes:
                path = QLatin1String("likes");
                contentType = FacebookInterface::Like;
                break;
            case FacebookInterface::Notifications:
                path = QLatin1String("notifications");
                contentType = FacebookInterface::Notification;
                break;
            case FacebookInterface::Photos:
                path = QLatin1String("photos");
                contentType = FacebookInterface::Photo;
                break;
            default:
                qWarning() << "Invalid connection";
                contentType = FacebookInterface::Unknown;
                break;
        }
    }

    // Create an item
    static inline ContentItemInterface * createItem(FacebookInterface::ContentItemType &contentType,
                                                    const QVariantMap &data,
                                                    SocialNetworkInterface *socialNetwork,
                                                    QObject *parent = 0)
    {
        ContentItemInterface *item = 0;

        switch (contentType) {
        case FacebookInterface::ObjectReference:
            item = new FacebookObjectReferenceInterface(parent);
            break;
        case FacebookInterface::Album:
            item = new FacebookAlbumInterface(parent);
            break;
        case FacebookInterface::Comment:
            item = new FacebookCommentInterface(parent);
            break;
        case FacebookInterface::Like:
            item = new FacebookLikeInterface(parent);
            break;
        case FacebookInterface::Notification:
            item = new FacebookNotificationInterface(parent);
            break;
        case FacebookInterface::Photo:
            item = new FacebookPhotoInterface(parent);
            break;
        case FacebookInterface::Post:
            item = new FacebookPostInterface(parent);
            break;
        case FacebookInterface::User:
            item = new FacebookUserInterface(parent);
            break;
        default:
            break;
        }

        if (item) {
            item->setSocialNetwork(socialNetwork);
            item->setData(data);
            item->classBegin();
            item->componentComplete();
        }

        return item;
    }

    // Actions
    static bool runLike(SocialNetworkInterface *socialNetwork,
                        IdentifiableContentItemInterface *item);
    static bool runUnlike(SocialNetworkInterface *socialNetwork,
                          IdentifiableContentItemInterface *item);
    static bool runUploadComment(SocialNetworkInterface *socialNetwork,
                                 IdentifiableContentItemInterface *item,
                                 const QString &message);
    static bool runRemoveComment(SocialNetworkInterface *socialNetwork,
                                 IdentifiableContentItemInterface *item,
                                 const QString &commentIdentifier);
    static bool runUploadAlbum(SocialNetworkInterface *socialNetwork,
                               IdentifiableContentItemInterface *item,
                               const QString &name, const QString &message,
                               const QVariantMap &privacy);
    static bool runRemoveAlbum(SocialNetworkInterface *socialNetwork,
                               IdentifiableContentItemInterface *item,
                               const QString &albumIdentifier);
    static bool runUploadPhoto(SocialNetworkInterface *socialNetwork,
                               IdentifiableContentItemInterface *item,
                               const QString &source, const QString &message);
    static bool runRemovePhoto(SocialNetworkInterface *socialNetwork,
                               IdentifiableContentItemInterface *item,
                               const QString &photoIdentifier);
    static bool runTagUser(SocialNetworkInterface *socialNetwork,
                           IdentifiableContentItemInterface *item,
                           const QString &userId, float xOffset, float yOffset);
    static bool runUntagUser(SocialNetworkInterface *socialNetwork,
                             IdentifiableContentItemInterface *item,
                             const QString &userId);
    static bool runTagText(SocialNetworkInterface *socialNetwork,
                           IdentifiableContentItemInterface *item,
                           const QString &text, float xOffset, float yOffset);
    static bool runUntagText(SocialNetworkInterface *socialNetwork,
                             IdentifiableContentItemInterface *item,
                             const QString &text);


protected:
    QByteArray preprocessData(const QByteArray &data);
    void performAction(IdentifiableContentItemInterface *item,
                       const QVariantMap &properties);
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
