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

#include "facebookcoverinterface.h"
#include "facebookinterface.h"
#include "facebookontology_p.h"
#include "contentiteminterface_p.h"
// <<< include
// Includes goes here
// >>> include

class FacebookCoverInterfacePrivate: public ContentItemInterfacePrivate
{
public:
    explicit FacebookCoverInterfacePrivate(FacebookCoverInterface *q);
    void emitPropertyChangeSignals(const QVariantMap &oldData, const QVariantMap &newData);
private:
    Q_DECLARE_PUBLIC(FacebookCoverInterface)
};

FacebookCoverInterfacePrivate::FacebookCoverInterfacePrivate(FacebookCoverInterface *q)
    : ContentItemInterfacePrivate(q)
{
}

void FacebookCoverInterfacePrivate::emitPropertyChangeSignals(const QVariantMap &oldData,
                                                              const QVariantMap &newData)
{
    Q_Q(FacebookCoverInterface);
    QVariant oldPhotoIdentifier = oldData.value(FACEBOOK_ONTOLOGY_COVER_PHOTOIDENTIFIER);
    QVariant newPhotoIdentifier = newData.value(FACEBOOK_ONTOLOGY_COVER_PHOTOIDENTIFIER);
    QVariant oldSource = oldData.value(FACEBOOK_ONTOLOGY_COVER_SOURCE);
    QVariant newSource = newData.value(FACEBOOK_ONTOLOGY_COVER_SOURCE);
    QVariant oldOffsetY = oldData.value(FACEBOOK_ONTOLOGY_COVER_OFFSETY);
    QVariant newOffsetY = newData.value(FACEBOOK_ONTOLOGY_COVER_OFFSETY);

    if (newPhotoIdentifier != oldPhotoIdentifier)
        emit q->photoIdentifierChanged();
    if (newSource != oldSource)
        emit q->sourceChanged();
    if (newOffsetY != oldOffsetY)
        emit q->offsetYChanged();

    // Call super class implementation
    ContentItemInterfacePrivate::emitPropertyChangeSignals(oldData, newData);
}

//-------------------------------

/*!
    \qmltype FacebookCover
    \instantiates FacebookCoverInterface
    The cover of an user
*/
FacebookCoverInterface::FacebookCoverInterface(QObject *parent)
    : ContentItemInterface(*(new FacebookCoverInterfacePrivate(this)), parent)
{
}

/*! \reimp */
int FacebookCoverInterface::type() const
{
    return FacebookInterface::Cover;
}


/*!
    \qmlproperty QString FacebookCover::photoIdentifier
    Holds the identifier of the cover photo
*/
QString FacebookCoverInterface::photoIdentifier() const
{
    Q_D(const FacebookCoverInterface);
    return d->data().value(FACEBOOK_ONTOLOGY_COVER_PHOTOIDENTIFIER).toString();
}

/*!
    \qmlproperty QString FacebookCover::source
    Holds an URL to the photo
*/
QString FacebookCoverInterface::source() const
{
    Q_D(const FacebookCoverInterface);
    return d->data().value(FACEBOOK_ONTOLOGY_COVER_SOURCE).toString();
}

/*!
    \qmlproperty int FacebookCover::offsetY
    Holds the vertical offset for the cover
*/
int FacebookCoverInterface::offsetY() const
{
    Q_D(const FacebookCoverInterface);
    QString numberString = d->data().value(FACEBOOK_ONTOLOGY_COVER_OFFSETY).toString();
    bool ok;
    int number = numberString.toInt(&ok);
    if (ok) {
        return number;
    }
    return -1;
}

