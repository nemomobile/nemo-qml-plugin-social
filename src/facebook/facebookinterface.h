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

#ifndef FACEBOOKINTERFACE_H
#define FACEBOOKINTERFACE_H

#include "socialnetworkinterface.h"

#include <QtCore/QStringList>
#include <QtCore/QString>

//class QNetworkReply;
//class FacebookObjectReferenceInterface;

/*
 * NOTE: if you construct one of these in C++ directly,
 * you MUST call classBegin() and componentCompleted()
 * directly after construction.
 */

class FilterInterface;
class FacebookInterfacePrivate;
class FacebookInterface : public SocialNetworkInterface
{
    Q_OBJECT

    Q_PROPERTY(QString accessToken READ accessToken WRITE setAccessToken NOTIFY accessTokenChanged)
    Q_PROPERTY(QString currentUserIdentifier READ currentUserIdentifier
               NOTIFY currentUserIdentifierChanged)
    Q_PROPERTY(QString locale READ locale WRITE setLocale NOTIFY localeChanged)

    Q_ENUMS(ContentItemType)
    Q_ENUMS(ConnectionType)

public:
    enum ContentItemType {
        NotInitialized = 0,
        Unknown = 1,
        ObjectReference,
        Album,
        Comment,
        Notification,
        Photo,
        Post,
        User,

        Application,
        Event,
        Location,

        Like,
        NameTag,
        PhotoImage,
        PhotoTag,
        PostAction,
        PostProperty,
        UserCover,
        UserPicture


    };
    enum ConnectionType {
        InvalidConnection,
        Albums,
        Comments,
        Feed,
        Friends,
        Home,
        Likes,
        Notifications,
        Photos
    };

public:
    explicit FacebookInterface(QObject *parent = 0);

    // properties
    QString accessToken() const;
    void setAccessToken(const QString &token);
    QString currentUserIdentifier() const;
    QString locale() const;
    void setLocale(const QString &locale);

    // Non QML API
    QObject * get(FilterInterface *filter, const QString &identifier,
                  const QString &graph = QString(), const QString &fields = QString(),
                  const QMap<QString, QString> &arguments = QMap<QString, QString>());


Q_SIGNALS:
    void accessTokenChanged();
    void currentUserIdentifierChanged();
    void localeChanged();
private:
    Q_DECLARE_PRIVATE(FacebookInterface)
};

#endif // FACEBOOKINTERFACE_H
