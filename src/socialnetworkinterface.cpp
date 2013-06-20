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

#include "socialnetworkinterface.h"
#include "socialnetworkinterface_p.h"

#include "contentiteminterface.h"
#include "contentiteminterface_p.h"
#include "identifiablecontentiteminterface.h"

#include "util_p.h"

#include <QtCore/QByteArray>
#include <QtCore/QCoreApplication>
#include <QtCore/QEvent>
#include <QtCore/QUrl>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkRequest>

#include <QtDebug>

#include <algorithm>

/*
    SocialNetworkInterface

    The SocialNetworkInterface is a conveinent way to represent data
    from a social network and query that social network for data.

    SocialNetworkInterface, often called SNI, is a QML mode (inheriting
    from QAbstractListModel), that can be used to display data in
    ListView or GridView. It is built to mirror most mobile applications
    behaviour.

    The SNI have a list of nodes, each node representing a page, and also
    something to display. A node contains the identifier of an entity, as
    well as some filters that indicates what are the data related to that
    entity to query. It then provides access to that node ine QML context,
    via SocialNetworkInterface::node(), and access to the related content
    via the data set in the model.

    When setting the identifier and filters to use for creating a new node,
    the user have to call SocialNetworkInterface::populate() to create a new
    node in the SNI. That node will be append to the other nodes, and SNI will
    load the data of that node in background. It will inform about the state of
    the loading using SocialNetworkInterface::status().

    In order to navigate between nodes, SocialNetworkInterface::previousNode()
    and SocialNetworkInterface::nextNode() should be used.
*/

/*
    CacheEntry

    A CacheEntry represents an entry in the cache. These entries
    represents entities that have an identifier, and that inherits
    from IdentifiableContentItemInterface.

    Each cache entry contains the identifier that is associated to
    the entity, the data, provided as QVariantMap, that is downloaded
    from the social network, the ContentItem pointer that is associated
    to the data, and a refCount, that is used to count the number of use
    of this cache entry.

    The ContentItem pointer is lazily constructed, and will be
    constructed only when it is needed. The refCount is used to know
    when a CacheEntry should be removed from the cache. It is increased
    when a node uses the entity, and decreased when a node is destroyed.
    If the refCount is 0, then it should be removed.

    CacheEntries are explicitely shared, so a modification to a CacheEntry
    will modify all the copies. Basically, it behaves like a pointer.
    CacheEntries can also be compared, and are considered equal if they
    share the same identifier, data and item.
*/

/*
    Node

    A Node represents one "page". A node is defined by an identifier, and
    a list of filters. It often defines a page in the application display.

    A Node consists of an identifier, and a set of filters that defines the
    node. It also have a CacheEntry, that contains the cached version of the
    node if it exists, or nothing if it is yet to be retrieved. It also
    contains a list of CacheEntry, that is the data in the model that is
    contained in a defined Node (page). This data is often called
    "related data"

    Remark: the identifier for a Node and for a CacheEntry might be different:
    in Facebook, you can use "me" to replace the identifier of the current user,
    so the initial Node might have "me" as identifier. However, the retrieved
    data will contain the identifier of current user, so the identifier of the
    CacheEntry associated to the Node will not be "me".

    Node are also explicitely shared. They can also be compared, and are
    considered equal if they have the same identifier and filters.

    When a CacheEntry is set as the data of a Node, or as related data, it's
    refcout is automatically increased, and decreased if is replaced.
 */

/*
    SorterFunctor

    The sorter functor is used to adapt a SorterInterface to a function, that
    can be used by std::sort or std::stable_sort. It simply takes a pointer to a
    SorterInterface as constructor, and provides an operator() method that is
    suited for sorting.
*/

/*
    Caching system

    The caching system is rather simple. It is a QHash that associates an
    identifier, that is represented by a QString, to a CacheEntry.

    Each "page" in an application is represented by a Node, and a QStack of Node
    represents the whole set of pages. As said in the documentation describing
    Node, each Node also contains a list of CacheEntry, either to represent the
    node, or to represent related data.

    While a CacheEntry is used by a Node, it's refcount is increased. If it is no longer
    used, often when a node is removed from the node stack, it's refcount is decreased.
    If a refcount for a CacheEntry drops to zero, it means that it is no longer used,
    and is removed from the cache.

    When the user makes a request for a Node (by setting an identifier and a set of
    filters), the Node stack is searched. If a Node already exists in the stack, then it
    is simply fetched and appended to the stack, without loading anything from the social
    network. If only the node data exists for a cached Node, then the other part (related
    data) is fetched. If nothing was found in the node stack, or if a Node that contains
    neither a valid CacheEntry as node nor related data is found, the the social network
    is queried for those two missing informations.
*/

/*
    Using the caching system and implementing SocialNetworkInterface

    The cache is rather fragile. If someone mess with the refcounting or with the
    insertion and removal, it might creates segmentation faults, or memory leaks.
    The best way is to never use methods and attributes that are private inside of
    SocialNetworkInterfacePrivate, and to never use CacheEntry::ref() and
    CacheEntry::deref().

    In order to implement a SocialNetworkInterface, the following methods should be implemented:
    - populateDataForLastNode()
    - populateRelatedDataforLastNode()
    - validateCacheEntryForLastNode()

    The first method is used to initiate a request to the data of the last node. That
    node is often freshly pushed to the stack using SocialNetworkInterface::populate().

    The second method is used to initiate a request to the related data of the last node.

    Finally, the last method is used when a query is in progress. Sometimes, the cached
    values are only partial, and this method should be used to validate if the data
    that is cached, and that is going to be used for the last node contains enough
    information. If not, SNI will reload the node in order to get more data.

    A set of private methods, available through the D-pointer, can be used:

    - setStatus() and setError() can be used to make status and error management easier.
      As usual setters, they set the data, and emit the corresponding signals.

    - lastNode() is very useful in order to get the last node, that is the node
      that is being populated.

    - createCacheEntry() is used to create a CacheEntry, if the entry do not exist
      in the cache. If it already exists, the entry from the cache is retrived. It
      is useful for populating related data and node.

    - setLastNodeCacheEntry() and setLastNodeData() are used to set the data of the
      last node.

    - atLastNode() is used to inform if the SocialNetworkInterface is currently
      displaying the last node. And if it is the case, updateNodePositionStatus() or
      updateRelatedData() can be used to automatically update all the attributes
      that influences the display.

    - insertContent() is used to perform population for related content. It can be used
      in append mode (default), or prepend mode, and also sets the paging properties
      (if there is a next or previous page).
*/

// TODO XXX: CacheEntry::ref and deref should be made private and friend with some classes
// to prevent misuse.

