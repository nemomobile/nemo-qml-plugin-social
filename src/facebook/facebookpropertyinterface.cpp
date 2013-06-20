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

#include "facebookpropertyinterface.h"
#include "facebookinterface.h"
#include "facebookontology_p.h"
#include "contentiteminterface_p.h"
// <<< include
// >>> include

class FacebookPropertyInterfacePrivate: public ContentItemInterfacePrivate
{
public:
    explicit FacebookPropertyInterfacePrivate(FacebookPropertyInterface *q);
    void emitPropertyChangeSignals(const QVariantMap &oldData, const QVariantMap &newData);
private:
    Q_DECLARE_PUBLIC(FacebookPropertyInterface)
};

FacebookPropertyInterfacePrivate::FacebookPropertyInterfacePrivate(FacebookPropertyInterface *q)
    : ContentItemInterfacePrivate(q)
{
}

void FacebookPropertyInterfacePrivate::emitPropertyChangeSignals(const QVariantMap &oldData,
                                                                 const QVariantMap &newData)
{
    Q_Q(FacebookPropertyInterface);
    QVariant oldName = oldData.value(FACEBOOK_ONTOLOGY_PROPERTY_NAME);
    QVariant newName = newData.value(FACEBOOK_ONTOLOGY_PROPERTY_NAME);
    QVariant oldText = oldData.value(FACEBOOK_ONTOLOGY_PROPERTY_TEXT);
    QVariant newText = newData.value(FACEBOOK_ONTOLOGY_PROPERTY_TEXT);

    if (newName != oldName)
        emit q->nameChanged();
    if (newText != oldText)
        emit q->textChanged();

    // Call super class implementation
    ContentItemInterfacePrivate::emitPropertyChangeSignals(oldData, newData);
}

//-------------------------------

/*!
    \qmltype FacebookProperty
    \instantiates FacebookPropertyInterface
    An entry representing a property for a post
*/
FacebookPropertyInterface::FacebookPropertyInterface(QObject *parent)
    : ContentItemInterface(*(new FacebookPropertyInterfacePrivate(this)), parent)
{
}

/*! \reimp */
int FacebookPropertyInterface::type() const
{
    return FacebookInterface::Property;
}


/*!
    \qmlproperty QString FacebookProperty::name
    Holds the name of the property
*/
QString FacebookPropertyInterface::name() const
{
    Q_D(const FacebookPropertyInterface);
    return d->data().value(FACEBOOK_ONTOLOGY_PROPERTY_NAME).toString();
}

/*!
    \qmlproperty QString FacebookProperty::text
    Holds the text contained in the property
*/
QString FacebookPropertyInterface::text() const
{
    Q_D(const FacebookPropertyInterface);
    return d->data().value(FACEBOOK_ONTOLOGY_PROPERTY_TEXT).toString();
}

