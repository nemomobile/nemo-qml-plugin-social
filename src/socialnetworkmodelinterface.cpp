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

/*!
    \qmltype SocialNetworkModel
    \instantiates SocialNetworkModelInterface
    \inqmlmodule org.nemomobile.social 1
    \brief Present information provided by a SocialNetwork

    A SocialNetwork provides access to data from a particular social network.
    This data usually consists of an entity, like an user, or a photo, and a
    set of related data attached to that entity, like a list of likes or
    comments, or the friends associated to an user.

    SocialNetworkModel is built around this data. It is a QML model, that derives
    from QAbstractListModel. It provides a central \l node attribute, that
    is used to represent the entity retrieved from the social network as a
    ContentItem. The data populated in the model are used to represent the
    related data.

    The model roles are as follows:
    \list
    \li contentItem - the instantiated ContentItem that represents the related item.
    \li contentItemType - the type of the ContentItem that represents the related item.
    \li contentItemData - the underlying QVariantMap data of the ContentItem that represents the related item.
    \li contentItemIdentifier - the identifier of the ContentItem that represents the related item.
    or an empty string
    \endlist

    If you want to access to these related data via JavaScript, the best way is to
    use the \l count property combined with \l relatedItem(), that allows direct
    access to the data stored into the model.

    The SocialNetworkModel also controls data retrieving. While it does not retrieve
    data itself, it asks a SocialNetwork do perform these operations. To attach
    a SocialNetwork to a SocialNetworkModel, the property \l socialNetwork should be
    used.

    In order to perform a request, the client should invoke populate() after setting
    the following properties:
    \list
    \li \l nodeIdentifier
    \li \l nodeType (optional)
    \li \l filters (optional)
    \endlist

    \l nodeIdentifier contains the identifier for the entity in the social network that
    should be retrieved and loaded into the \l node property. For some social networks,
    you might need to provide a \l nodeType as well, to provide the type of entity that
    should be retrieved. The \l filters property is a list of Filter, that are used to
    tune the request, like providing the type of related data that should be requested.
    See Facebook and Twitter for the list of supported filters for specific SocialNetwork
    implementations.

    A request is performed via populate() or repopulate(). populate() will retrieve data
    from the social network and cache it, while repopulate() will always retrieve data from
    the social network, and override the cache. The first method is useful to retrieve data
    about (for example) an user, because informations about an user do not change often. The
    second method is more suited for refreshing data that changes often, like likes, retweets
    or comments.

    SocialNetworkModel also support loading new entries for the related data. Often the
    related data are a very long list (a feed full of posts, or tweets), and might be
    updated with new content. Most social networks displays these related data in multiple
    pages. SocialNetworkModel provides \l hasNext, \l hasPrevious and \l loadNext() and
    \l loadPrevious() to manage these pages. The two properties are here to informed if there
    are more pages available at the end or the beginning, while the methods are used to perform
    the requests.

    SocialNetworkModel is also responsible of sorting data. You can use the \l sorters property
    and pass a list of Sorter to perform sorting operations in loaded related data.

    \note \c next loads the next page of related data appends it to the model, while \c previous
    loads the previous page of related data and prepends it to the model. Generally, timelines of
    data like posts or tweets are displayed with more recent data on top, and therefore \c next
    refers to older data and \c previous to newer data.

    \note it is safe to change the \l nodeIdentifier, \l nodeType or \l filters before calling
    \l populate() or \l repopulate(). These properties have no influence, and are only used when
    \l populate() or \l repopulate() are called in order to perform the request. Be careful while
    changing them when a request is running because they might not be updated with the correct
    data when the data is loaded, because SocialNetwork uses the \l nodeIdentifier, \l nodeType and
    \l filters to identify which models to update \b{when the data is loaded}

    Example:

    \qml
    import QtQuick 1.1
    import org.nemomobile.social 1.0

    Item {
        MySocialNetwork {
            id: mySocialNetwork
        }

        SocialNetworkModel {
            id: socialNetworkModel
            socialNetwork: mySocialNetwork
            nodeIdentifier: "1234" // Some identifier
            filters: [
                ContentItemTypeFilter {
                    // Retrieve the likes for "my-social-network"
                    type: MySocialNetwork.Likes
                }
            ]
            onStatusChanged: {
                if (status == SocialNetwork.Idle) {
                    // When the node is loaded, we output the name of the node
                    // (assuming it is about retrieving an user)
                    console.debug(socialNetworkModel.node.name)
                }
            }
        }

        ListView {
            anchors.fill: parent
            model: socialNetworkModel
            delegate: Text { text: model.contentItem.name }
        }

        // We populate the model when at the beginning
        Component.onCompleted: socialNetworkModel.populate()
    }
    \endqml

    \sa{The caching system}
*/




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