CacheEntryPrivate::CacheEntryPrivate()
    : QSharedData()
{
    item = 0;
    refCount = 0;
}

CacheEntryPrivate::~CacheEntryPrivate()
{
    if (item) {
        item->deleteLater();
        item = 0;
    }
}

CacheEntry::CacheEntry()
    : d_ptr(new CacheEntryPrivate())
{
}

CacheEntry::CacheEntry(const QVariantMap &data, ContentItemInterface *item)
    : d_ptr(new CacheEntryPrivate())
{
    Q_D(CacheEntry);
    d->data = data;
    d->item = item;
}

CacheEntry::CacheEntry(const QVariantMap &data, const QString &identifier,
                       ContentItemInterface *item)
    : d_ptr(new CacheEntryPrivate())
{
    Q_D(CacheEntry);
    d->data = data;
    d->identifier = identifier;
    d->item = item;
}

CacheEntry::CacheEntry(const CacheEntry &other)
    : d_ptr(other.d_ptr)
{
}

CacheEntry::~CacheEntry()
{
}

bool CacheEntry::operator==(const CacheEntry &other) const
{
    Q_D(const CacheEntry);
    return (d->item == other.item() && d->identifier == other.identifier()
            && d->data == other.data());
}

bool CacheEntry::operator!=(const CacheEntry &other) const
{
    return !(*this == other);
}

bool CacheEntry::isNull() const
{
    Q_D(const CacheEntry);
    return d->data.isEmpty() && (d->item == 0);
}

int CacheEntry::refcount() const
{
    Q_D(const CacheEntry);
    return d->refCount;
}

void CacheEntry::ref()
{
    Q_D(CacheEntry);
    d->refCount ++;
}

void CacheEntry::deref()
{
    Q_D(CacheEntry);
    d->refCount --;
}

QString CacheEntry::identifier() const
{
    Q_D(const CacheEntry);
    return d->identifier;
}

QVariantMap CacheEntry::data() const
{
    Q_D(const CacheEntry);
    return d->data;
}

void CacheEntry::setData(const QVariantMap &data)
{
    Q_D(CacheEntry);
    d->data = data;
}

const ContentItemInterface * CacheEntry::item() const
{
    Q_D(const CacheEntry);
    return d->item;
}

ContentItemInterface * CacheEntry::item()
{
    Q_D(CacheEntry);
    return d->item;
}

void CacheEntry::setItem(ContentItemInterface *item)
{
    Q_D(CacheEntry);
    d->item = item;
}

const IdentifiableContentItemInterface * CacheEntry::identifiableItem() const
{
    Q_D(const CacheEntry);
    return d->item->asIdentifiable();
}

IdentifiableContentItemInterface * CacheEntry::identifiableItem()
{
    Q_D(CacheEntry);
    return d->item->asIdentifiable();
}

void CacheEntry::deleteItem()
{
    Q_D(CacheEntry);
    d->item->deleteLater();
    d->item = 0;
}

NodePrivate::NodePrivate()
    : hasPrevious(false)
    , hasNext(false)
{
}

Node::Node()
    : d_ptr(new NodePrivate())
{
}

Node::Node(const QString &identifier, const QSet<FilterInterface *> &filters)
    : d_ptr(new NodePrivate())
{
    Q_D(Node);
    d->identifier = identifier;
    d->filters = filters;
    d->cacheEntry = CacheEntry();
}

Node::Node(const Node &other):
    d_ptr(other.d_ptr)
{
}

Node::~Node()
{
}

bool Node::operator==(const Node &other) const
{
    Q_D(const Node);
    return (d->identifier == other.identifier()) && (d->filters == other.filters());
}

bool Node::operator!=(const Node &other) const
{
    return !(*this == other);
}

bool Node::isNull() const
{
    Q_D(const Node);
    return d->identifier.isEmpty();
}

QString Node::identifier() const
{
    Q_D(const Node);
    return d->identifier;
}

QSet<FilterInterface *> Node::filters() const
{
    Q_D(const Node);
    return d->filters;
}

CacheEntry Node::cacheEntry() const
{
    Q_D(const Node);
    return d->cacheEntry;
}

void Node::setCacheEntry(const CacheEntry &cacheEntry)
{
    Q_D(Node);
    if (!d->cacheEntry.isNull()) {
        d->cacheEntry.deref();
    }
    d->cacheEntry = cacheEntry;
    d->cacheEntry.ref();
}

QList<CacheEntry> Node::data() const
{
    Q_D(const Node);
    return d->data;
}

void Node::setData(const QList<CacheEntry> &data)
{
    Q_D(Node);
    foreach (CacheEntry cacheEntry, d->data) {
        cacheEntry.deref();
    }

    d->data = data;

    foreach (CacheEntry cacheEntry, d->data) {
        cacheEntry.ref();
    }
}

bool Node::hasPrevious() const
{
    Q_D(const Node);
    return d->hasPrevious;
}

bool Node::hasNext() const
{
    Q_D(const Node);
    return d->hasNext;
}

void Node::setPreviousAndNext(bool hasPrevious, bool hasNext)
{
    Q_D(Node);
    d->hasPrevious = hasPrevious;
    d->hasNext = hasNext;
}

QVariantMap Node::extraInfo() const
{
    Q_D(const Node);
    return d->extraInfo;
}

void Node::setExtraInfo(const QVariantMap &extraInfo)
{
    Q_D(Node);
    d->extraInfo = extraInfo;
}

ArbitraryRequestHandler::ArbitraryRequestHandler(QNetworkAccessManager *networkAccessManager,
                                                 SocialNetworkInterface *parent)
    : QObject(parent), networkAccessManager(networkAccessManager), reply(0), isError(false)
{
}

ArbitraryRequestHandler::~ArbitraryRequestHandler()
{
    if (reply) {
        reply->deleteLater();
    }
}

