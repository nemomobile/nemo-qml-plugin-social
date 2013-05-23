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

#include "facebookphotointerface_p.h"
#include "facebookontology_p.h"
#include "facebookinterface.h"
// <<< include
#include <QtCore/QDebug>
// >>> include

FacebookPhotoInterfacePrivate::FacebookPhotoInterfacePrivate(FacebookPhotoInterface *q)
    : IdentifiableContentItemInterfacePrivate(q)
    , action(FacebookInterfacePrivate::NoAction)
// <<< custom
    , from(0)
    , liked(false)
    , pendingTagToRemoveIndex(-1)
// >>> custom
{
}

void FacebookPhotoInterfacePrivate::finishedHandler()
{
// <<< finishedHandler
    Q_Q(FacebookPhotoInterface);
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
        case FacebookInterfacePrivate::LikeAction:       // flow
        case FacebookInterfacePrivate::DeleteLikeAction: // flow
        case FacebookInterfacePrivate::TagAction:        // flow
        case FacebookInterfacePrivate::DeleteTagAction:  // flow
        case FacebookInterfacePrivate::DeleteCommentAction: {
            if (replyData == QString(QLatin1String("true"))) {
                status = SocialNetworkInterface::Idle;
                if (action == FacebookInterfacePrivate::LikeAction) {
                    liked = true;
                    emit q->likedChanged();
                } else if (action == FacebookInterfacePrivate::DeleteLikeAction) {
                    liked = false;
                    emit q->likedChanged();
                } else if (action == FacebookInterfacePrivate::DeleteTagAction) {
                    if (pendingTagToRemoveIndex != -1) {
                        FacebookTagInterface *doomedTag = tags.takeAt(pendingTagToRemoveIndex);
                        pendingTagToRemoveIndex = -1;
                        doomedTag->deleteLater();
                        emit q->tagsChanged();
                    }
                }
                emit q->statusChanged();
                emit q->responseReceived(responseData);
            } else {
                if (pendingTagToRemoveIndex != -1)
                    pendingTagToRemoveIndex = -1;
                error = SocialNetworkInterface::RequestError;
                errorMessage = QLatin1String("Photo: request failed");
                status = SocialNetworkInterface::Error;
                emit q->statusChanged();
                emit q->errorChanged();
                emit q->errorMessageChanged();
                emit q->responseReceived(responseData);
            }
        }
        break;

        case FacebookInterfacePrivate::UploadCommentAction: {
            if (!ok || responseData.value("id").toString().isEmpty()) {
                // failed.
                error = SocialNetworkInterface::RequestError;
                errorMessage = QLatin1String("Photo: add comment request failed");
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
void FacebookPhotoInterfacePrivate::emitPropertyChangeSignals(const QVariantMap &oldData,
                                                              const QVariantMap &newData)
{
    Q_Q(FacebookPhotoInterface);
    QString oldAlbumIdentifier = oldData.value(FACEBOOK_ONTOLOGY_PHOTO_ALBUMIDENTIFIER).toString();
    QString newAlbumIdentifier = newData.value(FACEBOOK_ONTOLOGY_PHOTO_ALBUMIDENTIFIER).toString();
    QString oldName = oldData.value(FACEBOOK_ONTOLOGY_PHOTO_NAME).toString();
    QString newName = newData.value(FACEBOOK_ONTOLOGY_PHOTO_NAME).toString();
    QUrl oldIcon = oldData.value(FACEBOOK_ONTOLOGY_PHOTO_ICON).toUrl();
    QUrl newIcon = newData.value(FACEBOOK_ONTOLOGY_PHOTO_ICON).toUrl();
    QUrl oldPicture = oldData.value(FACEBOOK_ONTOLOGY_PHOTO_PICTURE).toUrl();
    QUrl newPicture = newData.value(FACEBOOK_ONTOLOGY_PHOTO_PICTURE).toUrl();
    QUrl oldSource = oldData.value(FACEBOOK_ONTOLOGY_PHOTO_SOURCE).toUrl();
    QUrl newSource = newData.value(FACEBOOK_ONTOLOGY_PHOTO_SOURCE).toUrl();
    int oldHeight = oldData.value(FACEBOOK_ONTOLOGY_PHOTO_HEIGHT).toInt();
    int newHeight = newData.value(FACEBOOK_ONTOLOGY_PHOTO_HEIGHT).toInt();
    int oldWidth = oldData.value(FACEBOOK_ONTOLOGY_PHOTO_WIDTH).toInt();
    int newWidth = newData.value(FACEBOOK_ONTOLOGY_PHOTO_WIDTH).toInt();
    QUrl oldLink = oldData.value(FACEBOOK_ONTOLOGY_PHOTO_LINK).toUrl();
    QUrl newLink = newData.value(FACEBOOK_ONTOLOGY_PHOTO_LINK).toUrl();
    QVariantMap oldPlace = oldData.value(FACEBOOK_ONTOLOGY_PHOTO_PLACE).toMap();
    QVariantMap newPlace = newData.value(FACEBOOK_ONTOLOGY_PHOTO_PLACE).toMap();
    QString oldCreatedTime = oldData.value(FACEBOOK_ONTOLOGY_PHOTO_CREATEDTIME).toString();
    QString newCreatedTime = newData.value(FACEBOOK_ONTOLOGY_PHOTO_CREATEDTIME).toString();
    QString oldUpdatedTime = oldData.value(FACEBOOK_ONTOLOGY_PHOTO_UPDATEDTIME).toString();
    QString newUpdatedTime = newData.value(FACEBOOK_ONTOLOGY_PHOTO_UPDATEDTIME).toString();
    int oldPosition = oldData.value(FACEBOOK_ONTOLOGY_PHOTO_POSITION).toInt();
    int newPosition = newData.value(FACEBOOK_ONTOLOGY_PHOTO_POSITION).toInt();

    if (newAlbumIdentifier != oldAlbumIdentifier)
        emit q->albumIdentifierChanged();
    if (newName != oldName)
        emit q->nameChanged();
    if (newIcon != oldIcon)
        emit q->iconChanged();
    if (newPicture != oldPicture)
        emit q->pictureChanged();
    if (newSource != oldSource)
        emit q->sourceChanged();
    if (newHeight != oldHeight)
        emit q->heightChanged();
    if (newWidth != oldWidth)
        emit q->widthChanged();
    if (newLink != oldLink)
        emit q->linkChanged();
    if (newPlace != oldPlace)
        emit q->placeChanged();
    if (newCreatedTime != oldCreatedTime)
        emit q->createdTimeChanged();
    if (newUpdatedTime != oldUpdatedTime)
        emit q->updatedTimeChanged();
    if (newPosition != oldPosition)
        emit q->positionChanged();

// <<< emitPropertyChangeSignals
    QVariantMap oldFromMap = oldData.value(FACEBOOK_ONTOLOGY_ALBUM_FROM).toMap();
    QString oldFromId = oldFromMap.value(FACEBOOK_ONTOLOGY_OBJECTREFERENCE_OBJECTIDENTIFIER).toString();
    QString oldFromName = oldFromMap.value(FACEBOOK_ONTOLOGY_OBJECTREFERENCE_OBJECTNAME).toString();
    QVariantMap newFromMap = newData.value(FACEBOOK_ONTOLOGY_ALBUM_FROM).toMap();
    QString newFromId = newFromMap.value(FACEBOOK_ONTOLOGY_OBJECTREFERENCE_OBJECTIDENTIFIER).toString();
    QString newFromName = newFromMap.value(FACEBOOK_ONTOLOGY_OBJECTREFERENCE_OBJECTNAME).toString();

    // Update the from object if required
    if (newFromId != oldFromId || newFromName != oldFromName) {
        QVariantMap newFromData;
        newFromData.insert(FACEBOOK_ONTOLOGY_OBJECTREFERENCE_OBJECTTYPE, FacebookInterface::User);
        newFromData.insert(FACEBOOK_ONTOLOGY_OBJECTREFERENCE_OBJECTIDENTIFIER, newFromId);
        newFromData.insert(FACEBOOK_ONTOLOGY_OBJECTREFERENCE_OBJECTNAME, newFromName);
        qobject_cast<FacebookInterface*>(q->socialNetwork())->setFacebookContentItemData(from, newFromData);
        emit q->fromChanged();
    }

    // Update tags list
    QVariantMap newTagsDataMap = newData.value(FACEBOOK_ONTOLOGY_PHOTO_TAGS).toMap();
    QVariantList newTagsList = newTagsDataMap.value(FACEBOOK_ONTOLOGY_METADATA_DATA).toList();
    QVariantMap oldTagsDataMap = oldData.value(FACEBOOK_ONTOLOGY_PHOTO_TAGS).toMap();
    QVariantList oldTagsList = oldTagsDataMap.value(FACEBOOK_ONTOLOGY_METADATA_DATA).toList();

    if (newTagsList != oldTagsList) {
        // Clear the old tags
        foreach (FacebookTagInterface *tag, tags) {
            tag->deleteLater();
        }
        tags.clear();

        // Update with the new tag data
        foreach (QVariant tag, newTagsList) {
            QVariantMap tagMap = tag.toMap();
            FacebookTagInterface *tagInterface = new FacebookTagInterface(q);
            qobject_cast<FacebookInterface*>(q->socialNetwork())->setFacebookContentItemData(tagInterface, tagMap);
            tags.append(tagInterface);
        }

        // Emit change signal
        emit q->tagsChanged();
    }

    // Update name_tags list
    // This entry is tricky (not to say "/me vomit")
    // If there is only one tag, the name_tag property holds a list of lists with one entry
    // If there are more than one tag, the name_tag property is an object, with keys corresponding
    // to the offset, and values being lists containing just one object
    QVariant newNameTagsData = newData.value(FACEBOOK_ONTOLOGY_PHOTO_NAMETAGS);
    QVariant oldNameTagsData = oldData.value(FACEBOOK_ONTOLOGY_PHOTO_NAMETAGS);

    if (newNameTagsData != oldNameTagsData) {
        // Clear the old name_tags
        foreach (FacebookNameTagInterface *nameTag, nameTags) {
            nameTag->deleteLater();
        }
        nameTags.clear();
        // Update with the new name_tag data
        // Only one entry
        if (newNameTagsData.type() == QVariant::List) {
            QVariantList newNameTagsList = newNameTagsData.toList();
            if (newNameTagsList.count() == 1) {
                QVariantList nameTagList = newNameTagsList.at(0).toList();
                foreach (QVariant nameTag, nameTagList) {
                    QVariantMap nameTagMap = nameTag.toMap();
                    FacebookNameTagInterface *nameTagInterface = new FacebookNameTagInterface(q);
                    qobject_cast<FacebookInterface*>(q->socialNetwork())->setFacebookContentItemData(nameTagInterface, nameTagMap);
                    nameTags.append(nameTagInterface);
                }
            }
        } else if (newNameTagsData.type() == QVariant::Map) {
            foreach (QVariant nameTagList, newNameTagsData.toMap()) {
                foreach (QVariant nameTag, nameTagList.toList()) {
                    QVariantMap nameTagMap = nameTag.toMap();
                    FacebookNameTagInterface *nameTagInterface = new FacebookNameTagInterface(q);
                    qobject_cast<FacebookInterface*>(q->socialNetwork())->setFacebookContentItemData(nameTagInterface, nameTagMap);
                    nameTags.append(nameTagInterface);
                }
            }
        }

        // Emit change signal
        emit q->nameTagsChanged();
    }

    // Update images list
    QVariantList newImagesList = newData.value(FACEBOOK_ONTOLOGY_PHOTO_IMAGES).toList();
    QVariantList oldImagesList = oldData.value(FACEBOOK_ONTOLOGY_PHOTO_IMAGES).toList();

    if (newImagesList != oldImagesList) {
        // Clear the old images
        foreach (FacebookImageInterface *image, images) {
            image->deleteLater();
        }
        images.clear();

        // Update with the new tag data
        foreach (QVariant image, newImagesList) {
            QVariantMap imageMap = image.toMap();
            FacebookImageInterface *imageInterface = new FacebookImageInterface(q);
            qobject_cast<FacebookInterface*>(q->socialNetwork())->setFacebookContentItemData(imageInterface, imageMap);
            images.append(imageInterface);
        }

        // Emit change signal
        emit q->imagesChanged();
    }

    // Check if the user liked this photo
    QString currentUserIdentifier
            = qobject_cast<FacebookInterface*>(q->socialNetwork())->currentUserIdentifier();
    bool newLiked = false;
    QVariant likes = newData.value(FACEBOOK_ONTOLOGY_CONNECTIONS_LIKES);
    if (!likes.isNull()) {
        QVariantMap likesMap = likes.toMap();
        QVariantList data = likesMap.value(FACEBOOK_ONTOLOGY_METADATA_DATA).toList();
        foreach (QVariant dataEntry, data) {
            QVariant idVariant
                    = dataEntry.toMap().value(FACEBOOK_ONTOLOGY_OBJECTREFERENCE_OBJECTIDENTIFIER);
            if (idVariant.toString() == currentUserIdentifier) {
                newLiked = true;
            }
        }
    }

    if (liked != newLiked) {
        liked = newLiked;
        emit q->likedChanged();
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
void FacebookPhotoInterfacePrivate::tags_append(QDeclarativeListProperty<FacebookTagInterface> *list,
                                                FacebookTagInterface *data)
{
    FacebookPhotoInterface *interface = qobject_cast<FacebookPhotoInterface *>(list->object);
    if (interface) {
        data->setParent(interface);
        interface->d_func()->tags.append(data);
    }
}

FacebookTagInterface * FacebookPhotoInterfacePrivate::tags_at(QDeclarativeListProperty<FacebookTagInterface> *list,
                                                              int index)
{
    FacebookPhotoInterface *interface = qobject_cast<FacebookPhotoInterface *>(list->object);
    if (interface
        && index < interface->d_func()->tags.count()
        && index >= 0) {
        return interface->d_func()->tags.at(index);
    }
    return 0;
}

void FacebookPhotoInterfacePrivate::tags_clear(QDeclarativeListProperty<FacebookTagInterface> *list)
{
    FacebookPhotoInterface *interface = qobject_cast<FacebookPhotoInterface *>(list->object);
    if (interface) {
        foreach (FacebookTagInterface *entry, interface->d_func()->tags) {
            entry->deleteLater();
        }
        interface->d_func()->tags.clear();
    }
}

int FacebookPhotoInterfacePrivate::tags_count(QDeclarativeListProperty<FacebookTagInterface> *list)
{
    FacebookPhotoInterface *interface = qobject_cast<FacebookPhotoInterface *>(list->object);
    if (interface) {
        return interface->d_func()->tags.count();
    }
    return 0;
}

void FacebookPhotoInterfacePrivate::name_tags_append(QDeclarativeListProperty<FacebookNameTagInterface> *list,
                                                     FacebookNameTagInterface *data)
{
    FacebookPhotoInterface *interface = qobject_cast<FacebookPhotoInterface *>(list->object);
    if (interface) {
        data->setParent(interface);
        interface->d_func()->nameTags.append(data);
    }
}

FacebookNameTagInterface * FacebookPhotoInterfacePrivate::name_tags_at(QDeclarativeListProperty<FacebookNameTagInterface> *list,
                                                                       int index)
{
    FacebookPhotoInterface *interface = qobject_cast<FacebookPhotoInterface *>(list->object);
    if (interface
        && index < interface->d_func()->nameTags.count()
        && index >= 0) {
        return interface->d_func()->nameTags.at(index);
    }
    return 0;
}

void FacebookPhotoInterfacePrivate::name_tags_clear(QDeclarativeListProperty<FacebookNameTagInterface> *list)
{
    FacebookPhotoInterface *interface = qobject_cast<FacebookPhotoInterface *>(list->object);
    if (interface) {
        foreach (FacebookNameTagInterface *entry, interface->d_func()->nameTags) {
            entry->deleteLater();
        }
        interface->d_func()->nameTags.clear();
    }
}

int FacebookPhotoInterfacePrivate::name_tags_count(QDeclarativeListProperty<FacebookNameTagInterface> *list)
{
    FacebookPhotoInterface *interface = qobject_cast<FacebookPhotoInterface *>(list->object);
    if (interface) {
        return interface->d_func()->nameTags.count();
    }
    return 0;
}

void FacebookPhotoInterfacePrivate::images_append(QDeclarativeListProperty<FacebookImageInterface> *list,
                                                  FacebookImageInterface *data)
{
    FacebookPhotoInterface *interface = qobject_cast<FacebookPhotoInterface *>(list->object);
    if (interface) {
        data->setParent(interface);
        interface->d_func()->images.append(data);
    }
}

FacebookImageInterface * FacebookPhotoInterfacePrivate::images_at(QDeclarativeListProperty<FacebookImageInterface> *list,
                                                                  int index)
{
    FacebookPhotoInterface *interface = qobject_cast<FacebookPhotoInterface *>(list->object);
    if (interface
        && index < interface->d_func()->images.count()
        && index >= 0) {
        return interface->d_func()->images.at(index);
    }
    return 0;
}

void FacebookPhotoInterfacePrivate::images_clear(QDeclarativeListProperty<FacebookImageInterface> *list)
{
    FacebookPhotoInterface *interface = qobject_cast<FacebookPhotoInterface *>(list->object);
    if (interface) {
        foreach (FacebookImageInterface *entry, interface->d_func()->images) {
            entry->deleteLater();
        }
        interface->d_func()->images.clear();
    }
}

int FacebookPhotoInterfacePrivate::images_count(QDeclarativeListProperty<FacebookImageInterface> *list)
{
    FacebookPhotoInterface *interface = qobject_cast<FacebookPhotoInterface *>(list->object);
    if (interface) {
        return interface->d_func()->images.count();
    }
    return 0;
}


//-------------------------------

/*!
    \qmltype FacebookPhoto
    \instantiates FacebookPhotoInterface
    \inqmlmodule org.nemomobile.social 1
    \brief A FacebookPhoto represents a Photo object from the Facebook OpenGraph API
    
    Every FacebookPhoto has a unique identifier, and thus a photo may be
    set as the \c node (or central content item) in the Facebook
    adapter.  The content items related to a photo include various
    likes and comments.
    
    An example of usage of a FacebookPhoto as the node in a Facebook
    model is as follows:
    
    \qml
    import QtQuick 1.1
    import org.nemomobile.social 1.0
    
    Item {
        id: root
        width: 400
        height: 800
    
        Flickable {
            anchors.top: parent.verticalCenter
            anchors.bottom: parent.bottom
            anchors.left: parent.left
            anchors.right: parent.right
    
            ListView {
                model: fb
                anchors.fill: parent
                delegate: Label { text: "id: " + contentItemIdentifier } // Comment ids
            }
        }
    
        Facebook {
            id: fb
            accessToken: "your access token"    // you must supply a valid access token
            nodeIdentifier: "10150146071966729" // some valid Facebook photo id.
            filters: [ ContentItemTypeFilter { type: Facebook.Comment } ]
        }
    
        Component.onCompleted: {
            fb.populate()
        }
    }
    \endqml
    
    A FacebookPhoto may also be used "directly" by clients, in order to
    upload comments, or like the photo.  An example of direct
    usage of the FacebookPhoto type is as follows:
    
    \qml
    import QtQuick 1.1
    import org.nemomobile.social 1.0
    
    Item {
        id: root
        width: 400
        height: 800
    
        Facebook {
            id: fb
            accessToken: "your access token"    // you must supply a valid access token
        }
    
        FacebookPhoto {
            id: fbph
            socialNetwork: fb
            identifier: "10150146071966729"     // some valid Facebook Photo fbid
    
            onStatusChanged: {
                if (status == SocialNetwork.Idle) {
                    // could comment on the photo
                    fbph.uploadComment("I really like this photo!")
                    // could like the photo
                    fbph.like()
                    // could unlike the photo
                    fbph.unlike()
                }
            }
        }
    
        Image {
            anchors.fill: parent
            source: fbph.source
        }
    }
    \endqml
*/
FacebookPhotoInterface::FacebookPhotoInterface(QObject *parent)
    : IdentifiableContentItemInterface(*(new FacebookPhotoInterfacePrivate(this)), parent)
{
// <<< constructor
    Q_D(FacebookPhotoInterface);
    d->from = new FacebookObjectReferenceInterface(this);
// >>> constructor
}

/*! \reimp */
int FacebookPhotoInterface::type() const
{
    return FacebookInterface::Photo;
}

/*! \reimp */
bool FacebookPhotoInterface::remove()
{
// <<< remove
    return IdentifiableContentItemInterface::remove();
// >>> remove
}

/*! \reimp */
bool FacebookPhotoInterface::reload(const QStringList &whichFields)
{
// <<< reload
    return IdentifiableContentItemInterface::reload(whichFields);
// >>> reload
}

/*!
    \qmlmethod bool FacebookPhotoInterface::like()
    Initiates a "like" operation on the photo.
    
    If the network request was started successfully, the function
    will return true and the status of the photo will change to
    \c SocialNetwork::Busy.  Otherwise, the function will return
    false.*/

bool FacebookPhotoInterface::like()
{
// <<< like
    Q_D(FacebookPhotoInterface);
    bool requestMade = d->request(IdentifiableContentItemInterfacePrivate::Post,
                                  identifier(), QLatin1String("likes"));

    if (!requestMade)
        return false;

    d->action = FacebookInterfacePrivate::LikeAction;
    d->connectFinishedAndErrors();
    return true;
// >>> like
}
/*!
    \qmlmethod bool FacebookPhotoInterface::unlike()
    Initiates a "delete like" operation on the photo.
    
    If the network request was started successfully, the function
    will return true and the status of the photo will change to
    \c SocialNetwork::Busy.  Otherwise, the function will return
    false.*/

bool FacebookPhotoInterface::unlike()
{
// <<< unlike
    Q_D(FacebookPhotoInterface);
    bool requestMade = d->request(IdentifiableContentItemInterfacePrivate::Delete,
                                  identifier(), QLatin1String("likes"));

    if (!requestMade)
        return false;

    d->action = FacebookInterfacePrivate::DeleteLikeAction;
    d->connectFinishedAndErrors();
    return true;
// >>> unlike
}
/*!
    \qmlmethod bool FacebookPhotoInterface::tagUser(const QString &userId, float xOffset, float yOffset)
    Initiates a "tag user" operation on the photo.  The user specified
    by the given \a userId will be tagged into the photo at the position
    specified by the given \a xOffset and \a yOffset.
    
    If the network request was started successfully, the function
    will return true and the status of the photo will change to
    \c SocialNetwork::Busy.  Otherwise, the function will return
    false.
    
    Once the network request completes, the \c responseReceived()
    signal will be emitted.*/

bool FacebookPhotoInterface::tagUser(const QString &userId, float xOffset, float yOffset)
{
// <<< tagUser
    Q_D(FacebookPhotoInterface);
    QVariantMap postData;
    postData.insert("id", userId);
    if (xOffset != -1)
        postData.insert("x", QString::number(xOffset));
    if (yOffset != -1)
        postData.insert("y", QString::number(yOffset));

    bool requestMade = d->request(IdentifiableContentItemInterfacePrivate::Post,
                                  identifier(), QLatin1String("tags"),
                                  QStringList(), postData);

    if (!requestMade)
        return false;

    d->action = FacebookInterfacePrivate::TagAction;
    d->connectFinishedAndErrors();
    return true;
// >>> tagUser
}
/*!
    \qmlmethod bool FacebookPhotoInterface::untagUser(const QString &userId)
    Initiates a "delete tag" operation on the tag which tags the
    user specified by the given \a userId into the photo.
    
    If the network request was started successfully, the function
    will return true and the status of the photo will change to
    \c SocialNetwork::Busy.  Otherwise, the function will return
    false.*/

bool FacebookPhotoInterface::untagUser(const QString &userId)
{
// <<< untagUser
    Q_D(FacebookPhotoInterface);
    QVariantMap extraData;
    extraData.insert("to", userId);

    // try to find which tag will be removed if it succeeds
    int tempPendingTagToRemoveIndex = -1;
    for (int i = 0; i < d->tags.count(); ++i) {
        QString tagUserIdentifier = d->tags.at(i)->identifier();
        if (!tagUserIdentifier.isEmpty() && tagUserIdentifier == userId) {
            tempPendingTagToRemoveIndex = i;
            break;
        }
    }

    // possible that it's ok to not exist, since we might be out of sync with reality.
    if (tempPendingTagToRemoveIndex == -1)
        qWarning() << Q_FUNC_INFO << "Unknown tag specified for removal";

    bool requestMade = d->request(IdentifiableContentItemInterfacePrivate::Delete,
                                  identifier(), QLatin1String("tags"),
                                  QStringList(), QVariantMap(), extraData);

    if (!requestMade)
        return false;

    d->action = FacebookInterfacePrivate::DeleteTagAction;
    d->pendingTagToRemoveIndex = tempPendingTagToRemoveIndex;
    d->connectFinishedAndErrors();
    return true;
// >>> untagUser
}
/*!
    \qmlmethod bool FacebookPhotoInterface::tagText(const QString &text, float xOffset, float yOffset)
    Initiates a "tag text" operation on the photo.  The position
    specified by the given \a xOffset and \a yOffset will be tagged
    with the specified \a text.
    
    If the network request was started successfully, the function
    will return true and the status of the photo will change to
    \c SocialNetwork::Busy.  Otherwise, the function will return
    false.
    
    Once the network request completes, the \c responseReceived()
    signal will be emitted.*/

bool FacebookPhotoInterface::tagText(const QString &text, float xOffset, float yOffset)
{
// <<< tagText
    Q_D(FacebookPhotoInterface);
    QVariantMap postData;
    postData.insert("tag_text", text);
    if (xOffset != -1)
        postData.insert("x", QString::number(xOffset));
    if (yOffset != -1)
        postData.insert("y", QString::number(yOffset));

    bool requestMade = d->request(IdentifiableContentItemInterfacePrivate::Post,
                                  identifier(), QLatin1String("tags"),
                                  QStringList(), postData);

    if (!requestMade)
        return false;

    d->action = FacebookInterfacePrivate::TagAction;
    d->connectFinishedAndErrors();
    return true;
// >>> tagText
}
/*!
    \qmlmethod bool FacebookPhotoInterface::untagText(const QString &text)
    Initiates a "delete tag" operation on the tag specified by
    the given text.
    
    If the network request was started successfully, the function
    will return true and the status of the photo will change to
    \c SocialNetwork::Busy.  Otherwise, the function will return
    false.*/

bool FacebookPhotoInterface::untagText(const QString &text)
{
// <<< untagText
    Q_D(FacebookPhotoInterface);
    QVariantMap extraData;
    extraData.insert("tag_text", text);

    // try to find which tag will be removed if it succeeds
    int tempPendingTagToRemoveIndex = -1;
    for (int i = 0; i < d->tags.count(); ++i) {
        QString tagName = d->tags.at(i)->name();
        if (!tagName.isEmpty() && tagName == text) {
            tempPendingTagToRemoveIndex = i;
            break;
        }
    }

    // possible that it's ok to not exist, since we might be out of sync with reality.
    if (tempPendingTagToRemoveIndex == -1)
        qWarning() << Q_FUNC_INFO << "Unknown tag specified for removal";

    bool requestMade = d->request(IdentifiableContentItemInterfacePrivate::Delete,
                                  identifier(), QLatin1String("tags"),
                                  QStringList(), QVariantMap(), extraData);

    if (!requestMade)
        return false;

    d->action = FacebookInterfacePrivate::DeleteTagAction;
    d->pendingTagToRemoveIndex = tempPendingTagToRemoveIndex;
    d->connectFinishedAndErrors();
    return true;
// >>> untagText
}
/*!
    \qmlmethod bool FacebookPhotoInterface::uploadComment(const QString &message)
    Initiates a "post comment" operation on the photo.  The comment
    will contain the specified \a message.
    
    If the network request was started successfully, the function
    will return true and the status of the photo will change to
    \c SocialNetwork::Busy.  Otherwise, the function will return
    false.
    
    Once the network request completes, the \c responseReceived()
    signal will be emitted.  The \c data parameter of the signal
    will contain the \c id of the newly uploaded comment.*/

bool FacebookPhotoInterface::uploadComment(const QString &message)
{
// <<< uploadComment
    Q_D(FacebookPhotoInterface);
    QVariantMap postData;
    postData.insert("message", message);

    bool requestMade = d->request(IdentifiableContentItemInterfacePrivate::Post,
                                  identifier(), QLatin1String("comments"),
                                  QStringList(), postData);

    if (!requestMade)
        return false;

    d->action = FacebookInterfacePrivate::UploadCommentAction;
    d->connectFinishedAndErrors();
    return true;
// >>> uploadComment
}
/*!
    \qmlmethod bool FacebookPhotoInterface::removeComment(const QString &commentIdentifier)
    Initiates a "delete comment" operation on the comment specified by
    the given \a identifier.
    
    If the network request was started successfully, the function
    will return true and the status of the photo will change to
    \c SocialNetwork::Busy.  Otherwise, the function will return
    false.*/

bool FacebookPhotoInterface::removeComment(const QString &commentIdentifier)
{
// <<< removeComment
    Q_D(FacebookPhotoInterface);
    bool requestMade = d->request(IdentifiableContentItemInterfacePrivate::Delete,
                                  commentIdentifier);

    if (!requestMade)
        return false;

    d->action = FacebookInterfacePrivate::DeleteCommentAction;
    d->connectFinishedAndErrors();
    return true;
// >>> removeComment
}

/*!
    \qmlproperty QString FacebookPhotoInterface::albumIdentifier
    Holds the identifier of the album to which this photo belongs.
    Note: this property is currently not implemented.
    XXX TODO: implement this.
*/
QString FacebookPhotoInterface::albumIdentifier() const
{
    Q_D(const FacebookPhotoInterface);
    return d->data().value(FACEBOOK_ONTOLOGY_PHOTO_ALBUMIDENTIFIER).toString();
}

/*!
    \qmlproperty FacebookObjectReferenceInterface * FacebookPhotoInterface::from
    Holds a reference to the user or profile which uploaded this photo.
*/
FacebookObjectReferenceInterface * FacebookPhotoInterface::from() const
{
    Q_D(const FacebookPhotoInterface);
    return d->from;
}

/*!
    \qmlproperty QDeclarativeListProperty<FacebookTagInterface> FacebookPhotoInterface::tags
    Holds the tags which have been uploaded for this photo
*/
QDeclarativeListProperty<FacebookTagInterface> FacebookPhotoInterface::tags()
{
    return QDeclarativeListProperty<FacebookTagInterface>(
                this, 0,
                &FacebookPhotoInterfacePrivate::tags_append,
                &FacebookPhotoInterfacePrivate::tags_count,
                &FacebookPhotoInterfacePrivate::tags_at,
                &FacebookPhotoInterfacePrivate::tags_clear);
}

/*!
    \qmlproperty QString FacebookPhotoInterface::name
    Holds the name (caption) of the photo
*/
QString FacebookPhotoInterface::name() const
{
    Q_D(const FacebookPhotoInterface);
    return d->data().value(FACEBOOK_ONTOLOGY_PHOTO_NAME).toString();
}

/*!
    \qmlproperty QDeclarativeListProperty<FacebookNameTagInterface> FacebookPhotoInterface::nameTags
    Holds the names of various tagged entities
*/
QDeclarativeListProperty<FacebookNameTagInterface> FacebookPhotoInterface::nameTags()
{
    return QDeclarativeListProperty<FacebookNameTagInterface>(
                this, 0,
                &FacebookPhotoInterfacePrivate::name_tags_append,
                &FacebookPhotoInterfacePrivate::name_tags_count,
                &FacebookPhotoInterfacePrivate::name_tags_at,
                &FacebookPhotoInterfacePrivate::name_tags_clear);
}

/*!
    \qmlproperty QUrl FacebookPhotoInterface::icon
    Holds a url to the icon for the photo
*/
QUrl FacebookPhotoInterface::icon() const
{
    Q_D(const FacebookPhotoInterface);
    return d->data().value(FACEBOOK_ONTOLOGY_PHOTO_ICON).toUrl();
}

/*!
    \qmlproperty QUrl FacebookPhotoInterface::picture
    Holds a url to the picture for the photo
*/
QUrl FacebookPhotoInterface::picture() const
{
    Q_D(const FacebookPhotoInterface);
    return d->data().value(FACEBOOK_ONTOLOGY_PHOTO_PICTURE).toUrl();
}

/*!
    \qmlproperty QUrl FacebookPhotoInterface::source
    Holds a url to the source for the photo, full size
*/
QUrl FacebookPhotoInterface::source() const
{
    Q_D(const FacebookPhotoInterface);
    return d->data().value(FACEBOOK_ONTOLOGY_PHOTO_SOURCE).toUrl();
}

/*!
    \qmlproperty int FacebookPhotoInterface::height
    Holds the height of the photo
*/
int FacebookPhotoInterface::height() const
{
    Q_D(const FacebookPhotoInterface);
    return d->data().value(FACEBOOK_ONTOLOGY_PHOTO_HEIGHT).toInt();
}

/*!
    \qmlproperty int FacebookPhotoInterface::width
    Holds the width of the photo
*/
int FacebookPhotoInterface::width() const
{
    Q_D(const FacebookPhotoInterface);
    return d->data().value(FACEBOOK_ONTOLOGY_PHOTO_WIDTH).toInt();
}

/*!
    \qmlproperty QDeclarativeListProperty<FacebookImageInterface> FacebookPhotoInterface::images
    Holds links to and metadata about scaled versions of the photo
*/
QDeclarativeListProperty<FacebookImageInterface> FacebookPhotoInterface::images()
{
    return QDeclarativeListProperty<FacebookImageInterface>(
                this, 0,
                &FacebookPhotoInterfacePrivate::images_append,
                &FacebookPhotoInterfacePrivate::images_count,
                &FacebookPhotoInterfacePrivate::images_at,
                &FacebookPhotoInterfacePrivate::images_clear);
}

/*!
    \qmlproperty QUrl FacebookPhotoInterface::link
    Holds a url to the photo which may be used as an external link.
    Note that this link url contains the album identifier embedded
    within it.
*/
QUrl FacebookPhotoInterface::link() const
{
    Q_D(const FacebookPhotoInterface);
    return d->data().value(FACEBOOK_ONTOLOGY_PHOTO_LINK).toUrl();
}

/*!
    \qmlproperty QVariantMap FacebookPhotoInterface::place
    Holds information about the place associated with the photo.
    Note: this property will change in the future to return
    an object reference or location reference. (TODO)
*/
QVariantMap FacebookPhotoInterface::place() const
{
    Q_D(const FacebookPhotoInterface);
    return d->data().value(FACEBOOK_ONTOLOGY_PHOTO_PLACE).toMap();
}

/*!
    \qmlproperty QString FacebookPhotoInterface::createdTime
    Holds the creation time of the photo in an ISO8601-formatted string.
*/
QString FacebookPhotoInterface::createdTime() const
{
    Q_D(const FacebookPhotoInterface);
    return d->data().value(FACEBOOK_ONTOLOGY_PHOTO_CREATEDTIME).toString();
}

/*!
    \qmlproperty QString FacebookPhotoInterface::updatedTime
    Holds the last-update time of the photo in an ISO8601-formatted string.
*/
QString FacebookPhotoInterface::updatedTime() const
{
    Q_D(const FacebookPhotoInterface);
    return d->data().value(FACEBOOK_ONTOLOGY_PHOTO_UPDATEDTIME).toString();
}

/*!
    \qmlproperty int FacebookPhotoInterface::position
    Holds the position of the photo within the album.
*/
int FacebookPhotoInterface::position() const
{
    Q_D(const FacebookPhotoInterface);
    return d->data().value(FACEBOOK_ONTOLOGY_PHOTO_POSITION).toInt();
}

/*!
    \qmlproperty bool FacebookPhotoInterface::liked
    Whether the photo has been liked by the current user.
*/
bool FacebookPhotoInterface::liked() const
{
    Q_D(const FacebookPhotoInterface);
    return d->liked;
}

