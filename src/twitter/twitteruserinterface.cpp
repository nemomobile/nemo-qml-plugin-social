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

#include "twitteruserinterface_p.h"
#include "twitterinterface.h"
#include "twitterontology_p.h"
// <<< include
#include "twitterinterface_p.h"
#include <QtCore/QDebug>
// >>> include

TwitterUserInterfacePrivate::TwitterUserInterfacePrivate(TwitterUserInterface *q)
    : IdentifiableContentItemInterfacePrivate(q)
    , action(TwitterInterfacePrivate::NoAction)
{
}

void TwitterUserInterfacePrivate::finishedHandler()
{
// <<< finishedHandler
    Q_Q(TwitterUserInterface);
    if (!reply()) {
        // if an error occurred, it might have been deleted by the error handler.
        qWarning() << Q_FUNC_INFO << "network request finished but no reply";
        return;
    }

    QByteArray replyData = reply()->readAll();
    deleteReply();
    bool ok = false;
    QVariantMap responseData = ContentItemInterfacePrivate::parseReplyData(replyData, &ok);
    if (!ok)
        responseData.insert("response", replyData);

    switch (action) {
        case TwitterInterfacePrivate::TweetAction: {
            status = SocialNetworkInterface::Idle;
            emit q->statusChanged();
        }
        break;
        default: {
            error = SocialNetworkInterface::OtherError;
            errorMessage = QLatin1String("Request finished but no action currently in progress");
            status = SocialNetworkInterface::Error;
            emit q->statusChanged();
            emit q->errorChanged();
            emit q->errorMessageChanged();
            emit q->responseReceived(responseData);
        }
        break;
    }
// >>> finishedHandler
}
void TwitterUserInterfacePrivate::emitPropertyChangeSignals(const QVariantMap &oldData,
                                                            const QVariantMap &newData)
{
    Q_Q(TwitterUserInterface);
    QVariant oldContributorsEnabled = oldData.value(TWITTER_ONTOLOGY_USER_CONTRIBUTORSENABLED);
    QVariant newContributorsEnabled = newData.value(TWITTER_ONTOLOGY_USER_CONTRIBUTORSENABLED);
    QVariant oldCreatedAt = oldData.value(TWITTER_ONTOLOGY_USER_CREATEDAT);
    QVariant newCreatedAt = newData.value(TWITTER_ONTOLOGY_USER_CREATEDAT);
    QVariant oldDefaultProfile = oldData.value(TWITTER_ONTOLOGY_USER_DEFAULTPROFILE);
    QVariant newDefaultProfile = newData.value(TWITTER_ONTOLOGY_USER_DEFAULTPROFILE);
    QVariant oldDefaultProfileImage = oldData.value(TWITTER_ONTOLOGY_USER_DEFAULTPROFILEIMAGE);
    QVariant newDefaultProfileImage = newData.value(TWITTER_ONTOLOGY_USER_DEFAULTPROFILEIMAGE);
    QVariant oldDescription = oldData.value(TWITTER_ONTOLOGY_USER_DESCRIPTION);
    QVariant newDescription = newData.value(TWITTER_ONTOLOGY_USER_DESCRIPTION);
    QVariant oldFavouritesCount = oldData.value(TWITTER_ONTOLOGY_USER_FAVOURITESCOUNT);
    QVariant newFavouritesCount = newData.value(TWITTER_ONTOLOGY_USER_FAVOURITESCOUNT);
    QVariant oldFollowRequestSent = oldData.value(TWITTER_ONTOLOGY_USER_FOLLOWREQUESTSENT);
    QVariant newFollowRequestSent = newData.value(TWITTER_ONTOLOGY_USER_FOLLOWREQUESTSENT);
    QVariant oldFollowersCount = oldData.value(TWITTER_ONTOLOGY_USER_FOLLOWERSCOUNT);
    QVariant newFollowersCount = newData.value(TWITTER_ONTOLOGY_USER_FOLLOWERSCOUNT);
    QVariant oldFriendsCount = oldData.value(TWITTER_ONTOLOGY_USER_FRIENDSCOUNT);
    QVariant newFriendsCount = newData.value(TWITTER_ONTOLOGY_USER_FRIENDSCOUNT);
    QVariant oldGeoEnabled = oldData.value(TWITTER_ONTOLOGY_USER_GEOENABLED);
    QVariant newGeoEnabled = newData.value(TWITTER_ONTOLOGY_USER_GEOENABLED);
    QVariant oldIsTranslator = oldData.value(TWITTER_ONTOLOGY_USER_ISTRANSLATOR);
    QVariant newIsTranslator = newData.value(TWITTER_ONTOLOGY_USER_ISTRANSLATOR);
    QVariant oldLang = oldData.value(TWITTER_ONTOLOGY_USER_LANG);
    QVariant newLang = newData.value(TWITTER_ONTOLOGY_USER_LANG);
    QVariant oldListedCount = oldData.value(TWITTER_ONTOLOGY_USER_LISTEDCOUNT);
    QVariant newListedCount = newData.value(TWITTER_ONTOLOGY_USER_LISTEDCOUNT);
    QVariant oldLocation = oldData.value(TWITTER_ONTOLOGY_USER_LOCATION);
    QVariant newLocation = newData.value(TWITTER_ONTOLOGY_USER_LOCATION);
    QVariant oldName = oldData.value(TWITTER_ONTOLOGY_USER_NAME);
    QVariant newName = newData.value(TWITTER_ONTOLOGY_USER_NAME);
    QVariant oldProfileBackgroundColor = oldData.value(TWITTER_ONTOLOGY_USER_PROFILEBACKGROUNDCOLOR);
    QVariant newProfileBackgroundColor = newData.value(TWITTER_ONTOLOGY_USER_PROFILEBACKGROUNDCOLOR);
    QVariant oldProfileBackgroundImageUrl = oldData.value(TWITTER_ONTOLOGY_USER_PROFILEBACKGROUNDIMAGEURL);
    QVariant newProfileBackgroundImageUrl = newData.value(TWITTER_ONTOLOGY_USER_PROFILEBACKGROUNDIMAGEURL);
    QVariant oldProfileBackgroundImageUrlHttps = oldData.value(TWITTER_ONTOLOGY_USER_PROFILEBACKGROUNDIMAGEURLHTTPS);
    QVariant newProfileBackgroundImageUrlHttps = newData.value(TWITTER_ONTOLOGY_USER_PROFILEBACKGROUNDIMAGEURLHTTPS);
    QVariant oldProfileBackgroundTile = oldData.value(TWITTER_ONTOLOGY_USER_PROFILEBACKGROUNDTILE);
    QVariant newProfileBackgroundTile = newData.value(TWITTER_ONTOLOGY_USER_PROFILEBACKGROUNDTILE);
    QVariant oldProfileBannerUrl = oldData.value(TWITTER_ONTOLOGY_USER_PROFILEBANNERURL);
    QVariant newProfileBannerUrl = newData.value(TWITTER_ONTOLOGY_USER_PROFILEBANNERURL);
    QVariant oldProfileImageUrl = oldData.value(TWITTER_ONTOLOGY_USER_PROFILEIMAGEURL);
    QVariant newProfileImageUrl = newData.value(TWITTER_ONTOLOGY_USER_PROFILEIMAGEURL);
    QVariant oldProfileImageUrlHttps = oldData.value(TWITTER_ONTOLOGY_USER_PROFILEIMAGEURLHTTPS);
    QVariant newProfileImageUrlHttps = newData.value(TWITTER_ONTOLOGY_USER_PROFILEIMAGEURLHTTPS);
    QVariant oldProfileLinkColor = oldData.value(TWITTER_ONTOLOGY_USER_PROFILELINKCOLOR);
    QVariant newProfileLinkColor = newData.value(TWITTER_ONTOLOGY_USER_PROFILELINKCOLOR);
    QVariant oldProfileSidebarBorderColor = oldData.value(TWITTER_ONTOLOGY_USER_PROFILESIDEBARBORDERCOLOR);
    QVariant newProfileSidebarBorderColor = newData.value(TWITTER_ONTOLOGY_USER_PROFILESIDEBARBORDERCOLOR);
    QVariant oldProfileSidebarFillColor = oldData.value(TWITTER_ONTOLOGY_USER_PROFILESIDEBARFILLCOLOR);
    QVariant newProfileSidebarFillColor = newData.value(TWITTER_ONTOLOGY_USER_PROFILESIDEBARFILLCOLOR);
    QVariant oldProfileTextColor = oldData.value(TWITTER_ONTOLOGY_USER_PROFILETEXTCOLOR);
    QVariant newProfileTextColor = newData.value(TWITTER_ONTOLOGY_USER_PROFILETEXTCOLOR);
    QVariant oldProfileUseBackgroundImage = oldData.value(TWITTER_ONTOLOGY_USER_PROFILEUSEBACKGROUNDIMAGE);
    QVariant newProfileUseBackgroundImage = newData.value(TWITTER_ONTOLOGY_USER_PROFILEUSEBACKGROUNDIMAGE);
    QVariant oldIsProtected = oldData.value(TWITTER_ONTOLOGY_USER_ISPROTECTED);
    QVariant newIsProtected = newData.value(TWITTER_ONTOLOGY_USER_ISPROTECTED);
    QVariant oldScreenName = oldData.value(TWITTER_ONTOLOGY_USER_SCREENNAME);
    QVariant newScreenName = newData.value(TWITTER_ONTOLOGY_USER_SCREENNAME);
    QVariant oldShowAllInlineMedia = oldData.value(TWITTER_ONTOLOGY_USER_SHOWALLINLINEMEDIA);
    QVariant newShowAllInlineMedia = newData.value(TWITTER_ONTOLOGY_USER_SHOWALLINLINEMEDIA);
    QVariant oldStatusesCount = oldData.value(TWITTER_ONTOLOGY_USER_STATUSESCOUNT);
    QVariant newStatusesCount = newData.value(TWITTER_ONTOLOGY_USER_STATUSESCOUNT);
    QVariant oldTimeZone = oldData.value(TWITTER_ONTOLOGY_USER_TIMEZONE);
    QVariant newTimeZone = newData.value(TWITTER_ONTOLOGY_USER_TIMEZONE);
    QVariant oldUrl = oldData.value(TWITTER_ONTOLOGY_USER_URL);
    QVariant newUrl = newData.value(TWITTER_ONTOLOGY_USER_URL);
    QVariant oldUtcOffset = oldData.value(TWITTER_ONTOLOGY_USER_UTCOFFSET);
    QVariant newUtcOffset = newData.value(TWITTER_ONTOLOGY_USER_UTCOFFSET);
    QVariant oldVerified = oldData.value(TWITTER_ONTOLOGY_USER_VERIFIED);
    QVariant newVerified = newData.value(TWITTER_ONTOLOGY_USER_VERIFIED);
    QVariant oldWithheldInCountries = oldData.value(TWITTER_ONTOLOGY_USER_WITHHELDINCOUNTRIES);
    QVariant newWithheldInCountries = newData.value(TWITTER_ONTOLOGY_USER_WITHHELDINCOUNTRIES);
    QVariant oldWithheldScope = oldData.value(TWITTER_ONTOLOGY_USER_WITHHELDSCOPE);
    QVariant newWithheldScope = newData.value(TWITTER_ONTOLOGY_USER_WITHHELDSCOPE);

    if (newContributorsEnabled != oldContributorsEnabled)
        emit q->contributorsEnabledChanged();
    if (newCreatedAt != oldCreatedAt)
        emit q->createdAtChanged();
    if (newDefaultProfile != oldDefaultProfile)
        emit q->defaultProfileChanged();
    if (newDefaultProfileImage != oldDefaultProfileImage)
        emit q->defaultProfileImageChanged();
    if (newDescription != oldDescription)
        emit q->descriptionChanged();
    if (newFavouritesCount != oldFavouritesCount)
        emit q->favouritesCountChanged();
    if (newFollowRequestSent != oldFollowRequestSent)
        emit q->followRequestSentChanged();
    if (newFollowersCount != oldFollowersCount)
        emit q->followersCountChanged();
    if (newFriendsCount != oldFriendsCount)
        emit q->friendsCountChanged();
    if (newGeoEnabled != oldGeoEnabled)
        emit q->geoEnabledChanged();
    if (newIsTranslator != oldIsTranslator)
        emit q->isTranslatorChanged();
    if (newLang != oldLang)
        emit q->langChanged();
    if (newListedCount != oldListedCount)
        emit q->listedCountChanged();
    if (newLocation != oldLocation)
        emit q->locationChanged();
    if (newName != oldName)
        emit q->nameChanged();
    if (newProfileBackgroundColor != oldProfileBackgroundColor)
        emit q->profileBackgroundColorChanged();
    if (newProfileBackgroundImageUrl != oldProfileBackgroundImageUrl)
        emit q->profileBackgroundImageUrlChanged();
    if (newProfileBackgroundImageUrlHttps != oldProfileBackgroundImageUrlHttps)
        emit q->profileBackgroundImageUrlHttpsChanged();
    if (newProfileBackgroundTile != oldProfileBackgroundTile)
        emit q->profileBackgroundTileChanged();
    if (newProfileBannerUrl != oldProfileBannerUrl)
        emit q->profileBannerUrlChanged();
    if (newProfileImageUrl != oldProfileImageUrl)
        emit q->profileImageUrlChanged();
    if (newProfileImageUrlHttps != oldProfileImageUrlHttps)
        emit q->profileImageUrlHttpsChanged();
    if (newProfileLinkColor != oldProfileLinkColor)
        emit q->profileLinkColorChanged();
    if (newProfileSidebarBorderColor != oldProfileSidebarBorderColor)
        emit q->profileSidebarBorderColorChanged();
    if (newProfileSidebarFillColor != oldProfileSidebarFillColor)
        emit q->profileSidebarFillColorChanged();
    if (newProfileTextColor != oldProfileTextColor)
        emit q->profileTextColorChanged();
    if (newProfileUseBackgroundImage != oldProfileUseBackgroundImage)
        emit q->profileUseBackgroundImageChanged();
    if (newIsProtected != oldIsProtected)
        emit q->isProtectedChanged();
    if (newScreenName != oldScreenName)
        emit q->screenNameChanged();
    if (newShowAllInlineMedia != oldShowAllInlineMedia)
        emit q->showAllInlineMediaChanged();
    if (newStatusesCount != oldStatusesCount)
        emit q->statusesCountChanged();
    if (newTimeZone != oldTimeZone)
        emit q->timeZoneChanged();
    if (newUrl != oldUrl)
        emit q->urlChanged();
    if (newUtcOffset != oldUtcOffset)
        emit q->utcOffsetChanged();
    if (newVerified != oldVerified)
        emit q->verifiedChanged();
    if (newWithheldInCountries != oldWithheldInCountries)
        emit q->withheldInCountriesChanged();
    if (newWithheldScope != oldWithheldScope)
        emit q->withheldScopeChanged();

    // Call super class implementation
    QVariantMap oldDataWithId = oldData;
    oldDataWithId.insert(NEMOQMLPLUGINS_SOCIAL_CONTENTITEMID,
                         oldData.value(TWITTER_ONTOLOGY_METADATA_ID));
    QVariantMap newDataWithId = newData;
    newDataWithId.insert(NEMOQMLPLUGINS_SOCIAL_CONTENTITEMID,
                         newData.value(TWITTER_ONTOLOGY_METADATA_ID));
    IdentifiableContentItemInterfacePrivate::emitPropertyChangeSignals(oldDataWithId, newDataWithId);
}