/*!
    \qmlproperty enumeration SocialNetworkModel::status

    Hold the status of the SocialNetworkModel.

    The status can be one of:
    \list
    \li SocialNetwork.Initializing
    \li SocialNetwork.Idle - the default
    \li SocialNetwork.Busy
    \li SocialNetwork.Error
    \li SocialNetwork.Invalid
    \endlist

    When the status is \c Initializing, the component is initializing and nothing should be
    performed before it is initialized.

    When in \c Idle, the SocialNetworkModel loaded information from the SocialNetwork, or is
    waiting to perform a loading operation.

    When in \c Busy, the SocialNetworkModel is waiting for information from the social
    network.

    When in \c Error, the information that has been loaded is wrong, often because the
    loading process failed. Reloading information with repopulate() might fix the loaded
    informations. \l error and \l errorMessage might also carry information about
    the error.

    When in \c Invalid, the SocialNetworkModel is in an invalid state, and should not
    be used anymore.
*/

SocialNetworkInterface::Status SocialNetworkModelInterface::status() const
{
    Q_D(const SocialNetworkModelInterface);
    return d->status;
}

/*!
    \qmlproperty enumeration SocialNetworkModel::error

    Hold the type of the most recent error in SocialNetworkModel.
    Note that the \c error will not be reset if subsequent operations
    succeed.

    The error can be one of:
    \list
    \li SocialNetwork.NoError - the default
    \li SocialNetwork.AccountError - \b deprecated, error related to the account.
    \li SocialNetwork.SignOnError - \b deprecated, error related to the signon process.
    \li SocialNetwork.BusyError  - \b deprecated, error because the SocialNetworkModel interface
    is already busy.
    \li SocialNetwork.RequestError - error related to a failure during a request to the social API.
    \li SocialNetwork.DataUpdateError - error related during data update.
    \li SocialNetwork.OtherError - other error (not used).
    \li SocialNetwork.LastError - not used.
    \endlist
*/

SocialNetworkInterface::ErrorType SocialNetworkModelInterface::error() const
{
    Q_D(const SocialNetworkModelInterface);
    return d->error;
}

/*!
    \qmlproperty enumeration SocialNetworkModel::errorMessage

    Holds the message associated with the most recent error in
    SocialNetworkModel. Note that the \c errorMessage will
    not be reset if subsequent operations succeed.
*/

QString SocialNetworkModelInterface::errorMessage() const
{
    Q_D(const SocialNetworkModelInterface);
    return d->errorMessage;
}

/*!
    \qmlproperty SocialNetwork SocialNetworkModel::socialNetwork

    Hold the SocialNetwork object that is used by this SocialNetworkModel.
*/

SocialNetworkInterface * SocialNetworkModelInterface::socialNetwork() const
{
    Q_D(const SocialNetworkModelInterface);
    return d->socialNetwork;
}

/*!
    \qmlproperty QString SocialNetworkModel::nodeIdentifier

    Hold the identifier to the node that this SocialNetworkModel represents.
*/

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

/*!
    \qmlproperty IdentifiableContentItem SocialNetworkModel::node

    Hold a ContentItem representing the node in the social network.
    When loading, this property will be a null ContentItem, and when
    loaded, it will represent the node of a social network.
*/
IdentifiableContentItemInterface * SocialNetworkModelInterface::node() const
{
    Q_D(const SocialNetworkModelInterface);
    return d->node;
}

