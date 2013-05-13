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

#include "facebookuserinterface_p.h"
#include "facebookinterface.h"
#include "facebookontology_p.h"
// <<< include
#include <QtDebug>

/*
 * Doesn't expose (via API, although it does via data())
 * the following fields from the Facebook API:
 *     age_range
 *     currency
 *     devices
 *     education
 *     languages
 *     payment_pricepoints
 *     security_settings
 *     video_upload_limits
 *     work
 */
// >>> include

FacebookUserInterfacePrivate::FacebookUserInterfacePrivate(FacebookUserInterface *q)
    : IdentifiableContentItemInterfacePrivate(q)
    , action(FacebookInterfacePrivate::NoAction)
// <<< custom
    , gender(FacebookUserInterface::UnknownGender)
    , cover(0)
    , hometown(0)
    , location(0)
    , picture(0)
    , relationshipStatus(FacebookUserInterface::UnknownRelationshipStatus)
    , significantOther(0)
// >>> custom
{
}

void FacebookUserInterfacePrivate::finishedHandler()
{
// <<< finishedHandler
    Q_Q(FacebookUserInterface);
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
        case FacebookInterfacePrivate::DeletePhotoAction: // flow down.
        case FacebookInterfacePrivate::DeleteAlbumAction: {
            if (replyData == QString(QLatin1String("true"))) {
                status = SocialNetworkInterface::Idle;
                emit q->statusChanged();
                emit q->responseReceived(responseData);
            } else {
                error = SocialNetworkInterface::RequestError;
                errorMessage = QLatin1String("User: request failed");
                status = SocialNetworkInterface::Error;
                emit q->statusChanged();
                emit q->errorChanged();
                emit q->errorMessageChanged();
                emit q->responseReceived(responseData);
            }
        }
        break;

        case FacebookInterfacePrivate::UploadPhotoAction: // flow down.
        case FacebookInterfacePrivate::UploadAlbumAction: {
            if (!ok || responseData.value("id").toString().isEmpty()) {
                // failed.
                error = SocialNetworkInterface::RequestError;
                errorMessage = action == FacebookInterfacePrivate::UploadAlbumAction
                    ? QLatin1String("Album: add album request failed")
                    : QLatin1String("Album: add photo request failed");
                status = SocialNetworkInterface::Error;
                emit q->statusChanged();
                emit q->errorChanged();
                emit q->errorMessageChanged();
                emit q->responseReceived(responseData);
            } else {
                // succeeded.
                status = SocialNetworkInterface::Idle;
                emit q->statusChanged();
                emit q->responseReceived(responseData);
            }
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
void FacebookUserInterfacePrivate::emitPropertyChangeSignals(const QVariantMap &oldData,
                                                             const QVariantMap &newData)
{
    Q_Q(FacebookUserInterface);
    QVariant oldName = oldData.value(FACEBOOK_ONTOLOGY_USER_NAME);
    QVariant newName = newData.value(FACEBOOK_ONTOLOGY_USER_NAME);
    QVariant oldFirstName = oldData.value(FACEBOOK_ONTOLOGY_USER_FIRSTNAME);
    QVariant newFirstName = newData.value(FACEBOOK_ONTOLOGY_USER_FIRSTNAME);
    QVariant oldMiddleName = oldData.value(FACEBOOK_ONTOLOGY_USER_MIDDLENAME);
    QVariant newMiddleName = newData.value(FACEBOOK_ONTOLOGY_USER_MIDDLENAME);
    QVariant oldLastName = oldData.value(FACEBOOK_ONTOLOGY_USER_LASTNAME);
    QVariant newLastName = newData.value(FACEBOOK_ONTOLOGY_USER_LASTNAME);
    QVariant oldLocale = oldData.value(FACEBOOK_ONTOLOGY_USER_LOCALE);
    QVariant newLocale = newData.value(FACEBOOK_ONTOLOGY_USER_LOCALE);
    QVariant oldLink = oldData.value(FACEBOOK_ONTOLOGY_USER_LINK);
    QVariant newLink = newData.value(FACEBOOK_ONTOLOGY_USER_LINK);
    QVariant oldUsername = oldData.value(FACEBOOK_ONTOLOGY_USER_USERNAME);
    QVariant newUsername = newData.value(FACEBOOK_ONTOLOGY_USER_USERNAME);
    QVariant oldThirdPartyIdentifier = oldData.value(FACEBOOK_ONTOLOGY_USER_THIRDPARTYIDENTIFIER);
    QVariant newThirdPartyIdentifier = newData.value(FACEBOOK_ONTOLOGY_USER_THIRDPARTYIDENTIFIER);
    QVariant oldInstalled = oldData.value(FACEBOOK_ONTOLOGY_USER_INSTALLED);
    QVariant newInstalled = newData.value(FACEBOOK_ONTOLOGY_USER_INSTALLED);
    QVariant oldTimezone = oldData.value(FACEBOOK_ONTOLOGY_USER_TIMEZONE);
    QVariant newTimezone = newData.value(FACEBOOK_ONTOLOGY_USER_TIMEZONE);
    QVariant oldUpdatedTime = oldData.value(FACEBOOK_ONTOLOGY_USER_UPDATEDTIME);
    QVariant newUpdatedTime = newData.value(FACEBOOK_ONTOLOGY_USER_UPDATEDTIME);
    QVariant oldVerified = oldData.value(FACEBOOK_ONTOLOGY_USER_VERIFIED);
    QVariant newVerified = newData.value(FACEBOOK_ONTOLOGY_USER_VERIFIED);
    QVariant oldBio = oldData.value(FACEBOOK_ONTOLOGY_USER_BIO);
    QVariant newBio = newData.value(FACEBOOK_ONTOLOGY_USER_BIO);
    QVariant oldBirthday = oldData.value(FACEBOOK_ONTOLOGY_USER_BIRTHDAY);
    QVariant newBirthday = newData.value(FACEBOOK_ONTOLOGY_USER_BIRTHDAY);
    QVariant oldEmail = oldData.value(FACEBOOK_ONTOLOGY_USER_EMAIL);
    QVariant newEmail = newData.value(FACEBOOK_ONTOLOGY_USER_EMAIL);
    QVariant oldPolitical = oldData.value(FACEBOOK_ONTOLOGY_USER_POLITICAL);
    QVariant newPolitical = newData.value(FACEBOOK_ONTOLOGY_USER_POLITICAL);
    QVariant oldQuotes = oldData.value(FACEBOOK_ONTOLOGY_USER_QUOTES);
    QVariant newQuotes = newData.value(FACEBOOK_ONTOLOGY_USER_QUOTES);
    QVariant oldReligion = oldData.value(FACEBOOK_ONTOLOGY_USER_RELIGION);
    QVariant newReligion = newData.value(FACEBOOK_ONTOLOGY_USER_RELIGION);
    QVariant oldWebsite = oldData.value(FACEBOOK_ONTOLOGY_USER_WEBSITE);
    QVariant newWebsite = newData.value(FACEBOOK_ONTOLOGY_USER_WEBSITE);

    if (newName != oldName)
        emit q->nameChanged();
    if (newFirstName != oldFirstName)
        emit q->firstNameChanged();
    if (newMiddleName != oldMiddleName)
        emit q->middleNameChanged();
    if (newLastName != oldLastName)
        emit q->lastNameChanged();
    if (newLocale != oldLocale)
        emit q->localeChanged();
    if (newLink != oldLink)
        emit q->linkChanged();
    if (newUsername != oldUsername)
        emit q->usernameChanged();
    if (newThirdPartyIdentifier != oldThirdPartyIdentifier)
        emit q->thirdPartyIdentifierChanged();
    if (newInstalled != oldInstalled)
        emit q->installedChanged();
    if (newTimezone != oldTimezone)
        emit q->timezoneChanged();
    if (newUpdatedTime != oldUpdatedTime)
        emit q->updatedTimeChanged();
    if (newVerified != oldVerified)
        emit q->verifiedChanged();
    if (newBio != oldBio)
        emit q->bioChanged();
    if (newBirthday != oldBirthday)
        emit q->birthdayChanged();
    if (newEmail != oldEmail)
        emit q->emailChanged();
    if (newPolitical != oldPolitical)
        emit q->politicalChanged();
    if (newQuotes != oldQuotes)
        emit q->quotesChanged();
    if (newReligion != oldReligion)
        emit q->religionChanged();
    if (newWebsite != oldWebsite)
        emit q->websiteChanged();

// <<< emitPropertyChangeSignals
    // Handle gender
    QString oldGender = oldData.value(FACEBOOK_ONTOLOGY_USER_GENDER).toString().toLower();
    QString newGender = newData.value(FACEBOOK_ONTOLOGY_USER_GENDER).toString().toLower();
    if (newGender != oldGender) {
        if (newGender == "male") {
            gender = FacebookUserInterface::Male;
        } else if (newGender == "female") {
            gender = FacebookUserInterface::Female;
        }
        emit q->genderChanged();
    }

    // Handle cover
    QVariantMap oldCoverMap = oldData.value(FACEBOOK_ONTOLOGY_USER_COVER).toMap();
    QVariantMap newCoverMap = newData.value(FACEBOOK_ONTOLOGY_USER_COVER).toMap();

    if (newCoverMap != oldCoverMap) {
        qobject_cast<FacebookInterface*>(q->socialNetwork())->setFacebookContentItemData(cover, newCoverMap);
        emit q->coverChanged();
    }

    // Handle hometown
    QVariantMap oldHometownMap = oldData.value(FACEBOOK_ONTOLOGY_USER_HOMETOWN).toMap();
    QString oldHometownId = oldHometownMap.value(FACEBOOK_ONTOLOGY_OBJECTREFERENCE_OBJECTIDENTIFIER).toString();
    QString oldHometownName = oldHometownMap.value(FACEBOOK_ONTOLOGY_OBJECTREFERENCE_OBJECTNAME).toString();
    QVariantMap newHometownMap = newData.value(FACEBOOK_ONTOLOGY_USER_HOMETOWN).toMap();
    QString newHometownId = newHometownMap.value(FACEBOOK_ONTOLOGY_OBJECTREFERENCE_OBJECTIDENTIFIER).toString();
    QString newHometownName = newHometownMap.value(FACEBOOK_ONTOLOGY_OBJECTREFERENCE_OBJECTNAME).toString();

    if (newHometownId != oldHometownId || newHometownName != oldHometownName) {
        QVariantMap newHometownData;
        newHometownData.insert(FACEBOOK_ONTOLOGY_OBJECTREFERENCE_OBJECTTYPE, FacebookInterface::Location);
        newHometownData.insert(FACEBOOK_ONTOLOGY_OBJECTREFERENCE_OBJECTIDENTIFIER, newHometownId);
        newHometownData.insert(FACEBOOK_ONTOLOGY_OBJECTREFERENCE_OBJECTNAME, newHometownName);
        qobject_cast<FacebookInterface*>(q->socialNetwork())->setFacebookContentItemData(hometown, newHometownData);
        emit q->hometownChanged();
    }

    // Handle interested_in
    QStringList oldInterestedIn = oldData.value(FACEBOOK_ONTOLOGY_USER_INTERESTEDIN).toStringList();
    QStringList newInterestedIn = newData.value(FACEBOOK_ONTOLOGY_USER_INTERESTEDIN).toStringList();
    if (newInterestedIn != oldInterestedIn) {
        interestedIn = FacebookUserInterface::Genders();
        if (newInterestedIn.contains("male")) {
            interestedIn = interestedIn | FacebookUserInterface::Male;
        }
        if (newInterestedIn.contains("female")) {
            interestedIn = interestedIn | FacebookUserInterface::Female;
        }

        emit q->interestedInChanged();
    }

    // Handle location
    QVariantMap oldLocationMap = oldData.value(FACEBOOK_ONTOLOGY_USER_LOCATION).toMap();
    QString oldLocationId = oldLocationMap.value(FACEBOOK_ONTOLOGY_OBJECTREFERENCE_OBJECTIDENTIFIER).toString();
    QString oldLocationName = oldLocationMap.value(FACEBOOK_ONTOLOGY_OBJECTREFERENCE_OBJECTNAME).toString();
    QVariantMap newLocationMap = newData.value(FACEBOOK_ONTOLOGY_USER_LOCATION).toMap();
    QString newLocationId = newLocationMap.value(FACEBOOK_ONTOLOGY_OBJECTREFERENCE_OBJECTIDENTIFIER).toString();
    QString newLocationName = newLocationMap.value(FACEBOOK_ONTOLOGY_OBJECTREFERENCE_OBJECTNAME).toString();

    if (newLocationId != oldLocationId || newLocationName != oldLocationName) {
        QVariantMap newLocationData;
        newLocationData.insert(FACEBOOK_ONTOLOGY_OBJECTREFERENCE_OBJECTTYPE, FacebookInterface::Location);
        newLocationData.insert(FACEBOOK_ONTOLOGY_OBJECTREFERENCE_OBJECTIDENTIFIER, newLocationId);
        newLocationData.insert(FACEBOOK_ONTOLOGY_OBJECTREFERENCE_OBJECTNAME, newLocationName);
        qobject_cast<FacebookInterface*>(q->socialNetwork())->setFacebookContentItemData(location, newLocationData);
        emit q->locationChanged();
    }

    // Handle picture
    QVariantMap oldPictureMap = oldData.value(FACEBOOK_ONTOLOGY_USER_PICTURE).toMap().value(FACEBOOK_ONTOLOGY_METADATA_DATA).toMap();
    QVariantMap newPictureMap = newData.value(FACEBOOK_ONTOLOGY_USER_PICTURE).toMap().value(FACEBOOK_ONTOLOGY_METADATA_DATA).toMap();

    if (newPictureMap != oldPictureMap) {
        qobject_cast<FacebookInterface*>(q->socialNetwork())->setFacebookContentItemData(picture, newPictureMap);
        emit q->pictureChanged();
    }

    // Handle relationship_status
    QString oldRelationshipStatus = oldData.value(FACEBOOK_ONTOLOGY_USER_RELATIONSHIPSTATUS).toString().toLower();
    QString newRelationshipStatus = newData.value(FACEBOOK_ONTOLOGY_USER_RELATIONSHIPSTATUS).toString().toLower();
    if (newRelationshipStatus != oldRelationshipStatus) {
        if (newRelationshipStatus.toLower() == QLatin1String("single")) {
            relationshipStatus = FacebookUserInterface::Single;
        } else if (newRelationshipStatus.toLower() == QLatin1String("in a relationship")) {
            relationshipStatus = FacebookUserInterface::InARelationship;
        } else if (newRelationshipStatus.toLower() == QLatin1String("engaged")) {
            relationshipStatus = FacebookUserInterface::Engaged;
        } else if (newRelationshipStatus.toLower() == QLatin1String("married")) {
            relationshipStatus = FacebookUserInterface::Married;
        } else if (newRelationshipStatus.toLower() == QLatin1String("it's complicated")) {
            relationshipStatus = FacebookUserInterface::ItsComplicated;
        } else if (newRelationshipStatus.toLower() == QLatin1String("in an open relationship")) {
            relationshipStatus = FacebookUserInterface::InAnOpenRelationship;
        } else if (newRelationshipStatus.toLower() == QLatin1String("widowed")) {
            relationshipStatus = FacebookUserInterface::Widowed;
        } else if (newRelationshipStatus.toLower() == QLatin1String("separated")) {
            relationshipStatus = FacebookUserInterface::Separated;
        } else if (newRelationshipStatus.toLower() == QLatin1String("divorced")) {
            relationshipStatus = FacebookUserInterface::Divorced;
        } else if (newRelationshipStatus.toLower() == QLatin1String("in a civil union")) {
            relationshipStatus = FacebookUserInterface::InACivilUnion;
        } else if (newRelationshipStatus.toLower() == QLatin1String("in a domestic partnership")) {
            relationshipStatus = FacebookUserInterface::InADomesticPartnership;
        } else {
            relationshipStatus = FacebookUserInterface::UnknownRelationshipStatus;
        }
        emit q->relationshipStatusChanged();
    }
    // Handle significant_other
    QVariantMap oldSignificantOtherMap = oldData.value(FACEBOOK_ONTOLOGY_USER_SIGNIFICANTOTHER).toMap();
    QString oldSignificantOtherId = oldSignificantOtherMap.value(FACEBOOK_ONTOLOGY_OBJECTREFERENCE_OBJECTIDENTIFIER).toString();
    QString oldSignificantOtherName = oldSignificantOtherMap.value(FACEBOOK_ONTOLOGY_OBJECTREFERENCE_OBJECTNAME).toString();
    QVariantMap newSignificantOtherMap = newData.value(FACEBOOK_ONTOLOGY_USER_SIGNIFICANTOTHER).toMap();
    QString newSignificantOtherId = newSignificantOtherMap.value(FACEBOOK_ONTOLOGY_OBJECTREFERENCE_OBJECTIDENTIFIER).toString();
    QString newSignificantOtherName = newSignificantOtherMap.value(FACEBOOK_ONTOLOGY_OBJECTREFERENCE_OBJECTNAME).toString();

    if (newSignificantOtherId != oldSignificantOtherId || newSignificantOtherName != oldSignificantOtherName) {
        QVariantMap newSignificantOtherData;
        newSignificantOtherData.insert(FACEBOOK_ONTOLOGY_OBJECTREFERENCE_OBJECTTYPE, FacebookInterface::User);
        newSignificantOtherData.insert(FACEBOOK_ONTOLOGY_OBJECTREFERENCE_OBJECTIDENTIFIER, newSignificantOtherId);
        newSignificantOtherData.insert(FACEBOOK_ONTOLOGY_OBJECTREFERENCE_OBJECTNAME, newSignificantOtherName);
        qobject_cast<FacebookInterface*>(q->socialNetwork())->setFacebookContentItemData(significantOther, newSignificantOtherData);
        emit q->significantOtherChanged();
    }
// >>> emitPropertyChangeSignals

    // Call super class implementation
    QVariantMap oldDataWithId = oldData;
    oldDataWithId.insert(NEMOQMLPLUGINS_SOCIAL_CONTENTITEMID,
                         oldData.value(FACEBOOK_ONTOLOGY_METADATA_ID));
    QVariantMap newDataWithId = newData;
    newDataWithId.insert(NEMOQMLPLUGINS_SOCIAL_CONTENTITEMID,
                         newData.value(FACEBOOK_ONTOLOGY_METADATA_ID));
    IdentifiableContentItemInterfacePrivate::emitPropertyChangeSignals(oldDataWithId, newDataWithId);
}

//-------------------------------

/*!
    \qmltype FacebookUser
    \instantiates FacebookUserInterface
    l" user id
            filters: [ ContentItemTypeFilter { type: Facebook.Album } ]
        }
    }
    \endqml
    
    A FacebookUser may also be used "directly" by clients, in order to
    view specific information about the user or upload a new album.
    An example of direct usage of the FacebookUser type is as follows:
    
    \qml
    import QtQuick 1.1
    import org.nemomobile.social 1.0
    
    Item {
        id: root
        width: 400
        height: 800
    
        Facebook {
            id: fb
            accessToken: "your access token" // you must supply a valid access token
        }
    
        FacebookUser {
            id: fbu
            socialNetwork: fb
            identifier: "me" // some valid Facebook User fbid
    
            onStatusChanged: {
                if (status == SocialNetwork.Idle) {
                    // creates a new album, into which photos can be uploaded
                    uploadAlbum("World Cup Photos", "Photos taken at the world cup football event")
                }
            }
        }
    
        Text {
            anchors.fill: parent
            text: fbu.name // "name" field from the user's profile
        }
    }
    \endqml
*/
FacebookUserInterface::FacebookUserInterface(QObject *parent)
    : IdentifiableContentItemInterface(*(new FacebookUserInterfacePrivate(this)), parent)
{
// <<< constructor
    Q_D(FacebookUserInterface);
    d->cover = new FacebookUserCoverInterface(this);
    d->hometown = new FacebookObjectReferenceInterface(this);
    d->location = new FacebookObjectReferenceInterface(this);
    d->picture = new FacebookUserPictureInterface(this);
    d->significantOther = new FacebookObjectReferenceInterface(this);
// >>> constructor
}

/*! \reimp */
int FacebookUserInterface::type() const
{
    return FacebookInterface::User;
}

/*! \reimp */
bool FacebookUserInterface::remove()
{
// <<< remove
    return IdentifiableContentItemInterface::remove();
// >>> remove
}

/*! \reimp */
bool FacebookUserInterface::reload(const QStringList &whichFields)
{
// <<< reload
    return IdentifiableContentItemInterface::reload(whichFields);
// >>> reload
}

/*!
    \qmlmethod bool FacebookUser::uploadPhoto(const QUrl &source, const QString &message)
    Initiates a "post photo" operation on the user.  The photo will
    be loaded from the local filesystem and uploaded to Facebook with
    its caption set to the given \a message. It will be uploaded to
    the default album of the user.
    
    If the network request was started successfully, the function
    will return true and the status of the user will change to
    \c SocialNetwork::Busy.  Otherwise, the function will return
    false.
    
    Once the network request completes, the \c responseReceived()
    signal will be emitted.  The \c data parameter of the signal
    will contain the \c id of the newly uploaded photo.
    */

bool FacebookUserInterface::uploadPhoto(const QUrl &source, const QString &message)
{
// <<< uploadPhoto
    Q_D(FacebookUserInterface);
    // XXX TODO: privacy parameter?

    QVariantMap extraData; // image upload is handled specially by the facebook adapter.
    extraData.insert("isImageUpload", true);

    QVariantMap postData;
    postData.insert("source", source);
    if (!message.isEmpty())
        postData.insert("message", message);

    bool requestMade = d->request(IdentifiableContentItemInterfacePrivate::Post,
                                  identifier(), QLatin1String("photos"),
                                  QStringList(), postData, extraData);

    if (!requestMade)
        return false;

    d->action = FacebookInterfacePrivate::UploadPhotoAction;
    d->connectFinishedAndErrors();
    return true;
// >>> uploadPhoto
}
/*!
    \qmlmethod bool FacebookUser::removePhoto(const QString &photoIdentifier)
    Initiates a "delete photo" operation on the photo specified by
    the given \a identifier.
    
    If the network request was started successfully, the function
    will return true and the status of the user will change to
    \c SocialNetwork::Busy.  Otherwise, the function will return
    false.*/

bool FacebookUserInterface::removePhoto(const QString &photoIdentifier)
{
// <<< removePhoto
    Q_D(FacebookUserInterface);
    bool requestMade = d->request(IdentifiableContentItemInterfacePrivate::Delete, photoIdentifier);

    if (!requestMade)
        return false;

    d->action = FacebookInterfacePrivate::DeletePhotoAction;
    d->connectFinishedAndErrors();
    return true;
// >>> removePhoto
}
/*!
    \qmlmethod bool FacebookUser::uploadAlbum(const QString &name, const QString &message, const QVariantMap &privacy)
    Initiates a "post album" operation on the user.  The album
    will be created with the given \a name and be described by the
    given \a message, and will have the specified \a privacy.
    
    If the network request was started successfully, the function
    will return true and the status of the user will change to
    \c SocialNetwork::Busy.  Otherwise, the function will return
    false.
    
    Once the network request completes, the \c responseReceived()
    signal will be emitted.  The \c data parameter of the signal
    will contain the \c id of the newly uploaded album.*/

bool FacebookUserInterface::uploadAlbum(const QString &name, const QString &message, const QVariantMap &privacy)
{
// <<< uploadAlbum
    Q_D(FacebookUserInterface);
    QVariantMap postData;
    postData.insert("name", name);
    if (!message.isEmpty())
        postData.insert("message", message);
    if (privacy != QVariantMap())
        postData.insert("privacy", privacy);

    bool requestMade = d->request(IdentifiableContentItemInterfacePrivate::Post,
                                  identifier(), QLatin1String("albums"),
                                  QStringList(), postData);

    if (!requestMade)
        return false;

    d->action = FacebookInterfacePrivate::UploadAlbumAction;
    d->connectFinishedAndErrors();
    return true;
// >>> uploadAlbum
}
/*!
    \qmlmethod bool FacebookUser::removeAlbum(const QString &albumIdentifier)
    Initiates a "delete album" operation on the album specified by
    the given \a identifier.
    
    If the network request was started successfully, the function
    will return true and the status of the user will change to
    \c SocialNetwork::Busy.  Otherwise, the function will return
    false.*/

bool FacebookUserInterface::removeAlbum(const QString &albumIdentifier)
{
// <<< removeAlbum
    Q_D(FacebookUserInterface);
    bool requestMade = d->request(IdentifiableContentItemInterfacePrivate::Delete, albumIdentifier);

    if (!requestMade)
        return false;

    d->action = FacebookInterfacePrivate::DeleteAlbumAction;
    d->connectFinishedAndErrors();
    return true;
// >>> removeAlbum
}

/*!
    \qmlproperty QString FacebookUser::name
    Holds the full name of the user.
*/
QString FacebookUserInterface::name() const
{
    Q_D(const FacebookUserInterface);
    return d->data().value(FACEBOOK_ONTOLOGY_USER_NAME).toString();
}

/*!
    \qmlproperty QString FacebookUser::firstName
    Holds the first name of the user.
*/
QString FacebookUserInterface::firstName() const
{
    Q_D(const FacebookUserInterface);
    return d->data().value(FACEBOOK_ONTOLOGY_USER_FIRSTNAME).toString();
}

/*!
    \qmlproperty QString FacebookUser::middleName
    Holds the middle name of the user.
*/
QString FacebookUserInterface::middleName() const
{
    Q_D(const FacebookUserInterface);
    return d->data().value(FACEBOOK_ONTOLOGY_USER_MIDDLENAME).toString();
}

/*!
    \qmlproperty QString FacebookUser::lastName
    Holds the last name of the user.
*/
QString FacebookUserInterface::lastName() const
{
    Q_D(const FacebookUserInterface);
    return d->data().value(FACEBOOK_ONTOLOGY_USER_LASTNAME).toString();
}

/*!
    \qmlproperty FacebookUserInterface::Gender FacebookUser::gender
    Holds the gender of the user.
*/
FacebookUserInterface::Gender FacebookUserInterface::gender() const
{
    Q_D(const FacebookUserInterface);
    return d->gender;
}

/*!
    \qmlproperty QString FacebookUser::locale
    Holds the locale of the user.
*/
QString FacebookUserInterface::locale() const
{
    Q_D(const FacebookUserInterface);
    return d->data().value(FACEBOOK_ONTOLOGY_USER_LOCALE).toString();
}

/*!
    \qmlproperty QUrl FacebookUser::link
    Holds a link to the profile of the user.
*/
QUrl FacebookUserInterface::link() const
{
    Q_D(const FacebookUserInterface);
    return QUrl::fromEncoded(d->data().value(FACEBOOK_ONTOLOGY_USER_LINK).toString().toLocal8Bit());
}

/*!
    \qmlproperty QString FacebookUser::username
    Holds the username of the user.
*/
QString FacebookUserInterface::username() const
{
    Q_D(const FacebookUserInterface);
    return d->data().value(FACEBOOK_ONTOLOGY_USER_USERNAME).toString();
}

/*!
    \qmlproperty QString FacebookUser::thirdPartyIdentifier
    Holds the third party identifier of the user.
*/
QString FacebookUserInterface::thirdPartyIdentifier() const
{
    Q_D(const FacebookUserInterface);
    return d->data().value(FACEBOOK_ONTOLOGY_USER_THIRDPARTYIDENTIFIER).toString();
}

/*!
    \qmlproperty bool FacebookUser::installed
    Whether the user has installed the application associated with
    the access token which made the request to the Facebook service.
*/
bool FacebookUserInterface::installed() const
{
    Q_D(const FacebookUserInterface);
    return d->data().value(FACEBOOK_ONTOLOGY_USER_INSTALLED).toString() == QLatin1String("true");
}

/*!
    \qmlproperty float FacebookUser::timezone
    Holds the timezone of the user.
*/
float FacebookUserInterface::timezone() const
{
    Q_D(const FacebookUserInterface);
    QString numberString = d->data().value(FACEBOOK_ONTOLOGY_USER_TIMEZONE).toString();
    bool ok;
    float number = numberString.toFloat(&ok);
    if (ok) {
        return number;
    }
    return 0.;
}

/*!
    \qmlproperty QString FacebookUser::updatedTime
    Holds the last-update time of the user as an ISO8601-formatted string.
*/
QString FacebookUserInterface::updatedTime() const
{
    Q_D(const FacebookUserInterface);
    return d->data().value(FACEBOOK_ONTOLOGY_USER_UPDATEDTIME).toString();
}

/*!
    \qmlproperty bool FacebookUser::verified
    Whether the user has been verified.
*/
bool FacebookUserInterface::verified() const
{
    Q_D(const FacebookUserInterface);
    return d->data().value(FACEBOOK_ONTOLOGY_USER_VERIFIED).toString() == QLatin1String("true");
}

/*!
    \qmlproperty QString FacebookUser::bio
    Holds the biographical details of the user.
*/
QString FacebookUserInterface::bio() const
{
    Q_D(const FacebookUserInterface);
    return d->data().value(FACEBOOK_ONTOLOGY_USER_BIO).toString();
}

/*!
    \qmlproperty QString FacebookUser::birthday
    Holds the birthday of the user in MM/dd/YYYY format.
*/
QString FacebookUserInterface::birthday() const
{
    Q_D(const FacebookUserInterface);
    return d->data().value(FACEBOOK_ONTOLOGY_USER_BIRTHDAY).toString();
}

/*!
    \qmlproperty FacebookUserCoverInterface * FacebookUser::cover
    Holds the cover of the user.
*/
FacebookUserCoverInterface * FacebookUserInterface::cover() const
{
    Q_D(const FacebookUserInterface);
    return d->cover;
}

/*!
    \qmlproperty QString FacebookUser::email
    Holds the email address of the user.
*/
QString FacebookUserInterface::email() const
{
    Q_D(const FacebookUserInterface);
    return d->data().value(FACEBOOK_ONTOLOGY_USER_EMAIL).toString();
}

/*!
    \qmlproperty FacebookObjectReferenceInterface * FacebookUser::hometown
    Holds a reference to the place object which is the user's home town.
*/
FacebookObjectReferenceInterface * FacebookUserInterface::hometown() const
{
    Q_D(const FacebookUserInterface);
    return d->hometown;
}

/*!
    \qmlproperty FacebookUserInterface::Genders FacebookUser::interestedIn
    Holds a list of the user's personal gender preferences in the
    context of relationships.
*/
FacebookUserInterface::Genders FacebookUserInterface::interestedIn() const
{
    Q_D(const FacebookUserInterface);
    return d->interestedIn;
}

/*!
    \qmlproperty FacebookObjectReferenceInterface * FacebookUser::location
    Holds a reference to the place object which is the user's current domicile location.
*/
FacebookObjectReferenceInterface * FacebookUserInterface::location() const
{
    Q_D(const FacebookUserInterface);
    return d->location;
}

/*!
    \qmlproperty QString FacebookUser::political
    Holds the political views which the user identifies with.
*/
QString FacebookUserInterface::political() const
{
    Q_D(const FacebookUserInterface);
    return d->data().value(FACEBOOK_ONTOLOGY_USER_POLITICAL).toString();
}

/*!
    \qmlproperty FacebookUserPictureInterface * FacebookUser::picture
    Holds a reference to the picture associated with the user.
*/
FacebookUserPictureInterface * FacebookUserInterface::picture() const
{
    Q_D(const FacebookUserInterface);
    return d->picture;
}

/*!
    \qmlproperty QString FacebookUser::quotes
    Holds some of the user's favourite quotes.
*/
QString FacebookUserInterface::quotes() const
{
    Q_D(const FacebookUserInterface);
    return d->data().value(FACEBOOK_ONTOLOGY_USER_QUOTES).toString();
}

/*!
    \qmlproperty FacebookUserInterface::RelationshipStatus FacebookUser::relationshipStatus
    Holds the current relationship status of the user.
*/
FacebookUserInterface::RelationshipStatus FacebookUserInterface::relationshipStatus() const
{
    Q_D(const FacebookUserInterface);
    return d->relationshipStatus;
}

/*!
    \qmlproperty QString FacebookUser::religion
    Holds the religious views which the user identifies with.
*/
QString FacebookUserInterface::religion() const
{
    Q_D(const FacebookUserInterface);
    return d->data().value(FACEBOOK_ONTOLOGY_USER_RELIGION).toString();
}

/*!
    \qmlproperty FacebookObjectReferenceInterface * FacebookUser::significantOther
    Holds a reference to the person object which is listed as the user's
    significant other (spouse or partner).
*/
FacebookObjectReferenceInterface * FacebookUserInterface::significantOther() const
{
    Q_D(const FacebookUserInterface);
    return d->significantOther;
}

/*!
    \qmlproperty QUrl FacebookUser::website
    Holds a link the user's website.
*/
QUrl FacebookUserInterface::website() const
{
    Q_D(const FacebookUserInterface);
    return QUrl::fromEncoded(d->data().value(FACEBOOK_ONTOLOGY_USER_WEBSITE).toString().toLocal8Bit());
}