bool ArbitraryRequestHandler::request(SocialNetworkInterface::RequestType requestType,
                                      const QString &requestUri, const QVariantMap &queryItems,
                                      const QString &postData)
{
    if (reply) {
        qWarning() << Q_FUNC_INFO
                   << "Warning: cannot start arbitrary request: another arbitrary request is in progress";
        return false;
    }

    QList<QPair<QString, QString> > formattedQueryItems;
    QStringList queryItemKeys = queryItems.keys();
    foreach (const QString &key, queryItemKeys) {
        formattedQueryItems.append(qMakePair<QString, QString>(key,
                                                               queryItems.value(key).toString()));
    }

    QUrl url(requestUri);
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    QUrlQuery query;
    query.setQueryItems(formattedQueryItems);
    url.setQuery(query);
#else
    url.setQueryItems(formattedQueryItems);
#endif
    QNetworkReply *arbitraryRequestReply = 0;
    switch (requestType) {
    case SocialNetworkInterface::PostRequest:
        arbitraryRequestReply = networkAccessManager->post(QNetworkRequest(url),
                                           QByteArray::fromBase64(postData.toLatin1()));
        break;
    case SocialNetworkInterface::DeleteRequest:
        arbitraryRequestReply = networkAccessManager->deleteResource(QNetworkRequest(url));
        break;
    default:
        arbitraryRequestReply = networkAccessManager->get(QNetworkRequest(url));
        break;
    }

    if (arbitraryRequestReply) {
        reply = arbitraryRequestReply;
        connect(reply, SIGNAL(error(QNetworkReply::NetworkError)),
                this, SLOT(errorHandler(QNetworkReply::NetworkError)));
        connect(reply, SIGNAL(sslErrors(QList<QSslError>)),
                this, SLOT(sslErrorsHandler(QList<QSslError>)));
        connect(reply, SIGNAL(finished()), this, SLOT(finishedHandler()));
        return true;
    }

    qWarning() << Q_FUNC_INFO << "Warning: cannot start arbitrary request: null reply";
    return false;
}

void ArbitraryRequestHandler::finishedHandler()
{
    QByteArray replyData;
    if (reply) {
        replyData = reply->readAll();
        reply->deleteLater();
        reply = 0;
    }

    QVariantMap responseData;
    bool errorOccurred = isError;
    if (isError) {
        // note that errors to arbitrary requests don't cause the SocialNetwork
        // to transition to the Error state.  They are unrelated to the model.
        responseData.insert(QLatin1String("error"), errorMessage);
        errorMessage = QString();
        isError = false;
    } else {
        bool ok = false;
        QVariantMap parsed = ContentItemInterfacePrivate::parseReplyData(replyData, &ok);
        if (!ok) {
            responseData.insert(QLatin1String("response"), replyData);
        } else {
            responseData = parsed;
        }
    }

    emit arbitraryRequestResponseReceived(errorOccurred, responseData);
}

void ArbitraryRequestHandler::errorHandler(QNetworkReply::NetworkError networkError)
{
    errorMessage = networkErrorString(networkError);
    isError = true;
}

void ArbitraryRequestHandler::sslErrorsHandler(const QList<QSslError> &sslErrors)
{
    errorMessage = QLatin1String("SSL error: ");
    if (sslErrors.isEmpty()) {
        errorMessage += QLatin1String("unknown SSL error");
    } else {
        foreach (const QSslError &sslError, sslErrors)
            errorMessage += sslError.errorString() + QLatin1String("; ");
        errorMessage.chop(2);
    }

    isError = true;
}


SorterFunctor::SorterFunctor(SorterInterface *sorter):
    m_sorter(sorter)
{
}

bool SorterFunctor::operator()(const CacheEntry &first, const CacheEntry &second) const
{
    return m_sorter->firstLessThanSecond(first.data(), second.data());
}

SocialNetworkInterfacePrivate::SocialNetworkInterfacePrivate(SocialNetworkInterface *q)
    : networkAccessManager(0)
    , q_ptr(q)
    , resortUpdatePosted(false)
    , initialized(false)
    , populatePending(false)
    , status(SocialNetworkInterface::Initializing)
    , error(SocialNetworkInterface::NoError)
    , node(0)
    , hasPreviousNode(false)
    , hasNextNode(false)
    , hasPrevious(false)
    , hasNext(false)
    , nodeStackIndex(-1)
    , arbitraryRequestHandler(0)
{
}

SocialNetworkInterfacePrivate::~SocialNetworkInterfacePrivate()
{
    Q_Q(SocialNetworkInterface);
    // remove all cache entries.
    nodeStack.clear();
    if (!internalData.isEmpty()) {
        q->beginRemoveRows(QModelIndex(), 0, internalData.count() - 1);
        internalData.clear();
        q->endRemoveRows();
    }
}

QHash<int, QByteArray> SocialNetworkInterfacePrivate::roleNames()
{
    QHash<int, QByteArray> roles;
    roles.insert(SocialNetworkInterface::ContentItemRole, "contentItem");
    roles.insert(SocialNetworkInterface::ContentItemTypeRole, "contentItemType");
    roles.insert(SocialNetworkInterface::ContentItemDataRole, "contentItemData" );
    roles.insert(SocialNetworkInterface::ContentItemIdentifierRole, "contentItemIdentifier");
    roles.insert(SocialNetworkInterface::SectionRole, "section");
    return roles;
}

void SocialNetworkInterfacePrivate::setStatus(SocialNetworkInterface::Status newStatus)
{
    Q_Q(SocialNetworkInterface);
    if (status != newStatus) {
        status = newStatus;
        emit q->statusChanged();
    }
}

void SocialNetworkInterfacePrivate::setError(SocialNetworkInterface::ErrorType newError,
                                             const QString &newErrorMessage)
{
    Q_Q(SocialNetworkInterface);
    if (error != newError) {
        error = newError;
        emit q->errorChanged();
    }
    if (errorMessage != newErrorMessage) {
        errorMessage = newErrorMessage;
        emit q->errorMessageChanged();
    }
}

Node SocialNetworkInterfacePrivate::lastNode() const
{
    if (nodeStack.isEmpty()) {
        return Node();
    }
    return nodeStack.top();
}

CacheEntry SocialNetworkInterfacePrivate::createCacheEntry(const QVariantMap &data,
                                                           const QString &identifier)
{
    Q_Q(SocialNetworkInterface);
    // Check if there is already a cached entry that corresponds
    if (!identifier.isEmpty()) {
        if (cache.contains(identifier)) {
            CacheEntry entry = cache.value(identifier);
            entry.setData(data);
            if (entry.item()) {
                entry.deleteItem();
                entry.setItem(q->contentItemFromData(q, data));
            }
            return cache.value(identifier);
        }
    }

    if (!identifier.isEmpty()) {
        CacheEntry cacheEntry (data, identifier);
        cache.insert(identifier, cacheEntry);
        return cacheEntry;
    } else {
        return CacheEntry(data);
    }
}

void SocialNetworkInterfacePrivate::setLastNodeCacheEntry(const CacheEntry &cacheEntry)
{
    if (!nodeStack.top().cacheEntry().isNull()) {
        qWarning() << Q_FUNC_INFO << "Data for the last node is already set";
        return;
    }

    nodeStack.top().setCacheEntry(cacheEntry);
}

void SocialNetworkInterfacePrivate::setLastNodeData(const QList<CacheEntry> &data)
{
    nodeStack.top().setData(data);
}

