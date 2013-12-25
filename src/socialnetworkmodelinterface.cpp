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

class SorterFunctor
{
public:
    explicit SorterFunctor(SorterInterface *sorter);
    bool operator()(ContentItemInterface *first, ContentItemInterface *second) const;
private:
    SorterInterface *m_sorter;
};

SorterFunctor::SorterFunctor(SorterInterface *sorter):
    m_sorter(sorter)
{
}

bool SorterFunctor::operator()(ContentItemInterface *first, ContentItemInterface *second) const
{
    return m_sorter->firstLessThanSecond(first->data(), second->data());
}


SocialNetworkModelInterfacePrivate::SocialNetworkModelInterfacePrivate(SocialNetworkModelInterface *q)
    : status(SocialNetworkInterface::Initializing)
    , error(SocialNetworkInterface::NoError)
    , socialNetwork(0), filter(0)
    , hasPrevious(false), hasNext(false)
    , resortUpdatePosted(false)
    , initialized(false)
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

void SocialNetworkModelInterfacePrivate::socialNetworkInitializedChangedHandler()
{
    Q_Q(SocialNetworkModelInterface);
    if (socialNetwork && socialNetwork->isInitialized()) {
        q->disconnect(socialNetwork, SIGNAL(initializedChanged()),
                      q, SLOT(socialNetworkInitializedChangedHandler()));

        if (initialized) {
            status = SocialNetworkInterface::Idle;
            emit q->statusChanged();
        }
    }
}

void SocialNetworkModelInterfacePrivate::socialNetworkDestroyedHandler()
{
    Q_Q(SocialNetworkModelInterface);
    q->setSocialNetwork(0);
}

