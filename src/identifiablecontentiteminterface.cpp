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

#include "identifiablecontentiteminterface.h"
#include "identifiablecontentiteminterface_p.h"

#include "socialnetworkinterface.h"
#include "socialnetworkinterface_p.h"
#include "util_p.h"

#include <QtDebug>

IdentifiableContentItemInterfacePrivate
    ::IdentifiableContentItemInterfacePrivate(IdentifiableContentItemInterface *q)
    : ContentItemInterfacePrivate(q)
    , status(SocialNetworkInterface::Initializing)
    , error(SocialNetworkInterface::NoError)
    , actionStatus(SocialNetworkInterface::Idle)
    , actionError(SocialNetworkInterface::NoError)
    , filter(0)
{
}

IdentifiableContentItemInterfacePrivate::~IdentifiableContentItemInterfacePrivate()
{
}

/*! \reimp */
void IdentifiableContentItemInterfacePrivate::emitPropertyChangeSignals(const QVariantMap &oldData,
                                                                        const QVariantMap &newData)
{
    Q_Q(IdentifiableContentItemInterface);
    // most derived types will do:
    // {
    //     foreach (key, propKeys) {
    //         if (newData.value(key) != oldData.value(key)) {
    //             emit thatPropertyChanged();
    //         }
    //     }
    //     SuperClass::emitPropertyChangeSignals(oldData, newData);
    // }

    // check identifier - NOTE: derived types MUST fill out this field before calling this
    // class' implementation of emitPropertyChangeSignals.
    QString oldId = oldData.value(NEMOQMLPLUGINS_SOCIAL_CONTENTITEMID).toString();
    QString newId = newData.value(NEMOQMLPLUGINS_SOCIAL_CONTENTITEMID).toString();
    if (newId.isEmpty() && oldId.isEmpty()) {
        // this will fall through to being reported as an error due to identifier change
        // (to empty) below.
        qWarning() << Q_FUNC_INFO
                   << "ERROR: derived types MUST set the NEMOQMLPLUGINS_SOCIAL_CONTENTITEMID field appropriately prior to calling the superclass emitPropertyChangeSignals() function!";
    }

    // Might have been set directly by client via
    // IdentifiableContentItemInterface::setIdentifier() which sets d->identifier.
    if (oldId.isEmpty())
        oldId = identifier;

    if (oldId.isEmpty() && !newId.isEmpty()) {
        // This must be a new object created by the model.
        // We now have an identifier; set it and update.
        identifier = newId;
        emit q->identifierChanged();
    }

    // finally, as all derived classes must do, call super class implementation.
    ContentItemInterfacePrivate::emitPropertyChangeSignals(oldData, newData);

    if (status != SocialNetworkInterface::Idle) {
        status = SocialNetworkInterface::Idle;
        emit q->statusChanged();
    }
}

void IdentifiableContentItemInterfacePrivate::initializationIncomplete()
{
    Q_Q(IdentifiableContentItemInterface);
    status = SocialNetworkInterface::Initializing;
    emit q->statusChanged();

    ContentItemInterfacePrivate::initializationIncomplete();
}

/*! \reimp */
void IdentifiableContentItemInterfacePrivate::initializationComplete()
{
    Q_Q(IdentifiableContentItemInterface);
    // reload content if required.
//    if (needsLoad) {
//        needsLoad = false;
//        status = SocialNetworkInterface::Idle; // but DON'T emit, otherwise reload() will fail.
//        q->load(); // XXX TODO: allow specifying whichFields for first time initialization reload()?
//    } else {
        status = SocialNetworkInterface::Idle;
        emit q->statusChanged();
//    }

    // Finally, as all derived classes must do, call super class implementation.
    ContentItemInterfacePrivate::initializationComplete();
}

void IdentifiableContentItemInterfacePrivate::socialNetworkDestroyedHandler()
{
    Q_Q(IdentifiableContentItemInterface);
    if (actionStatus != SocialNetworkInterface::Idle) {
        q->setActionError(SocialNetworkInterface::InternalError,
                          "SocialNetwork is destroyed during request");
    }
    ContentItemInterfacePrivate::socialNetworkDestroyedHandler();
}