//-------------------------------

/*!
    \qmltype TwitterUser
    \instantiates TwitterUserInterface
    
*/
TwitterUserInterface::TwitterUserInterface(QObject *parent)
    : IdentifiableContentItemInterface(*(new TwitterUserInterfacePrivate(this)), parent)
{
}

/*! \reimp */
int TwitterUserInterface::type() const
{
    return TwitterInterface::User;
}

/*! \reimp */
bool TwitterUserInterface::remove()
{
// <<< remove
    // TODO Implement remove if needed
    return IdentifiableContentItemInterface::remove();
// >>> remove
}

/*! \reimp */
bool TwitterUserInterface::reload(const QStringList &whichFields)
{
// <<< reload
    Q_D(TwitterUserInterface);
    Q_UNUSED(whichFields)

    RequestInfo requestInfo = TwitterInterfacePrivate::requestUserInfo(d->identifier);
    if (!d->request(IdentifiableContentItemInterfacePrivate::Get, requestInfo.objectIdentifier,
                    requestInfo.extraPath, requestInfo.whichFields, requestInfo.postedData,
                    requestInfo.extraData)) {
        return false;
    }

    connect(d->reply(), SIGNAL(finished()), this, SLOT(reloadHandler()));
    d->connectErrors();
    return true;
// >>> reload
}

