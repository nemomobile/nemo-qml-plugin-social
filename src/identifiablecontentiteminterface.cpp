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
    , needsReload(false)
    , currentReply(0)
{
}

IdentifiableContentItemInterfacePrivate::~IdentifiableContentItemInterfacePrivate()
{
    deleteReply();
}

QNetworkReply *IdentifiableContentItemInterfacePrivate::reply()
{
    return currentReply;
}

void IdentifiableContentItemInterfacePrivate::deleteReply()
{
    if (currentReply) {
        currentReply->disconnect();
        currentReply->deleteLater();
        currentReply = 0;
    }
}

/*
    This convenience function should be called by derived types
    when they want to implement any form of network communication.

    If the request is created successfully, the state of the
    IdentifiableContentItem will change to Busy, and the function
    will return true.  The caller takes ownership of dd->reply()
    and must delete the reply via dd->deleteReply() when they are
    finished with it.

    postData is only valid for post requests
    extraData is specific to the social network
*/
bool IdentifiableContentItemInterfacePrivate::request(RequestType requestType,
                                                      const QString &objectIdentifier,
                                                      const QString &extraPath,
                                                      const QStringList &whichFields,
                                                      const QVariantMap &postData,
                                                      const QVariantMap &extraData)
{
    Q_Q(IdentifiableContentItemInterface);
    // Caller takes ownership of dd->reply() and must dd->deleteReply()
    // If request created successfully, changes state to Busy and returns true.

    if (status == SocialNetworkInterface::Initializing
            || status == SocialNetworkInterface::Busy
            || status == SocialNetworkInterface::Invalid) {
        qWarning() << Q_FUNC_INFO
                   << "Warning: cannot start request, because status is Initializing/Busy/Invalid";
        return false;
    }

    if (requestType != IdentifiableContentItemInterfacePrivate::Get
            && requestType != IdentifiableContentItemInterfacePrivate::Post
            && requestType != IdentifiableContentItemInterfacePrivate::Delete) {
        qWarning() << Q_FUNC_INFO
                   << "Warning: cannot start request, because request type is unknown";
        return false;
    }

    if (currentReply != 0) {
        qWarning() << Q_FUNC_INFO << "Error: not Busy and yet current reply is non-null!";
        return false;
    }

    if (!socialNetworkInterface) {
        qWarning() << Q_FUNC_INFO << "Error: social network is not valid!";
        return false;
    }

    QNetworkReply *reply = 0;
    switch (requestType) {
    case IdentifiableContentItemInterfacePrivate::Get:
        reply = socialNetworkInterface->d_func()->getRequest(objectIdentifier, extraPath,
                                                             whichFields, extraData);
        break;
    case IdentifiableContentItemInterfacePrivate::Post:
        reply = socialNetworkInterface->d_func()->postRequest(objectIdentifier, extraPath,
                                                              postData, extraData);
        break;
    default:
        reply = socialNetworkInterface->d_func()->deleteRequest(objectIdentifier, extraPath,
                                                                extraData);
        break;
    }

    if (reply) {
        currentReply = reply;
        status = SocialNetworkInterface::Busy;
        emit q->statusChanged();
        return true;
    }

    qWarning() << "Warning: social network was unable to create request";
    return false;
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
    // But this one is a bit special, since if the id changed, it's a terrible error.

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
    } else if (newId.isEmpty() || oldId != newId) {
        // The identifier changed.
        // This shouldn't happen in real life.  Must be an error.
        status = SocialNetworkInterface::Invalid;
        error = SocialNetworkInterface::DataUpdateError;
        errorMessage = QString(QLatin1String("identifier changed during data update from %1 to %2")).arg(oldId).arg(newId);
        socialNetworkInterface = 0;
        emit q->statusChanged();
        emit q->errorChanged();
        emit q->errorMessageChanged();
        emit q->socialNetworkChanged();
    }

    // finally, as all derived classes must do, call super class implementation.
    ContentItemInterfacePrivate::emitPropertyChangeSignals(oldData, newData);
}

