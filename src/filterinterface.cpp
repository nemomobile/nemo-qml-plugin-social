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

#include "filterinterface.h"
#include "filterinterface_p.h"
#include "identifiablecontentiteminterface.h"
#include "socialnetworkmodelinterface.h"

// Small note:
// handle = what SNI gives (usually QNetworkReply *). Used to trace the request.
// item = ICII or SNMI to populate.

FilterInterfacePrivate::FilterInterfacePrivate(FilterInterface *q)
    : q_ptr(q)
{
}

bool FilterInterfacePrivate::addHandle(QObject *handle, QObject *item,
                                       SocialNetworkInterface *socialNetwork,
                                       FilterInterface::LoadType loadType)
{
    if (handlesToItemMap.contains(handle) || handlesToSniMap.contains(handle)
        || handlesToLoadTypeMap.contains(handle)) {
        return false;
    }

    handlesToItemMap.insert(handle, item);
    handlesToSniMap.insert(handle, socialNetwork);
    handlesToLoadTypeMap.insert(handle, loadType);
    sniToHandlesMap.insert(socialNetwork, handle);
    itemToHandleMap.insert(item, handle);
    return true;
}

void FilterInterfacePrivate::addHandleProperties(QObject *handle, const QVariantMap &properties)
{
    if (handlesToItemMap.contains(handle) || handlesToSniMap.contains(handle)
        || handlesToLoadTypeMap.contains(handle)) {
        return;
    }

    handlesToPropertiesMap.insert(handle, properties);
}

void FilterInterfacePrivate::socialNetworkDestroyedHandler(QObject *object)
{
    Q_Q(FilterInterface);
    if (sniToHandlesMap.contains(object)) {
        foreach (QObject *handle, sniToHandlesMap.values(object)) {
            q->performSetError(handle, SocialNetworkInterface::InternalError,
                               "SocialNetwork is destroyed during request");
        }
    }
}

void FilterInterfacePrivate::itemDestroyedHandler(QObject *object)
{
    Q_Q(FilterInterface);
    if (itemToHandleMap.contains(object)) {
        object->disconnect(q);
        QObject *handle = itemToHandleMap.value(object);
        SocialNetworkInterface *socialNetwork = handlesToSniMap.value(handle);
        handlesToItemMap.remove(handle);
        handlesToSniMap.remove(handle);
        handlesToLoadTypeMap.remove(handle);
        handlesToPropertiesMap.remove(handle);
        sniToHandlesMap.remove(socialNetwork, handle);
        itemToHandleMap.remove(object);
    }
}

FilterInterface::FilterInterface(QObject *parent)
    : QObject(parent), d_ptr(new FilterInterfacePrivate(this))
{
}

FilterInterface::FilterInterface(FilterInterfacePrivate &dd, QObject *parent)
    : QObject(parent), d_ptr(&dd)
{
}

FilterInterface::~FilterInterface()
{
}

bool FilterInterface::isAcceptable(QObject *item, SocialNetworkInterface *socialNetwork) const
{
    Q_UNUSED(item);
    Q_UNUSED(socialNetwork)
    return false;
}

bool FilterInterface::performLoadRequest(QObject *item, SocialNetworkInterface *socialNetwork,
                                         LoadType loadType)
{

    bool ok = performLoadRequestImpl(item, socialNetwork, loadType);
    if (ok) {
        connect(socialNetwork, SIGNAL(destroyed(QObject*)),
                this, SLOT(socialNetworkDestroyedHandler(QObject*)));
        connect(item, SIGNAL(destroyed(QObject*)), this, SLOT(itemDestroyedHandler(QObject*)));
    }
    return ok;
}