/*!
    \qmlproperty bool SocialNetworkModel::hasPrevious

    Holds if the model have previous data to be loaded.
*/
bool SocialNetworkModelInterface::hasPrevious() const
{
    Q_D(const SocialNetworkModelInterface);
    return d->hasPrevious;
}

/*!
    \qmlproperty bool SocialNetworkModel::hasNext

    Holds if the model have next data to be loaded.
*/
bool SocialNetworkModelInterface::hasNext() const
{
    Q_D(const SocialNetworkModelInterface);
    return d->hasNext;
}

/*!
    \qmlproperty list<Filter> SocialNetworkModel::filters

    Hold a list of Filter that is used to perform the request to a social network.
    This list often describes the type of related data to be retrieved from the
    social network, but can also be filters that are used to get specific data for
    the \l node.

    Specific implementations of the SocialNetwork interface may not support
    certain standard filters types, or they may not support filtering at all.
*/
QDeclarativeListProperty<FilterInterface> SocialNetworkModelInterface::filters()
{
    return QDeclarativeListProperty<FilterInterface>(this, 0,
            &SocialNetworkModelInterfacePrivate::filters_append,
            &SocialNetworkModelInterfacePrivate::filters_count,
            &SocialNetworkModelInterfacePrivate::filters_at,
            &SocialNetworkModelInterfacePrivate::filters_clear);
}

/*!
    \qmlproperty list<Sorter> SocialNetworkModel::sorters

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

/*!
    \qmlproperty int SocialNetworkModel::count

    Holds the number of items in this model.
*/
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
    \qmlmethod QtObject SocialNetworkModel::relatedItem(int index)

    Returns the ContentItem which is related to the node from the given
    \a index of the model data.  This is identical to calling
    \c data() for the given model index and specifying the \c contentItem
    role.

    \note Although this function will always return a pointer to a
    ContentItem, the return type of the function is QtObject (or a QObject *
    in C++), so that this function can be used via QMetaObject::invokeMethod().
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

/*!
    \qmlmethod SocialNetworkModel::populate()

    Perform a request based on the set \l socialNetwork, \l nodeIdentifier,
    \l nodeType and \l filters.

    If data is available in cache, it will be used to populate that model.
*/

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

/*!
    \qmlmethod SocialNetworkModel::repopulate()

    Perform a request based on the set \l socialNetwork, \l nodeIdentifier,
    \l nodeType and \l filters.

    If data is available in cache, it will overriden by new data retrieved from
    the social network.
*/

void SocialNetworkModelInterface::repopulate()
{
    Q_D(SocialNetworkModelInterface);
    if (!d->socialNetwork) {
        qWarning() << Q_FUNC_INFO << "Cannot call repopulate when not SocialNetwork is set";
        return;
    }

    d->socialNetwork->d_func()->populate(this, d->nodeIdentifier, d->nodeType, d->filters, true);
}

/*!
    \qmlmethod SocialNetworkModel::loadNext()

    Perform the request used to load the next page of related data, based on the
    set \l socialNetwork, \l nodeIdentifier, \l nodeType and \l filters.
*/

void SocialNetworkModelInterface::loadNext()
{
    Q_D(SocialNetworkModelInterface);
    if (hasNext()) {
        d->socialNetwork->d_func()->loadNext(this);
    }
}

/*!
    \qmlmethod SocialNetworkModel::loadPrevious()

    Perform the request used to load the previous page of related data, based on the
    set \l socialNetwork, \l nodeIdentifier, \l nodeType and \l filters.
*/

void SocialNetworkModelInterface::loadPrevious()
{
    Q_D(SocialNetworkModelInterface);
    if (hasPrevious()) {
        d->socialNetwork->d_func()->loadPrevious(this);
    }
}

void SocialNetworkModelInterface::clean()
{
    // TODO: check if this should be public API
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
