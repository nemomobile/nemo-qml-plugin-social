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

#include "facebooknotificationinterface_p.h"
#include "facebookinterface.h"
#include "facebookontology_p.h"
// <<< include
// >>> include

FacebookNotificationInterfacePrivate::FacebookNotificationInterfacePrivate(FacebookNotificationInterface *q)
    : IdentifiableContentItemInterfacePrivate(q)
    , action(FacebookInterfacePrivate::NoAction)
// <<< custom
    , from(0)
    , to(0)
    , application(0)
// >>> custom
{
}

void FacebookNotificationInterfacePrivate::finishedHandler()
{
// <<< finishedHandler
// >>> finishedHandler
}
void FacebookNotificationInterfacePrivate::emitPropertyChangeSignals(const QVariantMap &oldData,
                                                                     const QVariantMap &newData)
{
    Q_Q(FacebookNotificationInterface);
    QVariant oldCreatedTime = oldData.value(FACEBOOK_ONTOLOGY_NOTIFICATION_CREATEDTIME);
    QVariant newCreatedTime = newData.value(FACEBOOK_ONTOLOGY_NOTIFICATION_CREATEDTIME);
    QVariant oldUpdatedTime = oldData.value(FACEBOOK_ONTOLOGY_NOTIFICATION_UPDATEDTIME);
    QVariant newUpdatedTime = newData.value(FACEBOOK_ONTOLOGY_NOTIFICATION_UPDATEDTIME);
    QVariant oldTitle = oldData.value(FACEBOOK_ONTOLOGY_NOTIFICATION_TITLE);
    QVariant newTitle = newData.value(FACEBOOK_ONTOLOGY_NOTIFICATION_TITLE);
    QVariant oldLink = oldData.value(FACEBOOK_ONTOLOGY_NOTIFICATION_LINK);
    QVariant newLink = newData.value(FACEBOOK_ONTOLOGY_NOTIFICATION_LINK);
    QVariant oldUnread = oldData.value(FACEBOOK_ONTOLOGY_NOTIFICATION_UNREAD);
    QVariant newUnread = newData.value(FACEBOOK_ONTOLOGY_NOTIFICATION_UNREAD);

    if (newCreatedTime != oldCreatedTime)
        emit q->createdTimeChanged();
    if (newUpdatedTime != oldUpdatedTime)
        emit q->updatedTimeChanged();
    if (newTitle != oldTitle)
        emit q->titleChanged();
    if (newLink != oldLink)
        emit q->linkChanged();
    if (newUnread != oldUnread)
        emit q->unreadChanged();

// <<< emitPropertyChangeSignals
    QVariantMap oldFromMap = oldData.value(FACEBOOK_ONTOLOGY_NOTIFICATION_FROM).toMap();
    QString oldFromId = oldFromMap.value(FACEBOOK_ONTOLOGY_OBJECTREFERENCE_OBJECTIDENTIFIER).toString();
    QString oldFromName = oldFromMap.value(FACEBOOK_ONTOLOGY_OBJECTREFERENCE_OBJECTNAME).toString();
    QVariantMap newFromMap = newData.value(FACEBOOK_ONTOLOGY_NOTIFICATION_FROM).toMap();
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

    QVariantMap oldToMap = oldData.value(FACEBOOK_ONTOLOGY_NOTIFICATION_TO).toMap();
    QString oldToId = oldToMap.value(FACEBOOK_ONTOLOGY_OBJECTREFERENCE_OBJECTIDENTIFIER).toString();
    QString oldToName = oldToMap.value(FACEBOOK_ONTOLOGY_OBJECTREFERENCE_OBJECTNAME).toString();
    QVariantMap newToMap = newData.value(FACEBOOK_ONTOLOGY_NOTIFICATION_TO).toMap();
    QString newToId = newToMap.value(FACEBOOK_ONTOLOGY_OBJECTREFERENCE_OBJECTIDENTIFIER).toString();
    QString newToName = newToMap.value(FACEBOOK_ONTOLOGY_OBJECTREFERENCE_OBJECTNAME).toString();

    // Update the To object if required
    if (newToId != oldToId || newToName != oldToName) {
        QVariantMap newToData;
        newToData.insert(FACEBOOK_ONTOLOGY_OBJECTREFERENCE_OBJECTTYPE, FacebookInterface::User);
        newToData.insert(FACEBOOK_ONTOLOGY_OBJECTREFERENCE_OBJECTIDENTIFIER, newToId);
        newToData.insert(FACEBOOK_ONTOLOGY_OBJECTREFERENCE_OBJECTNAME, newToName);
        qobject_cast<FacebookInterface*>(q->socialNetwork())->setFacebookContentItemData(to, newToData);
        emit q->toChanged();
    }

    QVariantMap oldApplicationMap = oldData.value(FACEBOOK_ONTOLOGY_NOTIFICATION_APPLICATION).toMap();
    QString oldApplicationId = oldApplicationMap.value(FACEBOOK_ONTOLOGY_OBJECTREFERENCE_OBJECTIDENTIFIER).toString();
    QString oldApplicationName = oldApplicationMap.value(FACEBOOK_ONTOLOGY_OBJECTREFERENCE_OBJECTNAME).toString();
    QVariantMap newApplicationMap = newData.value(FACEBOOK_ONTOLOGY_NOTIFICATION_APPLICATION).toMap();
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
    \qmltype FacebookNotification
    \instantiates FacebookNotificationInterface
    \inqmlmodule org.nemomobile.social 1
    \brief A FacebookNotification represents a notification from the Facebook OpenGraph API
    
    FacebookNotification is a specialized IdentifiableContentItem that is used
    to hold data that represents a notification in the Facebook OpenGraph API.
    
    \sa{IdentifiableContentItem}
    
*/
FacebookNotificationInterface::FacebookNotificationInterface(QObject *parent)
    : IdentifiableContentItemInterface(*(new FacebookNotificationInterfacePrivate(this)), parent)
{
// <<< constructor
    Q_D(FacebookNotificationInterface);
    d->from = new FacebookObjectReferenceInterface(this);
    d->to = new FacebookObjectReferenceInterface(this);
    d->application = new FacebookObjectReferenceInterface(this);
// >>> constructor
}

/*! \reimp */
int FacebookNotificationInterface::type() const
{
    return FacebookInterface::Notification;
}

/*! \reimp */
bool FacebookNotificationInterface::remove()
{
// <<< remove
    return IdentifiableContentItemInterface::remove();
// >>> remove
}

/*! \reimp */
bool FacebookNotificationInterface::reload(const QStringList &whichFields)
{
// <<< reload
    return IdentifiableContentItemInterface::reload(whichFields);
// >>> reload
}


/*!
    \qmlproperty FacebookObjectReference FacebookNotification::from
    Holds a reference to the person or profile whose action triggered the notification.
*/
FacebookObjectReferenceInterface * FacebookNotificationInterface::from() const
{
    Q_D(const FacebookNotificationInterface);
    return d->from;
}

/*!
    \qmlproperty FacebookObjectReference FacebookNotification::to
    Holds a reference to the person or profile to whom the notification was posted.
*/
FacebookObjectReferenceInterface * FacebookNotificationInterface::to() const
{
    Q_D(const FacebookNotificationInterface);
    return d->to;
}

/*!
    \qmlproperty FacebookObjectReference FacebookNotification::application
    Holds a reference to the application which posted the notification.
*/
FacebookObjectReferenceInterface * FacebookNotificationInterface::application() const
{
    Q_D(const FacebookNotificationInterface);
    return d->application;
}

/*!
    \qmlproperty string FacebookNotification::createdTime
    Holds the creation time of the notification in an ISO8601-formatted string.
*/
QString FacebookNotificationInterface::createdTime() const
{
    Q_D(const FacebookNotificationInterface);
    return d->data().value(FACEBOOK_ONTOLOGY_NOTIFICATION_CREATEDTIME).toString();
}

/*!
    \qmlproperty string FacebookNotification::updatedTime
    Holds the update time of the notification in an ISO8601-formatted string.
*/
QString FacebookNotificationInterface::updatedTime() const
{
    Q_D(const FacebookNotificationInterface);
    return d->data().value(FACEBOOK_ONTOLOGY_NOTIFICATION_UPDATEDTIME).toString();
}

/*!
    \qmlproperty string FacebookNotification::title
    Holds the title (message) of the notification.
*/
QString FacebookNotificationInterface::title() const
{
    Q_D(const FacebookNotificationInterface);
    return d->data().value(FACEBOOK_ONTOLOGY_NOTIFICATION_TITLE).toString();
}

/*!
    \qmlproperty url FacebookNotification::link
    Holds a link to the original content item about which the notification was posted.
*/
QUrl FacebookNotificationInterface::link() const
{
    Q_D(const FacebookNotificationInterface);
    return QUrl::fromEncoded(d->data().value(FACEBOOK_ONTOLOGY_NOTIFICATION_LINK).toString().toLocal8Bit());
}

/*!
    \qmlproperty int FacebookNotification::unread
    Will be zero if the notification has been marked as read.
*/
int FacebookNotificationInterface::unread() const
{
    Q_D(const FacebookNotificationInterface);
    QString numberString = d->data().value(FACEBOOK_ONTOLOGY_NOTIFICATION_UNREAD).toString();
    bool ok;
    int number = numberString.toInt(&ok);
    if (ok) {
        return number;
    }
    return -1;
}

