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

#include "facebookpostactioninterface.h"
#include "facebookinterface.h"
#include "facebookontology_p.h"
#include "contentiteminterface_p.h"
// <<< include
// >>> include

class FacebookPostActionInterfacePrivate: public ContentItemInterfacePrivate
{
public:
    explicit FacebookPostActionInterfacePrivate(FacebookPostActionInterface *q);
    void emitPropertyChangeSignals(const QVariantMap &oldData, const QVariantMap &newData);
private:
    Q_DECLARE_PUBLIC(FacebookPostActionInterface)
};

FacebookPostActionInterfacePrivate::FacebookPostActionInterfacePrivate(FacebookPostActionInterface *q)
    : ContentItemInterfacePrivate(q)
{
}

void FacebookPostActionInterfacePrivate::emitPropertyChangeSignals(const QVariantMap &oldData,
                                                                   const QVariantMap &newData)
{
    Q_Q(FacebookPostActionInterface);
    QVariant oldName = oldData.value(FACEBOOK_ONTOLOGY_POST_ACTION_NAME);
    QVariant newName = newData.value(FACEBOOK_ONTOLOGY_POST_ACTION_NAME);
    QVariant oldLink = oldData.value(FACEBOOK_ONTOLOGY_POST_ACTION_LINK);
    QVariant newLink = newData.value(FACEBOOK_ONTOLOGY_POST_ACTION_LINK);

    if (newName != oldName)
        emit q->nameChanged();
    if (newLink != oldLink)
        emit q->linkChanged();

    // Call super class implementation
    ContentItemInterfacePrivate::emitPropertyChangeSignals(oldData, newData);
}

//-------------------------------

/*!
    \qmltype FacebookPostAction
    \instantiates FacebookPostActionInterface
    An entry representing a action for a post
*/
FacebookPostActionInterface::FacebookPostActionInterface(QObject *parent)
    : ContentItemInterface(*(new FacebookPostActionInterfacePrivate(this)), parent)
{
}

/*! \reimp */
int FacebookPostActionInterface::type() const
{
    return FacebookInterface::PostAction;
}


/*!
    \qmlproperty QString FacebookPostAction::name
    Holds the name of the action
*/
QString FacebookPostActionInterface::name() const
{
    Q_D(const FacebookPostActionInterface);
    return d->data().value(FACEBOOK_ONTOLOGY_POST_ACTION_NAME).toString();
}

/*!
    \qmlproperty QString FacebookPostAction::link
    Holds the link used to perform the action
*/
QString FacebookPostActionInterface::link() const
{
    Q_D(const FacebookPostActionInterface);
    return d->data().value(FACEBOOK_ONTOLOGY_POST_ACTION_LINK).toString();
}