void SocialNetworkModelInterfacePrivate::filterDestroyedHandler()
{
    Q_Q(SocialNetworkModelInterface);
    q->setError(SocialNetworkInterface::OtherError, "Filter is destroyed during request");
    q->setFilter(0);
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

bool SocialNetworkModelInterfacePrivate::load(FilterInterface::LoadType loadType)
{
    Q_Q(SocialNetworkModelInterface);
    if (status == SocialNetworkInterface::Initializing) {
        qWarning()<< Q_FUNC_INFO
                  << "Cannot load SocialNetworkModelInterface: did you set socialNetwork ?";
        return false;
    }

    if (status == SocialNetworkInterface::Busy
            || status == SocialNetworkInterface::Invalid) {
        qWarning() << Q_FUNC_INFO
                   << "Cannot load SocialNetworkModelInterface: status is Busy/Invalid";
        return false;
    }

    if (!filter) {
        qWarning() << Q_FUNC_INFO << "Cannot load SocialNetworkModelInterface: No filter set";
        return false;
    }


    if (!socialNetwork) {
        qWarning() << Q_FUNC_INFO << "Cannot load SocialNetworkModelInterface: No socialNetwork set";
        return false;
    }

    if (!filter->isAcceptable(q, socialNetwork)) {
        qWarning() << Q_FUNC_INFO << "Cannot load SocialNetworkModelInterface: invalid filter";
        return false;
    }


    if (!filter->performLoadRequest(q, socialNetwork, loadType)) {
        qWarning() << Q_FUNC_INFO << "Failed to perform load request";
        return false;
    }

    status = SocialNetworkInterface::Busy;
    emit q->statusChanged();
    return true;
}

///*! \internal */
//void SocialNetworkModelInterfacePrivate::filters_append(QDeclarativeListProperty<FilterInterface> *list,
//                                                        FilterInterface *filter)
//{
//    SocialNetworkModelInterface *model = qobject_cast<SocialNetworkModelInterface *>(list->object);
//    if (model && filter) {
//        model->d_func()->filters.append(filter);
//    }
//}

///*! \internal */
//FilterInterface *SocialNetworkModelInterfacePrivate::filters_at(QDeclarativeListProperty<FilterInterface> *list, int index)
//{
//    SocialNetworkModelInterface *model = qobject_cast<SocialNetworkModelInterface *>(list->object);
//    if (model && model->d_func()->filters.count() > index && index >= 0)
//        return model->d_func()->filters.at(index);
//    return 0;
//}

///*! \internal */
//void SocialNetworkModelInterfacePrivate::filters_clear(QDeclarativeListProperty<FilterInterface> *list)
//{
//    SocialNetworkModelInterface *model = qobject_cast<SocialNetworkModelInterface *>(list->object);
//    if (model) {
//        model->d_func()->filters.clear();
//    }
//}

///*! \internal */
//int SocialNetworkModelInterfacePrivate::filters_count(QDeclarativeListProperty<FilterInterface> *list)
//{
//    SocialNetworkModelInterface *model = qobject_cast<SocialNetworkModelInterface *>(list->object);
//    if (model)
//        return model->d_func()->filters.count();
//    return 0;
//}

///*! \internal */
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

///*! \internal */
SorterInterface *SocialNetworkModelInterfacePrivate::sorters_at(QDeclarativeListProperty<SorterInterface> *list, int index)
{
    SocialNetworkModelInterface *model = qobject_cast<SocialNetworkModelInterface *>(list->object);
    if (model && model->d_func()->sorters.count() > index && index >= 0)
        return model->d_func()->sorters.at(index);
    return 0;
}

///*! \internal */
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

///*! \internal */
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

void SocialNetworkModelInterfacePrivate::clear()
{
    Q_Q(SocialNetworkModelInterface);
    if (modelData.isEmpty()) {
        return;
    }

    q->beginRemoveRows(QModelIndex(), 0, modelData.count() - 1);
    modelData.clear();
    emit q->countChanged();
    q->endRemoveRows();

//    setHavePreviousAndNext(false, false);
}

//void SocialNetworkModelInterfacePrivate::setData(const CacheEntry::List &data)
//{
//    Q_Q(SocialNetworkModelInterface);
//    if (data.isEmpty()) {
//        clear();
//        return;
//    }

//    if (!modelData.isEmpty()) {
//        q->beginRemoveRows(QModelIndex(), 0, modelData.count() - 1);
//        modelData.clear();
//        q->endRemoveRows();
//    }

//    prependData(data);
//    resort();
//}

//void SocialNetworkModelInterfacePrivate::prependData(const CacheEntry::List &data)
//{
//    Q_Q(SocialNetworkModelInterface);
//    if (data.isEmpty()) {
//        return;
//    }

//    q->beginInsertRows(QModelIndex(), 0, data.count() - 1);
//    CacheEntry::List newData = data;
//    newData.append(modelData);
//    modelData = newData;
//    emit q->countChanged();
//    q->endInsertRows();
//    resort();
//}

//void SocialNetworkModelInterfacePrivate::appendData(const CacheEntry::List &data)
//{
//    Q_Q(SocialNetworkModelInterface);
//    if (data.isEmpty()) {
//        return;
//    }

//    q->beginInsertRows(QModelIndex(), modelData.count(), modelData.count() + data.count() - 1);
//    modelData.append(data);
//    emit q->countChanged();
//    q->endInsertRows();
//    resort();
//}

//void SocialNetworkModelInterfacePrivate::setStatus(SocialNetworkInterface::Status newStatus)
//{
//    Q_Q(SocialNetworkModelInterface);
//    if (status != newStatus) {
//        status = newStatus;
//        emit q->statusChanged();
//    }
//}

//void SocialNetworkModelInterfacePrivate::setError(SocialNetworkInterface::ErrorType newError,
//                                                  const QString &newErrorMessage)
//{
//    Q_Q(SocialNetworkModelInterface);
//    if (error != newError) {
//        error = newError;
//        emit q->errorChanged();
//    }
//    if (errorMessage != newErrorMessage) {
//        errorMessage = newErrorMessage;
//        emit q->errorMessageChanged();
//    }
//    setStatus(SocialNetworkInterface::Error);
//}

//void SocialNetworkModelInterfacePrivate::setHavePreviousAndNext(bool newHasPrevious, bool newHasNext)
//{
//    Q_Q(SocialNetworkModelInterface);
//    if (hasPrevious != newHasPrevious) {
//        hasPrevious = newHasPrevious;
//        emit q->hasPreviousChanged();
//    }

//    if (hasNext != newHasNext) {
//        hasNext = newHasNext;
//        emit q->hasNextChanged();
//    }
//}

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
    Q_D(SocialNetworkModelInterface);
    d->initialized = true;
    if (d->socialNetwork && d->socialNetwork->isInitialized()) {
        d->status = SocialNetworkInterface::Idle;
        emit statusChanged();
    }
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

//    CacheEntry::Ptr cacheEntry = d->modelData.at(index.row());

    switch (role) {
    // We break item creation flow here: items are automatically created
    // TODO: we need to get this from the cache
//        case ContentItemTypeRole: {
//            return QVariant::fromValue(cacheEntry->data().value(NEMOQMLPLUGINS_SOCIAL_CONTENTITEMTYPE).toInt());
//        }
//        case ContentItemDataRole: {
//            return QVariant::fromValue(cacheEntry->data());
//        }
//        case ContentItemIdentifierRole: {
//            return QVariant::fromValue(cacheEntry->data().value(NEMOQMLPLUGINS_SOCIAL_CONTENTITEMID).toString());
//        }
        case ContentItemRole: {

//            if (cacheEntry->item()) {
//                return QVariant::fromValue(cacheEntry->item());
//            }
            return QVariant::fromValue(d->modelData.at(index.row()));
//            return QVariant::fromValue(d->socialNetwork->d_func()->createItem(cacheEntry));
        }
//        case SectionRole: {
//            return d->socialNetwork->d_func()->dataSection(cacheEntry->data().value(NEMOQMLPLUGINS_SOCIAL_CONTENTITEMTYPE).toInt(),
//                                                           cacheEntry->data());
//        }
        default: {
            return QVariant();
        }
    }
    return QVariant();
}

