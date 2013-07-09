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

#ifndef SOCIALNETWORKINTERFACE_P_H
#define SOCIALNETWORKINTERFACE_P_H

#include "socialnetworkinterface.h"

#include "filterinterface.h"
#include "sorterinterface.h"

#include <QtCore/QExplicitlySharedDataPointer>
#include <QtCore/QVariantMap>
#include <QtCore/QList>
#include <QtCore/QStack>
#include <QtCore/QSet>
#include <QtCore/QString>
#include <QtCore/QtGlobal>

#include <QtNetwork/QSslError>
#include <QtNetwork/QNetworkReply>

class QNetworkAccessManager;
class IdentifiableContentItemInterface;
class FilterInterface;
class SorterInterface;

class SocialNetworkInterfacePrivate;

class CacheEntry;
struct CacheEntryPrivate: public QSharedData
{
    explicit CacheEntryPrivate();
    virtual ~CacheEntryPrivate();
    // The data received from SNI
    QVariantMap data;
    // If the item is identifiable, store the identifier
    QString identifier;
    // The initialized item (lazily constructed)
    ContentItemInterface *item;
    // Ref when used (in a model or in a node)
    // If ref == 0 when cleaning, it is removed
    // from cache
    int refCount;
};

class CacheEntry
{
public:
    explicit CacheEntry();
    explicit CacheEntry(const QVariantMap &data, ContentItemInterface *item = 0);
    explicit CacheEntry(const QVariantMap &data, const QString &identifier,
                        ContentItemInterface *item = 0);
    CacheEntry(const CacheEntry &other);
    virtual ~CacheEntry();
    bool operator==(const CacheEntry &other) const;
    bool operator!=(const CacheEntry &other) const;
    bool isNull() const;
    int refcount() const;
    void ref();
    void deref();
    QString identifier() const;
    QVariantMap data() const;
    void setData(const QVariantMap &data);
    const ContentItemInterface * item() const;
    ContentItemInterface * item();
    void setItem(ContentItemInterface *item);
    const IdentifiableContentItemInterface * identifiableItem() const;
    IdentifiableContentItemInterface * identifiableItem();
    void deleteItem();
protected:
    QExplicitlySharedDataPointer<CacheEntryPrivate> d_ptr;
private:
    Q_DECLARE_PRIVATE(CacheEntry)
};

struct NodePrivate: public QSharedData
{
    enum Status {
        Initializing,
        Idle,
        LoadingNodeData,
        LoadingRelatedDataReplacing,
        LoadingRelatedDataPrepending,
        LoadingRelatedDataAppending,
        Error,
        Invalid
    };
    explicit NodePrivate();
    virtual ~NodePrivate();
    // The identifier associated to the node
    // It might be different from the one associated to the cache entry
    QString identifier;
    // The filters associated to the node
    QSet<FilterInterface *> filters;
    // A cache entry that describes the node
    CacheEntry cacheEntry;
    // A list of cache entries, that represents the data displayed in the model
    // for this node
    QList<CacheEntry> relatedData;
    bool hasPrevious;
    bool hasNext;
    // A set of extra informations, used to store, for example, cursors or
    // indexes for next and previous
    QVariantMap extraInfo;
    Status status;

    // Errors for the models
    SocialNetworkInterface::ErrorType error;
    QString errorMessage;
};

class Node
{
public:
    explicit Node();
    explicit Node(const QString &identifier, const QSet<FilterInterface *> &filters);
    Node(const Node &other);
    virtual ~Node();
    bool operator==(const Node &other) const;
    bool operator!=(const Node &other) const;
    bool isNull() const;
    QString identifier() const;
    QSet<FilterInterface *> filters() const;
    CacheEntry cacheEntry() const;
    void setCacheEntry(const CacheEntry &cacheEntry);
    QList<CacheEntry> relatedData() const;
    void setRelatedData(const QList<CacheEntry> &relatedData);
    void setFilters(const QSet<FilterInterface *> &filters);
    bool hasPrevious() const;
    bool hasNext() const;
    void setHavePreviousAndNext(bool hasPrevious, bool hasNext);
    QVariantMap extraInfo() const;
    void setExtraInfo(const QVariantMap &extraInfo);
    NodePrivate::Status status() const;
    void setStatus(NodePrivate::Status status);
    SocialNetworkInterface::ErrorType error() const;
    QString errorMessage() const;
    void setError(SocialNetworkInterface::ErrorType error, const QString &errorMessage);

protected:
    QExplicitlySharedDataPointer<NodePrivate> d_ptr;
private:
    Q_DECLARE_PRIVATE(Node)
    friend class SocialNetworkInterfacePrivate;
};

class ArbitraryRequestHandler : public QObject
{
    Q_OBJECT
public:
    explicit ArbitraryRequestHandler(QNetworkAccessManager *network,
                                     SocialNetworkInterface *parent);
    virtual ~ArbitraryRequestHandler();
    bool request(SocialNetworkInterface::RequestType requestType, const QString &requestUri,
                 const QVariantMap &queryItems = QVariantMap(),
                 const QString &postData = QString());