bool SocialNetworkInterfacePrivate::atLastNode() const
{
    return (nodeStackIndex == nodeStack.count() - 1);
}

void SocialNetworkInterfacePrivate::insertContent(const QList<CacheEntry> &newData,
                                                  PagingInfos pagingInfos,
                                                  UpdateMode updateMode)
{
    Q_Q(SocialNetworkInterface);
    if (nodeStack.isEmpty()) {
        return;
    }

    bool havePrevious = pagingInfos.testFlag(HavePrevious);
    bool haveNext = pagingInfos.testFlag(HaveNext);
    nodeStack.top().setPreviousAndNext(havePrevious, haveNext);
    updateNextAndPrevious();

    if (newData.isEmpty()) {
        return;
    }

    QList<CacheEntry> data;
    switch (updateMode) {
    case Prepend:
        data = newData;
        data.append(nodeStack.top().data());
        break;
    case Replace:
        data = newData;
        break;
    default:
        data = nodeStack.top().data();
        data.append(newData);
        break;
    }

    lastNode().setData(data);

    if (atLastNode()) {
        // Update model
        int initialIndex = 0;
        if (updateMode == Append) {
            initialIndex = internalData.count();
        }

        if (updateMode == Replace) {
            q->beginRemoveRows(QModelIndex(), 0, internalData.count() - 1);
            internalData.clear();
            q->endRemoveRows();
        }

        q->beginInsertRows(QModelIndex(), initialIndex, initialIndex + newData.count() - 1);
        internalData = q->sortedData(nodeStack.top().data());
        emit q->countChanged();
        q->endInsertRows();
        emit q->dataChanged(q->index(0), q->index(internalData.count() - 1));
    }
}

void SocialNetworkInterfacePrivate::updateNodePositionStatus()
{
    Q_Q(SocialNetworkInterface);
    // Update next and previous
    bool newHasPreviousNode = true;
    bool newHasNextNode = true;
    if (nodeStackIndex == -1) {
        newHasPreviousNode = false;
    }

    if (nodeStackIndex == nodeStack.count() - 1) {
        newHasNextNode = false;
    }

    if (hasPreviousNode != newHasPreviousNode) {
        hasPreviousNode = newHasPreviousNode;
        emit q->hasPreviousNodeChanged();
    }

    if (hasNextNode != newHasNextNode) {
        hasNextNode = newHasNextNode;
        emit q->hasNextNodeChanged();
    }

    // If we are at the first place holder position
    if (nodeStackIndex == -1) {

        // We should set the node to be empty
        if (node) {
            node = 0;
            emit q->nodeChanged();
        }

        // The model should be empty as well
        if (!internalData.isEmpty()) {
            q->beginRemoveRows(QModelIndex(), 0, internalData.count() - 1);
            internalData.clear();
            emit q->countChanged();
            q->endRemoveRows();
        }
        return;
    }

    // If the current node is not created
    if (!currentNode().cacheEntry().identifiableItem()
        && !currentNode().cacheEntry().data().isEmpty()) {
        ContentItemInterface *item
                = q->contentItemFromData(const_cast<SocialNetworkInterface*>(q),
                                         currentNode().cacheEntry().data());
        // Update the cache.
        currentNode().cacheEntry().setItem(item);
    }

    if (node != currentNode().cacheEntry().identifiableItem()) {
        // Create a node if not created

        node = currentNode().cacheEntry().identifiableItem();
        emit q->nodeChanged();
    }
}

void SocialNetworkInterfacePrivate::updateRelatedData()
{
    Q_Q(SocialNetworkInterface);
    if (nodeStackIndex == -1) {
        return;
    }


    bool modelShouldBeChanged = false;
    bool sameCount = false;
    QList<CacheEntry> newData = currentNode().data();

    // If the count is not the same, the model should be changed
    if (internalData.count() != newData.count()) {
        modelShouldBeChanged = true;
    }

    // If the count is the same, but not the same data
    // the model should be changed
    if (!modelShouldBeChanged) {
        for (int i = 0; i < newData.count(); i++) {
            if (internalData.at(i) != newData.at(i)) {
                modelShouldBeChanged = true;
                sameCount = true;
                break;
            }
        }
    }

    if (modelShouldBeChanged) {
        if (internalData.count() > 0) {
            q->beginRemoveRows(QModelIndex(), 0, internalData.count() - 1);
            internalData.clear();
            q->endRemoveRows();
        }

        if (currentNode().data().count() > 0) {
            q->beginInsertRows(QModelIndex(), 0, newData.count() - 1);
            internalData = q->sortedData(newData);
            q->endInsertRows();
        }

        if (!sameCount) {
            emit q->countChanged();
        }
    }

    updateNextAndPrevious();
}

void SocialNetworkInterfacePrivate::init()
{
    Q_Q(SocialNetworkInterface);
    networkAccessManager = new QNetworkAccessManager(q);

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
    q->setRoleNames(roleNames());
#endif
}

/*! \internal */
void SocialNetworkInterfacePrivate::filters_append(QDeclarativeListProperty<FilterInterface> *list,
                                                   FilterInterface *filter)
{
    SocialNetworkInterface *socialNetwork = qobject_cast<SocialNetworkInterface *>(list->object);
    if (socialNetwork && filter) {
        QObject::connect(filter, SIGNAL(destroyed(QObject*)),
                         socialNetwork, SLOT(filterDestroyedHandler(QObject*)));
        socialNetwork->d_func()->filters.append(filter);
    }
}

/*! \internal */
FilterInterface *SocialNetworkInterfacePrivate::filters_at(QDeclarativeListProperty<FilterInterface> *list, int index)
{
    SocialNetworkInterface *socialNetwork = qobject_cast<SocialNetworkInterface *>(list->object);
    if (socialNetwork && socialNetwork->d_func()->filters.count() > index && index >= 0)
        return socialNetwork->d_func()->filters.at(index);
    return 0;
}

/*! \internal */
void SocialNetworkInterfacePrivate::filters_clear(QDeclarativeListProperty<FilterInterface> *list)
{
    SocialNetworkInterface *socialNetwork = qobject_cast<SocialNetworkInterface *>(list->object);
    if (socialNetwork) {
        socialNetwork->d_func()->filters.clear();
    }
}

/*! \internal */
int SocialNetworkInterfacePrivate::filters_count(QDeclarativeListProperty<FilterInterface> *list)
{
    SocialNetworkInterface *socialNetwork = qobject_cast<SocialNetworkInterface *>(list->object);
    if (socialNetwork)
        return socialNetwork->d_func()->filters.count();
    return 0;
}

