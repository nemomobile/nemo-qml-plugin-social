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

#include "facebookitemfilterinterface.h"
#include "filterinterface_p.h"
#include "facebookinterface.h"
#include "identifiablecontentiteminterface.h"
#include "identifiablecontentiteminterface_p.h"
#include "facebookontology_p.h"
#include "facebookuserinterface.h"

// TODO: we need to find a way to solve types in FB

class FacebookItemFilterInterfacePrivate: public FilterInterfacePrivate
{
public:
    explicit FacebookItemFilterInterfacePrivate(FacebookItemFilterInterface *q);
private:
    Q_DECLARE_PUBLIC(FacebookItemFilterInterface)
    QString identifier;
    QString fields;
};

FacebookItemFilterInterfacePrivate::FacebookItemFilterInterfacePrivate(FacebookItemFilterInterface *q)
    : FilterInterfacePrivate(q)
{
}

FacebookItemFilterInterface::FacebookItemFilterInterface(QObject *parent) :
    FilterInterface(*(new FacebookItemFilterInterfacePrivate(this)), parent)
{
}

QString FacebookItemFilterInterface::identifier() const
{
    Q_D(const FacebookItemFilterInterface);
    return d->identifier;
}

QString FacebookItemFilterInterface::fields() const
{
    Q_D(const FacebookItemFilterInterface);
    return d->fields;
}

void FacebookItemFilterInterface::setIdentifier(const QString &identifier)
{
    Q_D(FacebookItemFilterInterface);
    if (d->identifier != identifier) {
        d->identifier = identifier;
        emit identifierChanged();
    }
}

void FacebookItemFilterInterface::setFields(const QString &fields)
{
    Q_D(FacebookItemFilterInterface);
    if (d->fields != fields) {
        d->fields = fields;
        emit fieldsChanged();
    }
}

bool FacebookItemFilterInterface::isAcceptable(QObject *item,
                                               SocialNetworkInterface *socialNetwork) const
{
    if (!testType<FacebookInterface>(socialNetwork)) {
        return false;
    }

    // This filter only works for items
    if (!testType<IdentifiableContentItemInterface>(item)) {
        return false;
    }

    // TODO: should we check for specific Facebook items ?
//    if (!testType<FacebookUserInterface>(item)) {
//        return false;
//    }

    return true;
}

bool FacebookItemFilterInterface::performLoadRequestImpl(QObject *item,
                                                         SocialNetworkInterface *socialNetwork,
                                                         LoadType loadType)
{
    Q_D(FacebookItemFilterInterface);
    Q_UNUSED(loadType)
    FacebookInterface *facebook = qobject_cast<FacebookInterface *>(socialNetwork);
    if (!facebook) {
        return false;
    }

    return d->addHandle(facebook->get(this, d->identifier, QString(), d->fields), item,
                        socialNetwork, loadType);
}

bool FacebookItemFilterInterface::performSetItemDataImpl(IdentifiableContentItemInterface *item,
                                                         SocialNetworkInterface *socialNetwork,
                                                         const QByteArray &data, LoadType loadType,
                                                         const QVariantMap &properties)
{
    Q_UNUSED(socialNetwork)
    Q_UNUSED(properties)
    Q_UNUSED(loadType)
    bool ok = false;
    QVariantMap dataMap = IdentifiableContentItemInterfacePrivate::parseReplyData(data, &ok);

    if (!ok) {
        QString errorMessage = QString(QLatin1String("Unable to parse downloaded data. "\
                                                     "Downloaded data: %1")).arg(QString(data));
        item->setError(SocialNetworkInterface::DataUpdateError, errorMessage);
        return false;
    }

    dataMap.insert(NEMOQMLPLUGINS_SOCIAL_CONTENTITEMID,
                   dataMap.value(FACEBOOK_ONTOLOGY_METADATA_ID).toString());
    item->setData(dataMap);
    return true;
}
