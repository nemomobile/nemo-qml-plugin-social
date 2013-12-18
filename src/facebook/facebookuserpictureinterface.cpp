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

#include "facebookuserpictureinterface.h"
#include "facebookinterface.h"
#include "facebookontology_p.h"
#include "contentiteminterface_p.h"
// <<< include
// >>> include

class FacebookUserPictureInterfacePrivate: public ContentItemInterfacePrivate
{
public:
    explicit FacebookUserPictureInterfacePrivate(FacebookUserPictureInterface *q);
    void emitPropertyChangeSignals(const QVariantMap &oldData, const QVariantMap &newData);
private:
    Q_DECLARE_PUBLIC(FacebookUserPictureInterface)
};

FacebookUserPictureInterfacePrivate::FacebookUserPictureInterfacePrivate(FacebookUserPictureInterface *q)
    : ContentItemInterfacePrivate(q)
{
}

void FacebookUserPictureInterfacePrivate::emitPropertyChangeSignals(const QVariantMap &oldData,
                                                                    const QVariantMap &newData)
{
    Q_Q(FacebookUserPictureInterface);
    QVariant oldUrl = oldData.value(FACEBOOK_ONTOLOGY_USER_PICTURE_URL);
    QVariant newUrl = newData.value(FACEBOOK_ONTOLOGY_USER_PICTURE_URL);
    QVariant oldIsSilhouette = oldData.value(FACEBOOK_ONTOLOGY_USER_PICTURE_ISSILHOUETTE);
    QVariant newIsSilhouette = newData.value(FACEBOOK_ONTOLOGY_USER_PICTURE_ISSILHOUETTE);

    if (newUrl != oldUrl)
        emit q->urlChanged();
    if (newIsSilhouette != oldIsSilhouette)
        emit q->isSilhouetteChanged();

    // Call super class implementation
    ContentItemInterfacePrivate::emitPropertyChangeSignals(oldData, newData);
}

//-------------------------------

/*!
    \qmltype FacebookUserPicture
    \instantiates FacebookUserPictureInterface
    An entry representing a picture
*/
FacebookUserPictureInterface::FacebookUserPictureInterface(QObject *parent)
    : ContentItemInterface(*(new FacebookUserPictureInterfacePrivate(this)), parent)
{
}

/*! \reimp */
int FacebookUserPictureInterface::type() const
{
    return FacebookInterface::UserPicture;
}

#if 0

#endif
/*!
    \qmlproperty QUrl FacebookUserPicture::url
    Holds the url to the image source of the picture.
*/
QUrl FacebookUserPictureInterface::url() const
{
    return QUrl::fromEncoded(data().value(FACEBOOK_ONTOLOGY_USER_PICTURE_URL).toString().toLocal8Bit());
}

/*!
    \qmlproperty bool FacebookUserPicture::isSilhouette
    Whether the picture is a default, anonymous silhouette image.
*/
bool FacebookUserPictureInterface::isSilhouette() const
{
    return data().value(FACEBOOK_ONTOLOGY_USER_PICTURE_ISSILHOUETTE).toString() == QLatin1String("true");
}