/*! \internal */
void SocialNetworkInterfacePrivate::sorters_append(QDeclarativeListProperty<SorterInterface> *list,
                                                   SorterInterface *sorter)
{
    SocialNetworkInterface *socialNetwork = qobject_cast<SocialNetworkInterface *>(list->object);
    if (socialNetwork && sorter) {
        QObject::connect(sorter, SIGNAL(destroyed(QObject*)),
                         socialNetwork, SLOT(sorterDestroyedHandler(QObject*)));
        socialNetwork->d_func()->sorters.append(sorter);
        if (!socialNetwork->d_func()->resortUpdatePosted) {
            socialNetwork->d_func()->resortUpdatePosted = true;
            QCoreApplication::instance()->postEvent(socialNetwork, new QEvent(QEvent::User));
        }
    }
}

/*! \internal */
SorterInterface *SocialNetworkInterfacePrivate::sorters_at(QDeclarativeListProperty<SorterInterface> *list, int index)
{
    SocialNetworkInterface *socialNetwork = qobject_cast<SocialNetworkInterface *>(list->object);
    if (socialNetwork && socialNetwork->d_func()->sorters.count() > index && index >= 0)
        return socialNetwork->d_func()->sorters.at(index);
    return 0;
}

/*! \internal */
void SocialNetworkInterfacePrivate::sorters_clear(QDeclarativeListProperty<SorterInterface> *list)
{
    SocialNetworkInterface *socialNetwork = qobject_cast<SocialNetworkInterface *>(list->object);
    if (socialNetwork) {
        socialNetwork->d_func()->sorters.clear();
    }
}

/*! \internal */
int SocialNetworkInterfacePrivate::sorters_count(QDeclarativeListProperty<SorterInterface> *list)
{
    SocialNetworkInterface *socialNetwork = qobject_cast<SocialNetworkInterface *>(list->object);
    if (socialNetwork)
        return socialNetwork->d_func()->sorters.count();
    return 0;
}

/*! \interface */
void SocialNetworkInterfacePrivate::filterDestroyedHandler(QObject *object)
{
    FilterInterface *filter = static_cast<FilterInterface *>(object);
    filters.removeAll(filter);
}

/*! \interface */
void SocialNetworkInterfacePrivate::sorterDestroyedHandler(QObject *object)
{
    SorterInterface *sorter = static_cast<SorterInterface *>(object);
    sorters.removeAll(sorter);
}

Node SocialNetworkInterfacePrivate::currentNode() const
{
    return nodeStack.at(nodeStackIndex);
}

void SocialNetworkInterfacePrivate::checkCacheEntryRefcount(const CacheEntry &entry)
{
    if (cache.contains(entry.identifier()) && entry.refcount() == 0) {
        cache.remove(entry.identifier());
    }
}

bool SocialNetworkInterfacePrivate::deleteLastNode()
{
    if (cache.isEmpty()) {
        qWarning() << Q_FUNC_INFO << "The node stack is empty";
    }

    Node node = nodeStack.pop();
    node.cacheEntry().deref();

    foreach (CacheEntry entry, node.data()) {
        entry.deref();
    }

    checkCacheEntryRefcount(node.cacheEntry());
    foreach (CacheEntry entry, node.data()) {
        checkCacheEntryRefcount(entry);
    }
    return true;
}

void SocialNetworkInterfacePrivate::itemDataChangedHandler()
{
    Q_Q(SocialNetworkInterface);
    IdentifiableContentItemInterface *item
            = qobject_cast<IdentifiableContentItemInterface *>(q->sender());
    if (!item) {
        return;
    }

    QString identifier = item->identifier();
    if (cache.contains(identifier)) {
        // Check data and update if needed
        CacheEntry cacheEntry = cache.value(identifier);

        if (cacheEntry.item() == item) {
            if (cacheEntry.data() != item->data()) {
                cacheEntry.setData(item->data());
            }
        }
    }
}

void SocialNetworkInterfacePrivate::updateNextAndPrevious()
{
    Q_Q(SocialNetworkInterface);
    bool newHasPrevious = false;
    bool newHasNext = false;

    if (nodeStackIndex > -1) {
        newHasPrevious = currentNode().hasPrevious();
        newHasNext = currentNode().hasNext();
    }
    if (hasPrevious != newHasPrevious) {
        hasPrevious = newHasPrevious;
        emit q->hasPreviousChanged();
    }
    if (hasNext != newHasNext) {
        hasNext = newHasNext;
        emit q->hasNextChanged();
    }
}


//----------------------------------------------------

/*!
    \qmltype SocialNetwork
    \instantiates SocialNetworkInterface
    \inqmlmodule org.nemomobile.social 1
    \brief Provides an abstraction API for graph- or model-based social
    network APIs.

    The SocialNetwork type should never be used directly by clients.
    Instead, clients should use specific implementations of the SocialNetwork
    interface, such as the Facebook adapter.

    The SocialNetwork type provides a generic API which allows content
    to be retrieved from a social network and exposed via a model.
    The API consists of a central \c node which is an IdentifiableContentItem,
    which may be specified by the client via the \c nodeIdentifier property.
    The data in the model will be populated from the graph connections of
    the node.

    The model roles are as follows:
    \list
    \li contentItem - the instantiated ContentItem related to the node
    \li contentItemType - the type of the ContentItem related to the node
    \li contentItemData - the underlying QVariantMap data of the ContentItem related to the node
    \li contentItemIdentifier - the identifier of the ContentItem related to the node, or an empty string
    \endlist

    Please see the documentation of the Facebook adapter for an example
    of how clients can use the SocialNetwork model in an application.
*/  

SocialNetworkInterface::SocialNetworkInterface(QObject *parent)
    : QAbstractListModel(parent), d_ptr(new SocialNetworkInterfacePrivate(this))
{
    Q_D(SocialNetworkInterface);
    d->init();
}

SocialNetworkInterface::SocialNetworkInterface(SocialNetworkInterfacePrivate &dd, QObject *parent)
    : QAbstractListModel(parent), d_ptr(&dd)
{
    Q_D(SocialNetworkInterface);
    d->init();
}

SocialNetworkInterface::~SocialNetworkInterface()
{
}

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
QHash<int, QByteArray> SocialNetworkInterface::roleNames() const
{
    return SocialNetworkInterfacePrivate::roleNames();
}
#endif


void SocialNetworkInterface::classBegin()
{
    Q_D(SocialNetworkInterface);
    d->initialized = false;
}

