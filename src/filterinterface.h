/*
 * Copyright (C) 2013 Jolla Ltd. <lucien.xu@jollamobile.com>
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

#ifndef FILTERINTERFACE_H
#define FILTERINTERFACE_H

#include <QtCore/QObject>
#include <QtCore/QVariantMap>
#include "socialnetworkinterface.h"

class IdentifiableContentItemInterface;
class SocialNetworkModelInterface;
class FilterInterfacePrivate;
class FilterInterface : public QObject
{
    Q_OBJECT
public:
    explicit FilterInterface(QObject *parent = 0);
    virtual ~FilterInterface();

    // Non QML API
    // Used by items
    virtual bool isAcceptable(QObject *item, SocialNetworkInterface *socialNetwork) const;
    virtual bool performLoadRequest(QObject *item, SocialNetworkInterface *socialNetwork);

    // Used by SNI
    bool performSetData(QObject *handle, const QByteArray &data);
    bool performSetError(QObject *handle, SocialNetworkInterface::ErrorType error,
                         const QString &errorMessage);
protected:
    virtual bool performLoadRequestImpl(QObject *item, SocialNetworkInterface *socialNetwork);
    virtual bool performSetItemDataImpl(IdentifiableContentItemInterface *item,
                                        SocialNetworkInterface *socialNetwork,
                                        const QByteArray &data,
                                        const QVariantMap &properties);
    virtual bool performSetModelDataImpl(SocialNetworkModelInterface *model,
                                         SocialNetworkInterface *socialNetwork,
                                         const QByteArray &data,
                                         const QVariantMap &properties);
    explicit FilterInterface(FilterInterfacePrivate &dd, QObject *parent = 0);
    QScopedPointer<FilterInterfacePrivate> d_ptr;
private:
    Q_DECLARE_PRIVATE(FilterInterface)
    Q_PRIVATE_SLOT(d_func(), void socialNetworkDestroyedHandler(QObject*))
    Q_PRIVATE_SLOT(d_func(), void itemDestroyedHandler(QObject*))
};

#endif // FILTERINTERFACE_H