/*! \reimp */
void IdentifiableContentItemInterfacePrivate::initializationComplete()
{
    Q_Q(IdentifiableContentItemInterface);
    // reload content if required.
    if (needsReload) {
        needsReload = false;
        status = SocialNetworkInterface::Idle; // but DON'T emit, otherwise reload() will fail.
        q->reload(); // XXX TODO: allow specifying whichFields for first time initialization reload()?
    } else {
        status = SocialNetworkInterface::Idle;
        emit q->statusChanged();
    }

    // Finally, as all derived classes must do, call super class implementation.
    ContentItemInterfacePrivate::initializationComplete();
}

void IdentifiableContentItemInterfacePrivate::connectFinishedAndErrors()
{
    Q_Q(IdentifiableContentItemInterface);
    connectErrors();
    QObject::connect(reply(), SIGNAL(finished()), q, SLOT(finishedHandler()));
}

void IdentifiableContentItemInterfacePrivate::connectErrors()
{
    Q_Q(IdentifiableContentItemInterface);
    QObject::connect(reply(), SIGNAL(error(QNetworkReply::NetworkError)),
                     q, SLOT(errorHandler(QNetworkReply::NetworkError)));
    QObject::connect(reply(), SIGNAL(sslErrors(QList<QSslError>)),
                     q, SLOT(sslErrorsHandler(QList<QSslError>)));
}

void IdentifiableContentItemInterfacePrivate::finishedHandler()
{
}

void IdentifiableContentItemInterfacePrivate::removeHandler()
{
    // TODO: this handler might not be very suited

    Q_Q(IdentifiableContentItemInterface);
    if (!reply()) {
        // if an error occurred, it might have been deleted by the error handler.
        qWarning() << Q_FUNC_INFO << "network request finished but no reply";
        return;
    }

    // Default just checks to see if the response is the text "true".
    // If it is, this handler deletes the reply(), sets the status to invalid.
    // If it isn't, this handler deletes the reply(), sets the status to error,
    // and emits responseReceived() with the given data.
    QByteArray replyData = reply()->readAll();
    deleteReply();
    bool ok = false;
    QVariantMap responseData = ContentItemInterfacePrivate::parseReplyData(replyData, &ok);
    if (!ok)
        responseData.insert("response", replyData);
    if (replyData == QString(QLatin1String("true"))) {
        status = SocialNetworkInterface::Invalid; // We have been removed, so we are now invalid.
        emit q->statusChanged();
        emit q->responseReceived(responseData);
    } else {
        status = SocialNetworkInterface::Error;
        errorMessage = "remove() request returned non-true value";
        error = SocialNetworkInterface::RequestError;
        emit q->statusChanged();
        emit q->errorChanged();
        emit q->errorMessageChanged();
        emit q->responseReceived(responseData);
    }
}

void IdentifiableContentItemInterfacePrivate::reloadHandler()
{
    Q_Q(IdentifiableContentItemInterface);
    if (!reply()) {
        // If an error occurred, it might have been deleted by the error handler.
        qWarning() << Q_FUNC_INFO << "Network request finished but no reply";
        return;
    }

    // Default just checks to see if the response is a valid FB Object
    // If it is, this handler deletes the reply(), sets the status to Idle,
    // and sets the internal data() to the object data.
    // If it isn't, this handler deletes the reply(), sets the status to error,
    // and emits responseReceived() with the given data.
    QByteArray replyData = reply()->readAll();
    deleteReply();
    bool ok = false;
    QVariantMap responseData = ContentItemInterfacePrivate::parseReplyData(replyData, &ok);
    if (!ok)
        responseData.insert("response", replyData);
    if (ok && !responseData.value("id").toString().isEmpty()) {
        // We should merge new data, replacing old data with new one
        // but keeping missing data (if the new data do not have everything)
        if (data() != responseData) {
            QVariantMap currentData = data();
            foreach (QString key, responseData.keys()) {
                currentData.insert(key, responseData.value(key));
            }
            setData(currentData);
        }
        status = SocialNetworkInterface::Idle;
        emit q->statusChanged();
        emit q->responseReceived(responseData);
    } else {
        status = SocialNetworkInterface::Error;
        errorMessage = "reload() request returned unidentifiable value";
        error = SocialNetworkInterface::RequestError;
        emit q->statusChanged();
        emit q->errorChanged();
        emit q->errorMessageChanged();
        emit q->responseReceived(responseData);
    }
}

