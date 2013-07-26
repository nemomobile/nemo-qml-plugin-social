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

#ifndef TWITTERTWEETINTERFACE_H
#define TWITTERTWEETINTERFACE_H

#include "identifiablecontentiteminterface.h"

#include <QtCore/QDateTime>
#include <QtCore/QString>
#include "twitteruserinterface.h"

/*
 * NOTE: if you construct one of these in C++ directly,
 * you MUST call classBegin() and componentCompleted()
 * directly after construction.
 */

class TwitterTweetInterfacePrivate;
class TwitterTweetInterface: public IdentifiableContentItemInterface
{
    Q_OBJECT
    Q_PROPERTY(QDateTime createdAt READ createdAt NOTIFY createdAtChanged)
    Q_PROPERTY(int favoriteCount READ favoriteCount NOTIFY favoriteCountChanged)
    Q_PROPERTY(bool favorited READ favorited NOTIFY favoritedChanged)
    Q_PROPERTY(QString filterLevel READ filterLevel NOTIFY filterLevelChanged)
    Q_PROPERTY(QString inReplyToScreenName READ inReplyToScreenName NOTIFY inReplyToScreenNameChanged)
    Q_PROPERTY(QString inReplyToStatusIdentifier READ inReplyToStatusIdentifier NOTIFY inReplyToStatusIdentifierChanged)
    Q_PROPERTY(QString inReplyToUserIdentifier READ inReplyToUserIdentifier NOTIFY inReplyToUserIdentifierChanged)
    Q_PROPERTY(QString lang READ lang NOTIFY langChanged)
    Q_PROPERTY(bool possiblySensitive READ possiblySensitive NOTIFY possiblySensitiveChanged)
    Q_PROPERTY(int retweetCount READ retweetCount NOTIFY retweetCountChanged)
    Q_PROPERTY(bool retweeted READ retweeted NOTIFY retweetedChanged)
    Q_PROPERTY(QString source READ source NOTIFY sourceChanged)
    Q_PROPERTY(QString text READ text NOTIFY textChanged)
    Q_PROPERTY(bool truncated READ truncated NOTIFY truncatedChanged)
    Q_PROPERTY(TwitterUserInterface * user READ user NOTIFY userChanged)
    Q_PROPERTY(bool withheldCopyright READ withheldCopyright NOTIFY withheldCopyrightChanged)
    Q_PROPERTY(QString withheldScope READ withheldScope NOTIFY withheldScopeChanged)
public:
    explicit TwitterTweetInterface(QObject *parent = 0);

    // Overrides.
    int type() const;
    Q_INVOKABLE bool remove();
    Q_INVOKABLE bool reload(const QStringList &whichFields = QStringList());

    // Invokable API.
    Q_INVOKABLE bool uploadRetweet();
    Q_INVOKABLE bool favorite();
    Q_INVOKABLE bool unfavorite();
    Q_INVOKABLE bool uploadReply(const QString &message, const QStringList &pathToMedias = QStringList());

    // Accessors
    QDateTime createdAt() const;
    int favoriteCount() const;
    bool favorited() const;
    QString filterLevel() const;
    QString inReplyToScreenName() const;
    QString inReplyToStatusIdentifier() const;
    QString inReplyToUserIdentifier() const;
    QString lang() const;
    bool possiblySensitive() const;
    int retweetCount() const;
    bool retweeted() const;
    QString source() const;
    QString text() const;
    bool truncated() const;
    TwitterUserInterface * user() const;
    bool withheldCopyright() const;
    QString withheldScope() const;
Q_SIGNALS:
    void createdAtChanged();
    void favoriteCountChanged();
    void favoritedChanged();
    void filterLevelChanged();
    void inReplyToScreenNameChanged();
    void inReplyToStatusIdentifierChanged();
    void inReplyToUserIdentifierChanged();
    void langChanged();
    void possiblySensitiveChanged();
    void retweetCountChanged();
    void retweetedChanged();
    void sourceChanged();
    void textChanged();
    void truncatedChanged();
    void userChanged();
    void withheldCopyrightChanged();
    void withheldScopeChanged();
private:
    Q_DECLARE_PRIVATE(TwitterTweetInterface)
};

#endif // TWITTERTWEETINTERFACE_H
