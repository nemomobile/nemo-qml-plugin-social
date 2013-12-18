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

#include "facebookphotoimageinterface.h"
#include "facebookinterface.h"
#include "facebookontology_p.h"
#include "contentiteminterface_p.h"
// <<< include
// >>> include

class FacebookPhotoImageInterfacePrivate: public ContentItemInterfacePrivate
{
public:
    explicit FacebookPhotoImageInterfacePrivate(FacebookPhotoImageInterface *q);
    void emitPropertyChangeSignals(const QVariantMap &oldData, const QVariantMap &newData);
private:
    Q_DECLARE_PUBLIC(FacebookPhotoImageInterface)
};

FacebookPhotoImageInterfacePrivate::FacebookPhotoImageInterfacePrivate(FacebookPhotoImageInterface *q)
    : ContentItemInterfacePrivate(q)
{
}

void FacebookPhotoImageInterfacePrivate::emitPropertyChangeSignals(const QVariantMap &oldData,
                                                                   const QVariantMap &newData)
{
    Q_Q(FacebookPhotoImageInterface);
    QVariant oldSource = oldData.value(FACEBOOK_ONTOLOGY_PHOTO_IMAGE_SOURCE);
    QVariant newSource = newData.value(FACEBOOK_ONTOLOGY_PHOTO_IMAGE_SOURCE);
    QVariant oldWidth = oldData.value(FACEBOOK_ONTOLOGY_PHOTO_IMAGE_WIDTH);
    QVariant newWidth = newData.value(FACEBOOK_ONTOLOGY_PHOTO_IMAGE_WIDTH);
    QVariant oldHeight = oldData.value(FACEBOOK_ONTOLOGY_PHOTO_IMAGE_HEIGHT);
    QVariant newHeight = newData.value(FACEBOOK_ONTOLOGY_PHOTO_IMAGE_HEIGHT);

    if (newSource != oldSource)
        emit q->sourceChanged();
    if (newWidth != oldWidth)
        emit q->widthChanged();
    if (newHeight != oldHeight)
        emit q->heightChanged();

    // Call super class implementation
    ContentItemInterfacePrivate::emitPropertyChangeSignals(oldData, newData);
}

//-------------------------------

/*!
    \qmltype FacebookPhotoImage
    \instantiates FacebookPhotoImageInterface
    An entry representing a image size for a photo
*/
FacebookPhotoImageInterface::FacebookPhotoImageInterface(QObject *parent)
    : ContentItemInterface(*(new FacebookPhotoImageInterfacePrivate(this)), parent)
{
}

/*! \reimp */
int FacebookPhotoImageInterface::type() const
{
    return FacebookInterface::PhotoImage;
}

#if 0

#endif
/*!
    \qmlproperty QUrl FacebookPhotoImage::source
    Holds the source of the image
*/
QUrl FacebookPhotoImageInterface::source() const
{
    return QUrl::fromEncoded(data().value(FACEBOOK_ONTOLOGY_PHOTO_IMAGE_SOURCE).toString().toLocal8Bit());
}

/*!
    \qmlproperty int FacebookPhotoImage::width
    Holds the width of the image
*/
int FacebookPhotoImageInterface::width() const
{
    QString numberString = data().value(FACEBOOK_ONTOLOGY_PHOTO_IMAGE_WIDTH).toString();
    bool ok;
    int number = numberString.toInt(&ok);
    if (ok) {
        return number;
    }
    return -1;
}

/*!
    \qmlproperty int FacebookPhotoImage::height
    Holds the height of the image
*/
int FacebookPhotoImageInterface::height() const
{
    QString numberString = data().value(FACEBOOK_ONTOLOGY_PHOTO_IMAGE_HEIGHT).toString();
    bool ok;
    int number = numberString.toInt(&ok);
    if (ok) {
        return number;
    }
    return -1;
}

