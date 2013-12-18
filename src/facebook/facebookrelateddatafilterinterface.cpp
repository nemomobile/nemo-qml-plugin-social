/*
 * Copyright (C) 2013 Jolla Ltd. <lucien.xu@jollamobile.com>
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

#include "facebookrelateddatafilterinterface.h"
#include "filterinterface_p.h"
#include "socialnetworkmodelinterface.h"
#include "identifiablecontentiteminterface_p.h"
#include "facebookontology_p.h"
#include "facebookobjectreferenceinterface.h"
#include "facebookalbuminterface.h"
#include "facebookcommentinterface.h"
#include "facebooklikeinterface.h"
#include "facebooknotificationinterface.h"
#include "facebookphotointerface.h"
#include "facebookpostinterface.h"
#include "facebookuserinterface.h"
#include <QtCore/QDebug>

static const char *CONTENTTYPE_KEY = "content_type";
static const char *PATH_KEY = "path";

class FacebookRelatedDataFilterInterfacePrivate: public FilterInterfacePrivate
{
public:
    explicit FacebookRelatedDataFilterInterfacePrivate(FacebookRelatedDataFilterInterface *q);
    QString fields;
    QString identifier;
    FacebookInterface::ConnectionType connection;
    // TODO: move to FacebookInterface
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
};

FacebookRelatedDataFilterInterfacePrivate::FacebookRelatedDataFilterInterfacePrivate(FacebookRelatedDataFilterInterface *q)
    : FilterInterfacePrivate(q)
{
}

FacebookRelatedDataFilterInterface::FacebookRelatedDataFilterInterface(QObject *parent) :
    FilterInterface(*(new FacebookRelatedDataFilterInterfacePrivate(this)), parent)
{
}

QString FacebookRelatedDataFilterInterface::identifier() const
{
    Q_D(const FacebookRelatedDataFilterInterface);
    return d->identifier;
}

void FacebookRelatedDataFilterInterface::setIdentifier(const QString &identifier)
{
    Q_D(FacebookRelatedDataFilterInterface);
    if (d->identifier != identifier) {
        d->identifier = identifier;
        emit identifierChanged();
    }
}

QString FacebookRelatedDataFilterInterface::fields() const
{
    Q_D(const FacebookRelatedDataFilterInterface);
    return d->fields;
}

void FacebookRelatedDataFilterInterface::setFields(const QString &fields)
{
    Q_D(FacebookRelatedDataFilterInterface);
    if (d->fields != fields) {
        d->fields = fields;
        emit fieldsChanged();
    }
}

FacebookInterface::ConnectionType FacebookRelatedDataFilterInterface::connection() const
{
    Q_D(const FacebookRelatedDataFilterInterface);
    return d->connection;
}

void FacebookRelatedDataFilterInterface::setConnection(FacebookInterface::ConnectionType connection)
{
    Q_D(FacebookRelatedDataFilterInterface);
    if (d->connection != connection) {
        d->connection = connection;
        emit connectionChanged();
    }
}

bool FacebookRelatedDataFilterInterface::isAcceptable(QObject *item,
                                                      SocialNetworkInterface *socialNetwork) const
{
    if (!testType<FacebookInterface>(socialNetwork)) {
        return false;
    }

    // This filter only works for models
    if (!testType<SocialNetworkModelInterface>(item)) {
        return false;
    }

    return true;
}

bool FacebookRelatedDataFilterInterface::performLoadRequestImpl(QObject *item,
                                                                SocialNetworkInterface *socialNetwork)
{
    Q_D(FacebookRelatedDataFilterInterface);
    FacebookInterface *facebook = qobject_cast<FacebookInterface *>(socialNetwork);
    if (!facebook) {
        return false;
    }

    FacebookInterface::ContentItemType contentType = FacebookInterface::Unknown;
    QString path;

    FacebookRelatedDataFilterInterfacePrivate::typeAndPath(d->connection, contentType, path);
    if (contentType == FacebookInterface::Unknown) {
        return false;
    }

    QObject *handle = facebook->get(this, d->identifier, path, d->fields);
    QVariantMap properties;
    properties.insert(CONTENTTYPE_KEY, contentType);
    properties.insert(PATH_KEY, path);

    d->addHandleProperties(handle, properties);
    return d->addHandle(handle, item, socialNetwork);
}

bool FacebookRelatedDataFilterInterface::performSetModelDataImpl(SocialNetworkModelInterface *model,
                                                                 SocialNetworkInterface *socialNetwork,
                                                                 const QByteArray &data,
                                                                 const QVariantMap &properties)
{
    Q_UNUSED(socialNetwork)
    bool ok = false;
    QVariantMap dataMap = IdentifiableContentItemInterfacePrivate::parseReplyData(data, &ok);
    if (!ok) {
        QString errorMessage = QString(QLatin1String("Unable to parse downloaded data. "\
                                                     "Downloaded data: %1")).arg(QString(data));
        model->setError(SocialNetworkInterface::DataUpdateError, errorMessage);
        return false;
    }

    // dataMap should contain information like this
    //
    // {
    //     "id": _yourid_,
    //     _path_: {
    //         "data": [
    //             {
    //                 "id": "someid"
    //                 "name": "Someone",
    //                 ...
    //             },
    //             ...
    //         ]
    //     }
    // }

    FacebookInterface::ContentItemType contentType = static_cast<FacebookInterface::ContentItemType>(properties.value(CONTENTTYPE_KEY).toInt());
    QString path = properties.value(PATH_KEY).toString();
    QVariantList dataList = dataMap.value(path).toMap().value(FACEBOOK_ONTOLOGY_METADATA_DATA).toList();
    QList<ContentItemInterface *> modelData;


    foreach (const QVariant &dataEntry, dataList) {
        modelData.append(FacebookRelatedDataFilterInterfacePrivate::createItem(contentType, dataEntry.toMap(),
                                                                               socialNetwork, model));
    }

    model->setModelData(modelData);
    return true;
}
