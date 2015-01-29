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

#include "contentiteminterface.h"
#include "contentiteminterface_p.h"

#include "identifiablecontentiteminterface.h"
#include "socialnetworkinterface.h"

#include <QtDebug>
#if (QT_VERSION < QT_VERSION_CHECK(5, 0, 0))
#include <qjson/parser.h>
#else
#include <QJsonDocument>
#endif

ContentItemInterfacePrivate::ContentItemInterfacePrivate(ContentItemInterface *q)
    : socialNetworkInterface(0), isInitialized(false), q_ptr(q)
{
}

ContentItemInterfacePrivate::~ContentItemInterfacePrivate()
{
}

QVariantMap ContentItemInterfacePrivate::data() const
{
    return m_data;
}

void ContentItemInterfacePrivate::setData(const QVariantMap &data)
{
    Q_Q(ContentItemInterface);
    if (m_data != data) {
        QVariantMap oldData = m_data;
        m_data = data;
        emitPropertyChangeSignals(oldData, data);
        emit q->dataChanged();
    }
}

/*
    Specific implementations of the ContentItem interface MUST implement this
    function.  It must be implemented so that the delta between the \c oldData
    and \c newData is examined, and appropriate property change signals emitted.
    All ContentItem derived types must then call the \c emitPropertyChangeSignals()
    function of its immediate superclass.

    If the derived type is an IdentifiableContentItem derived type, it MUST
    fill the NEMOQMLPLUGINS_SOCIAL_CONTENTITEMID (defined in identifiablecontentiteminterface.h)
    field of both the oldData and newData maps with the social-network-and-type-specific
    identifier for that content item PRIOR to calling the emitPropertyChangeSignals()
    function of its super class.
*/
void ContentItemInterfacePrivate::emitPropertyChangeSignals(const QVariantMap &, const QVariantMap &)
{
    // Default implementation does nothing.
    // All derived-types must call Super::emitPropertyChangeSignals() in their override.
}

/*
    Specific implementations of the ContentItem interface SHOULD implement this
    function.  If implemented, it should be used to trigger requests or actions
    which require the social network to be initialized and all properties of the
    content item to be initialized, prior to activation.  The implementation MUST
    then call the \c initializationComplete() function of its immediate superclass.
*/
void ContentItemInterfacePrivate::initializationComplete()
{
    // Default implementation does nothing.
    // All derived-types must call Super::initializationComplete() in their override.
}

QVariant ContentItemInterfacePrivate::parseReplyDataVariant(const QByteArray &replyData, bool *ok)
{
    QVariant parsed;

#if (QT_VERSION < QT_VERSION_CHECK(5, 0, 0))
    QJson::Parser jsonParser;
    parsed = jsonParser.parse(replyData, ok);
    if (!*ok) {
        qWarning() << Q_FUNC_INFO << "Error parsing JSON file:" << jsonParser.errorString()
                   << "at" << jsonParser.errorLine();
    }
#else
    QJsonParseError jsonError;
    QJsonDocument jsonDocument = QJsonDocument::fromJson(replyData, &jsonError);
    *ok = !jsonDocument.isEmpty();
    if (!*ok) {
        qWarning() << Q_FUNC_INFO << "Error parsing JSON file:" << jsonError.errorString()
                   << "at" << jsonError.offset;
    }
    parsed = jsonDocument.toVariant();
#endif

    if (!*ok) {
        parsed.clear();
    }

    return parsed;
}

QVariantMap ContentItemInterfacePrivate::parseReplyData(const QByteArray &replyData, bool *ok)
{
    QVariant data = parseReplyDataVariant(replyData, ok);
    if (!(*ok)) {
        return QVariantMap();
    }

    if (data.type() != QVariant::Map) {
        *ok = false;
        return QVariantMap();
    }

    return data.toMap();
}

void ContentItemInterfacePrivate::socialNetworkInitializedChangedHandler()
{
    Q_Q(ContentItemInterface);
    if (socialNetworkInterface && socialNetworkInterface->isInitialized()) {
        q->disconnect(socialNetworkInterface);
        isInitialized = true;
        initializationComplete();
    }
}


