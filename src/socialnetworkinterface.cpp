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
//#include "socialnetworkmodelinterface.h"
//#include "socialnetworkmodelinterface_p.h"

//#include "contentiteminterface.h"
#include "contentiteminterface_p.h"
#include "identifiablecontentiteminterface.h"

#include "util_p.h"

#include <QtCore/QByteArray>
#include <QtCore/QCoreApplication>
#include <QtCore/QEvent>
#include <QtCore/QUrl>
#include <QtCore/QStringList>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkRequest>

#include <QtDebug>

#include <algorithm>

// TODO: manage stuff like filters being destroyed and all

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


//SorterFunctor::SorterFunctor(SorterInterface *sorter):
//    m_sorter(sorter)
//{
//}

//bool SorterFunctor::operator()(CacheEntry::ConstPtr first, CacheEntry::ConstPtr second) const
//{
//    return m_sorter->firstLessThanSecond(first->data(), second->data());
//}

SocialNetworkInterfacePrivate::SocialNetworkInterfacePrivate(SocialNetworkInterface *q)
    : networkAccessManager(0)
    , q_ptr(q)
    , initialized(false)
    , arbitraryRequestHandler(0)
{
}

SocialNetworkInterfacePrivate::~SocialNetworkInterfacePrivate()
{
//    // We should say to all models that we are being destroyed
//    foreach (SocialNetworkModelInterface *model, models) {
//        model->d_func()->setNode(0);
//        model->d_func()->modelData.clear();
//    }

//    // Remove all nodes
//    foreach (Node::Ptr node, nodes) {
//        deleteNode(node);
//    }

//    // Remove all cache entries.
//    cache.clear();

//    // Remove all replies
//    foreach (QNetworkReply *reply, replyToNodeMap.keys()) {
//        reply->deleteLater();
//    }
//    replyToNodeMap.clear();
}

void SocialNetworkInterfacePrivate::runReply(QNetworkReply *reply, FilterInterface *filter)
{
    Q_Q(SocialNetworkInterface);
    replyToFilterMap.insert(reply, filter);
    filterToReplyMap.insert(filter, reply);
    QObject::connect(reply, SIGNAL(finished()), q, SLOT(finishedHandler()));
    QObject::connect(filter, SIGNAL(destroyed(QObject*)),
                     q, SLOT(filterDestroyedHandler(QObject*)));
}

bool SocialNetworkInterfacePrivate::runAction(QNetworkReply *reply,
                                              IdentifiableContentItemInterface *item,
                                              const QVariantMap &properties)
{
    Q_Q(SocialNetworkInterface);
    if (!reply) {
        return false;
    }


    replyToItemMap.insert(reply, item);
    replyToActionPropertiesMap.insert(reply, properties);
    itemToReplyMap.insert(item, reply);
    QObject::connect(reply, SIGNAL(finished()), q, SLOT(actionFinishedHandler()));
    QObject::connect(item, SIGNAL(destroyed(QObject*)),
                     q, SLOT(itemDestroyedHandler(QObject*)));
    return true;
}

void SocialNetworkInterfacePrivate::filterDestroyedHandler(QObject *object)
{
    foreach (QNetworkReply *reply, filterToReplyMap.values(object)) {
        replyToFilterMap.remove(reply);
        reply->deleteLater();
    }

    filterToReplyMap.remove(object);
}

void SocialNetworkInterfacePrivate::itemDestroyedHandler(QObject *object)
{
    QNetworkReply *reply = itemToReplyMap.value(object);
    replyToItemMap.remove(reply);
    replyToActionPropertiesMap.remove(reply);
    reply->deleteLater();
    itemToReplyMap.remove(object);
}

void SocialNetworkInterfacePrivate::finishedHandler()
{
    Q_Q(SocialNetworkInterface);
    QNetworkReply *reply = qobject_cast<QNetworkReply *>(q->sender());
    if (!reply) {
        return;
    }

    if (!replyToFilterMap.contains(reply)) {
        qWarning() << "Reply received but no filter associated";
        return;
    }

    FilterInterface *filter = replyToFilterMap.value(reply);
    QByteArray data = preprocessData(reply->readAll());
    if (reply->error() != QNetworkReply::NoError) {
        QString errorMessage = networkErrorString(reply->error());
        if (!data.isEmpty()) {
            errorMessage.append(QString(" Downloaded data: %1").arg(QString(data)));
        }
        filter->performSetError(reply, SocialNetworkInterface::RequestError, errorMessage);
    } else {
        filter->performSetData(reply, data);
    }

    replyToFilterMap.remove(reply);
    filterToReplyMap.remove(filter, reply);
    reply->deleteLater();
}

void SocialNetworkInterfacePrivate::actionFinishedHandler()
{
    Q_Q(SocialNetworkInterface);
    QNetworkReply *reply = qobject_cast<QNetworkReply *>(q->sender());
    if (!reply) {
        return;
    }

    if (!replyToItemMap.contains(reply) || !replyToActionPropertiesMap.contains(reply)) {
        qWarning() << "Reply received but no item or property associated";
        return;
    }

    IdentifiableContentItemInterface *item = replyToItemMap.value(reply);
    QVariantMap properties = replyToActionPropertiesMap.value(reply);

    QByteArray data = reply->readAll();
    if (reply->error() != QNetworkReply::NoError) {
        QString errorMessage = networkErrorString(reply->error());
        if (!data.isEmpty()) {
            errorMessage.append(QString(" Downloaded data: %1").arg(QString(data)));
        }
        item->setActionError(SocialNetworkInterface::RequestError, errorMessage);
    } else {
        performAction(item, properties);
    }

    replyToItemMap.remove(reply);
    replyToActionPropertiesMap.remove(reply);
    itemToReplyMap.remove(item);
    reply->deleteLater();
}

QByteArray SocialNetworkInterfacePrivate::preprocessData(const QByteArray &data)
{
    return data;
}

void SocialNetworkInterfacePrivate::performAction(IdentifiableContentItemInterface *item,
                                                  const QVariantMap &properties)
{
    // This function is not implemented, and will leave the
    // IdentifiableContentItemInterface in a Busy state. Be sure to call
    // IdentifiableContentItemInterface::setActionComplete or
    // IdentifiableContentItemInterface::setError to set the loading status
    Q_UNUSED(item)
    Q_UNUSED(properties)
}

//----------------------------------------------------

///*!
//    \qmltype SocialNetwork
//    \instantiates SocialNetworkInterface
//    \inqmlmodule org.nemomobile.social 1
//    \brief Provides an abstraction API for graph- or model-based social
//    network APIs.

//    The SocialNetwork type should never be used directly by clients.
//    Instead, clients should use specific implementations of the SocialNetwork
//    interface, such as the Facebook adapter.

//    The SocialNetwork type provides a generic API which allows content
//    to be retrieved from a social network and exposed via a model.
//    The API consists of a central \c node which is an IdentifiableContentItem,
//    which may be specified by the client via the \c nodeIdentifier property.
//    The data in the model will be populated from the graph connections of
//    the node.

//    The model roles are as follows:
//    \list
//    \li contentItem - the instantiated ContentItem related to the node
//    \li contentItemType - the type of the ContentItem related to the node
//    \li contentItemData - the underlying QVariantMap data of the ContentItem related to the node
//    \li contentItemIdentifier - the identifier of the ContentItem related to the node, or an empty string
//    \endlist

//    Please see the documentation of the Facebook adapter for an example
//    of how clients can use the SocialNetwork model in an application.
//*/

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

#include "moc_socialnetworkinterface.cpp"
