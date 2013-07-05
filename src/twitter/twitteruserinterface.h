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

#ifndef TWITTERUSERINTERFACE_H
#define TWITTERUSERINTERFACE_H

#include "identifiablecontentiteminterface.h"

#include <QtCore/QString>
#include <QtGui/QColor>
#include <QtCore/QUrl>

/*
 * NOTE: if you construct one of these in C++ directly,
 * you MUST call classBegin() and componentCompleted()
 * directly after construction.
 */

class TwitterUserInterfacePrivate;
class TwitterUserInterface: public IdentifiableContentItemInterface
{
    Q_OBJECT
    Q_PROPERTY(bool contributorsEnabled READ contributorsEnabled NOTIFY contributorsEnabledChanged)
    Q_PROPERTY(QString createdAt READ createdAt NOTIFY createdAtChanged)
    Q_PROPERTY(bool defaultProfile READ defaultProfile NOTIFY defaultProfileChanged)
    Q_PROPERTY(bool defaultProfileImage READ defaultProfileImage NOTIFY defaultProfileImageChanged)
    Q_PROPERTY(QString description READ description NOTIFY descriptionChanged)
    Q_PROPERTY(int favouritesCount READ favouritesCount NOTIFY favouritesCountChanged)
    Q_PROPERTY(bool followRequestSent READ followRequestSent NOTIFY followRequestSentChanged)
    Q_PROPERTY(int followersCount READ followersCount NOTIFY followersCountChanged)
    Q_PROPERTY(int friendsCount READ friendsCount NOTIFY friendsCountChanged)
    Q_PROPERTY(bool geoEnabled READ geoEnabled NOTIFY geoEnabledChanged)
    Q_PROPERTY(bool isTranslator READ isTranslator NOTIFY isTranslatorChanged)
    Q_PROPERTY(QString lang READ lang NOTIFY langChanged)
    Q_PROPERTY(int listedCount READ listedCount NOTIFY listedCountChanged)
    Q_PROPERTY(QString location READ location NOTIFY locationChanged)
    Q_PROPERTY(QString name READ name NOTIFY nameChanged)
    Q_PROPERTY(QColor profileBackgroundColor READ profileBackgroundColor NOTIFY profileBackgroundColorChanged)
    Q_PROPERTY(QUrl profileBackgroundImageUrl READ profileBackgroundImageUrl NOTIFY profileBackgroundImageUrlChanged)
    Q_PROPERTY(QUrl profileBackgroundImageUrlHttps READ profileBackgroundImageUrlHttps NOTIFY profileBackgroundImageUrlHttpsChanged)
    Q_PROPERTY(bool profileBackgroundTile READ profileBackgroundTile NOTIFY profileBackgroundTileChanged)
    Q_PROPERTY(QUrl profileBannerUrl READ profileBannerUrl NOTIFY profileBannerUrlChanged)
    Q_PROPERTY(QUrl profileImageUrl READ profileImageUrl NOTIFY profileImageUrlChanged)
    Q_PROPERTY(QUrl profileImageUrlHttps READ profileImageUrlHttps NOTIFY profileImageUrlHttpsChanged)
    Q_PROPERTY(QColor profileLinkColor READ profileLinkColor NOTIFY profileLinkColorChanged)
    Q_PROPERTY(QColor profileSidebarBorderColor READ profileSidebarBorderColor NOTIFY profileSidebarBorderColorChanged)
    Q_PROPERTY(QColor profileSidebarFillColor READ profileSidebarFillColor NOTIFY profileSidebarFillColorChanged)
    Q_PROPERTY(QColor profileTextColor READ profileTextColor NOTIFY profileTextColorChanged)
    Q_PROPERTY(bool profileUseBackgroundImage READ profileUseBackgroundImage NOTIFY profileUseBackgroundImageChanged)
    Q_PROPERTY(bool isProtected READ isProtected NOTIFY isProtectedChanged)
    Q_PROPERTY(QString screenName READ screenName NOTIFY screenNameChanged)
    Q_PROPERTY(bool showAllInlineMedia READ showAllInlineMedia NOTIFY showAllInlineMediaChanged)
    Q_PROPERTY(int statusesCount READ statusesCount NOTIFY statusesCountChanged)
    Q_PROPERTY(QString timeZone READ timeZone NOTIFY timeZoneChanged)
    Q_PROPERTY(QUrl url READ url NOTIFY urlChanged)
    Q_PROPERTY(int utcOffset READ utcOffset NOTIFY utcOffsetChanged)
    Q_PROPERTY(bool verified READ verified NOTIFY verifiedChanged)
    Q_PROPERTY(QString withheldInCountries READ withheldInCountries NOTIFY withheldInCountriesChanged)
    Q_PROPERTY(QString withheldScope READ withheldScope NOTIFY withheldScopeChanged)
public:
    explicit TwitterUserInterface(QObject *parent = 0);

