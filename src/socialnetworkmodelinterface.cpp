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

#include "socialnetworkmodelinterface.h"
#include "socialnetworkinterface_p.h"
#include "identifiablecontentiteminterface.h"
#include <QtCore/QCoreApplication>
#include <QtCore/QDebug>
#include <algorithm>

#include "socialnetworkmodelinterface_p.h"

SocialNetworkModelInterfacePrivate::SocialNetworkModelInterfacePrivate(SocialNetworkModelInterface *q)
    : status(SocialNetworkInterface::Initializing)
    , error(SocialNetworkInterface::NoError)
    , socialNetwork(0)
    , nodeType(0)
    , node(0), hasPrevious(false), hasNext(false)
    , resortUpdatePosted(false)
    , q_ptr(q)
{
}

SocialNetworkModelInterfacePrivate::~SocialNetworkModelInterfacePrivate()
{
}

void SocialNetworkModelInterfacePrivate::init()
{
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
    Q_Q(SocialNetworkModelInterface);
    q->setRoleNames(roleNames());
#endif
}

QHash<int, QByteArray> SocialNetworkModelInterfacePrivate::roleNames()
{
    QHash<int, QByteArray> roles;
    roles.insert(SocialNetworkModelInterface::ContentItemRole, "contentItem");
    roles.insert(SocialNetworkModelInterface::ContentItemTypeRole, "contentItemType");
    roles.insert(SocialNetworkModelInterface::ContentItemDataRole, "contentItemData" );
    roles.insert(SocialNetworkModelInterface::ContentItemIdentifierRole, "contentItemIdentifier");
    roles.insert(SocialNetworkModelInterface::SectionRole, "section");
    return roles;
}

/*! \internal */
void SocialNetworkModelInterfacePrivate::filters_append(QDeclarativeListProperty<FilterInterface> *list,
                                                        FilterInterface *filter)
{
    SocialNetworkModelInterface *model = qobject_cast<SocialNetworkModelInterface *>(list->object);
    if (model && filter) {
        model->d_func()->filters.append(filter);
    }
}

/*! \internal */
FilterInterface *SocialNetworkModelInterfacePrivate::filters_at(QDeclarativeListProperty<FilterInterface> *list, int index)
{
    SocialNetworkModelInterface *model = qobject_cast<SocialNetworkModelInterface *>(list->object);
    if (model && model->d_func()->filters.count() > index && index >= 0)
        return model->d_func()->filters.at(index);
    return 0;
}

/*! \internal */
void SocialNetworkModelInterfacePrivate::filters_clear(QDeclarativeListProperty<FilterInterface> *list)
{
    SocialNetworkModelInterface *model = qobject_cast<SocialNetworkModelInterface *>(list->object);
    if (model) {
        model->d_func()->filters.clear();
    }
}

/*! \internal */
int SocialNetworkModelInterfacePrivate::filters_count(QDeclarativeListProperty<FilterInterface> *list)
{
    SocialNetworkModelInterface *model = qobject_cast<SocialNetworkModelInterface *>(list->object);
    if (model)
        return model->d_func()->filters.count();
    return 0;
}

/*! \internal */
void SocialNetworkModelInterfacePrivate::sorters_append(QDeclarativeListProperty<SorterInterface> *list,
                                                   SorterInterface *sorter)
{
    SocialNetworkModelInterface *model = qobject_cast<SocialNetworkModelInterface *>(list->object);
    if (model && sorter) {
        QObject::connect(sorter, SIGNAL(destroyed(QObject*)),
                         model, SLOT(sorterDestroyedHandler(QObject*)));
        model->d_func()->sorters.append(sorter);
        if (!model->d_func()->resortUpdatePosted) {
            model->d_func()->resortUpdatePosted = true;
            QCoreApplication::instance()->postEvent(model, new QEvent(QEvent::User));
        }
    }
}

/*! \internal */
SorterInterface *SocialNetworkModelInterfacePrivate::sorters_at(QDeclarativeListProperty<SorterInterface> *list, int index)
{
    SocialNetworkModelInterface *model = qobject_cast<SocialNetworkModelInterface *>(list->object);
    if (model && model->d_func()->sorters.count() > index && index >= 0)
        return model->d_func()->sorters.at(index);
    return 0;
}

