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

#include "facebookphototaginterface.h"
#include "facebookinterface.h"
#include "facebookontology_p.h"
#include "contentiteminterface_p.h"
// <<< include
// >>> include

class FacebookPhotoTagInterfacePrivate: public ContentItemInterfacePrivate
{
public:
    explicit FacebookPhotoTagInterfacePrivate(FacebookPhotoTagInterface *q);
    void emitPropertyChangeSignals(const QVariantMap &oldData, const QVariantMap &newData);
    QString userName;
    QString text;
private:
    Q_DECLARE_PUBLIC(FacebookPhotoTagInterface)
};

FacebookPhotoTagInterfacePrivate::FacebookPhotoTagInterfacePrivate(FacebookPhotoTagInterface *q)
    : ContentItemInterfacePrivate(q)
// <<< custom
// >>> custom
{
}

void FacebookPhotoTagInterfacePrivate::emitPropertyChangeSignals(const QVariantMap &oldData,
                                                                 const QVariantMap &newData)
{
    Q_Q(FacebookPhotoTagInterface);
    QVariant oldUserIdentifier = oldData.value(FACEBOOK_ONTOLOGY_PHOTO_TAG_USERIDENTIFIER);
    QVariant newUserIdentifier = newData.value(FACEBOOK_ONTOLOGY_PHOTO_TAG_USERIDENTIFIER);
    QVariant oldX = oldData.value(FACEBOOK_ONTOLOGY_PHOTO_TAG_X);
    QVariant newX = newData.value(FACEBOOK_ONTOLOGY_PHOTO_TAG_X);
    QVariant oldY = oldData.value(FACEBOOK_ONTOLOGY_PHOTO_TAG_Y);
    QVariant newY = newData.value(FACEBOOK_ONTOLOGY_PHOTO_TAG_Y);
    QVariant oldCreatedTime = oldData.value(FACEBOOK_ONTOLOGY_PHOTO_TAG_CREATEDTIME);
    QVariant newCreatedTime = newData.value(FACEBOOK_ONTOLOGY_PHOTO_TAG_CREATEDTIME);

    if (newUserIdentifier != oldUserIdentifier)
        emit q->userIdentifierChanged();
    if (newX != oldX)
        emit q->xChanged();
    if (newY != oldY)
        emit q->yChanged();
    if (newCreatedTime != oldCreatedTime)
        emit q->createdTimeChanged();

// <<< emitPropertyChangeSignals
    QString newText;
    QString newUserName;

    if (newUserIdentifier.toString().isEmpty()) {
        newText = newData.value(FACEBOOK_ONTOLOGY_TAG_USERNAME).toString();
    } else {
        newUserName = newData.value(FACEBOOK_ONTOLOGY_TAG_USERNAME).toString();
    }

    if (text != newText) {
        text = newText;
        emit q->textChanged();
    }

    if (userName != newUserName) {
        userName = newUserName;
        emit q->userNameChanged();
    }
// >>> emitPropertyChangeSignals

    // Call super class implementation
    ContentItemInterfacePrivate::emitPropertyChangeSignals(oldData, newData);
}

//-------------------------------

/*!
    \qmltype FacebookPhotoTag
    \instantiates FacebookPhotoTagInterface
    An entry representing a tag
*/
FacebookPhotoTagInterface::FacebookPhotoTagInterface(QObject *parent)
    : ContentItemInterface(*(new FacebookPhotoTagInterfacePrivate(this)), parent)
{
// <<< constructor
    // TODO Implement initialization of custom attributes if needed
// >>> constructor
}

/*! \reimp */
int FacebookPhotoTagInterface::type() const
{
    return FacebookInterface::PhotoTag;
}

#if 0

#endif
/*!
    \qmlproperty QString FacebookPhotoTag::userIdentifier
    Holds the identifier of the tagged entity
*/
QString FacebookPhotoTagInterface::userIdentifier() const
{
    return data().value(FACEBOOK_ONTOLOGY_PHOTO_TAG_USERIDENTIFIER).toString();
}

/*!
    \qmlproperty QString FacebookPhotoTag::userName
    Holds the name of the tagged entity
*/
QString FacebookPhotoTagInterface::userName() const
{
    Q_D(const FacebookPhotoTagInterface);
    return d->userName;
}

/*!
    \qmlproperty QString FacebookPhotoTag::text
    Holds text that is used to tag something
*/
QString FacebookPhotoTagInterface::text() const
{
    Q_D(const FacebookPhotoTagInterface);
    return d->text;
}

/*!
    \qmlproperty float FacebookPhotoTag::x
    Holds the x position of the tagged entity or text in the photo.
    This coordinate is a percentage from the left edge of the photo.
*/
float FacebookPhotoTagInterface::x() const
{
    QString numberString = data().value(FACEBOOK_ONTOLOGY_PHOTO_TAG_X).toString();
    bool ok;
    float number = numberString.toFloat(&ok);
    if (ok) {
        return number;
    }
    return 0.;
}

/*!
    \qmlproperty float FacebookPhotoTag::y
    Holds the y position of the tagged entity or text in the photo.
    This coordinate is a percentage from the top edge of the photo.
*/
float FacebookPhotoTagInterface::y() const
{
    QString numberString = data().value(FACEBOOK_ONTOLOGY_PHOTO_TAG_Y).toString();
    bool ok;
    float number = numberString.toFloat(&ok);
    if (ok) {
        return number;
    }
    return 0.;
}

/*!
    \qmlproperty QString FacebookPhotoTag::createdTime
    Holds the creation time of the tag in an ISO8601-formatted string.
*/
QString FacebookPhotoTagInterface::createdTime() const
{
    return data().value(FACEBOOK_ONTOLOGY_PHOTO_TAG_CREATEDTIME).toString();
}


FacebookPhotoTagInterface::FacebookPhotoTagInterface(FacebookPhotoTagInterfacePrivate &dd, QObject *parent)
    : ContentItemInterface(dd, parent)
{
    // TODO Implement initialization of custom attributes if needed
}
