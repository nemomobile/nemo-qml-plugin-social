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
#include "socialnetworkmodelinterface.h"
#include "socialnetworkmodelinterface_p.h"

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
    Naming conventions

    We often call node two different things. node is the name of
    a property in SocialNetworkModelInterface. It is an
    IdentifiableContentInterface that represents an entity for
    a social network. We also have CacheNode, that is a class used
    internally to contain data used to perform requests. CacheNode
    contains data that are displayed in SocialNetworkModelInterface,
    including the IdentifiableContentInterface that is displayed
    using the "node" property.
*/
/*
    SocialNetworkInterface

    The SocialNetworkInterface is a central component that is used to
    perform network operations to query a social network, parsing
    the returned data, and storing them. It is also used to manage
    a set of models, that are used to present these retrieved data.

    SocialNetworkInterface, often called SNI, is therefore a QObject,
    that does not expose many public API. Its private API, accessible
    via SocialNetworkInterfacePrivate (SNIp), provides several methods that
    should be reimplemented in order to perform these network operations.
*/

/*
    SocialNetworkModelInterface

    The SocialNetworkModelInterface is tightly coupled to
    SocialNetworkInterface and is used to present the data that has been
    downloaded / cached.

    SocialdNetworkModelInterface, often called SNMI, is a QML model (inheriting
    from QAbstractListModel), that can be used to display data in
    ListView or GridView.

    CacheNode and SNMI are very close, as SNMI are used externally to represent
    the different CacheNodes, and also used to pass information to be stored
    to a CacheNode, like the identifier, type or list of filters.

    In order to query the social network, a SNMI have several properties
    to first get the "node" property and then the related content. First, a
    SNMI should be linked to a SNI by setting
    SocialNetworkModelInterface::socialNetwork().
    The SocialNetworkModelInterface::identifier() and
    SocialNetworkModelInterface::type() properties should be set in the
    SNMI to identify a node. A list of filters should also be passed via
    the SocialNetworkModelInterface::filters() field if related data
    have to be queried. The query is done by calling
    SocialNetworkModelInterface::populate() or
    SocialNetworkModelInterface::repopulate().
*/

/*
    CacheEntry

    A CacheEntry represents an entry in the cache. These entries
    represents any entities, (that inherits from ContentItemInterface),
    but work best with those that have an identifier.

    Each cache entry contains the identifier that is associated to
    the entity. This identifier can be null, in the case of a non identifiable
    entity. It also contains the data, provided as QVariantMap, that is downloaded
    from the social network, the ContentItem pointer that is associated
    to the data, and a refCount, that is used to count the number of use
    of this cache entry.

    The ContentItem pointer is lazily constructed, and will be
    constructed only when it is needed. The refCount is used to know
    when a CacheEntry should be removed from the cache. It is increased
    when a CacheNode uses the entity, and decreased when a CacheNode is
    destroyed. If the refCount is 0, then it should be removed.

    CacheEntries are explicitely shared, so a modification to a CacheEntry
    will modify all the copies. Basically, it behaves like a pointer.
    CacheEntries can also be compared, and are considered equal if they
    share the same identifier, data and item.

    Remark: CacheEntry are not put in the cache if the entity that they
    store is not identifiable.
*/

/*
    CacheNode

    A CacheNode stores data that SNMI have. In order to do that, a CacheNode
    must have an identifier, a type, and a list of filters. It can be
    attached to several SNMI that displays it's content.

    A CacheNode consists of an identifier, a type and a set of filters is used
    to define some specific entity and related content in a social network. It
    also have a CacheEntry, that contains the cached version of the social network
    entity if it has been retrieved, or nothing if it is yet to be retrieved. This
    cached entity is then exposed via the "node" property of SNMI. The CacheNode
    also contains a list of CacheEntry, that are the related data.

    A CacheNode also carries the status of it's loading status, via
    CachheNode::status(). This status do not only differentiates between a CacheNode
    being loaded, or a CacheNode that have loaded information, but also the
    different phases of loading, that are the loading of the central entity, or the
    related data, and how the related data should be added to the model (replace,
    append or prepend to existing data).

    For storing information about paging, a CacheNode also have information about
    the availablility of next and previous pages via CacheNode::hasNext() and
    CacheNode::hasPrevious(). For setting these two attributes,
    CacheNode::setHavePreviousAndNext() should be used.

    A CacheNode also have a nodeExtra attribute, that should be used internally
    to carry informations about the CacheNode, that could be reused in different
    methods. nodeExtra actually acts like a collection of attributes. It is
    already used in Facebook to store informations about paging cursors.

    Remark: the identifier for a CacheNode and for a CacheEntry might be different:
    in Facebook, you can use "me" to replace the identifier of the current user,
    so the initial CacheNode might have "me" as identifier. However, the retrieved
    data will contain the identifier of current user, so the identifier of the
    CacheEntry associated to the CacheNode will not be "me".

    CacheNode are also explicitely shared. They can also be compared, and are
    considered equal if they have the same identifier, type and filters.

    When a CacheEntry is set as the data of a CacheNode or as related data, it's
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

    CacheNodes are created when SNMI are attached to a SNI. The information associated
    with a CacheNode may be presented by multiple SNMI instances simultaneously,
    if different SNMI request the same information (same identifier, type and
    filters). A list of CacheNodes represents all the created CacheNodes. There is also
    a list of SNMI that represents all the models.

    While a CacheEntry is used by a CacheNode, it's refcount is increased. If it is no longer
    used, often when a CacheNode is removed from the CacheNode list, it's refcount is decreased.
    If a refcount for a CacheEntry drops to zero, it means that it is no longer used,
    and is removed from the cache.

    Refcount changing are triggered, either by loading of new data (old data get derefed)
    or by the removal of CacheNodes. A CacheNode is removed if there is no longer any model
    attached to it. If a CacheNode was the only one using some CacheEntry, their refcount will
    go to 0, and they will be removed from the cache.

    When an user perform a request by setting identifier, type and filters to a model, and
    calling SocialNetworkModelInterface::populate(), CacheNodes are created or removed,
    depending on the SNMI, and then, a CacheNode might be populated, or simply retrieved
    from cache, and models be updated. If the user calls
    SocialNetworkModelInterface::repopulate(), a CacheNode is sure to be updated.
*/