SocialNetworkInterface * SocialNetworkModelInterface::socialNetwork() const
{
    Q_D(const SocialNetworkModelInterface);
    return d->socialNetwork;
}

void SocialNetworkModelInterface::setSocialNetwork(SocialNetworkInterface *socialNetwork)
{
    Q_D(SocialNetworkModelInterface);
    if (d->socialNetwork != socialNetwork) {
        if (d->socialNetwork)
            // Disconnect from old social network (if needed)
            d->socialNetwork->disconnect(this);

        if (socialNetwork && !socialNetwork->isInitialized()) {
            // Connects to new social network (if needed)
            connect(socialNetwork, SIGNAL(initializedChanged()),
                    this, SLOT(socialNetworkInitializedChangedHandler()));

            // Initializing
            if (d->status != SocialNetworkInterface::Initializing) {
                d->status = SocialNetworkInterface::Initializing;
                emit statusChanged();
            }

        } else if (d->initialized && socialNetwork && socialNetwork->isInitialized()) {
            // Initialized, so we set as idle
            if (d->status != SocialNetworkInterface::Idle) {
                d->status = SocialNetworkInterface::Idle;
                emit statusChanged();
            }
        }
        d->socialNetwork = socialNetwork;
        if (d->socialNetwork) {
            connect(d->socialNetwork, SIGNAL(destroyed()),
                    this, SLOT(socialNetworkDestroyedHandler()));
        }

        emit socialNetworkChanged();
    }
}

FilterInterface * SocialNetworkModelInterface::filter() const
{
    Q_D(const SocialNetworkModelInterface);
    return d->filter;
}

