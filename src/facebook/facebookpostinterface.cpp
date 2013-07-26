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

#include "facebookpostinterface_p.h"
#include "facebookinterface.h"
#include "facebookontology_p.h"
// <<< include
#include <QtCore/QtDebug>
// >>> include

FacebookPostInterfacePrivate::FacebookPostInterfacePrivate(FacebookPostInterface *q)
    : IdentifiableContentItemInterfacePrivate(q)
    , action(FacebookInterfacePrivate::NoAction)
// <<< custom
    , from(0)
    , application(0)
    , likesCount(-1)
    , commentsCount(-1)
// >>> custom
{
}

void FacebookPostInterfacePrivate::finishedHandler()
{
// <<< finishedHandler
    Q_Q(FacebookPostInterface);
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
        case FacebookInterfacePrivate::LikeAction:        // flow down.
        case FacebookInterfacePrivate::DeleteLikeAction:  // flow down.
        case FacebookInterfacePrivate::DeleteCommentAction: {
            if (replyData == QString(QLatin1String("true"))) {
                status = SocialNetworkInterface::Idle;
                if (action == FacebookInterfacePrivate::LikeAction) {
                    liked = true;
                    emit q->likedChanged();
                } else if (action == FacebookInterfacePrivate::DeleteLikeAction) {
                    liked = false;
                    emit q->likedChanged();
                }
                emit q->statusChanged();
                emit q->responseReceived(responseData);
            } else {
                error = SocialNetworkInterface::RequestError;
                errorMessage = QLatin1String("Post: request failed");
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
                errorMessage = QLatin1String("Post: add comment request failed");
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
void FacebookPostInterfacePrivate::emitPropertyChangeSignals(const QVariantMap &oldData,
                                                             const QVariantMap &newData)
{
    Q_Q(FacebookPostInterface);
    QVariant oldMessage = oldData.value(FACEBOOK_ONTOLOGY_POST_MESSAGE);
    QVariant newMessage = newData.value(FACEBOOK_ONTOLOGY_POST_MESSAGE);
    QVariant oldPicture = oldData.value(FACEBOOK_ONTOLOGY_POST_PICTURE);
    QVariant newPicture = newData.value(FACEBOOK_ONTOLOGY_POST_PICTURE);
    QVariant oldLink = oldData.value(FACEBOOK_ONTOLOGY_POST_LINK);
    QVariant newLink = newData.value(FACEBOOK_ONTOLOGY_POST_LINK);
    QVariant oldName = oldData.value(FACEBOOK_ONTOLOGY_POST_NAME);
    QVariant newName = newData.value(FACEBOOK_ONTOLOGY_POST_NAME);
    QVariant oldCaption = oldData.value(FACEBOOK_ONTOLOGY_POST_CAPTION);
    QVariant newCaption = newData.value(FACEBOOK_ONTOLOGY_POST_CAPTION);
    QVariant oldDescription = oldData.value(FACEBOOK_ONTOLOGY_POST_DESCRIPTION);
    QVariant newDescription = newData.value(FACEBOOK_ONTOLOGY_POST_DESCRIPTION);
    QVariant oldSource = oldData.value(FACEBOOK_ONTOLOGY_POST_SOURCE);
    QVariant newSource = newData.value(FACEBOOK_ONTOLOGY_POST_SOURCE);
    QVariant oldIcon = oldData.value(FACEBOOK_ONTOLOGY_POST_ICON);
    QVariant newIcon = newData.value(FACEBOOK_ONTOLOGY_POST_ICON);
    QVariant oldPostType = oldData.value(FACEBOOK_ONTOLOGY_POST_POSTTYPE);
    QVariant newPostType = newData.value(FACEBOOK_ONTOLOGY_POST_POSTTYPE);
    QVariant oldStory = oldData.value(FACEBOOK_ONTOLOGY_POST_STORY);
    QVariant newStory = newData.value(FACEBOOK_ONTOLOGY_POST_STORY);
    QVariant oldObjectIdentifier = oldData.value(FACEBOOK_ONTOLOGY_POST_OBJECTIDENTIFIER);
    QVariant newObjectIdentifier = newData.value(FACEBOOK_ONTOLOGY_POST_OBJECTIDENTIFIER);
    QVariant oldCreatedTime = oldData.value(FACEBOOK_ONTOLOGY_POST_CREATEDTIME);
    QVariant newCreatedTime = newData.value(FACEBOOK_ONTOLOGY_POST_CREATEDTIME);
    QVariant oldUpdatedTime = oldData.value(FACEBOOK_ONTOLOGY_POST_UPDATEDTIME);
    QVariant newUpdatedTime = newData.value(FACEBOOK_ONTOLOGY_POST_UPDATEDTIME);
    QVariant oldShares = oldData.value(FACEBOOK_ONTOLOGY_POST_SHARES);
    QVariant newShares = newData.value(FACEBOOK_ONTOLOGY_POST_SHARES);
    QVariant oldHidden = oldData.value(FACEBOOK_ONTOLOGY_POST_HIDDEN);
    QVariant newHidden = newData.value(FACEBOOK_ONTOLOGY_POST_HIDDEN);
    QVariant oldStatusType = oldData.value(FACEBOOK_ONTOLOGY_POST_STATUSTYPE);
    QVariant newStatusType = newData.value(FACEBOOK_ONTOLOGY_POST_STATUSTYPE);

    if (newMessage != oldMessage)
        emit q->messageChanged();
    if (newPicture != oldPicture)
        emit q->pictureChanged();
    if (newLink != oldLink)
        emit q->linkChanged();
    if (newName != oldName)
        emit q->nameChanged();
    if (newCaption != oldCaption)
        emit q->captionChanged();
    if (newDescription != oldDescription)
        emit q->descriptionChanged();
    if (newSource != oldSource)
        emit q->sourceChanged();
    if (newIcon != oldIcon)
        emit q->iconChanged();
    if (newPostType != oldPostType)
        emit q->postTypeChanged();
    if (newStory != oldStory)
        emit q->storyChanged();
    if (newObjectIdentifier != oldObjectIdentifier)
        emit q->objectIdentifierChanged();
    if (newCreatedTime != oldCreatedTime)
        emit q->createdTimeChanged();
    if (newUpdatedTime != oldUpdatedTime)
        emit q->updatedTimeChanged();
    if (newShares != oldShares)
        emit q->sharesChanged();
    if (newHidden != oldHidden)
        emit q->hiddenChanged();
    if (newStatusType != oldStatusType)
        emit q->statusTypeChanged();

// <<< emitPropertyChangeSignals
    QVariantMap oldFromMap = oldData.value(FACEBOOK_ONTOLOGY_POST_FROM).toMap();
    QString oldFromId = oldFromMap.value(FACEBOOK_ONTOLOGY_OBJECTREFERENCE_OBJECTIDENTIFIER).toString();
    QString oldFromName = oldFromMap.value(FACEBOOK_ONTOLOGY_OBJECTREFERENCE_OBJECTNAME).toString();
    QVariantMap newFromMap = newData.value(FACEBOOK_ONTOLOGY_POST_FROM).toMap();
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

    // Update to
    QVariantMap newToMap = newData.value(FACEBOOK_ONTOLOGY_POST_TO).toMap();
    QVariantMap oldToMap = oldData.value(FACEBOOK_ONTOLOGY_POST_TO).toMap();

    if (newToMap != oldToMap) {
        QVariantList newToList = newToMap.value(FACEBOOK_ONTOLOGY_METADATA_DATA).toList();

        // Clear the old to
        foreach (FacebookObjectReferenceInterface *toEntry, to) {
            toEntry->deleteLater();
        }
        to.clear();

        // Update with the new tag data
        foreach (QVariant toEntry, newToList) {
            QVariantMap toMap = toEntry.toMap();
            FacebookObjectReferenceInterface *toInterface = new FacebookObjectReferenceInterface(q);
            toMap.insert(FACEBOOK_ONTOLOGY_OBJECTREFERENCE_OBJECTTYPE, FacebookInterface::User);
            qobject_cast<FacebookInterface*>(q->socialNetwork())->setFacebookContentItemData(toInterface, toMap);
            to.append(toInterface);
        }

        // Emit change signal
        emit q->toChanged();
    }

    // Update message_tags
    // It is a map that at each offset associates a list with one entry which contains the name tag
    QVariantMap newMessageTagsMap = newData.value(FACEBOOK_ONTOLOGY_POST_MESSAGETAGS).toMap();
    QVariantMap oldMessageTagsMap = oldData.value(FACEBOOK_ONTOLOGY_POST_MESSAGETAGS).toMap();

    if (newMessageTagsMap != oldMessageTagsMap) {
        // Clear the old message_tags
        foreach (FacebookNameTagInterface *messageTag, messageTags) {
            messageTag->deleteLater();
        }
        messageTags.clear();

        // Update with the new message_tags data
        foreach (QString offset, newMessageTagsMap.keys()) {
            QVariantMap messageTagMap
                    = newMessageTagsMap.value(offset).toList().first().toMap();
            FacebookNameTagInterface *messageTagInterface = new FacebookNameTagInterface(q);
            qobject_cast<FacebookInterface*>(q->socialNetwork())->setFacebookContentItemData(messageTagInterface, messageTagMap);
            messageTags.append(messageTagInterface);
        }

        // Emit change signal
        emit q->messageTagsChanged();
    }

    // Update properties
    QVariantList newPropertiesList = newData.value(FACEBOOK_ONTOLOGY_POST_PROPERTIES).toList();
    QVariantList oldPropertiesList = oldData.value(FACEBOOK_ONTOLOGY_POST_PROPERTIES).toList();

    if (newPropertiesList != oldPropertiesList) {
        // Clear the old properties
        foreach (FacebookPostPropertyInterface *property, properties) {
            property->deleteLater();
        }
        properties.clear();

        // Update with the new properties data
        foreach (QVariant property, newPropertiesList) {
            QVariantMap propertyMap = property.toMap();
            FacebookPostPropertyInterface *propertyInterface = new FacebookPostPropertyInterface(q);
            qobject_cast<FacebookInterface*>(q->socialNetwork())->setFacebookContentItemData(propertyInterface, propertyMap);
            properties.append(propertyInterface);
        }

        // Emit change signal
        emit q->propertiesChanged();
    }

    // Update actions
    QVariantList newActionsList = newData.value(FACEBOOK_ONTOLOGY_POST_ACTIONS).toList();
    QVariantList oldActionsList = oldData.value(FACEBOOK_ONTOLOGY_POST_ACTIONS).toList();

    if (newActionsList != oldActionsList) {
        // Clear the old actions
        foreach (FacebookPostActionInterface *action, actions) {
            action->deleteLater();
        }
        actions.clear();

        // Update with the new actions data
        foreach (QVariant action, newActionsList) {
            QVariantMap actionMap = action.toMap();
            FacebookPostActionInterface *actionInterface = new FacebookPostActionInterface(q);
            qobject_cast<FacebookInterface*>(q->socialNetwork())->setFacebookContentItemData(actionInterface, actionMap);
            actions.append(actionInterface);
        }

        // Emit change signal
        emit q->actionsChanged();
    }


    // Update story_tags
    // It is a map that at each offset associates a list with one entry which contains the name tag
    QVariantMap newStoryTagsMap = newData.value(FACEBOOK_ONTOLOGY_POST_STORYTAGS).toMap();
    QVariantMap oldStoryTagsMap = oldData.value(FACEBOOK_ONTOLOGY_POST_STORYTAGS).toMap();

    if (newStoryTagsMap != oldStoryTagsMap) {
        // Clear the old story_tags
        foreach (FacebookNameTagInterface *storyTag, storyTags) {
            storyTag->deleteLater();
        }
        storyTags.clear();

        // Update with the new message_tags data
        foreach (QString offset, newStoryTagsMap.keys()) {
            QVariantMap storyTagMap = newStoryTagsMap.value(offset).toList().first().toMap();
            FacebookNameTagInterface *storyTagInterface = new FacebookNameTagInterface(q);
            qobject_cast<FacebookInterface*>(q->socialNetwork())->setFacebookContentItemData(storyTagInterface, storyTagMap);
            storyTags.append(storyTagInterface);
        }

        // Emit change signal
        emit q->storyTagsChanged();
    }


    // Update with_tags
    QVariantMap newWithTagsMap = newData.value(FACEBOOK_ONTOLOGY_POST_WITHTAGS).toMap();
    QVariantMap oldWithTagsMap = oldData.value(FACEBOOK_ONTOLOGY_POST_WITHTAGS).toMap();

    if (newWithTagsMap != oldWithTagsMap) {
        QVariantList newWithTagsList = newToMap.value(FACEBOOK_ONTOLOGY_METADATA_DATA).toList();

        // Clear the old with_tags
        foreach (FacebookObjectReferenceInterface *withTagsEntry, withTags) {
            withTagsEntry->deleteLater();
        }
        withTags.clear();

        // Update with the new tag data
        foreach (QVariant withTagsEntry, newWithTagsList) {
            QVariantMap withTagsMap = withTagsEntry.toMap();
            FacebookObjectReferenceInterface *withtagsInterface = new FacebookObjectReferenceInterface(q);
            withTagsMap.insert(FACEBOOK_ONTOLOGY_OBJECTREFERENCE_OBJECTTYPE, FacebookInterface::User);
            qobject_cast<FacebookInterface*>(q->socialNetwork())->setFacebookContentItemData(withtagsInterface, withTagsMap);
            to.append(withtagsInterface);
        }

        // Emit change signal
        emit q->withTagsChanged();
    }

    QVariantMap oldApplicationMap = oldData.value(FACEBOOK_ONTOLOGY_POST_APPLICATION).toMap();
    QString oldApplicationId = oldApplicationMap.value(FACEBOOK_ONTOLOGY_OBJECTREFERENCE_OBJECTIDENTIFIER).toString();
    QString oldApplicationName = oldApplicationMap.value(FACEBOOK_ONTOLOGY_OBJECTREFERENCE_OBJECTNAME).toString();
    QVariantMap newApplicationMap = newData.value(FACEBOOK_ONTOLOGY_POST_APPLICATION).toMap();
    QString newApplicationId = newApplicationMap.value(FACEBOOK_ONTOLOGY_OBJECTREFERENCE_OBJECTIDENTIFIER).toString();
    QString newApplicationName = newApplicationMap.value(FACEBOOK_ONTOLOGY_OBJECTREFERENCE_OBJECTNAME).toString();

    // Update the Application object if required
    if (newApplicationId != oldApplicationId || newApplicationName != oldApplicationName) {
        QVariantMap newApplicationData;
        newApplicationData.insert(FACEBOOK_ONTOLOGY_OBJECTREFERENCE_OBJECTTYPE, FacebookInterface::Application);
        newApplicationData.insert(FACEBOOK_ONTOLOGY_OBJECTREFERENCE_OBJECTIDENTIFIER, newApplicationId);
        newApplicationData.insert(FACEBOOK_ONTOLOGY_OBJECTREFERENCE_OBJECTNAME, newApplicationName);
        qobject_cast<FacebookInterface*>(q->socialNetwork())->setFacebookContentItemData(application, newApplicationData);
        emit q->applicationChanged();
    }

    // Check if we are in the second phase (getting info about likes and comments)
    bool isSecondPhase = newData.contains(FACEBOOK_ONTOLOGY_METADATA_SECONDPHASE);

    // Check if the user liked this post and infos about likes
    QString currentUserIdentifier
            = qobject_cast<FacebookInterface*>(q->socialNetwork())->currentUserIdentifier();
    bool newLiked = false;
    int newLikesCount = isSecondPhase ? 0 : -1;
    QVariant likes = newData.value(FACEBOOK_ONTOLOGY_CONNECTIONS_LIKES);
    if (!likes.isNull()) {
        QVariantMap likesMap = likes.toMap();
        QVariantList data = likesMap.value(FACEBOOK_ONTOLOGY_METADATA_DATA).toList();

        // Get summary
        QVariantMap summary = likesMap.value(FACEBOOK_ONTOLOGY_METADATA_SUMMARY).toMap();
        bool ok;
        int castedLikesCount = summary.value(FACEBOOK_ONTOLOGY_METADATA_TOTALCOUNT).toInt(&ok);
        if (ok) {
            newLikesCount = castedLikesCount;
        }

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

    if (likesCount != newLikesCount) {
        likesCount = newLikesCount;
        emit q->likesCountChanged();
    }

    // Check infos about comments
    int newCommentsCount = isSecondPhase ? 0 : -1;
    QVariant comments = newData.value(FACEBOOK_ONTOLOGY_CONNECTIONS_COMMENTS);
    if (!comments.isNull()) {
        QVariantMap commentsMap = comments.toMap();

        // Get summary
        QVariantMap summary = commentsMap.value(FACEBOOK_ONTOLOGY_METADATA_SUMMARY).toMap();
        bool ok;
        int castedCommentCount = summary.value(FACEBOOK_ONTOLOGY_METADATA_TOTALCOUNT).toInt(&ok);
        if (ok) {
            newCommentsCount = castedCommentCount;
        }
    }

    if (commentsCount != newCommentsCount) {
        commentsCount = newCommentsCount;
        emit q->commentsCountChanged();
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
void FacebookPostInterfacePrivate::to_append(QDeclarativeListProperty<FacebookObjectReferenceInterface> *list,
                                             FacebookObjectReferenceInterface *data)
{
    FacebookPostInterface *interface = qobject_cast<FacebookPostInterface *>(list->object);
    if (interface) {
        data->setParent(interface);
        interface->d_func()->to.append(data);
    }
}

FacebookObjectReferenceInterface * FacebookPostInterfacePrivate::to_at(QDeclarativeListProperty<FacebookObjectReferenceInterface> *list,
                                                                       int index)
{
    FacebookPostInterface *interface = qobject_cast<FacebookPostInterface *>(list->object);
    if (interface
        && index < interface->d_func()->to.count()
        && index >= 0) {
        return interface->d_func()->to.at(index);
    }
    return 0;
}

void FacebookPostInterfacePrivate::to_clear(QDeclarativeListProperty<FacebookObjectReferenceInterface> *list)
{
    FacebookPostInterface *interface = qobject_cast<FacebookPostInterface *>(list->object);
    if (interface) {
        foreach (FacebookObjectReferenceInterface *entry, interface->d_func()->to) {
            entry->deleteLater();
        }
        interface->d_func()->to.clear();
    }
}

int FacebookPostInterfacePrivate::to_count(QDeclarativeListProperty<FacebookObjectReferenceInterface> *list)
{
    FacebookPostInterface *interface = qobject_cast<FacebookPostInterface *>(list->object);
    if (interface) {
        return interface->d_func()->to.count();
    }
    return 0;
}

void FacebookPostInterfacePrivate::message_tags_append(QDeclarativeListProperty<FacebookNameTagInterface> *list,
                                                       FacebookNameTagInterface *data)
{
    FacebookPostInterface *interface = qobject_cast<FacebookPostInterface *>(list->object);
    if (interface) {
        data->setParent(interface);
        interface->d_func()->messageTags.append(data);
    }
}

FacebookNameTagInterface * FacebookPostInterfacePrivate::message_tags_at(QDeclarativeListProperty<FacebookNameTagInterface> *list,
                                                                         int index)
{
    FacebookPostInterface *interface = qobject_cast<FacebookPostInterface *>(list->object);
    if (interface
        && index < interface->d_func()->messageTags.count()
        && index >= 0) {
        return interface->d_func()->messageTags.at(index);
    }
    return 0;
}

void FacebookPostInterfacePrivate::message_tags_clear(QDeclarativeListProperty<FacebookNameTagInterface> *list)
{
    FacebookPostInterface *interface = qobject_cast<FacebookPostInterface *>(list->object);
    if (interface) {
        foreach (FacebookNameTagInterface *entry, interface->d_func()->messageTags) {
            entry->deleteLater();
        }
        interface->d_func()->messageTags.clear();
    }
}

int FacebookPostInterfacePrivate::message_tags_count(QDeclarativeListProperty<FacebookNameTagInterface> *list)
{
    FacebookPostInterface *interface = qobject_cast<FacebookPostInterface *>(list->object);
    if (interface) {
        return interface->d_func()->messageTags.count();
    }
    return 0;
}

void FacebookPostInterfacePrivate::properties_append(QDeclarativeListProperty<FacebookPostPropertyInterface> *list,
                                                     FacebookPostPropertyInterface *data)
{
    FacebookPostInterface *interface = qobject_cast<FacebookPostInterface *>(list->object);
    if (interface) {
        data->setParent(interface);
        interface->d_func()->properties.append(data);
    }
}

FacebookPostPropertyInterface * FacebookPostInterfacePrivate::properties_at(QDeclarativeListProperty<FacebookPostPropertyInterface> *list,
                                                                            int index)
{
    FacebookPostInterface *interface = qobject_cast<FacebookPostInterface *>(list->object);
    if (interface
        && index < interface->d_func()->properties.count()
        && index >= 0) {
        return interface->d_func()->properties.at(index);
    }
    return 0;
}

void FacebookPostInterfacePrivate::properties_clear(QDeclarativeListProperty<FacebookPostPropertyInterface> *list)
{
    FacebookPostInterface *interface = qobject_cast<FacebookPostInterface *>(list->object);
    if (interface) {
        foreach (FacebookPostPropertyInterface *entry, interface->d_func()->properties) {
            entry->deleteLater();
        }
        interface->d_func()->properties.clear();
    }
}

int FacebookPostInterfacePrivate::properties_count(QDeclarativeListProperty<FacebookPostPropertyInterface> *list)
{
    FacebookPostInterface *interface = qobject_cast<FacebookPostInterface *>(list->object);
    if (interface) {
        return interface->d_func()->properties.count();
    }
    return 0;
}

void FacebookPostInterfacePrivate::actions_append(QDeclarativeListProperty<FacebookPostActionInterface> *list,
                                                  FacebookPostActionInterface *data)
{
    FacebookPostInterface *interface = qobject_cast<FacebookPostInterface *>(list->object);
    if (interface) {
        data->setParent(interface);
        interface->d_func()->actions.append(data);
    }
}

FacebookPostActionInterface * FacebookPostInterfacePrivate::actions_at(QDeclarativeListProperty<FacebookPostActionInterface> *list,
                                                                       int index)
{
    FacebookPostInterface *interface = qobject_cast<FacebookPostInterface *>(list->object);
    if (interface
        && index < interface->d_func()->actions.count()
        && index >= 0) {
        return interface->d_func()->actions.at(index);
    }
    return 0;
}

void FacebookPostInterfacePrivate::actions_clear(QDeclarativeListProperty<FacebookPostActionInterface> *list)
{
    FacebookPostInterface *interface = qobject_cast<FacebookPostInterface *>(list->object);
    if (interface) {
        foreach (FacebookPostActionInterface *entry, interface->d_func()->actions) {
            entry->deleteLater();
        }
        interface->d_func()->actions.clear();
    }
}

int FacebookPostInterfacePrivate::actions_count(QDeclarativeListProperty<FacebookPostActionInterface> *list)
{
    FacebookPostInterface *interface = qobject_cast<FacebookPostInterface *>(list->object);
    if (interface) {
        return interface->d_func()->actions.count();
    }
    return 0;
}

void FacebookPostInterfacePrivate::story_tags_append(QDeclarativeListProperty<FacebookNameTagInterface> *list,
                                                     FacebookNameTagInterface *data)
{
    FacebookPostInterface *interface = qobject_cast<FacebookPostInterface *>(list->object);
    if (interface) {
        data->setParent(interface);
        interface->d_func()->storyTags.append(data);
    }
}

FacebookNameTagInterface * FacebookPostInterfacePrivate::story_tags_at(QDeclarativeListProperty<FacebookNameTagInterface> *list,
                                                                       int index)
{
    FacebookPostInterface *interface = qobject_cast<FacebookPostInterface *>(list->object);
    if (interface
        && index < interface->d_func()->storyTags.count()
        && index >= 0) {
        return interface->d_func()->storyTags.at(index);
    }
    return 0;
}

void FacebookPostInterfacePrivate::story_tags_clear(QDeclarativeListProperty<FacebookNameTagInterface> *list)
{
    FacebookPostInterface *interface = qobject_cast<FacebookPostInterface *>(list->object);
    if (interface) {
        foreach (FacebookNameTagInterface *entry, interface->d_func()->storyTags) {
            entry->deleteLater();
        }
        interface->d_func()->storyTags.clear();
    }
}

int FacebookPostInterfacePrivate::story_tags_count(QDeclarativeListProperty<FacebookNameTagInterface> *list)
{
    FacebookPostInterface *interface = qobject_cast<FacebookPostInterface *>(list->object);
    if (interface) {
        return interface->d_func()->storyTags.count();
    }
    return 0;
}

void FacebookPostInterfacePrivate::with_tags_append(QDeclarativeListProperty<FacebookObjectReferenceInterface> *list,
                                                    FacebookObjectReferenceInterface *data)
{
    FacebookPostInterface *interface = qobject_cast<FacebookPostInterface *>(list->object);
    if (interface) {
        data->setParent(interface);
        interface->d_func()->withTags.append(data);
    }
}

FacebookObjectReferenceInterface * FacebookPostInterfacePrivate::with_tags_at(QDeclarativeListProperty<FacebookObjectReferenceInterface> *list,
                                                                              int index)
{
    FacebookPostInterface *interface = qobject_cast<FacebookPostInterface *>(list->object);
    if (interface
        && index < interface->d_func()->withTags.count()
        && index >= 0) {
        return interface->d_func()->withTags.at(index);
    }
    return 0;
}

void FacebookPostInterfacePrivate::with_tags_clear(QDeclarativeListProperty<FacebookObjectReferenceInterface> *list)
{
    FacebookPostInterface *interface = qobject_cast<FacebookPostInterface *>(list->object);
    if (interface) {
        foreach (FacebookObjectReferenceInterface *entry, interface->d_func()->withTags) {
            entry->deleteLater();
        }
        interface->d_func()->withTags.clear();
    }
}

int FacebookPostInterfacePrivate::with_tags_count(QDeclarativeListProperty<FacebookObjectReferenceInterface> *list)
{
    FacebookPostInterface *interface = qobject_cast<FacebookPostInterface *>(list->object);
    if (interface) {
        return interface->d_func()->withTags.count();
    }
    return 0;
}


//-------------------------------

/*!
    \qmltype FacebookPost
    \instantiates FacebookPostInterface
    
*/
FacebookPostInterface::FacebookPostInterface(QObject *parent)
    : IdentifiableContentItemInterface(*(new FacebookPostInterfacePrivate(this)), parent)
{
// <<< constructor
    Q_D(FacebookPostInterface);
    d->from = new FacebookObjectReferenceInterface(this);
    d->application = new FacebookObjectReferenceInterface(this);
// >>> constructor
}

/*! \reimp */
int FacebookPostInterface::type() const
{
    return FacebookInterface::Post;
}

/*! \reimp */
bool FacebookPostInterface::remove()
{
// <<< remove
    return IdentifiableContentItemInterface::remove();
// >>> remove
}

/*! \reimp */
bool FacebookPostInterface::reload(const QStringList &whichFields)
{
// <<< reload
    return IdentifiableContentItemInterface::reload(whichFields);
// >>> reload
}

/*!
    \qmlmethod bool FacebookPost::like()
    Initiates a "like" operation on the post.
    
    If the network request was started successfully, the function
    will return true and the status of the post will change to
    \c SocialNetwork::Busy.  Otherwise, the function will return
    false.*/

bool FacebookPostInterface::like()
{
// <<< like
    Q_D(FacebookPostInterface);
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
    \qmlmethod bool FacebookPost::unlike()
    Initiates a "delete like" operation on the post.
    
    If the network request was started successfully, the function
    will return true and the status of the post will change to
    \c SocialNetwork::Busy.  Otherwise, the function will return
    false.*/

bool FacebookPostInterface::unlike()
{
// <<< unlike
    Q_D(FacebookPostInterface);
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
    \qmlmethod bool FacebookPost::uploadComment(const QString &message)
    Initiates a "post comment" operation on the post.  The comment
    will contain the specified \a message.
    
    If the network request was started successfully, the function
    will return true and the status of the post will change to
    \c SocialNetwork::Busy.  Otherwise, the function will return
    false.
    
    Once the network request completes, the \c responseReceived()
    signal will be emitted.  The \c data parameter of the signal
    will contain the \c id of the newly uploaded comment.*/

bool FacebookPostInterface::uploadComment(const QString &message)
{
// <<< uploadComment
    Q_D(FacebookPostInterface);
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
    \qmlmethod bool FacebookPost::removeComment(const QString &commentIdentifier)
    Initiates a "delete comment" operation on the comment specified by
    the given \a identifier.
    
    If the network request was started successfully, the function
    will return true and the status of the post will change to
    \c SocialNetwork::Busy.  Otherwise, the function will return
    false.*/

bool FacebookPostInterface::removeComment(const QString &commentIdentifier)
{
// <<< removeComment
    Q_D(FacebookPostInterface);
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
    \qmlproperty FacebookObjectReferenceInterface * FacebookPost::from
    Holds a reference to the user or profile which uploaded this post.
*/
FacebookObjectReferenceInterface * FacebookPostInterface::from() const
{
    Q_D(const FacebookPostInterface);
    return d->from;
}

/*!
    \qmlproperty QDeclarativeListProperty<FacebookObjectReferenceInterface> FacebookPost::to
    Holds a list of references to the users or profiles which are targetted by this post.
*/
QDeclarativeListProperty<FacebookObjectReferenceInterface> FacebookPostInterface::to()
{
    return QDeclarativeListProperty<FacebookObjectReferenceInterface>(
                this, 0,
                &FacebookPostInterfacePrivate::to_append,
                &FacebookPostInterfacePrivate::to_count,
                &FacebookPostInterfacePrivate::to_at,
                &FacebookPostInterfacePrivate::to_clear);
}

/*!
    \qmlproperty QString FacebookPost::message
    Holds the message text of the post.
*/
QString FacebookPostInterface::message() const
{
    Q_D(const FacebookPostInterface);
    return d->data().value(FACEBOOK_ONTOLOGY_POST_MESSAGE).toString();
}

/*!
    \qmlproperty QDeclarativeListProperty<FacebookNameTagInterface> FacebookPost::messageTags
    Holds the objects tagged in the message.
*/
QDeclarativeListProperty<FacebookNameTagInterface> FacebookPostInterface::messageTags()
{
    return QDeclarativeListProperty<FacebookNameTagInterface>(
                this, 0,
                &FacebookPostInterfacePrivate::message_tags_append,
                &FacebookPostInterfacePrivate::message_tags_count,
                &FacebookPostInterfacePrivate::message_tags_at,
                &FacebookPostInterfacePrivate::message_tags_clear);
}

/*!
    \qmlproperty QUrl FacebookPost::picture
    Holds a link to the picture of the post, if available.
*/
QUrl FacebookPostInterface::picture() const
{
    Q_D(const FacebookPostInterface);
    return QUrl::fromEncoded(d->data().value(FACEBOOK_ONTOLOGY_POST_PICTURE).toString().toLocal8Bit());
}

/*!
    \qmlproperty QUrl FacebookPost::link
    Holds the link attached to this post.
*/
QUrl FacebookPostInterface::link() const
{
    Q_D(const FacebookPostInterface);
    return QUrl::fromEncoded(d->data().value(FACEBOOK_ONTOLOGY_POST_LINK).toString().toLocal8Bit());
}

/*!
    \qmlproperty QString FacebookPost::name
    Holds the name of the link that is attached to this post.
*/
QString FacebookPostInterface::name() const
{
    Q_D(const FacebookPostInterface);
    return d->data().value(FACEBOOK_ONTOLOGY_POST_NAME).toString();
}

/*!
    \qmlproperty QString FacebookPost::caption
    Holds the caption of the link that is attached to this post.
*/
QString FacebookPostInterface::caption() const
{
    Q_D(const FacebookPostInterface);
    return d->data().value(FACEBOOK_ONTOLOGY_POST_CAPTION).toString();
}

/*!
    \qmlproperty QString FacebookPost::description
    Holds the description of the link that is attached to this post.
*/
QString FacebookPostInterface::description() const
{
    Q_D(const FacebookPostInterface);
    return d->data().value(FACEBOOK_ONTOLOGY_POST_DESCRIPTION).toString();
}

/*!
    \qmlproperty QUrl FacebookPost::source
    Holds the link to a video or flash movie that is embedded in this post.
*/
QUrl FacebookPostInterface::source() const
{
    Q_D(const FacebookPostInterface);
    return QUrl::fromEncoded(d->data().value(FACEBOOK_ONTOLOGY_POST_SOURCE).toString().toLocal8Bit());
}

/*!
    \qmlproperty QDeclarativeListProperty<FacebookPostPropertyInterface> FacebookPost::properties
    Holds a list of properties for the attached content.
*/
QDeclarativeListProperty<FacebookPostPropertyInterface> FacebookPostInterface::properties()
{
    return QDeclarativeListProperty<FacebookPostPropertyInterface>(
                this, 0,
                &FacebookPostInterfacePrivate::properties_append,
                &FacebookPostInterfacePrivate::properties_count,
                &FacebookPostInterfacePrivate::properties_at,
                &FacebookPostInterfacePrivate::properties_clear);
}

/*!
    \qmlproperty QUrl FacebookPost::icon
    Holds a link to an icon representing the type of this post.
*/
QUrl FacebookPostInterface::icon() const
{
    Q_D(const FacebookPostInterface);
    return QUrl::fromEncoded(d->data().value(FACEBOOK_ONTOLOGY_POST_ICON).toString().toLocal8Bit());
}

/*!
    \qmlproperty QDeclarativeListProperty<FacebookPostActionInterface> FacebookPost::actions
    Holds a list of actions that can be done with this post. (not generated yet)
*/
QDeclarativeListProperty<FacebookPostActionInterface> FacebookPostInterface::actions()
{
    return QDeclarativeListProperty<FacebookPostActionInterface>(
                this, 0,
                &FacebookPostInterfacePrivate::actions_append,
                &FacebookPostInterfacePrivate::actions_count,
                &FacebookPostInterfacePrivate::actions_at,
                &FacebookPostInterfacePrivate::actions_clear);
}

/*!
    \qmlproperty QString FacebookPost::postType
    Holds the type of this post.
*/
QString FacebookPostInterface::postType() const
{
    Q_D(const FacebookPostInterface);
    return d->data().value(FACEBOOK_ONTOLOGY_POST_POSTTYPE).toString();
}

/*!
    \qmlproperty QString FacebookPost::story
    Holds the story, if this post represents a story.
*/
QString FacebookPostInterface::story() const
{
    Q_D(const FacebookPostInterface);
    return d->data().value(FACEBOOK_ONTOLOGY_POST_STORY).toString();
}

/*!
    \qmlproperty QDeclarativeListProperty<FacebookNameTagInterface> FacebookPost::storyTags
    Holds the objects tagged in the story.
*/
QDeclarativeListProperty<FacebookNameTagInterface> FacebookPostInterface::storyTags()
{
    return QDeclarativeListProperty<FacebookNameTagInterface>(
                this, 0,
                &FacebookPostInterfacePrivate::story_tags_append,
                &FacebookPostInterfacePrivate::story_tags_count,
                &FacebookPostInterfacePrivate::story_tags_at,
                &FacebookPostInterfacePrivate::story_tags_clear);
}

/*!
    \qmlproperty QDeclarativeListProperty<FacebookObjectReferenceInterface> FacebookPost::withTags
    Holds the objects or users that are tagged with the "Who are you with ?" field. (not generated yet)
*/
QDeclarativeListProperty<FacebookObjectReferenceInterface> FacebookPostInterface::withTags()
{
    return QDeclarativeListProperty<FacebookObjectReferenceInterface>(
                this, 0,
                &FacebookPostInterfacePrivate::with_tags_append,
                &FacebookPostInterfacePrivate::with_tags_count,
                &FacebookPostInterfacePrivate::with_tags_at,
                &FacebookPostInterfacePrivate::with_tags_clear);
}

/*!
    \qmlproperty QString FacebookPost::objectIdentifier
    Hold the identifier of the uploaded photo or video attached to this post.
*/
QString FacebookPostInterface::objectIdentifier() const
{
    Q_D(const FacebookPostInterface);
    return d->data().value(FACEBOOK_ONTOLOGY_POST_OBJECTIDENTIFIER).toString();
}

/*!
    \qmlproperty FacebookObjectReferenceInterface * FacebookPost::application
    Hold the application that was used to upload this post.
*/
FacebookObjectReferenceInterface * FacebookPostInterface::application() const
{
    Q_D(const FacebookPostInterface);
    return d->application;
}

/*!
    \qmlproperty QString FacebookPost::createdTime
    Holds the creation time of the post in an ISO8601-formatted string.
*/
QString FacebookPostInterface::createdTime() const
{
    Q_D(const FacebookPostInterface);
    return d->data().value(FACEBOOK_ONTOLOGY_POST_CREATEDTIME).toString();
}

/*!
    \qmlproperty QString FacebookPost::updatedTime
    Holds the last-update time of the post in an ISO8601-formatted string.
*/
QString FacebookPostInterface::updatedTime() const
{
    Q_D(const FacebookPostInterface);
    return d->data().value(FACEBOOK_ONTOLOGY_POST_UPDATEDTIME).toString();
}

/*!
    \qmlproperty int FacebookPost::shares
    Holds the number of times this post has been shared.
*/
int FacebookPostInterface::shares() const
{
    Q_D(const FacebookPostInterface);
    QString numberString = d->data().value(FACEBOOK_ONTOLOGY_POST_SHARES).toString();
    bool ok;
    int number = numberString.toInt(&ok);
    if (ok) {
        return number;
    }
    return -1;
}

/*!
    \qmlproperty bool FacebookPost::hidden
    Holds if this post is hidden from timeline.
*/
bool FacebookPostInterface::hidden() const
{
    Q_D(const FacebookPostInterface);
    return d->data().value(FACEBOOK_ONTOLOGY_POST_HIDDEN).toString() == QLatin1String("true");
}

/*!
    \qmlproperty QString FacebookPost::statusType
    Hold the type of status update.
*/
QString FacebookPostInterface::statusType() const
{
    Q_D(const FacebookPostInterface);
    return d->data().value(FACEBOOK_ONTOLOGY_POST_STATUSTYPE).toString();
}

/*!
    \qmlproperty bool FacebookPost::liked
    Whether the post has been liked by the current user.
*/
bool FacebookPostInterface::liked() const
{
    Q_D(const FacebookPostInterface);
    return d->liked;
}

/*!
    \qmlproperty int FacebookPost::likesCount
    The number of likes on this post.
*/
int FacebookPostInterface::likesCount() const
{
    Q_D(const FacebookPostInterface);
    return d->likesCount;
}

/*!
    \qmlproperty int FacebookPost::commentsCount
    The number of likes on this post.
*/
int FacebookPostInterface::commentsCount() const
{
    Q_D(const FacebookPostInterface);
    return d->commentsCount;
}