void IdentifiableContentItemInterfacePrivate::errorHandler(QNetworkReply::NetworkError networkError)
{
    Q_Q(IdentifiableContentItemInterface);
    deleteReply();

    // TODO: This huge switch should be better, with QMetaEnum for example.
    // It should also be exported, in order to be used elsewhere
    errorMessage = networkErrorString(networkError);
    error = SocialNetworkInterface::RequestError;
    status = SocialNetworkInterface::Error;

    emit q->statusChanged();
    emit q->errorChanged();
    emit q->errorMessageChanged();
}

void IdentifiableContentItemInterfacePrivate::sslErrorsHandler(const QList<QSslError> &sslErrors)
{
    Q_Q(IdentifiableContentItemInterface);
    deleteReply();

    errorMessage = QLatin1String("SSL error: ");
    if (sslErrors.isEmpty()) {
        errorMessage += QLatin1String("unknown SSL error");
    } else {
        foreach (const QSslError &sslError, sslErrors)
            errorMessage += sslError.errorString() + QLatin1String("; ");
        errorMessage.chop(2);
    }

    error = SocialNetworkInterface::RequestError;
    status = SocialNetworkInterface::Error;

    emit q->statusChanged();
    emit q->errorChanged();
    emit q->errorMessageChanged();
}

//-------------------------------------------------

/*!
    \qmltype IdentifiableContentItem
    \instantiates IdentifiableContentItemInterface
    \inqmlmodule org.nemomobile.social 1
    \brief An IdentifiableContentItem represents an identifiable data object in a social network

    A data object which is identifiable is represented by an
    IdentifiableContentItem. Instances of this sort of ContentItem
    can be used as the \c node in a SocialNetworkModel.

    An IdentifiableContentItem may also have more operations
    performed on it than a non-identifiable content item.
    As these operations result in network communication, the
    IdentifiableContentItem type has \l status, \l error and
    \l errorMessage properties.

    The operations supported by default are \l reload() and
    \l remove().  More operations may be supported by derived
    types provided by specific implementations of the SocialNetwork
    interface.

    An IdentifiableContentItem is usually constructed via a request done using a
    SocialNetworkModel. It can also be constructed directly using a SocialNetwork
    that is provided to \l socialNetwork, and setting the \l identifier.

    The following example shows IdentifiableContentItem retrieved using
    a SocialNetworkModel. Note that the \c data property is used to retrieve
    data associated to the IdentifiableContentItem.

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
            nodeIdentifier: "1234567"
        }

        ListView {
            model: socialNetworkModel
            // This delegate uses the IdentifiableContentItem
            delegate: Text { text: contentItem.data["description"] }
        }

        Component.onCompleted: socialNetworkModel.populate()
    }
    \endqml

    The following example shows IdentifiableContentItem retrieved directly.

    \qml
    import QtQuick 1.1
    import org.nemomobile.social 1.0

    Item {
        MySocialNetwork {
            id: mySocialNetwork
            nodeIdentifier: "1234567"
        }

        IdentifiableContentItem {
            id: item
            socialNetwork: mySocialNetwork
            identifier: "12345" // Some identifier
        }

        Text {
            text: item != null ? item.data["description"] : ""
        }
    }
    \endqml
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

void IdentifiableContentItemInterface::setIdentifier(const QString &id)
{
    Q_D(IdentifiableContentItemInterface);
    d->identifier = id;
    if (d->status == SocialNetworkInterface::Initializing) {
        d->needsReload = true;
    } else {
        reload(); // XXX TODO: allow user to set whichFields for first-time initialization?
    }
}

/*!
    \qmlproperty enumeration IdentifiableContentItem::status

    Hold the status of the IdentifiableContentItem.

    The status can be one of:
    \list
    \li SocialNetwork.Initializing
    \li SocialNetwork.Idle - the default
    \li SocialNetwork.Busy
    \li SocialNetwork.Error
    \li SocialNetwork.Invalid
    \endlist

    When the status is \c Initializing, the component is initializing.

    When in \c Idle, the IdentifiableContentItem loaded information from the
    SocialNetwork, or is waiting to perform a loading operation.

    When in \c Busy, the IdentifiableContentItem is waiting for information from
    the social network

    When in \c Error, the information that has been loaded is wrong, often because the
    loading process failed. Reloading information with reload() might fix the loaded
    information. \l error and \l errorMessage might also carry information about
    the error.

    When in \c Invalid, the IdentifiableContentItem is in an invalid state, and should not
    be used anymore.
*/
SocialNetworkInterface::Status IdentifiableContentItemInterface::status() const
{
    Q_D(const IdentifiableContentItemInterface);
    return d->status;
}