void SocialNetworkModelInterface::setFilter(FilterInterface *filter)
{
    Q_D(SocialNetworkModelInterface);
    if (d->filter != filter) {
        if (d->status == SocialNetworkInterface::Busy) {
            qWarning() << Q_FUNC_INFO << "Cannot set filter when item is in Busy state";
            return;
        }

        if (d->filter) {
            d->filter->disconnect(this);
        }

        d->filter = filter;

        if (d->filter) {
            connect(d->filter, SIGNAL(destroyed()), this, SLOT(filterDestroyedHandler()));
        }
        emit filterChanged();
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
    Q_D(const SocialNetworkModelInterface);
    if (index < 0 || index >= rowCount()) {
        return 0;
    }

    return d->modelData.value(index);
}

bool SocialNetworkModelInterface::load()
{
    Q_D(SocialNetworkModelInterface);
    return d->load(FilterInterface::Load);
}

bool SocialNetworkModelInterface::loadPrevious()
{
    Q_D(SocialNetworkModelInterface);
    if (d->hasPrevious) {
        return d->load(FilterInterface::LoadPrevious);
    } else {
        return false;
    }
}

bool SocialNetworkModelInterface::loadNext()
{
    Q_D(SocialNetworkModelInterface);
    if (d->hasNext) {
        return d->load(FilterInterface::LoadNext);
    } else {
        return false;
    }
}

//void SocialNetworkModelInterface::populate()
//{
//    Q_D(SocialNetworkModelInterface);
//    if (!d->socialNetwork) {
//        qWarning() << Q_FUNC_INFO << "Cannot call populate when not SocialNetwork is set";
//        return;
//    }

//    d->socialNetwork->d_func()->populate(this, d->nodeIdentifier, d->nodeType, d->filters);
//}

//void SocialNetworkModelInterface::repopulate()
//{
//    Q_D(SocialNetworkModelInterface);
//    if (!d->socialNetwork) {
//        qWarning() << Q_FUNC_INFO << "Cannot call repopulate when not SocialNetwork is set";
//        return;
//    }

//    d->socialNetwork->d_func()->populate(this, d->nodeIdentifier, d->nodeType, d->filters, true);
//}

//void SocialNetworkModelInterface::loadNext()
//{
//    Q_D(SocialNetworkModelInterface);
//    if (hasNext()) {
//        d->socialNetwork->d_func()->loadNext(this);
//    }
//}

//void SocialNetworkModelInterface::loadPrevious()
//{
//    Q_D(SocialNetworkModelInterface);
//    if (hasPrevious()) {
//        d->socialNetwork->d_func()->loadPrevious(this);
//    }
//}



void SocialNetworkModelInterface::setModelData(const QList<ContentItemInterface *> &data)
{
    Q_D(SocialNetworkModelInterface);

    if (rowCount() > 0) {
        beginRemoveRows(QModelIndex(), 0, rowCount() - 1);
        qDeleteAll(d->modelData);
        d->modelData.clear();
        endRemoveRows();
    }

    if (data.count() > 0) {
        beginInsertRows(QModelIndex(), 0, data.count() - 1);
        d->modelData = data;
        emit countChanged();
        endInsertRows();
    }

    d->resort();

    if (d->status == SocialNetworkInterface::Busy) {
        d->status = SocialNetworkInterface::Idle;
        emit statusChanged();
    }
}

void SocialNetworkModelInterface::prependModelData(const QList<ContentItemInterface *> &data)
{
    Q_D(SocialNetworkModelInterface);
    if (data.count() > 0) {
        beginInsertRows(QModelIndex(), 0, data.count() - 1);
        QList<ContentItemInterface *> oldData = d->modelData;
        d->modelData = data;
        d->modelData.append(oldData);
        emit countChanged();
        endInsertRows();
    }

    d->resort();

    if (d->status == SocialNetworkInterface::Busy) {
        d->status = SocialNetworkInterface::Idle;
        emit statusChanged();
    }
}

void SocialNetworkModelInterface::appendModelData(const QList<ContentItemInterface *> &data)
{
    Q_D(SocialNetworkModelInterface);
    if (data.count() > 0) {
        beginInsertRows(QModelIndex(), rowCount(), rowCount() + data.count() - 1);
        d->modelData.append(data);
        emit countChanged();
        endInsertRows();
    }

    d->resort();

    if (d->status == SocialNetworkInterface::Busy) {
        d->status = SocialNetworkInterface::Idle;
        emit statusChanged();
    }
}

QVariantMap SocialNetworkModelInterface::extraData() const
{
    Q_D(const SocialNetworkModelInterface);
    return d->extraData;
}

void SocialNetworkModelInterface::setExtraData(const QVariantMap &extraData)
{
    Q_D(SocialNetworkModelInterface);
    d->extraData = extraData;
}

void SocialNetworkModelInterface::setPagination(bool hasPrevious, bool hasNext)
{
    Q_D(SocialNetworkModelInterface);
    if (d->hasPrevious != hasPrevious) {
        d->hasPrevious = hasPrevious;
        emit hasPreviousChanged();
    }

    if (d->hasNext != hasNext) {
        d->hasNext = hasNext;
        emit hasNextChanged();
    }
}

void SocialNetworkModelInterface::setError(SocialNetworkInterface::ErrorType error,
                                           const QString &errorMessage)
{
    Q_D(SocialNetworkModelInterface);

    if (d->errorMessage != errorMessage) {
        d->errorMessage = errorMessage;
        emit errorMessageChanged();
    }

    if (d->error != error) {
        d->error = error;
        emit errorChanged();
    }

    if (d->status != SocialNetworkInterface::Error) {
        d->status = SocialNetworkInterface::Error;
        emit statusChanged();
    }
}

void SocialNetworkModelInterface::clear()
{
    Q_D(SocialNetworkModelInterface);
    d->clear();
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
