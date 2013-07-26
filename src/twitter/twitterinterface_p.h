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

#ifndef TWITTERINTERFACE_P_H
#define TWITTERINTERFACE_P_H

#include "socialnetworkinterface.h"
#include "socialnetworkinterface_p.h"
#include "twitterinterface.h"

#include <QtCore/QObject>
#include <QtCore/QVariantMap>
#include <QtCore/QStringList>
#include <QtCore/QString>
#include <QtCore/QUrl>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QSslError>

class ContentItemInterface;

struct RequestInfo
{
    QString objectIdentifier;
    QString extraPath;
    QStringList whichFields;
    QVariantMap extraData;
    QVariantMap postedData;
};

class TwitterInterfacePrivate : public SocialNetworkInterfacePrivate
{
public:
    explicit TwitterInterfacePrivate(TwitterInterface *q);

    QString oauthToken;
    QString oauthTokenSecret;
    QString consumerKey;
    QString consumerSecret;
    QString currentUserIdentifier;

    QNetworkRequest networkRequest(const QString &extraPath, const QVariantMap &extraData,
                                   const QByteArray &requestMethod,
                                   const QVariantMap &postData = QVariantMap());

    int detectTypeFromData(const QVariantMap &data) const;
    void handlePopulateRelatedData(Node &node, const QVariant &relatedData,
                                   const QUrl &requestUrl);

    // Requests
    static RequestInfo requestUserInfo(const QString &identifier);
    static RequestInfo requestTweetInfo(const QString &identifier, bool trimUser = false,
                                        bool includeMyRetweet = false, bool includeEntities = true);

public:
    // the following is for identifiable content item "actions"
    enum TwitterAction {
        NoAction = 0,
        FollowAction,
        UnfollowAction,
        TweetAction,
        RetweetAction,
        FavoriteAction,
        UnfavoriteAction
    };
protected:
    // Reimplemented
    void populateDataForNode(Node &node);
    void populateRelatedDataforNode(Node &node);
    bool validateCacheEntryForNode(const CacheEntry &cacheEntry);
    QString dataSection(int type, const QVariantMap &data) const;
    ContentItemInterface * contentItemFromData(const QVariantMap &data, QObject *parent = 0) const;
    QNetworkReply * getRequest(const QString &objectIdentifier, const QString &extraPath,
                               const QStringList &whichFields, const QVariantMap &extraData);
    QNetworkReply * postRequest(const QString &objectIdentifier, const QString &extraPath,
                                const QVariantMap &data, const QVariantMap &extraData);
    QNetworkReply * deleteRequest(const QString &objectIdentifier, const QString &extraPath,
                                  const QVariantMap &extraData);
    int guessType(const QString &identifier, int type, const QSet<FilterInterface *> &filters);
    void handleFinished(Node &node, QNetworkReply *reply);

private:
    bool performRelatedDataRequest(Node &node, const QString &identifier,
                                   const QList<FilterInterface *> &filters);
    Q_DECLARE_PUBLIC(TwitterInterface)
};

#endif // TWITTERINTERFACE_P_H