/*!
    \qmlproperty enumeration IdentifiableContentItem::error

    Hold the type of the most recent error in IdentifiableContentItem.
    Note that the \c error will not be reset if subsequent operations
    succeed.

    The error can be one of:
    \list
    \li SocialNetwork.NoError - the default
    \li SocialNetwork.AccountError - \b deprecated, error related to the account.
    \li SocialNetwork.SignOnError - \b deprecated, error related to the signon process.
    \li SocialNetwork.BusyError  - \b deprecated, error because the IdentifiableContentItem
    is already busy.
    \li SocialNetwork.RequestError - error related to a failure during a request to the social API.
    \li SocialNetwork.DataUpdateError - error related during data update.
    \li SocialNetwork.OtherError - other error (not used).
    \li SocialNetwork.LastError - not used.
    \endlist
*/
SocialNetworkInterface::ErrorType IdentifiableContentItemInterface::error() const
{
    Q_D(const IdentifiableContentItemInterface);
    return d->error;
}

/*!
    \qmlproperty QString IdentifiableContentItem::errorMessage

    Holds the message associated with the most recent error in
    IdentifiableContentItem. Note that the \c errorMessage will
    not be reset if subsequent operations succeed.
*/
QString IdentifiableContentItemInterface::errorMessage() const
{
    Q_D(const IdentifiableContentItemInterface);
    return d->errorMessage;
}

/*!
    \qmlmethod bool IdentifiableContentItem::remove()

    Removes the object from the social network.

    The default implementation sends a HTTP DELETE request for the
    specified object identifier.
*/
bool IdentifiableContentItemInterface::remove()
{
    Q_D(IdentifiableContentItemInterface);
    if (!d->request(IdentifiableContentItemInterfacePrivate::Delete, d->identifier))
        return false;

    connect(d->reply(), SIGNAL(finished()), this, SLOT(removeHandler()));
    d->connectErrors();
    return true;
}

/*!
    \qmlmethod bool IdentifiableContentItem::reload()

    Reloads the object data from the social network.  Only the fields
    specified in the \a whichFields list will be requested from the
    remote service.

    The default implementation sends a HTTP GET request for the
    specified object identifier.
*/
bool IdentifiableContentItemInterface::reload(const QStringList &whichFields)
{
    Q_D(IdentifiableContentItemInterface);
    if (!d->request(IdentifiableContentItemInterfacePrivate::Get, d->identifier,
                    QString(), whichFields))
        return false;

    connect(d->reply(), SIGNAL(finished()), this, SLOT(reloadHandler()));
    d->connectErrors();
    return true;
}

#include "moc_identifiablecontentiteminterface.cpp"