/*
    Using the caching system and implementing SocialNetworkInterface

    The cache is rather fragile. If someone mess with the refcounting or with the
    insertion and removal, it might creates segmentation faults, or memory leaks.
    The best way is to never use methods and attributes that are private inside of
    SocialNetworkInterfacePrivate, and to never use CacheEntry::ref() and
    CacheEntry::deref().

    In order to implement a SocialNetworkInterface, the following methods from
    SocialNetworkInterfacePrivate should be implemented:
    - populateDataForNode()
    - populateRelatedDataforNode()
    - contentItemFromData()
    - validateCacheEntryForNode()

    The first method is used to initiate a request to the data of the central entity
    that should be in a given CacheNode. That CacheNode is often freshly pushed to
    the list using SocialNetworkInterface::populate().

    The second method is used to initiate a request to the related data of a given CacheNode.

    The third method is used to translate the data that is retrieved to C++ objects,
    that should derives from ContentItemInterface.

    Finally, the last method is used when a query is in progress. Sometimes, the cached
    values are only partial, and this method should be used to validate if the data
    that is cached, and that is going to be used for the central entity for a new
    CacheNode contains enough information. If not, SNI will reload the central entity
    in order to get more data.

    A practical example of that is in Facebook, when you query a list of friends, you
    only obtain a list of id / name. But when you click on a friend, you might want
    to retrieve all the informations related to that friend. So the implementation
    of FacebookInterface::validateCacheEntryForLastNode() checks if a cache entry
    contains a FacebookUserInterface that only have name and id, and returns false in
    that case, forcing a retrieving of new data for that CacheEntry that will become
    the central entity for a new CacheNode.

    You will also have to implement the following methods, that are used to perform
    network requests:
    - getRequest()
    - postRequest()
    - deleteRequest()
    - handleFinished()

    The last method is particulary important, because it handles the reply from the
    social network. When implementing that method, be sure to differentiate the
    two different loading scenarii, that is the loading of the central entity, and then,
    the loading of the related data.

    Optionally, you may also reimplement the following functions if you have specific
    requirements:

    - dataSection()
    - handleError()
    - handleSslError()

    The first method is used to display a data section for the model. It can
    return a data section, like the one specified in QML ListView documentation.

    The two others relates to error management. You can implement them if you
    want to handle errors in a specific way.

    A set of private methods can be used:

    - setReply() can be used to set the reply associated to a CacheNode. It is not
      needed to retrieve the reply afterwards since all the methods that should
      be reimplemented provide both the CacheNode and the reply that is associated
      to it.

    - setStatus() and setError() can be used to make status and error management
      easier. As usual setters, they set the data, and emit the corresponding
      signals. setError also sets the status to SocialNetworkInterface::Error.

    - deleteReply() should be used to delete reply in finishedHandler(). Replies
      are not automatically deleted, and their deletion should be handled by hand.

    - createCacheEntry() is used to create a CacheEntry, if the entry do not exist
      in the cache. If it already exists, the entry from the cache is retrived. It
      is useful for populating related data and CacheNodes.

    - updateModelNode() and updateModelRelatedData() are used to update either the
      "node" attribute of models or related data of models that are associated to
      a CacheNode. They should be called just after setting the data in the
      CacheNode.

      Be careful when using updateModelNode(), because it requires that the QVariantMap
      associated to the social network entity have a type attribute set, otherwise,
      creation of the ContentItem interface associated to that data will fail.
*/


// TODO XXX: We should document the contentItemFromData method, to use the saved type inside
// data to create the item.

CacheEntryPrivate::CacheEntryPrivate()
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

CacheEntry::Ptr CacheEntry::create()
{
    return CacheEntry::Ptr(new CacheEntry());
}

CacheEntry::Ptr CacheEntry::create(const QVariantMap &data, ContentItemInterface *item)
{
    return CacheEntry::Ptr(new CacheEntry(data, item));
}

