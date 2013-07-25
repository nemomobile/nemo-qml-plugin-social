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

#ifndef TWITTERINTERFACE_H
#define TWITTERINTERFACE_H

#include "socialnetworkinterface.h"

#include <QtCore/QStringList>
#include <QtCore/QString>

class QNetworkReply;

class TwitterTweetInterface;
class TwitterPlaceInterface;
class TwitterUserInterface;
class IdentifiableContentItemInterface;

/*
 * NOTE: if you construct one of these in C++ directly,
 * you MUST call classBegin() and componentCompleted()
 * directly after construction.
 */

class CacheEntry;
class TwitterInterfacePrivate;
class TwitterInterface : public SocialNetworkInterface
{
    Q_OBJECT

    Q_PROPERTY(QString oauthToken READ oauthToken WRITE setOAuthToken NOTIFY oauthTokenChanged)
    Q_PROPERTY(QString oauthTokenSecret READ oauthTokenSecret WRITE setOAuthTokenSecret
               NOTIFY oauthTokenSecretChanged)
    Q_PROPERTY(QString consumerKey READ consumerKey WRITE setConsumerKey NOTIFY consumerKeyChanged)
    Q_PROPERTY(QString consumerSecret READ consumerSecret WRITE setConsumerSecret
               NOTIFY consumerSecretChanged)
    Q_PROPERTY(QString currentUserIdentifier READ currentUserIdentifier
               WRITE setCurrentUserIdentifier NOTIFY currentUserIdentifierChanged)

    Q_ENUMS(ContentItemType)

public:
    enum ContentItemType {
        NotInitialized = 0,
        Unknown = 1,
        User,
        Tweet,
        Place,

        Home = 128,

        Friends = 256,
        Followers = 257
    };

public:
    explicit TwitterInterface(QObject *parent = 0);

    // properties
    QString oauthToken() const;
    void setOAuthToken(const QString &oauthToken);
    QString oauthTokenSecret() const;
    void setOAuthTokenSecret(const QString &oauthTokenSecret);
    QString consumerKey() const;
    void setConsumerKey(const QString &consumerKey);
    QString consumerSecret() const;
    void setConsumerSecret(const QString &consumerSecret);
    QString currentUserIdentifier() const;
    void setCurrentUserIdentifier(const QString &currentUserIdentifier);

Q_SIGNALS:
    void oauthTokenChanged();
    void oauthTokenSecretChanged();
    void consumerKeyChanged();
    void consumerSecretChanged();
    void currentUserIdentifierChanged();
private:
    QVariantMap twitterContentItemData(ContentItemInterface *contentItem);
    void setTwitterContentItemData(ContentItemInterface *contentItem, const QVariantMap &data);
    friend class TwitterPlaceInterfacePrivate;
    friend class TwitterTweetInterfacePrivate;
    friend class TwitterUserInterfacePrivate;
private:
    Q_DECLARE_PRIVATE(TwitterInterface)
};

#endif // TWITTERINTERFACE_H
