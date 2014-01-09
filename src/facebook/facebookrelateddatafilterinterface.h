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

#ifndef FACEBOOKRELATEDDATAFILTERINTERFACE_H
#define FACEBOOKRELATEDDATAFILTERINTERFACE_H

#include "filterinterface.h"
#include "facebookinterface.h"

class FacebookRelatedDataFilterInterfacePrivate;
class FacebookRelatedDataFilterInterface : public FilterInterface
{
    Q_OBJECT
    Q_PROPERTY(QString identifier READ identifier WRITE setIdentifier NOTIFY identifierChanged)
    Q_PROPERTY(QString fields READ fields WRITE setFields NOTIFY fieldsChanged)
    Q_PROPERTY(int limit READ limit WRITE setLimit NOTIFY limitChanged)
    Q_PROPERTY(int offset READ offset WRITE setOffset NOTIFY offsetChanged)
    Q_PROPERTY(FacebookInterface::ConnectionType connection READ connection WRITE setConnection
               NOTIFY connectionChanged)
    Q_PROPERTY(QString sectionField READ sectionField WRITE setSectionField NOTIFY sectionFieldChanged)
public:
    explicit FacebookRelatedDataFilterInterface(QObject *parent = 0);

    // Properties
    QString identifier() const;
    void setIdentifier(const QString &identifier);
    QString fields() const;
    void setFields(const QString &fields);
    int limit() const;
    void setLimit(int limit);
    int offset() const;
    void setOffset(int offset);
    FacebookInterface::ConnectionType connection() const;
    void setConnection(FacebookInterface::ConnectionType connection);
    QString sectionField() const;
    void setSectionField(const QString &sectionField);

    // Non QML API
    // Used by items
    bool isAcceptable(QObject *item, SocialNetworkInterface *socialNetwork) const;

    // Non QML API
    // Used by SNMI
    QString dataSection(const QVariantMap &data);
Q_SIGNALS:
    void identifierChanged();
    void fieldsChanged();
    void limitChanged();
    void offsetChanged();
    void connectionChanged();
    void sectionFieldChanged();
protected:
    bool performLoadRequestImpl(QObject *item, SocialNetworkInterface *socialNetwork,
                                LoadType loadType);
    bool performSetModelDataImpl(SocialNetworkModelInterface *model,
                                 SocialNetworkInterface *socialNetwork, const QByteArray &data,
                                 LoadType loadType, const QVariantMap &properties);
    virtual bool isDataAcceptable(const QVariantMap &data);
private:
    Q_DECLARE_PRIVATE(FacebookRelatedDataFilterInterface)
};

#endif // FACEBOOKRELATEDDATAFILTERINTERFACE_H
