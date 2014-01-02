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

#ifndef FACEBOOKITEMFILTERINTERFACE_H
#define FACEBOOKITEMFILTERINTERFACE_H

#include "filterinterface.h"

class FacebookItemFilterInterfacePrivate;
class FacebookItemFilterInterface : public FilterInterface
{
    Q_OBJECT
    Q_PROPERTY(QString identifier READ identifier WRITE setIdentifier NOTIFY identifierChanged)
    Q_PROPERTY(QString fields READ fields WRITE setFields NOTIFY fieldsChanged)
public:
    explicit FacebookItemFilterInterface(QObject *parent = 0);

    // Properties
    QString identifier() const;
    void setIdentifier(const QString &identifier);
    QString fields() const;
    void setFields(const QString &fields);

    // Non QML API
    // Used by items
    bool isAcceptable(QObject *item, SocialNetworkInterface *socialNetwork) const;
    bool performLoadRequestImpl(QObject *item, SocialNetworkInterface *socialNetwork,
                                LoadType loadType);

Q_SIGNALS:
    void identifierChanged();
    void fieldsChanged();

protected:
    bool performSetItemDataImpl(IdentifiableContentItemInterface *item,
                                SocialNetworkInterface *socialNetwork,
                                const QByteArray &data, LoadType loadType,
                                const QVariantMap &properties);
private:
    Q_DECLARE_PRIVATE(FacebookItemFilterInterface)

};

#endif // FACEBOOKITEMFILTERINTERFACE_H