/*! \internal */
void SocialNetworkModelInterfacePrivate::sorters_clear(QDeclarativeListProperty<SorterInterface> *list)
{
    SocialNetworkModelInterface *model = qobject_cast<SocialNetworkModelInterface *>(list->object);
    if (model) {
        model->d_func()->sorters.clear();
        if (!model->d_func()->resortUpdatePosted) {
            model->d_func()->resortUpdatePosted = true;
            QCoreApplication::instance()->postEvent(model, new QEvent(QEvent::User));
        }
    }
}

/*! \internal */
int SocialNetworkModelInterfacePrivate::sorters_count(QDeclarativeListProperty<SorterInterface> *list)
{
    SocialNetworkModelInterface *model = qobject_cast<SocialNetworkModelInterface *>(list->object);
    if (model)
        return model->d_func()->sorters.count();
    return 0;
}

void SocialNetworkModelInterfacePrivate::resort()
{
    Q_Q(SocialNetworkModelInterface);
    if (sorters.isEmpty()) {
        return;
    }
    foreach (SorterInterface *sorter, sorters) {
        SorterFunctor sorterFunctor(sorter);
        std::stable_sort(modelData.begin(), modelData.end(), sorterFunctor);
    }
    emit q->dataChanged(q->index(0), q->index(modelData.count() - 1));
}

void SocialNetworkModelInterfacePrivate::setNode(IdentifiableContentItemInterface *newNode)
{
    Q_Q(SocialNetworkModelInterface);
    if (node != newNode) {
        node = newNode;
        emit q->nodeChanged();
    }
}

void SocialNetworkModelInterfacePrivate::clean()
{
    Q_Q(SocialNetworkModelInterface);
    if (modelData.isEmpty()) {
        return;
    }

    q->beginRemoveRows(QModelIndex(), 0, modelData.count() - 1);
    modelData.clear();
    emit q->countChanged();
    q->endRemoveRows();

    setHavePreviousAndNext(false, false);
}

void SocialNetworkModelInterfacePrivate::setData(const CacheEntry::List &data)
{
    Q_Q(SocialNetworkModelInterface);
    if (data.isEmpty()) {
        clean();
        return;
    }

    if (!modelData.isEmpty()) {
        q->beginRemoveRows(QModelIndex(), 0, modelData.count() - 1);
        modelData.clear();
        q->endRemoveRows();
    }

    prependData(data);
    resort();
}

void SocialNetworkModelInterfacePrivate::prependData(const CacheEntry::List &data)
{
    Q_Q(SocialNetworkModelInterface);
    if (data.isEmpty()) {
        return;
    }

    q->beginInsertRows(QModelIndex(), 0, data.count() - 1);
    CacheEntry::List newData = data;
    newData.append(modelData);
    modelData = newData;
    emit q->countChanged();
    q->endInsertRows();
    resort();
}

void SocialNetworkModelInterfacePrivate::appendData(const CacheEntry::List &data)
{
    Q_Q(SocialNetworkModelInterface);
    if (data.isEmpty()) {
        return;
    }

    q->beginInsertRows(QModelIndex(), modelData.count(), modelData.count() + data.count() - 1);
    modelData.append(data);
    emit q->countChanged();
    q->endInsertRows();
    resort();
}

void SocialNetworkModelInterfacePrivate::setStatus(SocialNetworkInterface::Status newStatus)
{
    Q_Q(SocialNetworkModelInterface);
    if (status != newStatus) {
        status = newStatus;
        emit q->statusChanged();
    }
}

void SocialNetworkModelInterfacePrivate::setError(SocialNetworkInterface::ErrorType newError,
                                                  const QString &newErrorMessage)
{
    Q_Q(SocialNetworkModelInterface);
    if (error != newError) {
        error = newError;
        emit q->errorChanged();
    }
    if (errorMessage != newErrorMessage) {
        errorMessage = newErrorMessage;
        emit q->errorMessageChanged();
    }
    setStatus(SocialNetworkInterface::Error);
}

void SocialNetworkModelInterfacePrivate::setHavePreviousAndNext(bool newHasPrevious, bool newHasNext)
{
    Q_Q(SocialNetworkModelInterface);
    if (hasPrevious != newHasPrevious) {
        hasPrevious = newHasPrevious;
        emit q->hasPreviousChanged();
    }

    if (hasNext != newHasNext) {
        hasNext = newHasNext;
        emit q->hasNextChanged();
    }
}