/*!
    \qmlmethod bool TwitterUser::uploadTweet(const QString &message, const QStringList &pathToMedias)
    */

bool TwitterUserInterface::uploadTweet(const QString &message, const QStringList &pathToMedias)
{
// <<< uploadTweet
    Q_D(TwitterUserInterface);
    TwitterInterface *twitterInterface = qobject_cast<TwitterInterface *>(socialNetwork());
    if (!twitterInterface) {
        qWarning() << Q_FUNC_INFO << "Cannot upload tweet without compatible social network";
        return false;
    }

    if (identifier() != twitterInterface->currentUserIdentifier()) {
        qWarning() << Q_FUNC_INFO << "Cannot upload tweet if the identifier of this user is "\
                      "different from the current user identifier defined in Twitter.";
        return false;
    }

    QString path;
    if (pathToMedias.isEmpty()) {
        path = TWITTER_ONTOLOGY_CONNECTION_STATUS_UPDATE;
    } else {
        qWarning() << Q_FUNC_INFO << "Uploading media is not supported yet";
        return false;
    }

    QVariantMap postData;
    postData.insert(TWITTER_ONTOLOGY_CONNECTION_STATUS_UPDATE_STATUS_KEY, message);
    bool requestMade = d->request(IdentifiableContentItemInterfacePrivate::Post,
                                  QString(), path, QStringList(), postData);

    if (!requestMade) {
        return false;
    }

    d->action = TwitterInterfacePrivate::TweetAction;
    d->connectFinishedAndErrors();
    return true;
// >>> uploadTweet
}

