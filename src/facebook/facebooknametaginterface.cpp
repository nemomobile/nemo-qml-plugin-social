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

#include "facebooknametaginterface.h"
#include "facebookinterface.h"
#include "facebookontology_p.h"
#include "contentiteminterface_p.h"
// <<< include
// Includes goes here
// >>> include

class FacebookNameTagInterfacePrivate: public ContentItemInterfacePrivate
{
public:
    explicit FacebookNameTagInterfacePrivate(FacebookNameTagInterface *q);
    void emitPropertyChangeSignals(const QVariantMap &oldData, const QVariantMap &newData);
private:
    Q_DECLARE_PUBLIC(FacebookNameTagInterface)
};

FacebookNameTagInterfacePrivate::FacebookNameTagInterfacePrivate(FacebookNameTagInterface *q)
    : ContentItemInterfacePrivate(q)
{
}

void FacebookNameTagInterfacePrivate::emitPropertyChangeSignals(const QVariantMap &oldData,
                                                                const QVariantMap &newData)
{
    Q_Q(FacebookNameTagInterface);
    QString oldIdentifier = oldData.value(FACEBOOK_ONTOLOGY_NAME_TAG_IDENTIFIER).toString();
    QString newIdentifier = newData.value(FACEBOOK_ONTOLOGY_NAME_TAG_IDENTIFIER).toString();
    QString oldName = oldData.value(FACEBOOK_ONTOLOGY_NAME_TAG_NAME).toString();
    QString newName = newData.value(FACEBOOK_ONTOLOGY_NAME_TAG_NAME).toString();
    QString oldNameTagType = oldData.value(FACEBOOK_ONTOLOGY_NAME_TAG_NAMETAGTYPE).toString();
    QString newNameTagType = newData.value(FACEBOOK_ONTOLOGY_NAME_TAG_NAMETAGTYPE).toString();
    int oldOffset = oldData.value(FACEBOOK_ONTOLOGY_NAME_TAG_OFFSET).toInt();
    int newOffset = newData.value(FACEBOOK_ONTOLOGY_NAME_TAG_OFFSET).toInt();
    int oldLength = oldData.value(FACEBOOK_ONTOLOGY_NAME_TAG_LENGTH).toInt();
    int newLength = newData.value(FACEBOOK_ONTOLOGY_NAME_TAG_LENGTH).toInt();

    if (newIdentifier != oldIdentifier)
        emit q->identifierChanged();
    if (newName != oldName)
        emit q->nameChanged();
    if (newNameTagType != oldNameTagType)
        emit q->nameTagTypeChanged();
    if (newOffset != oldOffset)
        emit q->offsetChanged();
    if (newLength != oldLength)
        emit q->lengthChanged();

    // Call super class implementation
    ContentItemInterfacePrivate::emitPropertyChangeSignals(oldData, newData);
}

//-------------------------------

/*!
    \qmltype FacebookNameTag
    \instantiates FacebookNameTagInterface
    An entry representing a name tag
*/
FacebookNameTagInterface::FacebookNameTagInterface(QObject *parent)
    : ContentItemInterface(*(new FacebookNameTagInterfacePrivate(this)), parent)
{
}

/*! \reimp */
int FacebookNameTagInterface::type() const
{
    return FacebookInterface::NameTag;
}


/*!
    \qmlproperty QString FacebookNameTagInterface::identifier
    Holds the identifier of the tagged entity
*/
QString FacebookNameTagInterface::identifier() const
{
    Q_D(const FacebookNameTagInterface);
    return d->data().value(FACEBOOK_ONTOLOGY_NAME_TAG_IDENTIFIER).toString();
}

/*!
    \qmlproperty QString FacebookNameTagInterface::name
    Holds the name of the tagged entity
*/
QString FacebookNameTagInterface::name() const
{
    Q_D(const FacebookNameTagInterface);
    return d->data().value(FACEBOOK_ONTOLOGY_NAME_TAG_NAME).toString();
}

/*!
    \qmlproperty QString FacebookNameTagInterface::nameTagType
    Holds the type of the taggeed entity
*/
QString FacebookNameTagInterface::nameTagType() const
{
    Q_D(const FacebookNameTagInterface);
    return d->data().value(FACEBOOK_ONTOLOGY_NAME_TAG_NAMETAGTYPE).toString();
}

/*!
    \qmlproperty int FacebookNameTagInterface::offset
    Holds the offset in the name for a tag
*/
int FacebookNameTagInterface::offset() const
{
    Q_D(const FacebookNameTagInterface);
    return d->data().value(FACEBOOK_ONTOLOGY_NAME_TAG_OFFSET).toInt();
}

/*!
    \qmlproperty int FacebookNameTagInterface::length
    Holds the length of a tag
*/
int FacebookNameTagInterface::length() const
{
    Q_D(const FacebookNameTagInterface);
    return d->data().value(FACEBOOK_ONTOLOGY_NAME_TAG_LENGTH).toInt();
}