void SocialNetworkModelInterfacePrivate::sorterDestroyedHandler(QObject *object)
{
    Q_Q(SocialNetworkModelInterface);
    SorterInterface *sorter = static_cast<SorterInterface *>(object);
    sorters.removeAll(sorter);

    // We should resort with the new filters
    if (!resortUpdatePosted) {
        resortUpdatePosted = true;
        QCoreApplication::instance()->postEvent(q, new QEvent(QEvent::User));
    }
}

//----------------------------------------------------

SocialNetworkModelInterface::SocialNetworkModelInterface(QObject *parent):
    QAbstractListModel(parent), d_ptr(new SocialNetworkModelInterfacePrivate(this))
{
    Q_D(SocialNetworkModelInterface);
    d->init();
}

SocialNetworkModelInterface::~SocialNetworkModelInterface()
{
}

void SocialNetworkModelInterface::classBegin()
{
}

void SocialNetworkModelInterface::componentComplete()
{
}

int SocialNetworkModelInterface::rowCount(const QModelIndex &index) const
{
    Q_D(const SocialNetworkModelInterface);
    Q_UNUSED(index)
    return d->modelData.count();
}

QVariant SocialNetworkModelInterface::data(const QModelIndex &index, int role) const
{
    Q_D(const SocialNetworkModelInterface);
    if (!index.isValid() || index.row() >= d->modelData.count() || index.row() < 0) {
        return QVariant();
    }

    CacheEntry::Ptr cacheEntry = d->modelData.at(index.row());

    switch (role) {
        case ContentItemTypeRole: {
            return QVariant::fromValue(cacheEntry->data().value(NEMOQMLPLUGINS_SOCIAL_CONTENTITEMTYPE).toInt());
        }
        case ContentItemDataRole: {
            return QVariant::fromValue(cacheEntry->data());
        }
        case ContentItemIdentifierRole: {
            return QVariant::fromValue(cacheEntry->data().value(NEMOQMLPLUGINS_SOCIAL_CONTENTITEMID).toString());
        }
        case ContentItemRole: {
            if (cacheEntry->item()) {
                return QVariant::fromValue(cacheEntry->item());
            }

            return QVariant::fromValue(d->socialNetwork->d_func()->createItem(cacheEntry));
        }
        case SectionRole: {
            return d->socialNetwork->d_func()->dataSection(cacheEntry->data().value(NEMOQMLPLUGINS_SOCIAL_CONTENTITEMTYPE).toInt(),
                                                           cacheEntry->data());
        }
        default: {
            return QVariant();
        }
    }
}

SocialNetworkInterface::Status SocialNetworkModelInterface::status() const
{
    Q_D(const SocialNetworkModelInterface);
    return d->status;
}

SocialNetworkInterface::ErrorType SocialNetworkModelInterface::error() const
{
    Q_D(const SocialNetworkModelInterface);
    return d->error;
}

QString SocialNetworkModelInterface::errorMessage() const
{
    Q_D(const SocialNetworkModelInterface);
    return d->errorMessage;
}

SocialNetworkInterface * SocialNetworkModelInterface::socialNetwork() const
{
    Q_D(const SocialNetworkModelInterface);
    return d->socialNetwork;
}

QString SocialNetworkModelInterface::nodeIdentifier() const
{
    Q_D(const SocialNetworkModelInterface);
    return d->nodeIdentifier;
}

int SocialNetworkModelInterface::nodeType() const
{
    Q_D(const SocialNetworkModelInterface);
    return d->nodeType;
}

IdentifiableContentItemInterface * SocialNetworkModelInterface::node() const
{
    Q_D(const SocialNetworkModelInterface);
    return d->node;
}

bool SocialNetworkModelInterface::hasPrevious() const
{
    Q_D(const SocialNetworkModelInterface);
    return d->hasPrevious;
}

bool SocialNetworkModelInterface::hasNext() const
{
    Q_D(const SocialNetworkModelInterface);
    return d->hasNext;
}

QDeclarativeListProperty<FilterInterface> SocialNetworkModelInterface::filters()
{
    return QDeclarativeListProperty<FilterInterface>(this, 0,
            &SocialNetworkModelInterfacePrivate::filters_append,
            &SocialNetworkModelInterfacePrivate::filters_count,
            &SocialNetworkModelInterfacePrivate::filters_at,
            &SocialNetworkModelInterfacePrivate::filters_clear);
}