void IdentifiableContentItemInterfacePrivate::filterDestroyedHandler()
{
    Q_Q(IdentifiableContentItemInterface);
    q->setError(SocialNetworkInterface::InternalError, "Filter is destroyed during request");
    q->setFilter(0);
}

//-------------------------------------------------

/*!
    \qmltype IdentifiableContentItem
    \instantiates IdentifiableContentItemInterface
    \inqmlmodule org.nemomobile.social 1
    \brief An IdentifiableContentItem represents an identifiable data object in a social network graph

    A data object which is identifiable is represented by an
    IdentifiableContentItem.  Instances of this sort of ContentItem
    can be used as the \c node (or central content item) in a
    SocialNetwork model.

    An IdentifiableContentItem may also have more operations
    performed on it than a non-identifiable content item.
    As these operations result in network communication, the
    IdentifiableContentItem type has \c status and \c error
    properties.

    The operations supported by default are \c reload() and
    \c remove().  More operations may be supported by derived
    types provided by specific implementations of the SocialNetwork
    interface.

    The data related to an IdentifiableContentItem are exposed
    as ContentItem instances in the model data.  For example:

    \qml
    import QtQuick 1.1
    import org.nemomobile.social 1.0

    Item {
        SocialNetwork {
            id: socialNetwork
            nodeIdentifier: "1234567"
        }

        ListView {
            model: socialNetwork
            delegate: Text { text: contentItem.data["description"] }
        }
    }
    \endqml

    Note that the preceding example will in reality fail,
    since the default SocialNetwork implementation does nothing.
    Please see the Facebook implementation documentation for
    real-world examples.
*/

IdentifiableContentItemInterface::IdentifiableContentItemInterface(QObject *parent)
    : ContentItemInterface(*(new IdentifiableContentItemInterfacePrivate(this)), parent)
{
}

IdentifiableContentItemInterface
    ::IdentifiableContentItemInterface(IdentifiableContentItemInterfacePrivate &dd, QObject *parent)
    : ContentItemInterface(dd, parent)
{
}

/*! \reimp */
bool IdentifiableContentItemInterface::isIdentifiable() const
{
    return true;
}

/*!
    \qmlproperty QString IdentifiableContentItem::identifier
    Holds the identifier of the identifiable content item.

    If the identifier is set after the IdentifiableContentItem
    has been initialized, the entire item will be reloaded.
    In some cases, this may cause the IdentifiableContentItem
    to become invalid (for example, if the specified identifier
    does not identify a valid object of the same type in the
    social network graph).
*/
QString IdentifiableContentItemInterface::identifier() const
{
    Q_D(const IdentifiableContentItemInterface);
    return d->identifier;
}

FilterInterface * IdentifiableContentItemInterface::filter() const
{
    Q_D(const IdentifiableContentItemInterface);
    return d->filter;
}

void IdentifiableContentItemInterface::setFilter(FilterInterface *filter)
{
    Q_D(IdentifiableContentItemInterface);
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
//        if (d->status == SocialNetworkInterface::Initializing) {
//            d->needsLoad = true;
//        } else {
//            load();
//        }
        emit filterChanged();
    }
}

/*!
    \qmlproperty SocialNetwork::Status IdentifiableContentItem::status
    Holds the current status of the IdentifiableContentItem.
*/
SocialNetworkInterface::Status IdentifiableContentItemInterface::status() const
{
    Q_D(const IdentifiableContentItemInterface);
    return d->status;
}

/*!
    \qmlproperty SocialNetwork::ErrorType IdentifiableContentItem::error
    Holds the most recent error which occurred during initialization or
    during network requests associated with this IdentifiableContentItem.

    Note that the \c error will not be reset if subsequent operations
    succeed.
*/
SocialNetworkInterface::ErrorType IdentifiableContentItemInterface::error() const
{
    Q_D(const IdentifiableContentItemInterface);
    return d->error;
}

/*!
    \qmlproperty QString IdentifiableContentItem::errorMessage
    Holds the message associated with the most recent error which occurred
    during initialization or during network requests associated with this
    IdentifiableContentItem.

    Note that the \c errorMessage will not be reset if subsequent operations
    succeed.
*/
QString IdentifiableContentItemInterface::errorMessage() const
{
    Q_D(const IdentifiableContentItemInterface);
    return d->errorMessage;
}