/*!
    \qmlproperty bool TwitterUser::contributorsEnabled
    
*/
bool TwitterUserInterface::contributorsEnabled() const
{
    Q_D(const TwitterUserInterface);
    return d->data().value(TWITTER_ONTOLOGY_USER_CONTRIBUTORSENABLED).toString() == QLatin1String("true");
}

/*!
    \qmlproperty QString TwitterUser::createdAt
    
*/
QString TwitterUserInterface::createdAt() const
{
    Q_D(const TwitterUserInterface);
    return d->data().value(TWITTER_ONTOLOGY_USER_CREATEDAT).toString();
}

/*!
    \qmlproperty bool TwitterUser::defaultProfile
    
*/
bool TwitterUserInterface::defaultProfile() const
{
    Q_D(const TwitterUserInterface);
    return d->data().value(TWITTER_ONTOLOGY_USER_DEFAULTPROFILE).toString() == QLatin1String("true");
}

/*!
    \qmlproperty bool TwitterUser::defaultProfileImage
    
*/
bool TwitterUserInterface::defaultProfileImage() const
{
    Q_D(const TwitterUserInterface);
    return d->data().value(TWITTER_ONTOLOGY_USER_DEFAULTPROFILEIMAGE).toString() == QLatin1String("true");
}

/*!
    \qmlproperty QString TwitterUser::description
    
*/
QString TwitterUserInterface::description() const
{
    Q_D(const TwitterUserInterface);
    return d->data().value(TWITTER_ONTOLOGY_USER_DESCRIPTION).toString();
}

