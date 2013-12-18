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

#include <QtCore/QVariantMap>
#include <QtCore/QList>
#include <QtCore/QStack>
#include <QtCore/QSet>
#include <QtCore/QString>
#include <QtCore/QtGlobal>

#include <QtNetwork/QSslError>
#include <QtNetwork/QNetworkReply>

//#define NODE_EXTRA_PAGING_PREVIOUS_KEY QLatin1String("previous")
//#define NODE_EXTRA_PAGING_NEXT_KEY QLatin1String("next")

class QNetworkAccessManager;
//class IdentifiableContentItemInterface;
//class FilterInterface;
//class SorterInterface;

//class SocialNetworkInterfacePrivate;

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
signals:
    void arbitraryRequestResponseReceived(bool success, const QVariantMap &data);

public Q_SLOTS:
    void finishedHandler();
    void errorHandler(QNetworkReply::NetworkError networkError);
    void sslErrorsHandler(const QList<QSslError> &sslErrors);
};

//class SorterFunctor
//{
//public:
//    explicit SorterFunctor(SorterInterface *sorter);
//    bool operator()(CacheEntry::ConstPtr first, CacheEntry::ConstPtr second) const;
//private:
//    SorterInterface *m_sorter;
//};

class SocialNetworkInterfacePrivate
{
public:
    explicit SocialNetworkInterfacePrivate(SocialNetworkInterface *q);
    virtual ~SocialNetworkInterfacePrivate();
    void runReply(QNetworkReply *reply, FilterInterface *filter);
    QNetworkAccessManager *networkAccessManager;
protected:
    // Functions to be reimplemented
//    virtual void populateDataForNode(Node::Ptr node);
//    virtual void populateRelatedDataforNode(Node::Ptr node);
//    virtual bool validateCacheEntryForNode(CacheEntry::ConstPtr cacheEntry);
//    virtual QString dataSection(int type, const QVariantMap &data) const;
//    virtual ContentItemInterface * contentItemFromData(const QVariantMap &data,
//                                                       QObject *parent = 0) const;
//    virtual QNetworkReply * getRequest(const QString &objectIdentifier, const QString &extraPath,
//                                       const QStringList &whichFields, const QVariantMap &extraData);
//    virtual QNetworkReply * postRequest(const QString &objectIdentifier, const QString &extraPath,
//                                        const QVariantMap &data, const QVariantMap &extraData);
//    virtual QNetworkReply * deleteRequest(const QString &objectIdentifier, const QString &extraPath,
//                                          const QVariantMap &extraData);
//    friend class IdentifiableContentItemInterfacePrivate;

    // Function to guess the type, implement if needed
//    virtual int guessType(const QString &identifier, int type,
//                          const QSet<FilterInterface *> &filters);

    // Handlers, implement if needed
//    virtual void handleFinished(Node::Ptr node, QNetworkReply *reply);
//    virtual void handleError(Node::Ptr node, QNetworkReply *reply,
//                             QNetworkReply::NetworkError networkError);
//    virtual void handleSslError(Node::Ptr node, QNetworkReply *reply,
//                                const QList<QSslError> &sslErrors);

    // Helper functions
//    void setReply(Node::Ptr node, QNetworkReply *reply);
//    void setStatus(Node::Ptr node, NodePrivate::Status status);
//    void setError(Node::Ptr node, SocialNetworkInterface::ErrorType error,
//                  const QString &errorMessage);
//    void deleteReply(QNetworkReply *reply);
//    void updateModelNode(Node::Ptr node);
//    void updateModelRelatedData(Node::Ptr node, const CacheEntry::List &relatedData);
//    void updateModelHavePreviousAndNext(Node::Ptr node, bool havePrevious, bool haveNext);
//    CacheEntry::Ptr createCacheEntry(const QVariantMap &data,
//                                     const QString &nodeIdentifier = QString());
//    static void setNodeExtraPaging(QVariantMap &nodeExtra,
//                                   const QVariantMap &previousExtra, const QVariantMap &nextExtra,
//                                   NodePrivate::Status insertionMode);

    // Aliases map
//    QMap<QString, QString> aliases;
//private:
    // Used by NSMI
//    void populate(SocialNetworkModelInterface *model, const QString &identifier, int type,
//                  const QList<FilterInterface *> &filters, bool reload = false);
//    void addModel(SocialNetworkModelInterface *model);
//    void removeModel(SocialNetworkModelInterface *model);
//    void loadNext(SocialNetworkModelInterface *model);
//    void loadPrevious(SocialNetworkModelInterface *model);
//    ContentItemInterface * createItem(CacheEntry::Ptr cacheEntry);
//    friend class SocialNetworkModelInterface;

    // Slots
    void filterDestroyedHandler(QObject *object);
    void finishedHandler();
//    void finishedHandler();
//    void errorHandler(QNetworkReply::NetworkError networkError);
//    void sslErrorsHandler(const QList<QSslError> &sslErrors);
//    void filterDestroyedHandler(QObject *object);
//    void itemDataChangedHandler();
//    void modelDestroyedHandler(QObject *object);


    // Implementation details
//    inline bool matches(const QString &identifier, int type, const QSet<FilterInterface *> &filters,
//                        SocialNetworkModelInterface *model);
//    inline bool matches(Node::ConstPtr node, SocialNetworkModelInterface *model);
//    inline static SocialNetworkInterface::Status correspondingStatus(NodePrivate::Status status);
//    Node::Ptr getOrCreateNode(const QString &identifier, int type,
//                              const QSet<FilterInterface *> &filters);
//    Node::Ptr getNode(const QString &identifier, int type, const QSet<FilterInterface *> &filters);
//    void checkDoomedNodes();
//    void checkCacheEntryRefcount(CacheEntry::Ptr entry);
//    void deleteNode(Node::Ptr node);

protected:
    virtual QByteArray preprocessData(const QByteArray &data);
    SocialNetworkInterface * const q_ptr;
private:
    // Attributes
    bool initialized;
//    QHash<QString, CacheEntry::Ptr> cache;
//    Node::List nodes;
//    QList<SocialNetworkModelInterface *> models;
//    QMap<QNetworkReply *, Node::Ptr> replyToNodeMap;
    QMap<QNetworkReply *, FilterInterface *> replyToFilterMap;
    QMultiMap<QObject *,QNetworkReply *> filterToReplyMap;

    // Arbitrary request handler
    ArbitraryRequestHandler *arbitraryRequestHandler;
private:
    Q_DECLARE_PUBLIC(SocialNetworkInterface)
};

#endif // SOCIALNETWORKINTERFACE_P_H