SocialNetworkInterface::Status IdentifiableContentItemInterface::actionStatus() const
{
    Q_D(const IdentifiableContentItemInterface);
    return d->actionStatus;
}

SocialNetworkInterface::ErrorType IdentifiableContentItemInterface::actionError() const
{
    Q_D(const IdentifiableContentItemInterface);
    return d->actionError;
}

QString IdentifiableContentItemInterface::actionErrorMessage() const
{
    Q_D(const IdentifiableContentItemInterface);
    return d->actionErrorMessage;
}


bool IdentifiableContentItemInterface::load()
{
    Q_D(IdentifiableContentItemInterface);
    if (d->status == SocialNetworkInterface::Initializing) {
        qWarning()<< Q_FUNC_INFO
                  << "Cannot load IdentifiableContentItem: did you set socialNetwork ?";
        return false;
    }

    if (d->status == SocialNetworkInterface::Busy
            || d->status == SocialNetworkInterface::Invalid) {
        qWarning() << Q_FUNC_INFO << "Cannot load IdentifiableContentItem: status is Busy/Invalid";
        return false;
    }

    if (!d->filter) {
        qWarning() << Q_FUNC_INFO << "Cannot load IdentifiableContentItem: No filter set";
        return false;
    }


    if (!d->socialNetwork) {
        qWarning() << Q_FUNC_INFO << "Cannot load IdentifiableContentItem: No socialNetwork set";
        return false;
    }

    if (!d->filter->isAcceptable(this, d->socialNetwork)) {
        qWarning() << Q_FUNC_INFO << "Cannot load IdentifiableContentItem: invalid filter";
        return false;
    }


    if (!d->filter->performLoadRequest(this, d->socialNetwork)) {
        qWarning() << Q_FUNC_INFO << "Failed to perform load request";
        return false;
    }

    d->status = SocialNetworkInterface::Busy;
    emit statusChanged();
    return true;
}

void IdentifiableContentItemInterface::setData(const QVariantMap &data)
{
    ContentItemInterface::setData(data);
    emit loaded(true);
}

void IdentifiableContentItemInterface::setError(SocialNetworkInterface::ErrorType error,
                                                const QString &errorMessage)
{
    Q_D(IdentifiableContentItemInterface);
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

    emit loaded(false);
}

void IdentifiableContentItemInterface::setActionComplete()
{
    Q_D(IdentifiableContentItemInterface);
    if (d->actionStatus != SocialNetworkInterface::Idle) {
        d->actionStatus = SocialNetworkInterface::Idle;
        emit actionStatusChanged();
    }
    emit actionComplete(true);
}

void IdentifiableContentItemInterface::setActionError(SocialNetworkInterface::ErrorType actionError,
                                                      const QString &actionErrorMessage)
{
    Q_D(IdentifiableContentItemInterface);
    if (d->actionErrorMessage != actionErrorMessage) {
        d->actionErrorMessage = actionErrorMessage;
        emit actionErrorMessageChanged();
    }

    if (d->actionError != actionError) {
        d->actionError = actionError;
        emit actionErrorChanged();
    }

    if (d->actionStatus != SocialNetworkInterface::Error) {
        d->actionStatus = SocialNetworkInterface::Error;
        emit actionStatusChanged();
    }

    emit actionComplete(false);
}

bool IdentifiableContentItemInterface::prepareAction()
{
    Q_D(IdentifiableContentItemInterface);
    if (d->status == SocialNetworkInterface::Initializing) {
        qWarning()<< Q_FUNC_INFO
                  << "Cannot perform action: did you set socialNetwork ?";
        return false;
    }

    if (d->status == SocialNetworkInterface::Busy
            || d->status == SocialNetworkInterface::Invalid
            || d->actionStatus == SocialNetworkInterface::Busy
            || d->actionStatus == SocialNetworkInterface::Invalid) {
        qWarning() << Q_FUNC_INFO << "Cannot perform action: status is Busy/Invalid";
        return false;
    }

    if (!d->socialNetwork) {
        qWarning() << Q_FUNC_INFO << "Cannot load IdentifiableContentItem: No socialNetwork set";
        return false;
    }

    d->actionStatus = SocialNetworkInterface::Busy;
    emit actionStatusChanged();
    return true;
}

#include "moc_identifiablecontentiteminterface.cpp"