/*!
    \qmlproperty int TwitterUser::favouritesCount
    
*/
int TwitterUserInterface::favouritesCount() const
{
    Q_D(const TwitterUserInterface);
    QString numberString = d->data().value(TWITTER_ONTOLOGY_USER_FAVOURITESCOUNT).toString();
    bool ok;
    int number = numberString.toInt(&ok);
    if (ok) {
        return number;
    }
    return -1;
}

/*!
    \qmlproperty bool TwitterUser::followRequestSent
    
*/
bool TwitterUserInterface::followRequestSent() const
{
    Q_D(const TwitterUserInterface);
    return d->data().value(TWITTER_ONTOLOGY_USER_FOLLOWREQUESTSENT).toString() == QLatin1String("true");
}

/*!
    \qmlproperty int TwitterUser::followersCount
    
*/
int TwitterUserInterface::followersCount() const
{
    Q_D(const TwitterUserInterface);
    QString numberString = d->data().value(TWITTER_ONTOLOGY_USER_FOLLOWERSCOUNT).toString();
    bool ok;
    int number = numberString.toInt(&ok);
    if (ok) {
        return number;
    }
    return -1;
}

/*!
    \qmlproperty int TwitterUser::friendsCount
    
*/
int TwitterUserInterface::friendsCount() const
{
    Q_D(const TwitterUserInterface);
    QString numberString = d->data().value(TWITTER_ONTOLOGY_USER_FRIENDSCOUNT).toString();
    bool ok;
    int number = numberString.toInt(&ok);
    if (ok) {
        return number;
    }
    return -1;
}

