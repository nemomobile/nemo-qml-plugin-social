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

#include "facebooktaginterface.h"
#include "facebookinterface.h"
#include "facebookontology_p.h"
#include "contentiteminterface_p.h"
// <<< include
// Includes goes here
// >>> include

class FacebookTagInterfacePrivate: public ContentItemInterfacePrivate
{
public:
    explicit FacebookTagInterfacePrivate(FacebookTagInterface *q);
    void emitPropertyChangeSignals(const QVariantMap &oldData, const QVariantMap &newData);
private:
    Q_DECLARE_PUBLIC(FacebookTagInterface)
};

FacebookTagInterfacePrivate::FacebookTagInterfacePrivate(FacebookTagInterface *q)
    : ContentItemInterfacePrivate(q)
{
}

void FacebookTagInterfacePrivate::emitPropertyChangeSignals(const QVariantMap &oldData,
                                                            const QVariantMap &newData)
{
    Q_Q(FacebookTagInterface);
    QString oldIdentifier = oldData.value(FACEBOOK_ONTOLOGY_TAG_IDENTIFIER).toString();
    QString newIdentifier = newData.value(FACEBOOK_ONTOLOGY_TAG_IDENTIFIER).toString();
    QString oldName = oldData.value(FACEBOOK_ONTOLOGY_TAG_NAME).toString();
    QString newName = newData.value(FACEBOOK_ONTOLOGY_TAG_NAME).toString();
    float oldX = oldData.value(FACEBOOK_ONTOLOGY_TAG_X).toFloat();
    float newX = newData.value(FACEBOOK_ONTOLOGY_TAG_X).toFloat();
    float oldY = oldData.value(FACEBOOK_ONTOLOGY_TAG_Y).toFloat();
    float newY = newData.value(FACEBOOK_ONTOLOGY_TAG_Y).toFloat();
    QString oldCreatedTime = oldData.value(FACEBOOK_ONTOLOGY_TAG_CREATEDTIME).toString();
    QString newCreatedTime = newData.value(FACEBOOK_ONTOLOGY_TAG_CREATEDTIME).toString();

    if (newIdentifier != oldIdentifier)
        emit q->identifierChanged();
    if (newName != oldName)
        emit q->nameChanged();
    if (newX != oldX)
        emit q->xChanged();
    if (newY != oldY)
        emit q->yChanged();
    if (newCreatedTime != oldCreatedTime)
        emit q->createdTimeChanged();

    // Call super class implementation
    ContentItemInterfacePrivate::emitPropertyChangeSignals(oldData, newData);
}

//-------------------------------

/*!
    \qmltype FacebookTag
    \instantiates FacebookTagInterface
    An entry representing a tag
*/
FacebookTagInterface::FacebookTagInterface(QObject *parent)
    : ContentItemInterface(*(new FacebookTagInterfacePrivate(this)), parent)
{
}

/*! \reimp */
int FacebookTagInterface::type() const
{
    return FacebookInterface::Tag;
}


/*!
    \qmlproperty QString FacebookTagInterface::identifier
    Holds the identifier of the tagged entity
*/
QString FacebookTagInterface::identifier() const
{
    Q_D(const FacebookTagInterface);
    return d->data().value(FACEBOOK_ONTOLOGY_TAG_IDENTIFIER).toString();
}

/*!
    \qmlproperty QString FacebookTagInterface::name
    Holds the name of the tagged entity
*/
QString FacebookTagInterface::name() const
{
    Q_D(const FacebookTagInterface);
    return d->data().value(FACEBOOK_ONTOLOGY_TAG_NAME).toString();
}

/*!
    \qmlproperty float FacebookTagInterface::x
    Holds the x position of the tagged entity or text in the photo.
    This coordinate is a percentage from the left edge of the photo.
*/
float FacebookTagInterface::x() const
{
    Q_D(const FacebookTagInterface);
    return d->data().value(FACEBOOK_ONTOLOGY_TAG_X).toFloat();
}

/*!
    \qmlproperty float FacebookTagInterface::y
    Holds the y position of the tagged entity or text in the photo.
    This coordinate is a percentage from the top edge of the photo.
*/
float FacebookTagInterface::y() const
{
    Q_D(const FacebookTagInterface);
    return d->data().value(FACEBOOK_ONTOLOGY_TAG_Y).toFloat();
}

/*!
    \qmlproperty QString FacebookTagInterface::createdTime
    Holds the creation time of the tag in an ISO8601-formatted string.
*/
QString FacebookTagInterface::createdTime() const
{
    Q_D(const FacebookTagInterface);
    return d->data().value(FACEBOOK_ONTOLOGY_TAG_CREATEDTIME).toString();
}