CacheEntry::Ptr CacheEntry::create(const QVariantMap &data, const QString &identifier,
                                   ContentItemInterface *item)
{
    return CacheEntry::Ptr(new CacheEntry(data, identifier, item));
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

void CacheEntry::ref()
{
    Q_D(CacheEntry);
    ++ d->refCount;
}

void CacheEntry::deref()
{
    Q_D(CacheEntry);
    -- d->refCount;
}

// This operator is used to compare the content of each CacheEntry
// instead of comapring the value of the pointer. It is used for
// caching. You usually want to retrieve one CacheEntry that have
// already been created instead of creating a new one. So we compare
// the content in order to find the entry using QList::contains or
// QList::indexOf.
bool operator==(CacheEntry::Ptr cacheEntry1, CacheEntry::Ptr cacheEntry2)
{
    return *(cacheEntry1.data()) == *(cacheEntry2.data());
}

CacheNodePrivate::CacheNodePrivate()
    : type(SocialNetworkInterface::Unknown)
    , hasPrevious(false)
    , hasNext(false)
    , status(Initializing)
{
}

CacheNodePrivate::~CacheNodePrivate()
{
}

CacheNode::Ptr CacheNode::create()
{
    return CacheNode::Ptr(new CacheNode());
}

CacheNode::Ptr CacheNode::create(const QString &identifier, int type,
                                 const QSet<FilterInterface *> &filters)
{
    return CacheNode::Ptr(new CacheNode(identifier, type, filters));
}

CacheNode::CacheNode()
    : d_ptr(new CacheNodePrivate())
{
}

CacheNode::CacheNode(const QString &identifier, int type, const QSet<FilterInterface *> &filters)
    : d_ptr(new CacheNodePrivate())
{
    Q_D(CacheNode);
    d->identifier = identifier;
    d->type = type;
    d->filters = filters;
    d->cacheEntry = CacheEntry::Ptr();
}

CacheNode::~CacheNode()
{
}

bool CacheNode::operator==(const CacheNode &other) const
{
    Q_D(const CacheNode);
    return (d->identifier == other.identifier()) && (d->filters == other.filters());
}

bool CacheNode::operator!=(const CacheNode &other) const
{
    return !(*this == other);
}

bool CacheNode::isNull() const
{
    Q_D(const CacheNode);
    return d->identifier.isEmpty();
}

QString CacheNode::identifier() const
{
    Q_D(const CacheNode);
    return d->identifier;
}

int CacheNode::type() const
{
    Q_D(const CacheNode);
    return d->type;
}

QSet<FilterInterface *> CacheNode::filters() const
{
    Q_D(const CacheNode);
    return d->filters;
}

CacheEntry::Ptr CacheNode::cacheEntry() const
{
    Q_D(const CacheNode);
    return d->cacheEntry;
}

void CacheNode::setCacheEntry(CacheEntry::Ptr cacheEntry)
{
    Q_D(CacheNode);
    if (!d->cacheEntry.isNull()) {
        d->cacheEntry->deref();
    }
    d->cacheEntry = cacheEntry;
    d->cacheEntry->ref();
}

CacheEntry::List CacheNode::relatedData() const
{
    Q_D(const CacheNode);
    return d->relatedData;
}

void CacheNode::setRelatedData(const CacheEntry::List &relatedData)
{
    Q_D(CacheNode);
    foreach (CacheEntry::Ptr cacheEntry, d->relatedData) {
        cacheEntry->deref();
    }

    d->relatedData = relatedData;

    foreach (CacheEntry::Ptr cacheEntry, d->relatedData) {
        cacheEntry->ref();
    }
}

bool CacheNode::hasPrevious() const
{
    Q_D(const CacheNode);
    return d->hasPrevious;
}

bool CacheNode::hasNext() const
{
    Q_D(const CacheNode);
    return d->hasNext;
}

void CacheNode::setHavePreviousAndNext(bool hasPrevious, bool hasNext)
{
    Q_D(CacheNode);
    d->hasPrevious = hasPrevious;
    d->hasNext = hasNext;
}

QVariantMap CacheNode::extraInfo() const
{
    Q_D(const CacheNode);
    return d->extraInfo;
}

void CacheNode::setExtraInfo(const QVariantMap &extraInfo)
{
    Q_D(CacheNode);
    d->extraInfo = extraInfo;
}

CacheNodePrivate::Status CacheNode::status() const
{
    Q_D(const CacheNode);
    return d->status;
}

void CacheNode::setStatus(CacheNodePrivate::Status status)
{
    Q_D(CacheNode);
    d->status = status;
}

// This operator is used to compare the content of each Node
// instead of comapring the value of the pointer. It is used for
// caching. You usually want to retrieve one Node that have
// already been created instead of creating a new one. So we compare
// the content in order to find the node using QList::contains or
// QList::indexOf.
bool operator==(CacheNode::Ptr node1, CacheNode::Ptr node2)
{
    return *(node1.data()) == *(node2.data());
}

SocialNetworkInterface::ErrorType CacheNode::error() const
{
    Q_D(const CacheNode);
    return d->error;
}

QString CacheNode::errorMessage() const
{
    Q_D(const CacheNode);
    return d->errorMessage;
}

void CacheNode::setError(SocialNetworkInterface::ErrorType error, const QString &errorMessage)
{
    Q_D(CacheNode);
    d->error = error;
    d->errorMessage = errorMessage;
}

ArbitraryRequestHandler::ArbitraryRequestHandler(QNetworkAccessManager *networkAccessManager,
                                                 SocialNetworkInterface *parent)
    : QObject(parent), networkAccessManager(networkAccessManager), reply(0)
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
                   << "Warning: cannot start arbitrary request: "\
                      "another arbitrary request is in progress";
        return false;
    }

    QList<QPair<QString, QString> > formattedQueryItems;
    QStringList queryItemKeys = queryItems.keys();
    foreach (const QString &key, queryItemKeys) {
        formattedQueryItems.append(
                    qMakePair<QString, QString>(key, queryItems.value(key).toString()));
    }

    QUrl url(requestUri);
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    QUrlQuery query;
    query.setQueryItems(formattedQueryItems);
    url.setQuery(query);
#else
    url.setQueryItems(formattedQueryItems);
#endif
    QNetworkReply *newReply = 0;
    switch (requestType) {
        case SocialNetworkInterface::PostRequest:{
            newReply = networkAccessManager->post(QNetworkRequest(url),
                                               QByteArray::fromBase64(postData.toLatin1()));
        }
        break;
        case SocialNetworkInterface::DeleteRequest: {
            newReply = networkAccessManager->deleteResource(QNetworkRequest(url));
        }
        break;
        default: {
            newReply = networkAccessManager->get(QNetworkRequest(url));
        }
        break;
    }

    if (newReply) {
        reply = newReply;
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
    bool success = false;
    if (reply) {
        success = (reply->error() == QNetworkReply::NoError);
        replyData = reply->readAll();
        reply->deleteLater();
        reply = 0;
    }

    QVariantMap responseData;
    if (!success) {
        // note that errors to arbitrary requests don't cause the SocialNetwork
        // to transition to the Error state.  They are unrelated to the model.
        responseData.insert(QLatin1String("error"), errorMessage);
        errorMessage = QString();
    } else {
        bool ok = false;
        QVariantMap parsedData = ContentItemInterfacePrivate::parseReplyData(replyData, &ok);
        if (!ok) {
            responseData.insert(QLatin1String("response"), replyData);
        } else {
            responseData = parsedData;
        }
    }

    emit arbitraryRequestResponseReceived(success, responseData);
}

void ArbitraryRequestHandler::errorHandler(QNetworkReply::NetworkError networkError)
{
    errorMessage = networkErrorString(networkError);
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
}


SorterFunctor::SorterFunctor(SorterInterface *sorter):
    m_sorter(sorter)
{
}

bool SorterFunctor::operator()(CacheEntry::ConstPtr first, CacheEntry::ConstPtr second) const
{
    return m_sorter->firstLessThanSecond(first->data(), second->data());
}

SocialNetworkInterfacePrivate::SocialNetworkInterfacePrivate(SocialNetworkInterface *q)
    : networkAccessManager(0)
    , q_ptr(q)
    , initialized(false)
    , arbitraryRequestHandler(0)
{
}

SocialNetworkInterfacePrivate::~SocialNetworkInterfacePrivate()
{
    // We should say to all models that we are being destroyed
    foreach (SocialNetworkModelInterface *model, models) {
        model->d_func()->setNode(0);
        model->d_func()->modelData.clear();
    }

    // Remove all nodes
    foreach (CacheNode::Ptr cacheNode, cacheNodes) {
        deleteNode(cacheNode);
    }

    // Remove all cache entries.
    cache.clear();

    // Remove all replies
    foreach (QNetworkReply *reply, replyToNodeMap.keys()) {
        reply->deleteLater();
    }
    replyToNodeMap.clear();
}

void SocialNetworkInterfacePrivate::populateDataForNode(CacheNode::Ptr node)
{
    Q_UNUSED(node)
    qWarning() << Q_FUNC_INFO << "Error: this function MUST be implemented by derived types!";
}