void SocialNetworkInterface::componentComplete()
{
    Q_D(SocialNetworkInterface);
    // If you override this implementation, you MUST set d->initialized=true.
    d->initialized = true;
    d->setStatus(Idle);

    if (d->populatePending) {
        populate();
        d->populatePending = false;
        return;
    }
}

/*!
    \qmlmethod void SocialNetwork::nextNode()
    Navigates to the next node in the node stack, if it exists.
    The data in the model will be populated from the cache, if cached
    data for the node exists.  If you want to repopulate the data from
    the social network, you must call \c setNodeIdentifier() manually.

    The node stack is built up from successive changes to the
    \c nodeIdentifier property.
*/
void SocialNetworkInterface::nextNode()
{
    Q_D(SocialNetworkInterface);

    d->nodeStackIndex ++;
    if (d->nodeStackIndex >= d->nodeStack.count()) {
        d->nodeStackIndex = d->nodeStack.count() - 1;
        qWarning() << Q_FUNC_INFO << "Already at last node in cache !";
        return;
    }

    d->updateNodePositionStatus();
    d->updateRelatedData();
}

/*!
    \qmlmethod void SocialNetwork::previousNode()
    Navigates to the previous node in the node stack, if it exists.
    The data in the model will be populated from the cache, if cached
    data for the node exists.  If you want to repopulate the data from
    the social network, you must call \c setNodeIdentifier() manually.

    The node stack is built up from successive changes to the
    \c nodeIdentifier property.
*/
void SocialNetworkInterface::previousNode()
{
    Q_D(SocialNetworkInterface);
    if (d->nodeStackIndex <= -1) {
        d->nodeStackIndex = -1;
        qWarning() << Q_FUNC_INFO << "Already at first node in cache !";
        return;
    }


    d->nodeStackIndex --;
    d->updateNodePositionStatus();
    d->updateRelatedData();
}

void SocialNetworkInterface::popNode()
{
    Q_D(SocialNetworkInterface);
    previousNode();
    d->deleteLastNode();
}

/*!
    \qmlmethod QObject *SocialNetwork::relatedItem(int index)
    Returns the ContentItem which is related to the node from the given
    \a index of the model data.  This is identical to calling
    \c data() for the given model index and specifying the \c contentItem
    role.

    \note Although this function will always return a pointer to a
    ContentItem, the return type of the function is QObject*, so that
    this function can be used via QMetaObject::invokeMethod().
*/
QObject *SocialNetworkInterface::relatedItem(int index) const
{
    QVariant cv = data(QAbstractListModel::index(index), SocialNetworkInterface::ContentItemRole);
    if (!cv.isValid())
        return 0;
    ContentItemInterface *ci = cv.value<ContentItemInterface*>();
    
    return ci;
}

void SocialNetworkInterface::loadNext()
{
    qWarning() << Q_FUNC_INFO << "Error: this function MUST be implemented by derived types!";
}

void SocialNetworkInterface::loadPrevious()
{
    qWarning() << Q_FUNC_INFO << "Error: this function MUST be implemented by derived types!";
}

/*!
    \qmlproperty SocialNetwork::Status SocialNetwork::status
    Holds the current status of the social network.
*/
SocialNetworkInterface::Status SocialNetworkInterface::status() const
{
    Q_D(const SocialNetworkInterface);
    return d->status;
}

/*!
    \qmlproperty SocialNetwork::ErrorType SocialNetwork::error
    Holds the most recent error which occurred during initialization
    or a network request.  Note that it will not be reset if subsequent
    operations are successful.
*/
SocialNetworkInterface::ErrorType SocialNetworkInterface::error() const
{
    Q_D(const SocialNetworkInterface);
    return d->error;
}

/*!
    \qmlproperty QString SocialNetwork::errorMessage
    Holds the message associated with the most recent error which occurred
    during initialization or a network request.  Note that it will not be
    reset if subsequent operations are successful.
*/
QString SocialNetworkInterface::errorMessage() const
{
    Q_D(const SocialNetworkInterface);
    return d->errorMessage;
}

/*!
    \qmlproperty QString SocialNetwork::nodeIdentifier
    Holds the identifier of the "central" content item.  This content item
    is the \c node of the current view of the social network graph.
    The data in the model will be populated from the graph connections
    to the node.

    If this property is not set, the node will be initialized to the
    current user node by the specific social network implementation adapter.

    As the client changes the \c nodeIdentifier, the SocialNetwork will
    request the related data from the network, and build up a node stack
    of visited nodes.  For each visited node, a cache of related content
    items (model data) is stored.  The size of the cache is implementation
    specific.

    Clients can later navigate down and up the stack using the \l previousNode() and
    \l nextNode() functions respectively.  Those operations are very cheap
    as they do not trigger any network requests in the common case.

    If the \c nodeIdentifier is set to an identifier which isn't represented
    in the node stack, the \c node property will be set to an empty placeholder
    node until the network request completes and the node can be populated with
    the downloaded data.

    If the \c nodeIdentifier is set to the identifier of the current node,
    the cached data for the node will be cleared and the node and its related
    data will be reloaded from the network.
*/
QString SocialNetworkInterface::nodeIdentifier() const
{
    Q_D(const SocialNetworkInterface);
    return d->nodeIdentifier;
}

void SocialNetworkInterface::setNodeIdentifier(const QString &contentItemIdentifier)
{
    Q_D(SocialNetworkInterface);
    if (d->nodeIdentifier != contentItemIdentifier) {
        d->nodeIdentifier = contentItemIdentifier;
        emit nodeIdentifierChanged();
    }
}

/*!
    \qmlproperty IdentifiableContentItem *SocialNetwork::node
    Holds the "central" content item, or node, which defines the
    current view of the social network graph.  The data exposed in the
    SocialNetwork model will reflect the connections to the node.

    The node must be an identifiable content item (that is, it must
    have a unique identifier in the social network graph).
    Clients cannot set the node property directly, but instead must
    set the \c nodeIdentifier property.
*/
IdentifiableContentItemInterface *SocialNetworkInterface::node() const
{
    Q_D(const SocialNetworkInterface);
    return d->node;
}

/*!
    \qmlproperty QVariantMap SocialNetwork::relevanceCriteria
    Holds the social-network-specific relevance criteria which will
    be used to calculate the relevance of related content items.
    This relevance can be important in filtering and sorting operations.
*/
QVariantMap SocialNetworkInterface::relevanceCriteria() const
{
    Q_D(const SocialNetworkInterface);
    return d->relevanceCriteria;
}

bool SocialNetworkInterface::hasPreviousNode() const
{
    Q_D(const SocialNetworkInterface);
    return d->hasPreviousNode;
}

bool SocialNetworkInterface::hasNextNode() const
{
    Q_D(const SocialNetworkInterface);
    return d->hasNextNode;
}

