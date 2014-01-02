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

#include <QtCore/QMap>
#include <QtCore/QObject>
#include <QtCore/QVariantMap>

#include "filterinterface.h"

template<class T> bool testType(QObject *object)
{
    return qobject_cast<T *>(object);
}

class SocialNetworkInterface;
class FilterInterfacePrivate
{
public:
    explicit FilterInterfacePrivate(FilterInterface *q);
    bool addHandle(QObject *handle, QObject *item, SocialNetworkInterface *socialNetwork,
                   FilterInterface::LoadType loadType);
    void addHandleProperties(QObject *handle, const QVariantMap &properties);
protected:
    FilterInterface * const q_ptr;
private:
    Q_DECLARE_PUBLIC(FilterInterface)
    // Slots
    void socialNetworkDestroyedHandler(QObject *object);
    void itemDestroyedHandler(QObject *object);

    QMap<QObject *, QObject *> handlesToItemMap;
    QMap<QObject *, SocialNetworkInterface *> handlesToSniMap;
    QMap<QObject *, QVariantMap> handlesToPropertiesMap;
    QMap<QObject *, FilterInterface::LoadType> handlesToLoadTypeMap;
    QMultiMap<QObject *, QObject *> sniToHandlesMap;
    QMap<QObject *, QObject *> itemToHandleMap;
};