/*!
    \qmlproperty bool TwitterUser::geoEnabled
    
*/
bool TwitterUserInterface::geoEnabled() const
{
    Q_D(const TwitterUserInterface);
    return d->data().value(TWITTER_ONTOLOGY_USER_GEOENABLED).toString() == QLatin1String("true");
}

/*!
    \qmlproperty bool TwitterUser::isTranslator
    
*/
bool TwitterUserInterface::isTranslator() const
{
    Q_D(const TwitterUserInterface);
    return d->data().value(TWITTER_ONTOLOGY_USER_ISTRANSLATOR).toString() == QLatin1String("true");
}

/*!
    \qmlproperty QString TwitterUser::lang
    
*/
QString TwitterUserInterface::lang() const
{
    Q_D(const TwitterUserInterface);
    return d->data().value(TWITTER_ONTOLOGY_USER_LANG).toString();
}

/*!
    \qmlproperty int TwitterUser::listedCount
    
*/
int TwitterUserInterface::listedCount() const
{
    Q_D(const TwitterUserInterface);
    QString numberString = d->data().value(TWITTER_ONTOLOGY_USER_LISTEDCOUNT).toString();
    bool ok;
    int number = numberString.toInt(&ok);
    if (ok) {
        return number;
    }
    return -1;
}

/*!
    \qmlproperty QString TwitterUser::location
    
*/
QString TwitterUserInterface::location() const
{
    Q_D(const TwitterUserInterface);
    return d->data().value(TWITTER_ONTOLOGY_USER_LOCATION).toString();
}

/*!
    \qmlproperty QString TwitterUser::name
    
*/
QString TwitterUserInterface::name() const
{
    Q_D(const TwitterUserInterface);
    return d->data().value(TWITTER_ONTOLOGY_USER_NAME).toString();
}

/*!
    \qmlproperty QColor TwitterUser::profileBackgroundColor
    
*/
QColor TwitterUserInterface::profileBackgroundColor() const
{
    Q_D(const TwitterUserInterface);
    QString color = d->data().value(TWITTER_ONTOLOGY_USER_PROFILEBACKGROUNDCOLOR).toString();
    if (color.startsWith("#")) {
        return QColor(color);
    } else {
        color.prepend("#");
        return QColor(color);
    }
}