    QNetworkAccessManager *networkAccessManager;
    QNetworkReply *reply;
    QString errorMessage;
    bool isError;
signals:
    void arbitraryRequestResponseReceived(bool isError, const QVariantMap &data);

public Q_SLOTS:
    void finishedHandler();
    void errorHandler(QNetworkReply::NetworkError networkError);
    void sslErrorsHandler(const QList<QSslError> &sslErrors);
};

class SorterFunctor
{
public:
    explicit SorterFunctor(SorterInterface *sorter);
    bool operator()(const CacheEntry &first, const CacheEntry &second) const;
private:
    SorterInterface *m_sorter;
};

class SocialNetworkInterfacePrivate
{
public:
    explicit SocialNetworkInterfacePrivate(SocialNetworkInterface *q);
    virtual ~SocialNetworkInterfacePrivate();

protected:
    // Functions to be reimplemented
    virtual void populateDataForNode(Node &node);
    virtual void populateRelatedDataforNode(Node &node);
    virtual bool validateCacheEntryForNode(const CacheEntry &cacheEntry);
    virtual QString dataSection(int type, const QVariantMap &data) const;
    virtual ContentItemInterface * contentItemFromData(const QVariantMap &data,
                                                       QObject *parent = 0) const;
    virtual QNetworkReply * getRequest(const QString &objectIdentifier, const QString &extraPath,
                                       const QStringList &whichFields, const QVariantMap &extraData);
    virtual QNetworkReply * postRequest(const QString &objectIdentifier, const QString &extraPath,
                                        const QVariantMap &data, const QVariantMap &extraData);
    virtual QNetworkReply * deleteRequest(const QString &objectIdentifier, const QString &extraPath,
                                          const QVariantMap &extraData);
    friend class IdentifiableContentItemInterfacePrivate;

    // Handlers, implement if needed
    virtual void handleFinished(Node &node, QNetworkReply *reply);
    virtual void handleError(Node &node, QNetworkReply *reply,
                             QNetworkReply::NetworkError networkError);
    virtual void handleSslError(Node &node, QNetworkReply *reply,
                                const QList<QSslError> &sslErrors);

    // Helper functions
    void setReply(const Node &node, QNetworkReply *reply);
    void setStatus(Node &node, NodePrivate::Status status);
    void setError(Node &node, SocialNetworkInterface::ErrorType error,
                  const QString &errorMessage);
    void deleteReply(QNetworkReply *reply);
    void updateModelNode(Node &node);
    void updateModelRelatedData(Node &node, const QList<CacheEntry> &relatedData);
    CacheEntry createCacheEntry(const QVariantMap &data, const QString &nodeIdentifier = QString());
private:
    // Used by NSMI
    void populate(SocialNetworkModelInterface *model, const QString &identifier,
                  const QList<FilterInterface *> &filters, bool reload = false);
    void addModel(SocialNetworkModelInterface *model);
    void removeModel(SocialNetworkModelInterface *model);
    void loadNext(SocialNetworkModelInterface *model);
    void loadPrevious(SocialNetworkModelInterface *model);
    ContentItemInterface * createItem(const CacheEntry &cacheEntry);
    friend class SocialNetworkModelInterface;

    // Slots
    void finishedHandler();
    void errorHandler(QNetworkReply::NetworkError networkError);
    void sslErrorsHandler(const QList<QSslError> &sslErrors);
    void filterDestroyedHandler(QObject *object);
    void itemDataChangedHandler();
    void modelDestroyedHandler(QObject *object);


    // Implementation details
    inline static bool matches(const Node &node, SocialNetworkModelInterface *model);
    inline static SocialNetworkInterface::Status correspondingStatus(NodePrivate::Status status);
    Node getOrCreateNode(const QString &identifier, const QSet<FilterInterface *> &filters);
    Node getNode(const QString &identifier, const QSet<FilterInterface *> &filters);
    void checkDoomedNodes();
    void checkCacheEntryRefcount(const CacheEntry &entry);
    void deleteNode(const Node &node);

protected:
    QNetworkAccessManager *networkAccessManager;
    SocialNetworkInterface * const q_ptr;
private:
    // Attributes
    bool initialized;
    QHash<QString, CacheEntry> cache;
    QList<Node> nodes;
    QList<SocialNetworkModelInterface *> models;
    QMap<QNetworkReply *, Node> replyToNodeMap;

    // Arbitrary request handler
    ArbitraryRequestHandler *arbitraryRequestHandler;

    // Testing functions
    // TODO XXX: remove them !
//#ifdef ENABLE_TESTS
//    inline QStack<Node> publicNodeStack() const
//    {
//        return nodeStack;
//    }
//    inline QHash<QString, CacheEntry> publicCache() const
//    {
//        return cache;
//    }
//    inline void publicSetFilters(const QList<FilterInterface *> &newFilters)
//    {
//        filters = newFilters;
//    }
//    inline QList<FilterInterface *> publicFilters() const
//    {
//        return filters;
//    }
//    inline void publicDeleteLastNode()
//    {
//        deleteLastNode();
//    }

//#endif
private:
    Q_DECLARE_PUBLIC(SocialNetworkInterface)
};

//Q_DECLARE_OPERATORS_FOR_FLAGS(SocialNetworkInterfacePrivate::RelatedDataPagingFlags)

#endif // SOCIALNETWORKINTERFACE_P_H