void SocialNetworkInterfacePrivate::populateRelatedDataforNode(CacheNode::Ptr node)
{
    Q_UNUSED(node)
    qWarning() << Q_FUNC_INFO << "Error: this function MUST be implemented by derived types!";
}

bool SocialNetworkInterfacePrivate::validateCacheEntryForNode(CacheEntry::ConstPtr cacheEntry)
{
    Q_UNUSED(cacheEntry)
    qWarning() << Q_FUNC_INFO << "Error: this function MUST be implemented by derived types!";
    return true;
}

QString SocialNetworkInterfacePrivate::dataSection(int type, const QVariantMap &data) const
{
    Q_UNUSED(type)
    Q_UNUSED(data)
    qWarning() << Q_FUNC_INFO << "Error: this function MUST be implemented by derived types!";
    return QString();
}

// TODO: document the fact that you are expected to have the type set when calling this method
ContentItemInterface * SocialNetworkInterfacePrivate::contentItemFromData(const QVariantMap &data,
                                                                          QObject *parent) const
{
    Q_UNUSED(data)
    Q_UNUSED(parent)
    qWarning() << Q_FUNC_INFO << "Error: this function MUST be implemented by derived types!";
    return 0;
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
QNetworkReply * SocialNetworkInterfacePrivate::getRequest(const QString &objectIdentifier,
                                                          const QString &extraPath,
                                                          const QStringList &whichFields,
                                                          const QVariantMap &extraData)
{
    Q_UNUSED(objectIdentifier)
    Q_UNUSED(extraPath)
    Q_UNUSED(whichFields)
    Q_UNUSED(extraData)
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
QNetworkReply * SocialNetworkInterfacePrivate::postRequest(const QString &objectIdentifier,
                                                           const QString &extraPath,
                                                           const QVariantMap &data,
                                                           const QVariantMap &extraData)
{
    Q_UNUSED(objectIdentifier)
    Q_UNUSED(extraPath)
    Q_UNUSED(data)
    Q_UNUSED(extraData)
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
QNetworkReply * SocialNetworkInterfacePrivate::deleteRequest(const QString &objectIdentifier,
                                                             const QString &extraPath,
                                                             const QVariantMap &extraData)
{
    Q_UNUSED(objectIdentifier)
    Q_UNUSED(extraPath)
    Q_UNUSED(extraData)
    qWarning() << Q_FUNC_INFO << "Error: this function MUST be implemented by derived types!";
    return 0;
}

/*
    Depending on the implementation of SocialNetwork, this method
    could be reimplemented in order for the SocialNetwork class to guess
    the type property (SocialNetworkModelInterface::nodeType) that should
    be set.

    An example is with Twitter, where this type property is mandatory, while
    sometimes useless. If an user requests a list of tweets, we automatically
    knows that the type refers to an user, because the "tweet" connection
    can only relates to an user. In TwitterInterface, this method is then
    implemented to automatically return TwitterInterface::User when the
    filter contains a filter that is used to request tweets.

    This method is also used to automatically sets the nodeType property
    of SocialNetworkModel, saving the user to set it.

    Guessing type can be done with the identifier, the type the user set (that
    should most of the time be SocialNetworkInterface::Unknown) and the
    list of filters that is used (ie the 3 properties of a
    SocialNetworkModelInterface)
*/
int SocialNetworkInterfacePrivate::guessType(const QString &identifier, int type,
                                             const QSet<FilterInterface *> &filters)
{
    Q_UNUSED(identifier)
    Q_UNUSED(type)
    Q_UNUSED(filters)
    return -1;
}

void SocialNetworkInterfacePrivate::handleFinished(CacheNode::Ptr node, QNetworkReply *reply)
{
    Q_UNUSED(node)
    Q_UNUSED(reply)
    qWarning() << Q_FUNC_INFO << "Error: this function MUST be implemented by derived types!";
}

void SocialNetworkInterfacePrivate::handleError(CacheNode::Ptr node, QNetworkReply *reply,
                                                QNetworkReply::NetworkError networkError)
{
    Q_UNUSED(reply)
    QString errorMessage = networkErrorString(networkError);
    setError(node, SocialNetworkInterface::RequestError, errorMessage);

    qWarning() << Q_FUNC_INFO << "Error: network error occurred:"
               << networkError << ":" << errorMessage;
    qWarning() << Q_FUNC_INFO << "URL:" << reply->url();
    qWarning() << Q_FUNC_INFO << "Content of the reply:" << reply->readAll();
}

void SocialNetworkInterfacePrivate::handleSslError(CacheNode::Ptr node, QNetworkReply *reply,
                                                   const QList<QSslError> &sslErrors)
{
    Q_UNUSED(reply)
    QString errorMessage = QLatin1String("SSL error: ");
    if (sslErrors.isEmpty()) {
        errorMessage += QLatin1String("unknown SSL error");
    } else {
        foreach (const QSslError &error, sslErrors)
            errorMessage += error.errorString() + QLatin1String("; ");
        errorMessage.chop(2);
    }

    setError(node, SocialNetworkInterface::RequestError, errorMessage);

    qWarning() << Q_FUNC_INFO << "Error: SSL error occurred:" << errorMessage;
}

void SocialNetworkInterfacePrivate::setReply(CacheNode::Ptr node, QNetworkReply *reply)
{
    Q_Q(SocialNetworkInterface);
    if (!reply) {
        qWarning() << Q_FUNC_INFO << "Cannot set null reply";
        return;
    }
    replyToNodeMap.insert(reply, node);
    QObject::connect(reply, SIGNAL(finished()), q, SLOT(finishedHandler()));
    QObject::connect(reply, SIGNAL(error(QNetworkReply::NetworkError)),
                     q, SLOT(errorHandler(QNetworkReply::NetworkError)));
    QObject::connect(reply, SIGNAL(sslErrors(QList<QSslError>)),
                     q, SLOT(sslErrorsHandler(QList<QSslError>)));
}

void SocialNetworkInterfacePrivate::setStatus(CacheNode::Ptr node, CacheNodePrivate::Status status)
{
    node->setStatus(status);
    SocialNetworkInterface::Status socialNetworkStatus = correspondingStatus(status);

    foreach (SocialNetworkModelInterface *model, models) {
        if (matches(node, model)) {
            model->d_func()->setStatus(socialNetworkStatus);
        }
    }
}

void SocialNetworkInterfacePrivate::setError(CacheNode::Ptr node,
                                             SocialNetworkInterface::ErrorType error,
                                             const QString &errorMessage)
{
    setStatus(node, CacheNodePrivate::Error);
    node->setError(error, errorMessage);
    foreach (SocialNetworkModelInterface *model, models) {
        if (matches(node, model)) {
            model->d_func()->setStatus(SocialNetworkInterface::Error);
            model->d_func()->setError(error, errorMessage);
        }
    }
}

void SocialNetworkInterfacePrivate::deleteReply(QNetworkReply *reply)
{
    if (!reply) {
        return;
    }

    if (!replyToNodeMap.contains(reply)) {
        qWarning() << Q_FUNC_INFO << "Unknown reply";
        return;
    }

    replyToNodeMap.remove(reply);
    reply->disconnect();
    reply->deleteLater();
}

void SocialNetworkInterfacePrivate::updateModelNode(CacheNode::Ptr node)
{
    Q_Q(SocialNetworkInterface);
    // If the current node is not created
    if (!node->cacheEntry()->item()
        && !node->cacheEntry()->data().isEmpty()) {

        // Beware, we suppose that data already contains the type
        // as createItem should make use of the type inside the data
        // to build the item
        ContentItemInterface *item = createItem(node->cacheEntry());

        // Update the cache.
        node->cacheEntry()->setItem(item);
    } else if (node->cacheEntry()->item()
               && !node->cacheEntry()->data().isEmpty()) {
        // Update the item
        ContentItemInterface *item = node->cacheEntry()->item();
        q->setContentItemData(item, node->cacheEntry()->data());
    }

    // Update the node for each model
    IdentifiableContentItemInterface *item = node->cacheEntry()->identifiableItem();
    foreach (SocialNetworkModelInterface *model, models) {
        if (matches(node, model)) {
            model->d_func()->setNode(item);
            model->d_func()->clean();
        }
    }
}

void SocialNetworkInterfacePrivate::updateModelRelatedData(CacheNode::Ptr node,
                                                           const CacheEntry::List &relatedData)
{
    // We use the status flag to see how the internal data should be updated
    switch (node->status()) {
        case CacheNodePrivate::LoadingRelatedDataReplacing: {
            node->setRelatedData(relatedData);
            foreach (SocialNetworkModelInterface *model, models) {
                if (matches(node, model)) {
                    model->d_func()->setData(relatedData);
                }
            }
        }
        break;
        case CacheNodePrivate::LoadingRelatedDataPrepending: {
            CacheEntry::List data = relatedData;
            data.append(node->relatedData());
            node->setRelatedData(data);
            foreach (SocialNetworkModelInterface *model, models) {
                if (matches(node, model)) {
                    model->d_func()->prependData(relatedData);
                }
            }
        }
        break;
        case CacheNodePrivate::LoadingRelatedDataAppending: {
            CacheEntry::List data = node->relatedData();
            data.append(relatedData);
            node->setRelatedData(data);
            foreach (SocialNetworkModelInterface *model, models) {
                if (matches(node, model)) {
                    model->d_func()->appendData(relatedData);
                }
            }
        }
        break;
        default: break;
    }


}

void SocialNetworkInterfacePrivate::updateModelHavePreviousAndNext(CacheNode::Ptr node,
                                                                   bool havePrevious,
                                                                   bool haveNext)
{
    node->setHavePreviousAndNext(havePrevious, haveNext);
    foreach (SocialNetworkModelInterface *model, models) {
        if (matches(node, model)) {
            model->d_func()->setHavePreviousAndNext(havePrevious, haveNext);
        }
    }
}

CacheEntry::Ptr SocialNetworkInterfacePrivate::createCacheEntry(const QVariantMap &data,
                                                                const QString &identifier)
{
    // Check if there is already a cached entry that corresponds
    if (!identifier.isEmpty()) {
        if (cache.contains(identifier)) {
            CacheEntry::Ptr cacheEntry = cache.value(identifier);

            // Set the new data in the CacheEntry
            cacheEntry->setData(data);
            return cache.value(identifier);
        }
    }

    if (!identifier.isEmpty()) {
        CacheEntry::Ptr cacheEntry = CacheEntry::create(data, identifier);
        cache.insert(identifier, cacheEntry);
        return cacheEntry;
    } else {
        return CacheEntry::create(data);
    }
}

void SocialNetworkInterfacePrivate::setNodeExtraPaging(QVariantMap &nodeExtra,
                                                       const QVariantMap &previousExtra,
                                                       const QVariantMap &nextExtra,
                                                       CacheNodePrivate::Status insertionMode)
{
    // Node::extraInfo is used to provide the cursors that are
    // often used to load next or previous page of data in social API.
    //
    // This data is stored in Node::extraInfo using the
    // NODE_EXTRA_PAGING_PREVIOUS_KEY and NODE_EXTRA_PAGING_NEXT_KEY keys,
    // that are defined in socialnetworkinterface_p.h.
    //
    // Updating this data is sometimes tricky, because when a newer page
    // is loaded a new set of cursors used for loading other pages is
    // provided, and if the page is appended to the model, the new cursors
    // might conflicts with the actual ones. This method is here to
    // override the good set of cursors when prepending or appending
    // data to the result model.
    //
    // The type of storage, as well as managing the fact that there is new
    // data to be loaded is done by the implementer.

    if (insertionMode != CacheNodePrivate::LoadingRelatedDataReplacing
        && insertionMode != CacheNodePrivate::LoadingRelatedDataPrepending
        && insertionMode != CacheNodePrivate::LoadingRelatedDataAppending) {
        return;
    }

    // If we are prepending data, we should override "previous"
    if (insertionMode == CacheNodePrivate::LoadingRelatedDataReplacing
        || insertionMode == CacheNodePrivate::LoadingRelatedDataPrepending) {
        nodeExtra.insert(NODE_EXTRA_PAGING_PREVIOUS_KEY, previousExtra);
    }

    // If we are appending data, we should override "next"
    if (insertionMode == CacheNodePrivate::LoadingRelatedDataReplacing
        || insertionMode == CacheNodePrivate::LoadingRelatedDataAppending) {
        nodeExtra.insert(NODE_EXTRA_PAGING_NEXT_KEY, nextExtra);
    }
}

void SocialNetworkInterfacePrivate::populate(SocialNetworkModelInterface *model,
                                             const QString &identifier,
                                             int type,
                                             const QList<FilterInterface *> &filters, bool reload)
{
    // Refuse when not initialized
    if (!initialized) {
        qWarning() << Q_FUNC_INFO << "SocialNetworkInterface not initialized, the model "\
                      "will not be populated";
        return;
    }

    // If the model do not carry a valid node, we do not add the node
    if (identifier.isEmpty() && filters.isEmpty()) {
        qWarning() << Q_FUNC_INFO << "The node is null. It should not be loaded";
        return;
    }

    QSet<FilterInterface *> filterSet = filters.toSet();

    int guessedType = guessType(identifier, type, filterSet);
    // If we guessed a nice type, we should update all models that old description
    // and set the new type
    if (guessedType != -1) {
        foreach (SocialNetworkModelInterface *model, models) {
            if (matches(identifier, type, filterSet, model)) {
                model->setNodeType(guessedType);
            }
        }
    } else {
        guessedType = type;
    }


    // Create or get the node from cache
    CacheNode::Ptr node = getOrCreateNode(identifier, guessedType, filterSet);

    // Purge the nodes if needed
    // This is needed if (eg) a model was associated to a Node,
    // and then the user performs a request with different id / type / filters
    // with the same model. The old Node that was created is then
    // no longer valid
    checkDoomedNodes();

    // If the node is already loaded, and we didn't ask for a reload,
    // we should just set the nodes
    if (node->status() == CacheNodePrivate::Idle && !reload) {
        model->d_func()->setNode(node->cacheEntry()->identifiableItem());
        model->d_func()->setData(node->relatedData());
        return;
    }

    bool loading = node->status() == CacheNodePrivate::LoadingNodeData
                   || node->status() == CacheNodePrivate::LoadingRelatedDataReplacing
                   || node->status() == CacheNodePrivate::LoadingRelatedDataPrepending
                   || node->status() == CacheNodePrivate::LoadingRelatedDataAppending;

    // If the node is already being loaded and that we called reload
    // we should not perform a reload
    if (loading && reload) {
        qWarning() << Q_FUNC_INFO << "Cannot reload a node when it is loading";
        return;
    }

    // If it is loading or already loaded, we should simply
    // set the loaded data, and set the status to idle
    // before leaving.
    if (loading) {
        model->d_func()->setNode(node->cacheEntry()->identifiableItem());
        model->d_func()->setData(node->relatedData());
        model->d_func()->setStatus(correspondingStatus(node->status()));
        return;
    }

    // If the node is not loaded (or if it is invalid or error)
    // or if the node have to be reloaded, we should start
    // loading the node

    CacheNodePrivate::Status status = CacheNodePrivate::LoadingNodeData;
    IdentifiableContentItemInterface *newItem = 0;

    // We first check if the node is already in the stack
    // (we don't do that if we are asked to reload)
    if (cache.contains(identifier) && !reload) {
        if (validateCacheEntryForNode(cache.value(identifier))) {
            CacheEntry::Ptr cacheEntry = cache.value(identifier);
            cacheEntry->setItem(createItem(cacheEntry));
            node->setCacheEntry(cacheEntry);
            newItem = node->cacheEntry()->identifiableItem();

            status = CacheNodePrivate::LoadingRelatedDataReplacing;
        }
    }

    // Set the status of all nodes to be Busy (should be only one here)
    // and cleans the model
    setStatus(node, status);

    foreach (SocialNetworkModelInterface *model, models) {
        if (matches(node, model)) {
            model->d_func()->setNode(newItem);
            model->d_func()->clean();
        }
    }

    switch (status) {
        case CacheNodePrivate::LoadingNodeData: {
            populateDataForNode(node);
        }
        break;
        case CacheNodePrivate::LoadingRelatedDataReplacing: {
            populateRelatedDataforNode(node);
        }
        break;
        default: {
            return;
        }
        break;
    }
}

void SocialNetworkInterfacePrivate::addModel(SocialNetworkModelInterface *model)
{
    Q_Q(SocialNetworkInterface);

    // Add the model if it do not exists
    if (!models.contains(model)) {
        models.append(model);
        QObject::connect(model, SIGNAL(destroyed(QObject*)),
                         q, SLOT(modelDestroyedHandler(QObject*)));
    }
}

void SocialNetworkInterfacePrivate::removeModel(SocialNetworkModelInterface *model)
{
    if (!models.contains(model)) {
        return;
    }

    // We scan the list of nodes to see if there are nodes that are not used anymore
    if (false) {
        // ^ this false should be replaced by an enum checking for cache management
        // TODO cache management mode
        return;
    }

    models.removeAll(model);
    checkDoomedNodes();
}

void SocialNetworkInterfacePrivate::loadNext(SocialNetworkModelInterface *model)
{
    CacheNode::Ptr node = getNode(model->nodeIdentifier(), model->nodeType(),
                                  model->d_func()->filters.toSet());
    if (node.isNull()) {
        qWarning() << Q_FUNC_INFO << "The model is not loaded. Please call populate() first";
        return;
    }

    setStatus(node, CacheNodePrivate::LoadingRelatedDataAppending);
    populateRelatedDataforNode(node);
}

void SocialNetworkInterfacePrivate::loadPrevious(SocialNetworkModelInterface *model)
{
    CacheNode::Ptr node = getNode(model->nodeIdentifier(), model->nodeType(),
                                  model->d_func()->filters.toSet());
    if (node.isNull()) {
        qWarning() << Q_FUNC_INFO << "The model is not loaded. Please call populate() first";
        return;
    }

    setStatus(node, CacheNodePrivate::LoadingRelatedDataPrepending);
    populateRelatedDataforNode(node);
}

ContentItemInterface * SocialNetworkInterfacePrivate::createItem(CacheEntry::Ptr cacheEntry)
{
    Q_Q(SocialNetworkInterface);
    ContentItemInterface *item = contentItemFromData(cacheEntry->data(), q);
    QObject::connect(item, SIGNAL(dataChanged()), q, SLOT(itemDataChangedHandler()));
    cacheEntry->setItem(item);
    return item;
}

void SocialNetworkInterfacePrivate::finishedHandler()
{
    Q_Q(SocialNetworkInterface);
    QNetworkReply * reply = qobject_cast<QNetworkReply *>(q->sender());
    if (!reply) {
        qWarning() << Q_FUNC_INFO << "Called, but not by a QNetworkReply";
        return;
    }

    if (!replyToNodeMap.contains(reply)) {
        qWarning() << Q_FUNC_INFO << "Called, but reply not associated to a node";
        return;
    }

    CacheNode::Ptr node = replyToNodeMap.value(reply);
    handleFinished(node, reply);
    replyToNodeMap.remove(reply);
}

void SocialNetworkInterfacePrivate::errorHandler(QNetworkReply::NetworkError networkError)
{
    Q_Q(SocialNetworkInterface);
    QNetworkReply * reply = qobject_cast<QNetworkReply *>(q->sender());
    if (!reply) {
        qWarning() << Q_FUNC_INFO << "Called, but not by a QNetworkReply";
        return;
    }

    if (!replyToNodeMap.contains(reply)) {
        qWarning() << Q_FUNC_INFO << "Called, but reply not associated to a node";
        return;
    }

    CacheNode::Ptr node = replyToNodeMap.value(reply);
    handleError(node, reply, networkError);
}

void SocialNetworkInterfacePrivate::sslErrorsHandler(const QList<QSslError> &sslErrors)
{
    Q_Q(SocialNetworkInterface);
    QNetworkReply * reply = qobject_cast<QNetworkReply *>(q->sender());
    if (!reply) {
        qWarning() << Q_FUNC_INFO << "Called, but not by a QNetworkReply";
        return;
    }

    if (!replyToNodeMap.contains(reply)) {
        qWarning() << Q_FUNC_INFO << "Called, but reply not associated to a node";
        return;
    }

    CacheNode::Ptr node = replyToNodeMap.value(reply);
    handleSslError(node, reply, sslErrors);
}

/*! \internal */
void SocialNetworkInterfacePrivate::filterDestroyedHandler(QObject *object)
{
    FilterInterface *filter = static_cast<FilterInterface *>(object);
    if (!filter) {
        return;
    }

    // We search all nodes and check if they have the filter
    // If the node have the filter, we invalidate the content
    // of the node (and set the models associated to invalid)

    CacheNode::List doomedCacheNodes;

    foreach (CacheNode::Ptr cacheNode, cacheNodes) {
        if (cacheNode->filters().contains(filter)) {
            doomedCacheNodes.append(cacheNode);
        }
    }

    foreach (CacheNode::Ptr cacheNode, doomedCacheNodes) {
        qWarning() << "Destroying a filter affected the node associated to" << cacheNode->identifier();
        cacheNodes.removeAll(cacheNode);
        foreach (SocialNetworkModelInterface *model, models) {
            if (matches(cacheNode, model)) {
                model->d_func()->clean();
            }
        }
        setStatus(cacheNode, CacheNodePrivate::Invalid);
        deleteNode(cacheNode);
    }
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
        CacheEntry::Ptr cacheEntry = cache.value(identifier);

        if (cacheEntry->item() == item) {
            if (cacheEntry->data() != item->data()) {
                cacheEntry->setData(item->data());
            }
        }
    }
}

void SocialNetworkInterfacePrivate::modelDestroyedHandler(QObject *object)
{
    SocialNetworkModelInterface *model = static_cast<SocialNetworkModelInterface *>(object);
    removeModel(model);
}

bool SocialNetworkInterfacePrivate::matches(const QString &identifier, int type,
                                            const QSet<FilterInterface *> &filters,
                                            SocialNetworkModelInterface *model)
{
    // Try to be economic on comparisons
    // First compare what is the less costly
    // and use the QSet's O(1) cost to be
    // slight more efficient (?)

    QString aliasedIdentifier = aliases.value(identifier);
    QString modelIdentifier = model->nodeIdentifier();

    if (identifier != modelIdentifier && aliasedIdentifier != modelIdentifier) {
        return false;
    }

    if (type != model->nodeType()) {
        return false;
    }

    QList<FilterInterface *> modelFilters = model->d_func()->filters;
    if (filters.count() != modelFilters.count()) {
        return false;
    }

    foreach (FilterInterface *filter, modelFilters) {
        if (!filters.contains(filter)) {
            return false;
        }
    }

    return true;
}

bool SocialNetworkInterfacePrivate::matches(CacheNode::ConstPtr node,
                                            SocialNetworkModelInterface *model)
{
    return matches(node->identifier(), node->type(), node->filters(), model);
}

SocialNetworkInterface::Status SocialNetworkInterfacePrivate::correspondingStatus(CacheNodePrivate::Status status)
{
    SocialNetworkInterface::Status newStatus;
    switch (status) {
        case CacheNodePrivate::Initializing: {
            newStatus = SocialNetworkInterface::Initializing;
        }
        break;
        case CacheNodePrivate::Idle: {
            newStatus = SocialNetworkInterface::Idle;
        }
        break;
        case CacheNodePrivate::LoadingNodeData:
        case CacheNodePrivate::LoadingRelatedDataReplacing:
        case CacheNodePrivate::LoadingRelatedDataPrepending:
        case CacheNodePrivate::LoadingRelatedDataAppending: {
            newStatus = SocialNetworkInterface::Busy;
        }
        break;
        case CacheNodePrivate::Error: {
            newStatus = SocialNetworkInterface::Error;
        }
        break;
        case CacheNodePrivate::Invalid: {
            newStatus = SocialNetworkInterface::Invalid;
        }
        break;
    }

    return newStatus;
}

/*
    Helper method to handle node creation

    This method is used to get a node from the cache, if the
    node is already cached. If not, it creates a node, connects
    the filters to the destruction handlers, and adds the new
    node into the cache.
*/
CacheNode::Ptr SocialNetworkInterfacePrivate::getOrCreateNode(const QString &identifier,
                                                              int type,
                                                              const QSet<FilterInterface *> &filters)
{
    Q_Q(SocialNetworkInterface);
    CacheNode::Ptr cacheNode = CacheNode::create(identifier, type, filters);
    int index = cacheNodes.indexOf(cacheNode);
    if (index == -1) {
        // Connect to filter destruction handler
        foreach (FilterInterface *filter, filters) {
            QObject::connect(filter, SIGNAL(destroyed(QObject*)),
                             q, SLOT(filterDestroyedHandler(QObject*)));
        }
        cacheNodes.append(cacheNode);
        return cacheNode;
    } else {
        return cacheNodes[index];
    }
}

CacheNode::Ptr SocialNetworkInterfacePrivate::getNode(const QString &identifier, int type,
                                                      const QSet<FilterInterface *> &filters)
{
    CacheNode::Ptr cacheNode = CacheNode::create(identifier, type, filters);
    int index = cacheNodes.indexOf(cacheNode);
    if (index == -1) {
        return CacheNode::Ptr();
    } else {
        return cacheNodes[index];
    }
}

void SocialNetworkInterfacePrivate::checkDoomedNodes()
{
    CacheNode::List currentCacheNodes = cacheNodes;
    CacheNode::List doomedCacheNodes;
    CacheNode::List savedCacheNodes;
    while (!currentCacheNodes.isEmpty()) {
        bool doomed = true;
        CacheNode::Ptr node = currentCacheNodes.takeFirst();
        foreach (SocialNetworkModelInterface *otherModel, models) {
            if (matches(node, otherModel)) {
                doomed = false;
                break;
            }
        }

        if (doomed) {
            doomedCacheNodes.append(node);
        } else {
            savedCacheNodes.append(node);
        }
    }

    cacheNodes = savedCacheNodes;

    // Destroy the doomed nodes
    foreach (CacheNode::Ptr node, doomedCacheNodes) {
        deleteNode(node);
    }
}

void SocialNetworkInterfacePrivate::checkCacheEntryRefcount(CacheEntry::Ptr entry)
{
    if (cache.contains(entry->identifier()) && entry->refcount() == 0) {
        entry->deleteItem();
        cache.remove(entry->identifier());
    }
}

void SocialNetworkInterfacePrivate::deleteNode(CacheNode::Ptr node)
{
    Q_Q(SocialNetworkInterface);
    int index = cacheNodes.indexOf(node);
    if (index == -1) {
        return;
    }

    // Disconnect all the filters
    foreach (FilterInterface *filter, node->filters()) {

        // Be sure that the filter is not used by another node
        bool doomed = true;
        foreach (CacheNode::Ptr otherCacheNode, cacheNodes) {
            if (otherCacheNode != node && otherCacheNode->filters().contains(filter)) {
                doomed = false;
                break;
            }
        }

        if (doomed) {
            filter->disconnect(q);
        }
    }

    if (!node->cacheEntry().isNull()) {
        node->cacheEntry()->deref();
    }

    foreach (CacheEntry::Ptr entry, node->relatedData()) {
        entry->deref();
    }

    if (!node->cacheEntry().isNull()) {
        checkCacheEntryRefcount(node->cacheEntry());
    }

    foreach (CacheEntry::Ptr entry, node->relatedData()) {
        checkCacheEntryRefcount(entry);
    }

    cacheNodes.removeAt(index);
}

//----------------------------------------------------

/*!
    \qmltype SocialNetwork
    \instantiates SocialNetworkInterface
    \inqmlmodule org.nemomobile.social 1
    \brief Provides an abstraction for social network APIs.

    The \c SocialNetwork type should never be used directly by clients.
    Instead, clients should use specific implementations of the \c SocialNetwork
    interface, such as the \l{Facebook} or the \l{Twitter} adapter.

    \c SocialNetwork is used to interface to social networks. It do not provide public
    API but is used by other components, such as \l{SocialNetworkModel} or
    \l{ContentItem} based components. In fact, it is used to cache the data that
    are retrieved from a given social network, and provide them in a formatted
    way to these components.


    Please see the documentation of the \l{Facebook} or \l{Twitter} adapter for an
    example of how clients can use a \c SocialNetwork in an application.

    \sa {The caching system}
    \sa {Facebook}
    \sa {Twitter}
*/

/*!
    \qmlproperty bool SocialNetwork::initialized

    Holds if the \c SocialNetwork is initialized. The initialization process is
    (currently) the process to initialize a QML component.

    A non-initialized \c SocialNetwork will refuse all tasks that are sent
    to it. You should wait for the \c SocialNetwork to be initialized before
    performing other actions.
*/

SocialNetworkInterface::SocialNetworkInterface(QObject *parent)
    : QObject(parent), d_ptr(new SocialNetworkInterfacePrivate(this))
{
    Q_D(SocialNetworkInterface);
    d->networkAccessManager = new QNetworkAccessManager(this);
}

SocialNetworkInterface::SocialNetworkInterface(SocialNetworkInterfacePrivate &dd, QObject *parent)
    : QObject(parent), d_ptr(&dd)
{
    Q_D(SocialNetworkInterface);
    d->networkAccessManager = new QNetworkAccessManager(this);
}

SocialNetworkInterface::~SocialNetworkInterface()
{
}

bool SocialNetworkInterface::isInitialized() const
{
    Q_D(const SocialNetworkInterface);
    return d->initialized;
}


void SocialNetworkInterface::classBegin()
{
    Q_D(SocialNetworkInterface);
    d->initialized = false;
}

void SocialNetworkInterface::componentComplete()
{
    Q_D(SocialNetworkInterface);
    // If you override this implementation, you MUST set d->initialized=true and emit the signal.
    d->initialized = true;
    emit initializedChanged();
}

/*
    \qmlmethod SocialNetwork::arbitraryRequest(SocialNetwork::RequestType requestType, const QString &requestUri, const QVariantMap &queryItems = QVariantMap(), const QString &postData = QString())
*/
/*
    \qmlmethod SocialNetworkInterface::arbitraryRequest()
*/

/*!
    \qmlsignal SocialNetwork::arbitraryRequestResponseReceived(bool isSuccess,
    const QVariantMap &data)

    This signal is emitted when an arbitrary request handler is performed. It carries
    the status of the request via \a isSuccess, and the data that is retrieved via
    \a data.


    The corresponding handler is \c onArbitraryRequestResponseReceived.
*/

/*!
    \qmlmethod bool SocialNetwork::arbitraryRequest(RequestType requestType, const QString &requestUri,
                                      const QVariantMap &queryItems = QVariantMap(),
                                      const QString &postData = QString())

    Performs the HTTP request of the specified \a requestType with the specified \a requestUri
    and \a queryItems.  If the request is a Post request, the given \a postData will be converted
    to a QByteArray via \c{QByteArray::fromBase64(postData.toLatin1())} and used as the HTTP POST
    data.

    \a requestType can be one of:
    \list
    \li SocialNetwork.Get
    \li SocialNetwork.Post
    \li SocialNetwork.Delete
    \endlist

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
    // TODO: migrate to private class
    // Helper function for SocialNetworkInterface-derived types
    contentItem->setDataPrivate(data);
}

#include "moc_socialnetworkinterface.cpp"