/*!
    \qmlproperty QUrl TwitterUser::profileBackgroundImageUrl
    
*/
QUrl TwitterUserInterface::profileBackgroundImageUrl() const
{
    Q_D(const TwitterUserInterface);
    return QUrl::fromEncoded(d->data().value(TWITTER_ONTOLOGY_USER_PROFILEBACKGROUNDIMAGEURL).toString().toLocal8Bit());
}

/*!
    \qmlproperty QUrl TwitterUser::profileBackgroundImageUrlHttps
    
*/
QUrl TwitterUserInterface::profileBackgroundImageUrlHttps() const
{
    Q_D(const TwitterUserInterface);
    return QUrl::fromEncoded(d->data().value(TWITTER_ONTOLOGY_USER_PROFILEBACKGROUNDIMAGEURLHTTPS).toString().toLocal8Bit());
}

/*!
    \qmlproperty bool TwitterUser::profileBackgroundTile
    
*/
bool TwitterUserInterface::profileBackgroundTile() const
{
    Q_D(const TwitterUserInterface);
    return d->data().value(TWITTER_ONTOLOGY_USER_PROFILEBACKGROUNDTILE).toString() == QLatin1String("true");
}

/*!
    \qmlproperty QUrl TwitterUser::profileBannerUrl
    
*/
QUrl TwitterUserInterface::profileBannerUrl() const
{
    Q_D(const TwitterUserInterface);
    return QUrl::fromEncoded(d->data().value(TWITTER_ONTOLOGY_USER_PROFILEBANNERURL).toString().toLocal8Bit());
}

/*!
    \qmlproperty QUrl TwitterUser::profileImageUrl
    
*/
QUrl TwitterUserInterface::profileImageUrl() const
{
    Q_D(const TwitterUserInterface);
    return QUrl::fromEncoded(d->data().value(TWITTER_ONTOLOGY_USER_PROFILEIMAGEURL).toString().toLocal8Bit());
}

/*!
    \qmlproperty QUrl TwitterUser::profileImageUrlHttps
    
*/
QUrl TwitterUserInterface::profileImageUrlHttps() const
{
    Q_D(const TwitterUserInterface);
    return QUrl::fromEncoded(d->data().value(TWITTER_ONTOLOGY_USER_PROFILEIMAGEURLHTTPS).toString().toLocal8Bit());
}

/*!
    \qmlproperty QColor TwitterUser::profileLinkColor
    
*/
QColor TwitterUserInterface::profileLinkColor() const
{
    Q_D(const TwitterUserInterface);
    QString color = d->data().value(TWITTER_ONTOLOGY_USER_PROFILELINKCOLOR).toString();
    if (color.startsWith("#")) {
        return QColor(color);
    } else {
        color.prepend("#");
        return QColor(color);
    }
}

/*!
    \qmlproperty QColor TwitterUser::profileSidebarBorderColor
    
*/
QColor TwitterUserInterface::profileSidebarBorderColor() const
{
    Q_D(const TwitterUserInterface);
    QString color = d->data().value(TWITTER_ONTOLOGY_USER_PROFILESIDEBARBORDERCOLOR).toString();
    if (color.startsWith("#")) {
        return QColor(color);
    } else {
        color.prepend("#");
        return QColor(color);
    }
}

/*!
    \qmlproperty QColor TwitterUser::profileSidebarFillColor
    
*/
QColor TwitterUserInterface::profileSidebarFillColor() const
{
    Q_D(const TwitterUserInterface);
    QString color = d->data().value(TWITTER_ONTOLOGY_USER_PROFILESIDEBARFILLCOLOR).toString();
    if (color.startsWith("#")) {
        return QColor(color);
    } else {
        color.prepend("#");
        return QColor(color);
    }
}