/*!
    \qmltype ContentItem
    \instantiates ContentItemInterface
    \inqmlmodule org.nemomobile.social 1
    \brief A ContentItem represents a data object in a social network graph

    All entities in a social network graph are represented by the ContentItem
    component.  A particular ContentItem may be either identifiable (that is,
    have a unique identifier in the social network) or non-identifiable (that
    is, be anonymous in the social network). An identifiable content item may
    be used as the \c node in a SocialNetworkModel, but a non-identifiable
    content item may not.

    Every ContentItem exposes some data retrieved from the social network.
    Each specific implementation of the SocialNetwork interface should also
    provide specific implementations derived from the ContentItem interface
    which provide convenience API for accessing properties or performing
    operations on particular content item types.

    A ContentItem is usually constructed via a request done using a
    SocialNetworkModel. Its subclasses, like IdentifiableContentItem
    can also be constructed directly using a SocialNetwork that is
    provided to \l socialNetwork, and setting more properties.
*/

ContentItemInterface::ContentItemInterface(QObject *parent)
    : QObject(parent), d_ptr(new ContentItemInterfacePrivate(this))
{
}

ContentItemInterface::ContentItemInterface(ContentItemInterfacePrivate &dd, QObject *parent)
    : QObject(parent), d_ptr(&dd)
{
}

ContentItemInterface::~ContentItemInterface()
{
}

void ContentItemInterface::classBegin()
{
}

void ContentItemInterface::componentComplete()
{
    Q_D(ContentItemInterface);
    if (d->socialNetworkInterface && d->socialNetworkInterface->isInitialized()) {
        d->isInitialized = true;
        d->initializationComplete();
    }
}

/*!
    \qmlproperty SocialNetwork ContentItem::socialNetwork

    This property must be defined by the client if they wish to construct
    a ContentItem directly. It holds the SocialNetwork component to use to
    create this ContentItem.

    This property should be set before setting identifier when
    creating a ContentItem directly.
*/
SocialNetworkInterface *ContentItemInterface::socialNetwork() const
{
    Q_D(const ContentItemInterface);
    return d->socialNetworkInterface;
}

void ContentItemInterface::setSocialNetwork(SocialNetworkInterface *socialNetwork)
{
    Q_D(ContentItemInterface);
    if (d->isInitialized) {
        qWarning() << Q_FUNC_INFO
                   << "Can't change social network after content item has been initialized!";
        return;
    }

    if (d->socialNetworkInterface != socialNetwork) {
        if (d->socialNetworkInterface)
            disconnect(d->socialNetworkInterface);
        if (socialNetwork && !socialNetwork->isInitialized()) {
            connect(socialNetwork, SIGNAL(initializedChanged()),
                    this, SLOT(socialNetworkInitializedChangedHandler()));
        }
        d->socialNetworkInterface = socialNetwork;
        emit socialNetworkChanged();
    }
}

/*!
    \qmlproperty enumeration ContentItem::type

    Holds the type of the content item.  Every implementation of
    the SocialNetwork interface will define its own specific
    types.
*/
int ContentItemInterface::type() const
{
    return SocialNetworkInterface::NotInitialized;
}

/*!
    \qmlproperty QVariantMap ContentItem::data

    Holds the underlying data exposed by the ContentItem.
    Note that this data may be formatted differently to how
    it is exposed in the ContentItem-derived-type's API, as it
    should consist of the data as it was received from the
    social network.
*/
QVariantMap ContentItemInterface::data() const
{
    Q_D(const ContentItemInterface);
    return d->data();
}

/*!
    \qmlproperty bool ContentItem::identifiable() const

    Returns true if the content item has a unique identifier
    in the social network from which it was retrieved, otherwise
    returns false.
*/
bool ContentItemInterface::isIdentifiable() const
{
    return false;
}

/*!
    \qmlmethod IdentifiableContentItem ContentItem::asIdentifiable()

    Returns the ContentItem as an IdentifiableContentItem if it is
    identifiable, otherwise returns null.
*/
IdentifiableContentItemInterface * ContentItemInterface::asIdentifiable()
{
    return qobject_cast<IdentifiableContentItemInterface*>(this);
}

void ContentItemInterface::setDataPrivate(const QVariantMap &data)
{
    Q_D(ContentItemInterface);
    d->setData(data);
}

QVariantMap ContentItemInterface::dataPrivate() const
{
    Q_D(const ContentItemInterface);
    return d->data();
}

bool ContentItemInterface::isInitialized() const
{
    Q_D(const ContentItemInterface);
    return d->isInitialized;
}

#include "moc_contentiteminterface.cpp"