/*!
    \qmlproperty QDeclarativeListProperty<Sorter> SocialNetwork::sorters
    Holds the list of sorters which will be applied to the related content
    of the node.  The order of sorters in the list is important, as it
    defines which sorting is applied first.

    Specific implementations of the SocialNetwork interface may not support
    certain standard sorter types, or they may not support sorting at all.
*/
QDeclarativeListProperty<SorterInterface> SocialNetworkModelInterface::sorters()
{
    return QDeclarativeListProperty<SorterInterface>(this, 0,
            &SocialNetworkModelInterfacePrivate::sorters_append,
            &SocialNetworkModelInterfacePrivate::sorters_count,
            &SocialNetworkModelInterfacePrivate::sorters_at,
            &SocialNetworkModelInterfacePrivate::sorters_clear);
}

int SocialNetworkModelInterface::count() const
{
    Q_D(const SocialNetworkModelInterface);
    return d->modelData.count();
}

void SocialNetworkModelInterface::setSocialNetwork(SocialNetworkInterface *socialNetwork)
{
    Q_D(SocialNetworkModelInterface);
    if (d->socialNetwork != socialNetwork) {
        if (d->socialNetwork) {
            d->socialNetwork->d_func()->removeModel(this);
        }

        d->socialNetwork = socialNetwork;
        d->socialNetwork->d_func()->addModel(this);
        emit socialNetworkChanged();
    }
}

void SocialNetworkModelInterface::setNodeIdentifier(const QString &nodeIdentifier)
{
    Q_D(SocialNetworkModelInterface);
    if (d->nodeIdentifier != nodeIdentifier) {
        d->nodeIdentifier = nodeIdentifier;
        emit nodeIdentifierChanged();
    }
}

/*!
    \qmlmethod QObject *SocialNetworkModel::relatedItem(int index)
    Returns the ContentItem which is related to the node from the given
    \a index of the model data.  This is identical to calling
    \c data() for the given model index and specifying the \c contentItem
    role.

    \note Although this function will always return a pointer to a
    ContentItem, the return type of the function is QObject *, so that
    this function can be used via QMetaObject::invokeMethod().
*/
QObject * SocialNetworkModelInterface::relatedItem(int index) const
{
    QVariant itemVariant = data(QAbstractListModel::index(index),
                                SocialNetworkInterface::ContentItemRole);
    if (!itemVariant.isValid()) {
        return 0;
    }
    return itemVariant.value<ContentItemInterface*>();
}


void SocialNetworkModelInterface::setNodeType(int nodeType)
{
    Q_D(SocialNetworkModelInterface);
    if (d->nodeType != nodeType) {
        d->nodeType = nodeType;
        emit nodeTypeChanged();
    }
}

void SocialNetworkModelInterface::populate()
{
    Q_D(SocialNetworkModelInterface);
    if (!d->socialNetwork) {
        qWarning() << Q_FUNC_INFO << "Cannot call populate when not SocialNetwork is set";
        return;
    }

    d->socialNetwork->d_func()->populate(this, d->nodeIdentifier, d->nodeType, d->filters);
}

void SocialNetworkModelInterface::repopulate()
{
    Q_D(SocialNetworkModelInterface);
    if (!d->socialNetwork) {
        qWarning() << Q_FUNC_INFO << "Cannot call repopulate when not SocialNetwork is set";
        return;
    }

    d->socialNetwork->d_func()->populate(this, d->nodeIdentifier, d->nodeType, d->filters, true);
}

void SocialNetworkModelInterface::loadNext()
{
    Q_D(SocialNetworkModelInterface);
    if (hasNext()) {
        d->socialNetwork->d_func()->loadNext(this);
    }
}

void SocialNetworkModelInterface::loadPrevious()
{
    Q_D(SocialNetworkModelInterface);
    if (hasPrevious()) {
        d->socialNetwork->d_func()->loadPrevious(this);
    }
}

void SocialNetworkModelInterface::clean()
{
    Q_D(SocialNetworkModelInterface);
    d->clean();
}

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
QHash<int, QByteArray> SocialNetworkModelInterface::roleNames() const
{
    return SocialNetworkModelInterfacePrivate::roleNames();
}
#endif

bool SocialNetworkModelInterface::event(QEvent *e)
{
    Q_D(SocialNetworkModelInterface);
    if (e->type() == QEvent::User) {
        d->resortUpdatePosted = false;
        d->resort();
        return true;
    }
    return QObject::event(e);
}

#include "moc_socialnetworkmodelinterface.cpp"