/*!
    \qmlproperty QColor TwitterUser::profileTextColor
    
*/
QColor TwitterUserInterface::profileTextColor() const
{
    Q_D(const TwitterUserInterface);
    QString color = d->data().value(TWITTER_ONTOLOGY_USER_PROFILETEXTCOLOR).toString();
    if (color.startsWith("#")) {
        return QColor(color);
    } else {
        color.prepend("#");
        return QColor(color);
    }
}

/*!
    \qmlproperty bool TwitterUser::profileUseBackgroundImage
    
*/
bool TwitterUserInterface::profileUseBackgroundImage() const
{
    Q_D(const TwitterUserInterface);
    return d->data().value(TWITTER_ONTOLOGY_USER_PROFILEUSEBACKGROUNDIMAGE).toString() == QLatin1String("true");
}

/*!
    \qmlproperty bool TwitterUser::isProtected
    
*/
bool TwitterUserInterface::isProtected() const
{
    Q_D(const TwitterUserInterface);
    return d->data().value(TWITTER_ONTOLOGY_USER_ISPROTECTED).toString() == QLatin1String("true");
}

/*!
    \qmlproperty QString TwitterUser::screenName
    
*/
QString TwitterUserInterface::screenName() const
{
    Q_D(const TwitterUserInterface);
    return d->data().value(TWITTER_ONTOLOGY_USER_SCREENNAME).toString();
}

/*!
    \qmlproperty bool TwitterUser::showAllInlineMedia
    
*/
bool TwitterUserInterface::showAllInlineMedia() const
{
    Q_D(const TwitterUserInterface);
    return d->data().value(TWITTER_ONTOLOGY_USER_SHOWALLINLINEMEDIA).toString() == QLatin1String("true");
}

/*!
    \qmlproperty int TwitterUser::statusesCount
    
*/
int TwitterUserInterface::statusesCount() const
{
    Q_D(const TwitterUserInterface);
    QString numberString = d->data().value(TWITTER_ONTOLOGY_USER_STATUSESCOUNT).toString();
    bool ok;
    int number = numberString.toInt(&ok);
    if (ok) {
        return number;
    }
    return -1;
}

/*!
    \qmlproperty QString TwitterUser::timeZone
    
*/
QString TwitterUserInterface::timeZone() const
{
    Q_D(const TwitterUserInterface);
    return d->data().value(TWITTER_ONTOLOGY_USER_TIMEZONE).toString();
}

/*!
    \qmlproperty QUrl TwitterUser::url
    
*/
QUrl TwitterUserInterface::url() const
{
    Q_D(const TwitterUserInterface);
    return QUrl::fromEncoded(d->data().value(TWITTER_ONTOLOGY_USER_URL).toString().toLocal8Bit());
}

/*!
    \qmlproperty int TwitterUser::utcOffset
    
*/
int TwitterUserInterface::utcOffset() const
{
    Q_D(const TwitterUserInterface);
    QString numberString = d->data().value(TWITTER_ONTOLOGY_USER_UTCOFFSET).toString();
    bool ok;
    int number = numberString.toInt(&ok);
    if (ok) {
        return number;
    }
    return -1;
}

/*!
    \qmlproperty bool TwitterUser::verified
    
*/
bool TwitterUserInterface::verified() const
{
    Q_D(const TwitterUserInterface);
    return d->data().value(TWITTER_ONTOLOGY_USER_VERIFIED).toString() == QLatin1String("true");
}

/*!
    \qmlproperty QString TwitterUser::withheldInCountries
    
*/
QString TwitterUserInterface::withheldInCountries() const
{
    Q_D(const TwitterUserInterface);
    return d->data().value(TWITTER_ONTOLOGY_USER_WITHHELDINCOUNTRIES).toString();
}

/*!
    \qmlproperty QString TwitterUser::withheldScope
    
*/
QString TwitterUserInterface::withheldScope() const
{
    Q_D(const TwitterUserInterface);
    return d->data().value(TWITTER_ONTOLOGY_USER_WITHHELDSCOPE).toString();
}

