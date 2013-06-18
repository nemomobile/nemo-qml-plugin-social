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

#include "facebookpictureinterface.h"
#include "facebookinterface.h"
#include "facebookontology_p.h"
#include "contentiteminterface_p.h"
// <<< include
// >>> include

class FacebookPictureInterfacePrivate: public ContentItemInterfacePrivate
{
public:
    explicit FacebookPictureInterfacePrivate(FacebookPictureInterface *q);
    void emitPropertyChangeSignals(const QVariantMap &oldData, const QVariantMap &newData);
private:
    Q_DECLARE_PUBLIC(FacebookPictureInterface)
};

FacebookPictureInterfacePrivate::FacebookPictureInterfacePrivate(FacebookPictureInterface *q)
    : ContentItemInterfacePrivate(q)
{
}

void FacebookPictureInterfacePrivate::emitPropertyChangeSignals(const QVariantMap &oldData,
                                                                const QVariantMap &newData)
{
    Q_Q(FacebookPictureInterface);
    QVariant oldSource = oldData.value(FACEBOOK_ONTOLOGY_PICTURE_SOURCE);
    QVariant newSource = newData.value(FACEBOOK_ONTOLOGY_PICTURE_SOURCE);
    QVariant oldIsSilhouette = oldData.value(FACEBOOK_ONTOLOGY_PICTURE_ISSILHOUETTE);
    QVariant newIsSilhouette = newData.value(FACEBOOK_ONTOLOGY_PICTURE_ISSILHOUETTE);

    if (newSource != oldSource)
        emit q->sourceChanged();
    if (newIsSilhouette != oldIsSilhouette)
        emit q->isSilhouetteChanged();

    // Call super class implementation
    ContentItemInterfacePrivate::emitPropertyChangeSignals(oldData, newData);
}

//-------------------------------

/*!
    \qmltype FacebookPicture
    \instantiates FacebookPictureInterface
    An entry representing a picture
*/
FacebookPictureInterface::FacebookPictureInterface(QObject *parent)
    : ContentItemInterface(*(new FacebookPictureInterfacePrivate(this)), parent)
{
}

/*! \reimp */
int FacebookPictureInterface::type() const
{
    return FacebookInterface::Picture;
}


/*!
    \qmlproperty QUrl FacebookPicture::source
    Holds the url to the image source of the picture.
*/
QUrl FacebookPictureInterface::source() const
{
    Q_D(const FacebookPictureInterface);
    return QUrl(d->data().value(FACEBOOK_ONTOLOGY_PICTURE_SOURCE).toString());
}

/*!
    \qmlproperty bool FacebookPicture::isSilhouette
    Whether the picture is a default, anonymous silhouette image.
*/
bool FacebookPictureInterface::isSilhouette() const
{
    Q_D(const FacebookPictureInterface);
    return d->data().value(FACEBOOK_ONTOLOGY_PICTURE_ISSILHOUETTE).toString() == QLatin1String("true");
}

