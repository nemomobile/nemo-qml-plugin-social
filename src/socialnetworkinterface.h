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

#ifndef SOCIALNETWORKINTERFACE_H
#define SOCIALNETWORKINTERFACE_H

#include <QtCore/QAbstractListModel>
#include <QtCore/QObject>

#include <QtGlobal>
#if QT_VERSION_5
#include <QtQml>
#include <QQmlParserStatus>
#include <QQmlListProperty>
#define QDeclarativeParserStatus QQmlParserStatus
#define QDeclarativeListProperty QQmlListProperty
#else
#include <qdeclarative.h>
#include <QDeclarativeParserStatus>
#include <QDeclarativeListProperty>
#endif

#include "filterinterface.h"
#include "sorterinterface.h"


class QNetworkReply;
class ContentItemInterface;
class IdentifiableContentItemInterface;

/*
 * NOTE: if you construct one of these in C++ directly,
 * you MUST call classBegin() and componentCompleted()
 * directly after construction.
 */

class CacheEntry;
class SocialNetworkModelInterface;
class SocialNetworkInterfacePrivate;
class SocialNetworkInterface : public QObject, public QDeclarativeParserStatus
{
    Q_OBJECT
    Q_INTERFACES(QDeclarativeParserStatus)
    Q_PROPERTY(bool initialized READ isInitialized NOTIFY initializedChanged)

    Q_ENUMS(Status)
    Q_ENUMS(ErrorType)
    Q_ENUMS(ContentType)
    Q_ENUMS(RequestType)

public:
    enum Status {
        Initializing,
        Idle,
        Busy,
        Error,
        Invalid
    };
    enum ErrorType {
        NoError = 0,
        AccountError = 1,
        SignOnError = 2,
        BusyError = 3,
        RequestError = 4,
        DataUpdateError = 5,
        OtherError = 6,
        LastError = 255
    };
    enum Roles {
        ContentItemRole = Qt::UserRole + 1,
        ContentItemTypeRole,
        ContentItemDataRole,
        ContentItemIdentifierRole, // 0 for unidentifiable content items
        SectionRole
    };
    enum ContentType {
        NotInitialized = 0,
        Unknown = 1
    };
    enum RequestType {
        GetRequest = 0,
        PostRequest,
        DeleteRequest
    };
public:
    explicit SocialNetworkInterface(QObject *parent = 0);
    virtual ~SocialNetworkInterface();
    bool isInitialized() const;

    // QDeclarativeParserStatus
    virtual void classBegin();
    virtual void componentComplete();

public:
    Q_INVOKABLE bool arbitraryRequest(RequestType requestType, const QString &requestUri,
                                      const QVariantMap &queryItems = QVariantMap(),
                                      const QString &postData = QString());
Q_SIGNALS:
    void arbitraryRequestResponseReceived(bool isError, const QVariantMap &data);
    void initializedChanged();

protected:
    SocialNetworkInterface(SocialNetworkInterfacePrivate &dd, QObject *parent = 0);

    QVariantMap contentItemData(ContentItemInterface *contentItem) const;
    void setContentItemData(ContentItemInterface *contentItem, const QVariantMap &data) const;

    QScopedPointer<SocialNetworkInterfacePrivate> d_ptr;
    friend class IdentifiableContentItemInterfacePrivate;
    friend class ContentItemInterface;
    friend class ContentItemInterfacePrivate;

private:
    Q_DECLARE_PRIVATE(SocialNetworkInterface)
    Q_PRIVATE_SLOT(d_func(), void filterDestroyedHandler(QObject*))
    Q_PRIVATE_SLOT(d_func(), void finishedHandler())
    Q_PRIVATE_SLOT(d_func(), void errorHandler(QNetworkReply::NetworkError))
    Q_PRIVATE_SLOT(d_func(), void sslErrorsHandler(const QList<QSslError>&))
    Q_PRIVATE_SLOT(d_func(), void itemDataChangedHandler())
    Q_PRIVATE_SLOT(d_func(), void modelDestroyedHandler(QObject*))
    friend class SocialNetworkModelInterface;
};

#endif // SOCIALNETWORKINTERFACE_H
