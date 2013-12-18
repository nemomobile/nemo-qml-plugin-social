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

#ifndef IDENTIFIABLECONTENTITEMINTERFACE_H
#define IDENTIFIABLECONTENTITEMINTERFACE_H

#include "contentiteminterface.h"

#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVariantMap>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QSslError>

#include "socialnetworkinterface.h"
#include "filterinterface.h"

#define NEMOQMLPLUGINS_SOCIAL_CONTENTITEMID QLatin1String("org.nemomobile.social.contentitem.id")

/*
 * NOTE: if you construct one of these in C++ directly,
 * you MUST call classBegin() and componentCompleted()
 * directly after construction.
 */

/*
 * An IdentifiableContentItem may be used as a node in a SocialNetwork.
 * Derived types may be instantiated directly by the user if they provide
 * a valid identifier.
 *
 * Every IdentifiableContentItem may be reload()ed or remove()d, which
 * will return true if successfully able to trigger a network request.
 *
 * To upload a new IdentifiableContentItem to the social network, use
 * the invokable functions provided by derived types.  For example, the
 * FacebookAlbum type has the uploadPhoto() function.  Such functions
 * should return true if successfully able to trigger a network request.
 * When a response to the request is received, the responseReceived()
 * signal will be emitted, containing the response \c data.
 */

class IdentifiableContentItemInterfacePrivate;
class IdentifiableContentItemInterface : public ContentItemInterface
{
    Q_OBJECT

    Q_PROPERTY(QString identifier READ identifier NOTIFY identifierChanged)
    Q_PROPERTY(FilterInterface * filter READ filter WRITE setFilter NOTIFY filterChanged)
    Q_PROPERTY(SocialNetworkInterface::Status status READ status NOTIFY statusChanged)
    Q_PROPERTY(SocialNetworkInterface::ErrorType error READ error NOTIFY errorChanged)
    Q_PROPERTY(QString errorMessage READ errorMessage NOTIFY errorMessageChanged)

public:
    explicit IdentifiableContentItemInterface(QObject *parent = 0);

    // Overrides
    bool isIdentifiable() const;

    // Properties
    QString identifier() const;
    FilterInterface * filter() const;
    void setFilter(FilterInterface *filter);
    SocialNetworkInterface::Status status() const;
    SocialNetworkInterface::ErrorType error() const;
    QString errorMessage() const;

    // Invokable API
    Q_INVOKABLE bool load();

    // Non QML API
    void setError(SocialNetworkInterface::ErrorType error, const QString &errorMessage);


Q_SIGNALS:
    void responseReceived(const QVariantMap &data);
    void identifierChanged();
    void filterChanged();
    void statusChanged();
    void errorChanged();
    void errorMessageChanged();

protected:
    explicit IdentifiableContentItemInterface(IdentifiableContentItemInterfacePrivate &dd,
                                              QObject *parent = 0);
private:
    Q_DECLARE_PRIVATE(IdentifiableContentItemInterface)
    Q_PRIVATE_SLOT(d_func(), void filterDestroyedHandler())
};

Q_DECLARE_METATYPE(IdentifiableContentItemInterface*)

#endif // IDENTIFIABLECONTENTITEMINTERFACE_H