bool SocialNetworkInterface::hasPrevious() const
{
    Q_D(const SocialNetworkInterface);
    return d->hasPrevious;
}

bool SocialNetworkInterface::hasNext() const
{
    Q_D(const SocialNetworkInterface);
    return d->hasNext;
}

void SocialNetworkInterface::setRelevanceCriteria(const QVariantMap &relevanceCriteria)
{
    Q_D(SocialNetworkInterface);
    if (d->relevanceCriteria != relevanceCriteria) {
        d->relevanceCriteria = relevanceCriteria;
        emit relevanceCriteriaChanged();
    }
}

/*!
    \qmlproperty QDeclarativeListProperty<Filter> SocialNetwork::filters
    Holds the list of filters which will be applied to the related content
    of the node.  Only those related content items which match each of the
    filters will be exposed as data in the model.

    Specific implementations of the SocialNetwork interface may not support
    certain standard filter types, or they may not support filtering at all.
*/
QDeclarativeListProperty<FilterInterface> SocialNetworkInterface::filters()
{
    return QDeclarativeListProperty<FilterInterface>(this, 0,
            &SocialNetworkInterfacePrivate::filters_append,
            &SocialNetworkInterfacePrivate::filters_count,
            &SocialNetworkInterfacePrivate::filters_at,
            &SocialNetworkInterfacePrivate::filters_clear);
}

/*!
    \qmlproperty QDeclarativeListProperty<Sorter> SocialNetwork::sorters
    Holds the list of sorters which will be applied to the related content
    of the node.  The order of sorters in the list is important, as it
    defines which sorting is applied first.

    Specific implementations of the SocialNetwork interface may not support
    certain standard sorter types, or they may not support sorting at all.
*/
QDeclarativeListProperty<SorterInterface> SocialNetworkInterface::sorters()
{
    return QDeclarativeListProperty<SorterInterface>(this, 0,
            &SocialNetworkInterfacePrivate::sorters_append,
            &SocialNetworkInterfacePrivate::sorters_count,
            &SocialNetworkInterfacePrivate::sorters_at,
            &SocialNetworkInterfacePrivate::sorters_clear);
}

/*!
    \qmlproperty int SocialNetwork::count
    Returns the number of content items related to the \c node
    are exposed in the model.  Only those content items which
    match the specified \c filters will be exposed in the model,
    if the specific implementation of the SocialNetwork interface
    supports every filter in the \c filters list.
*/
int SocialNetworkInterface::count() const
{
    Q_D(const SocialNetworkInterface);
    return d->internalData.count();
}

int SocialNetworkInterface::rowCount(const QModelIndex &index) const
{
    Q_D(const SocialNetworkInterface);
    // only allow non-valid (default) parent index.
    if (index.isValid())
        return 0;
    return d->internalData.count();
}

int SocialNetworkInterface::columnCount(const QModelIndex &index) const
{
    // only allow non-valid (default) parent index.
    if (index.isValid())
        return 0;
    return 1;
}

QVariant SocialNetworkInterface::data(const QModelIndex &index, int role) const
{
    Q_D(const SocialNetworkInterface);
    if (!index.isValid() || index.row() >= d->internalData.count() || index.row() < 0) {
        return QVariant();
    }

    CacheEntry cacheEntry = d->internalData.at(index.row());

    switch (role) {
    case ContentItemTypeRole:
        return QVariant::fromValue(cacheEntry.data().value(NEMOQMLPLUGINS_SOCIAL_CONTENTITEMTYPE).toInt());
    case ContentItemDataRole:
        return QVariant::fromValue(cacheEntry.data());
    case ContentItemIdentifierRole:
        return QVariant::fromValue(cacheEntry.data().value(NEMOQMLPLUGINS_SOCIAL_CONTENTITEMID).toString());
    case ContentItemRole:
        {
            if (cacheEntry.item()) {
                return QVariant::fromValue(cacheEntry.item());
            }
            // Method here ?
            // Instantiate the item.
            // Should be exported into a specific method
            ContentItemInterface *newItem = contentItemFromData(const_cast<SocialNetworkInterface*>(this),
                                                                cacheEntry.data());
            connect(newItem, SIGNAL(dataChanged()), this, SLOT(itemDataChangedHandler()));

            // Update the cache.
            cacheEntry.setItem(newItem);
            return QVariant::fromValue(newItem);
        }
        break;
    case SectionRole:
        return dataSection(cacheEntry.data().value(NEMOQMLPLUGINS_SOCIAL_CONTENTITEMTYPE).toInt(),
                           cacheEntry.data());
    default:
        return QVariant();
    }
}

/*!
    \qmlmethod bool SocialNetwork::arbitraryRequest(SocialNetwork::RequestType requestType, const QString &requestUri, const QVariantMap &queryItems = QVariantMap(), const QString &postData = QString())

    Performs the HTTP request of the specified \a requestType (\c Get, \c Post or \c Delete) with
    the specified \a requestUri and \a queryItems.  If the request is a Post request, the given
    \a postData will be converted to a QByteArray via \c{QByteArray::fromBase64(postData.toLatin1())}
    and used as the \c Post data.

    When a successfully started request is completed, the \c arbitraryRequestResponseReceived()
    signal will be emitted, with the response data included as the \c data parameter.

    The request will not be started successfully if another arbitrary request is in progress.
    Returns true if the request could be started successfully, false otherwise. 
*/
bool SocialNetworkInterface::arbitraryRequest(RequestType requestType, const QString &requestUri,
                                              const QVariantMap &queryItems,
                                              const QString &postData)
{
    Q_D(SocialNetworkInterface);
    if (!d->arbitraryRequestHandler) {
        d->arbitraryRequestHandler = new ArbitraryRequestHandler(d->networkAccessManager, this);
        connect(d->arbitraryRequestHandler,
                SIGNAL(arbitraryRequestResponseReceived(bool,QVariantMap)),
                this,
                SIGNAL(arbitraryRequestResponseReceived(bool,QVariantMap)));
    }
    return d->arbitraryRequestHandler->request(requestType, requestUri, queryItems, postData);
}

QVariantMap SocialNetworkInterface::contentItemData(ContentItemInterface *contentItem) const
{
    // Helper function for SocialNetworkInterface-derived types
    return contentItem->dataPrivate();
}

void SocialNetworkInterface::setContentItemData(ContentItemInterface *contentItem, const QVariantMap &data) const
{
    // Helper function for SocialNetworkInterface-derived types
    contentItem->setDataPrivate(data);
}

