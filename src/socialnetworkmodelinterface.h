/*
 * Copyright (C) 2013 Lucien XU <sfietkonstantin@free.fr>
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
 *   * The names of its contributors may not be used to endorse or promote 
 *     products derived from this software without specific prior written 
 *     permission.
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

#ifndef SOCIALNETWORKMODELINTERFACE_H
#define SOCIALNETWORKMODELINTERFACE_H

#include <QtCore/QAbstractListModel>
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
#include "socialnetworkinterface.h"

class SocialNetworkInterface;
class ContentItemInterface;
class IdentifiableContentItemInterface;
class SocialNetworkModelInterfacePrivate;
class SocialNetworkModelInterface: public QAbstractListModel, public QDeclarativeParserStatus
{
    Q_OBJECT
    Q_INTERFACES(QDeclarativeParserStatus)
    Q_PROPERTY(SocialNetworkInterface * socialNetwork READ socialNetwork WRITE setSocialNetwork
               NOTIFY socialNetworkChanged)
    Q_PROPERTY(FilterInterface * filter READ filter WRITE setFilter NOTIFY filterChanged)
    Q_PROPERTY(SocialNetworkInterface::Status status READ status NOTIFY statusChanged)
    Q_PROPERTY(SocialNetworkInterface::ErrorType error READ error NOTIFY errorChanged)
    Q_PROPERTY(QString errorMessage READ errorMessage NOTIFY errorMessageChanged)
    Q_PROPERTY(bool hasPrevious READ hasPrevious NOTIFY hasPreviousChanged)
    Q_PROPERTY(bool hasNext READ hasNext NOTIFY hasNextChanged)
//    Q_PROPERTY(QDeclarativeListProperty<SorterInterface> sorters READ sorters)
    Q_PROPERTY(int count READ count NOTIFY countChanged)
public:
    enum Roles {
        ContentItemRole = Qt::UserRole + 1,
        ContentItemTypeRole,
        ContentItemDataRole,
        ContentItemIdentifierRole, // 0 for unidentifiable content items
        SectionRole
    };

    explicit SocialNetworkModelInterface(QObject *parent = 0);
    virtual ~SocialNetworkModelInterface();

    // QDeclarativeParserStatus
    virtual void classBegin();
    virtual void componentComplete();

    // QAbstractListModel
    int rowCount(const QModelIndex &index = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role) const;

    // Properties
    SocialNetworkInterface * socialNetwork() const;
    void setSocialNetwork(SocialNetworkInterface *socialNetwork);
    FilterInterface * filter() const;
    void setFilter(FilterInterface *filter);
    SocialNetworkInterface::Status status() const;
    SocialNetworkInterface::ErrorType error() const;
    QString errorMessage() const;
    bool hasPrevious() const;
    bool hasNext() const;

//    QDeclarativeListProperty<SorterInterface> sorters();
    int count() const;

    // Invokable API
    Q_INVOKABLE QObject * relatedItem(int index) const;

    Q_INVOKABLE bool load();
//    Q_INVOKABLE void loadNext();
//    Q_INVOKABLE void loadPrevious();

    // Non QML API
    void setModelData(const QList<ContentItemInterface *> &data);
    QVariantMap extraData() const;
    void setExtraData(const QVariantMap &extraData);
    void setError(SocialNetworkInterface::ErrorType error, const QString &errorMessage);
public Q_SLOTS:
    void clear();

Q_SIGNALS:
    void socialNetworkChanged();
    void filterChanged();
    void statusChanged();
    void errorChanged();
    void errorMessageChanged();
    void hasPreviousChanged();
    void hasNextChanged();
    void countChanged();

protected:
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    QHash<int, QByteArray> roleNames() const;
#endif
//    bool event(QEvent *e);
    QScopedPointer<SocialNetworkModelInterfacePrivate> d_ptr;
private:
    Q_DECLARE_PRIVATE(SocialNetworkModelInterface)
    Q_PRIVATE_SLOT(d_func(), void socialNetworkInitializedChangedHandler())
    Q_PRIVATE_SLOT(d_func(), void socialNetworkDestroyedHandler())
    Q_PRIVATE_SLOT(d_func(), void filterDestroyedHandler())
//    Q_PRIVATE_SLOT(d_func(), void sorterDestroyedHandler(QObject*))
};

#endif // SOCIALNETWORKMODELINTERFACE_H