bool FilterInterface::performSetData(QObject *handle, const QByteArray &data)
{
    Q_D(FilterInterface);
    // TODO: merge the 3 maps to one
    if (!d->handlesToItemMap.contains(handle)) {
        return false;
    }

    if (!d->handlesToSniMap.contains(handle)) {
        return false;
    }

    if (!d->handlesToLoadTypeMap.contains(handle)) {
        return false;
    }

    QVariantMap properties = d->handlesToPropertiesMap.value(handle);
    SocialNetworkInterface *socialNetwork = d->handlesToSniMap.value(handle);
    socialNetwork->disconnect(this);

    LoadType loadType = d->handlesToLoadTypeMap.value(handle, Load);

    QObject *rawItem = d->handlesToItemMap.value(handle);
    IdentifiableContentItemInterface *item
            = qobject_cast<IdentifiableContentItemInterface *>(rawItem);
    SocialNetworkModelInterface *model = qobject_cast<SocialNetworkModelInterface *>(rawItem);

    bool ok = false;
    if (item) {
        ok = performSetItemDataImpl(item, socialNetwork, data, loadType, properties);
    }
    if (model) {
        ok = performSetModelDataImpl(model, socialNetwork, data, loadType, properties);
    }

    d->handlesToItemMap.remove(handle);
    d->handlesToSniMap.remove(handle);
    d->handlesToPropertiesMap.remove(handle);
    d->handlesToLoadTypeMap.remove(handle);
    d->sniToHandlesMap.remove(socialNetwork, handle);
    d->itemToHandleMap.remove(rawItem);
    return ok;
}

bool FilterInterface::performSetError(QObject *handle, SocialNetworkInterface::ErrorType error,
                                      const QString &errorMessage)
{
    Q_D(FilterInterface);
    if (!d->handlesToItemMap.contains(handle)) {
        return false;
    }

    if (!d->handlesToSniMap.contains(handle)) {
        return false;
    }

    if (!d->handlesToLoadTypeMap.contains(handle)) {
        return false;
    }

    SocialNetworkInterface *socialNetwork = d->handlesToSniMap.value(handle);
    socialNetwork->disconnect(this);

    QObject *rawItem = d->handlesToItemMap.value(handle);

    IdentifiableContentItemInterface *item
            = qobject_cast<IdentifiableContentItemInterface *>(rawItem);
    SocialNetworkModelInterface *model = qobject_cast<SocialNetworkModelInterface *>(rawItem);
    if (item) {
        item->setError(error, errorMessage);
    }
    if (model) {
        model->setError(error, errorMessage);
    }

    d->handlesToItemMap.remove(handle);
    d->handlesToSniMap.remove(handle);
    d->handlesToPropertiesMap.remove(handle);
    d->handlesToLoadTypeMap.remove(handle);
    d->sniToHandlesMap.remove(socialNetwork, handle);
    d->itemToHandleMap.remove(rawItem);
    return true;
}

bool FilterInterface::performLoadRequestImpl(QObject *item, SocialNetworkInterface *socialNetwork,
                                             LoadType loadType)
{
    Q_UNUSED(item)
    Q_UNUSED(socialNetwork)
    Q_UNUSED(loadType)
    return false;
}

bool FilterInterface::performSetItemDataImpl(IdentifiableContentItemInterface *item,
                                             SocialNetworkInterface *socialNetwork,
                                             const QByteArray &data, LoadType loadType,
                                             const QVariantMap &properties)
{
    Q_UNUSED(item)
    Q_UNUSED(socialNetwork)
    Q_UNUSED(data)
    Q_UNUSED(loadType)
    Q_UNUSED(properties)
    return false;
}

bool FilterInterface::performSetModelDataImpl(SocialNetworkModelInterface *model,
                                              SocialNetworkInterface *socialNetwork,
                                              const QByteArray &data, LoadType loadType,
                                              const QVariantMap &properties)
{
    Q_UNUSED(model)
    Q_UNUSED(socialNetwork)
    Q_UNUSED(data)
    Q_UNUSED(loadType)
    Q_UNUSED(properties)
    return false;
}

#include "moc_filterinterface.cpp"