/*
    Specific implementations of the SocialNetwork interface MUST implement this
    function.  It will be called to populate the model data as a filtered and
    sorted view of the content items related to the specified node in the
    social network.
*/
void SocialNetworkInterface::populate()
{
    Q_D(SocialNetworkInterface);

    switch (d->status) {
    case Initializing:
        // Should queue the populate and wait for initialized complete
        d->populatePending = true;
        return;
    case Busy:
        qWarning() << Q_FUNC_INFO << "Cannot populate: another populate is already running";
        return;
    case Invalid:
        qWarning() << Q_FUNC_INFO
                   << "The social network is not in a valid state, no operation can be complete";
        return;
    default:
        break;
    }

    Node node (d->nodeIdentifier, d->filters.toSet());
    if (node.isNull()) {
        qWarning() << Q_FUNC_INFO << "The node is null. It should not be added";
        return;
    }

    if (!d->nodeStack.isEmpty()) {
        if (node == d->lastNode()) {
            qWarning() << Q_FUNC_INFO
                       << "The node being added is already the last one.";
            return;
        }
    }

    bool callPopulateNodeData = false;
    bool callPopulateNodeRelatedData = false;

    // Check if there is already a node corresponding to that node
    int cachedNodeIndex = d->nodeStack.indexOf(node);
    if (cachedNodeIndex != -1) {
        const Node &cachedNode = d->nodeStack.at(cachedNodeIndex);

        node.setCacheEntry(cachedNode.cacheEntry());
        node.setData(cachedNode.data());

        d->setStatus(SocialNetworkInterface::Idle);
    } else if (d->cache.contains(d->nodeIdentifier)) {
        if (validateCacheEntryForLastNode(d->cache.value(d->nodeIdentifier).data())) {
            node.setCacheEntry(d->cache.value(d->nodeIdentifier));
            callPopulateNodeRelatedData = true;
        } else {
            callPopulateNodeData = true;
        }
        d->setStatus(SocialNetworkInterface::Busy);
    } else {
        d->setStatus(SocialNetworkInterface::Busy);
        callPopulateNodeData = true;
    }

    while (d->nodeStack.count() > d->nodeStackIndex + 1) {
        d->deleteLastNode();
    }
    d->nodeStack.append(node);

    if (callPopulateNodeData) {
        populateDataForLastNode();
    }

    if (callPopulateNodeRelatedData) {
        populateRelatedDataforLastNode();
    }
}

bool SocialNetworkInterface::isInitialized() const
{
    Q_D(const SocialNetworkInterface);
    return d->initialized;
}

bool SocialNetworkInterface::event(QEvent *e)
{
    Q_D(SocialNetworkInterface);
    if (e->type() == QEvent::User) {
        d->resortUpdatePosted = false;
        d->internalData = sortedData(d->internalData);
        emit dataChanged(index(0), index(d->internalData.count() - 1));
        return true;
    }
    return QObject::event(e);
}

/*
    Specific implementations of the SocialNetwork interface MUST implement this
    function.  It must be implemented such that it performs the appropriate
    get request to retrieve the data for the specified \c objectIdentifier, or
    data related to that object according to the given \c extraPath parameter.
    If possible, only the data specified by the \c whichFields parameter should
    be retrieved, to minimise network usage.  The \c extraData parameter is
    implementation specific, and may be used to modify the behaviour of the request.
*/
QNetworkReply *SocialNetworkInterface::getRequest(const QString &, const QString &,
                                                  const QStringList &, const QVariantMap &)
{
    qWarning() << Q_FUNC_INFO << "Error: this function MUST be implemented by derived types!";
    return 0;
}

/*
    Specific implementations of the SocialNetwork interface MUST implement this
    function.  It must be implemented such that it performs the appropriate
    post request to upload the \c data for the specified \c objectIdentifier, or
    data related to that object according to the given \c extraPath parameter.
    The \c extraData parameter is implementation specific, and may be used to
    modify the behaviour of the request.
*/
QNetworkReply *SocialNetworkInterface::postRequest(const QString &, const QString &,
                                                   const QVariantMap &, const QVariantMap &)
{
    qWarning() << Q_FUNC_INFO << "Error: this function MUST be implemented by derived types!";
    return 0;
}

/*
    Specific implementations of the SocialNetwork interface MUST implement this
    function.  It must be implemented such that it performs the appropriate
    delete request to delete the object identified by the specified
    \c objectIdentifier, or data related to that object according to the given
    \c extraPath parameter.  The \c extraData parameter is implementation specific,
    and may be used to modify the behaviour of the request.
*/
QNetworkReply *SocialNetworkInterface::deleteRequest(const QString &, const QString &,
                                                     const QVariantMap &)
{
    qWarning() << Q_FUNC_INFO << "Error: this function MUST be implemented by derived types!";
    return 0;
}

/*
    TODO: do the documentation
*/
QString SocialNetworkInterface::dataSection(int type, const QVariantMap &data) const
{
    Q_UNUSED(type)
    Q_UNUSED(data)
    return QString();
}

/*
    Specific implementations of the SocialNetwork interface MUST implement this
    function.  It must return an instance of the correct ContentItem-derived type
    given the QVariantMap of data.  This function is called when the \c contentItem
    role for a specific model index is requested via the model data() function, to
    instantiate the content item from the content item data lazily.
*/
ContentItemInterface *SocialNetworkInterface::contentItemFromData(QObject *parent,
                                                                  const QVariantMap &data) const
{
    Q_UNUSED(parent)
    Q_UNUSED(data)
    qWarning() << Q_FUNC_INFO << "Error: this function MUST be implemented by derived types!";
    return 0;
}

QList<CacheEntry> SocialNetworkInterface::sortedData(const QList<CacheEntry> &data)
{
    Q_D(SocialNetworkInterface);
    QList<CacheEntry> sortedData = data;
    foreach (SorterInterface *sorter, d->sorters) {
        SorterFunctor functor (sorter);
        std::stable_sort(sortedData.begin(), sortedData.end(), functor);
    }

    return sortedData;
}

void SocialNetworkInterface::populateDataForLastNode()
{
    qWarning() << Q_FUNC_INFO << "Error: this function MUST be implemented by derived types!";
    return;
}

void SocialNetworkInterface::populateRelatedDataforLastNode()
{
    qWarning() << Q_FUNC_INFO << "Error: this function MUST be implemented by derived types!";
    return;
}

bool SocialNetworkInterface::validateCacheEntryForLastNode(const QVariantMap &cacheEntryData)
{
    Q_UNUSED(cacheEntryData)
    qWarning() << Q_FUNC_INFO << "Error: this function MUST be implemented by derived types!";
    return true;
}

#include "moc_socialnetworkinterface.cpp"