    // Overrides.
    int type() const;
    Q_INVOKABLE bool remove();
    Q_INVOKABLE bool reload(const QStringList &whichFields = QStringList());

    // Invokable API.
    Q_INVOKABLE bool uploadTweet(const QString &message, const QStringList &pathToMedias = QStringList());

    // Accessors
    bool contributorsEnabled() const;
    QString createdAt() const;
    bool defaultProfile() const;
    bool defaultProfileImage() const;
    QString description() const;
    int favouritesCount() const;
    bool followRequestSent() const;
    int followersCount() const;
    int friendsCount() const;
    bool geoEnabled() const;
    bool isTranslator() const;
    QString lang() const;
    int listedCount() const;
    QString location() const;
    QString name() const;
    QColor profileBackgroundColor() const;
    QUrl profileBackgroundImageUrl() const;
    QUrl profileBackgroundImageUrlHttps() const;
    bool profileBackgroundTile() const;
    QUrl profileBannerUrl() const;
    QUrl profileImageUrl() const;
    QUrl profileImageUrlHttps() const;
    QColor profileLinkColor() const;
    QColor profileSidebarBorderColor() const;
    QColor profileSidebarFillColor() const;
    QColor profileTextColor() const;
    bool profileUseBackgroundImage() const;
    bool isProtected() const;
    QString screenName() const;
    bool showAllInlineMedia() const;
    int statusesCount() const;
    QString timeZone() const;
    QUrl url() const;
    int utcOffset() const;
    bool verified() const;
    QString withheldInCountries() const;
    QString withheldScope() const;
Q_SIGNALS:
    void contributorsEnabledChanged();
    void createdAtChanged();
    void defaultProfileChanged();
    void defaultProfileImageChanged();
    void descriptionChanged();
    void favouritesCountChanged();
    void followRequestSentChanged();
    void followersCountChanged();
    void friendsCountChanged();
    void geoEnabledChanged();
    void isTranslatorChanged();
    void langChanged();
    void listedCountChanged();
    void locationChanged();
    void nameChanged();
    void profileBackgroundColorChanged();
    void profileBackgroundImageUrlChanged();
    void profileBackgroundImageUrlHttpsChanged();
    void profileBackgroundTileChanged();
    void profileBannerUrlChanged();
    void profileImageUrlChanged();
    void profileImageUrlHttpsChanged();
    void profileLinkColorChanged();
    void profileSidebarBorderColorChanged();
    void profileSidebarFillColorChanged();
    void profileTextColorChanged();
    void profileUseBackgroundImageChanged();
    void isProtectedChanged();
    void screenNameChanged();
    void showAllInlineMediaChanged();
    void statusesCountChanged();
    void timeZoneChanged();
    void urlChanged();
    void utcOffsetChanged();
    void verifiedChanged();
    void withheldInCountriesChanged();
    void withheldScopeChanged();
private:
    Q_DECLARE_PRIVATE(TwitterUserInterface)
};

#endif // TWITTERUSERINTERFACE_H
