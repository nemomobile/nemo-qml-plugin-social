/*
 * Copyright (C) 2013 Jolla Ltd. <lucien.xu@jollamobile.com>
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

#include "facebookrelateddatafilterinterface.h"
#include "filterinterface_p.h"
#include "socialnetworkmodelinterface.h"
#include "identifiablecontentiteminterface_p.h"
#include "facebookontology_p.h"
#include "facebookinterface_p.h"
#include <QtCore/QDebug>

static const char *CONTENTTYPE_KEY = "content_type";
static const char *PATH_KEY = "path";

class FacebookRelatedDataFilterInterfacePrivate: public FilterInterfacePrivate
{
public:
    explicit FacebookRelatedDataFilterInterfacePrivate(FacebookRelatedDataFilterInterface *q);
    QString fields;
    int limit;
    int offset;
    QString identifier;
    FacebookInterface::ConnectionType connection;
};

FacebookRelatedDataFilterInterfacePrivate::FacebookRelatedDataFilterInterfacePrivate(FacebookRelatedDataFilterInterface *q)
    : FilterInterfacePrivate(q), limit(0), offset(0)
{
}

FacebookRelatedDataFilterInterface::FacebookRelatedDataFilterInterface(QObject *parent) :
    FilterInterface(*(new FacebookRelatedDataFilterInterfacePrivate(this)), parent)
{
}

QString FacebookRelatedDataFilterInterface::identifier() const
{
    Q_D(const FacebookRelatedDataFilterInterface);
    return d->identifier;
}

void FacebookRelatedDataFilterInterface::setIdentifier(const QString &identifier)
{
    Q_D(FacebookRelatedDataFilterInterface);
    if (d->identifier != identifier) {
        d->identifier = identifier;
        emit identifierChanged();
    }
}

QString FacebookRelatedDataFilterInterface::fields() const
{
    Q_D(const FacebookRelatedDataFilterInterface);
    return d->fields;
}

void FacebookRelatedDataFilterInterface::setFields(const QString &fields)
{
    Q_D(FacebookRelatedDataFilterInterface);
    if (d->fields != fields) {
        d->fields = fields;
        emit fieldsChanged();
    }
}

int FacebookRelatedDataFilterInterface::limit() const
{
    Q_D(const FacebookRelatedDataFilterInterface);
    return d->limit;
}

void FacebookRelatedDataFilterInterface::setLimit(int limit)
{
    Q_D(FacebookRelatedDataFilterInterface);
    int trueLimit = qMax(0, limit);

    if (d->limit != trueLimit) {
        d->limit = trueLimit;
        emit limitChanged();
    }
}

int FacebookRelatedDataFilterInterface::offset() const
{
    Q_D(const FacebookRelatedDataFilterInterface);
    return d->offset;
}

void FacebookRelatedDataFilterInterface::setOffset(int offset)
{
    Q_D(FacebookRelatedDataFilterInterface);
    int trueOffset = qMax(0, offset);
    if (d->offset != trueOffset) {
        d->offset = trueOffset;
        emit offsetChanged();
    }
}

FacebookInterface::ConnectionType FacebookRelatedDataFilterInterface::connection() const
{
    Q_D(const FacebookRelatedDataFilterInterface);
    return d->connection;
}

void FacebookRelatedDataFilterInterface::setConnection(FacebookInterface::ConnectionType connection)
{
    Q_D(FacebookRelatedDataFilterInterface);
    if (d->connection != connection) {
        d->connection = connection;
        emit connectionChanged();
    }
}

bool FacebookRelatedDataFilterInterface::isAcceptable(QObject *item,
                                                      SocialNetworkInterface *socialNetwork) const
{
    if (!testType<FacebookInterface>(socialNetwork)) {
        return false;
    }

    // This filter only works for models
    if (!testType<SocialNetworkModelInterface>(item)) {
        return false;
    }

    return true;
}

bool FacebookRelatedDataFilterInterface::performLoadRequestImpl(QObject *item,
                                                                SocialNetworkInterface *socialNetwork,
                                                                LoadType loadType)
{
    Q_D(FacebookRelatedDataFilterInterface);
    FacebookInterface *facebook = qobject_cast<FacebookInterface *>(socialNetwork);
    if (!facebook) {
        return false;
    }

    SocialNetworkModelInterface *model = qobject_cast<SocialNetworkModelInterface *>(item);
    if (!model) {
        return false;
    }

    FacebookInterface::ContentItemType contentType = FacebookInterface::Unknown;
    QString path;

    QMap<QString, QString> extraArguments;
    FacebookInterfacePrivate::typeAndPath(d->connection, contentType, path, extraArguments);
    if (contentType == FacebookInterface::Unknown) {
        return false;
    }

    QVariantMap properties;
    QMap<QString, QString> arguments;
    if (d->limit > 0 && loadType == FilterInterface::Load) {
        arguments.insert(FACEBOOK_ONTOLOGY_METADATA_PAGING_LIMIT, QString::number(d->limit));
        properties.insert(FACEBOOK_ONTOLOGY_METADATA_PAGING_LIMIT, d->limit);
    }

    if (d->offset > 0 && loadType == FilterInterface::Load) {
        arguments.insert(FACEBOOK_ONTOLOGY_METADATA_PAGING_OFFSET, QString::number(d->offset));
    }

    for (QMap<QString, QString>::iterator i = extraArguments.begin(); i != extraArguments.end(); ++i) {
        arguments.insert(i.key(), i.value());
    }

    // We use extra parameters if needed
    QVariantMap extra;
    switch (loadType) {
    case FilterInterface::LoadNext:
        extra = model->extraData().value(FACEBOOK_ONTOLOGY_METADATA_PAGING_NEXT).toMap();
        properties.insert(FACEBOOK_ONTOLOGY_METADATA_PAGING_LIMIT,
                          extra.value(FACEBOOK_ONTOLOGY_METADATA_PAGING_LIMIT).toInt());
        break;
    case FilterInterface::LoadPrevious:
        extra = model->extraData().value(FACEBOOK_ONTOLOGY_METADATA_PAGING_PREVIOUS).toMap();
        properties.insert(FACEBOOK_ONTOLOGY_METADATA_PAGING_LIMIT,
                          extra.value(FACEBOOK_ONTOLOGY_METADATA_PAGING_LIMIT).toInt());
        break;
    default:
        break;
    }

    for (QVariantMap::ConstIterator i = extra.constBegin(); i != extra.constEnd(); i++) {
        arguments.insert(i.key(), i.value().toString());
    }

    QObject *handle = facebook->get(this, d->identifier, path, d->fields, arguments);
    properties.insert(CONTENTTYPE_KEY, contentType);
    properties.insert(PATH_KEY, path);

    d->addHandleProperties(handle, properties);
    return d->addHandle(handle, item, socialNetwork, loadType);
}

bool FacebookRelatedDataFilterInterface::performSetModelDataImpl(SocialNetworkModelInterface *model,
                                                                 SocialNetworkInterface *socialNetwork,
                                                                 const QByteArray &data,
                                                                 LoadType loadType,
                                                                 const QVariantMap &properties)
{
    Q_UNUSED(socialNetwork)
    bool ok = false;
    QVariantMap dataMap = IdentifiableContentItemInterfacePrivate::parseReplyData(data, &ok);
    if (!ok) {
        QString errorMessage = QString(QLatin1String("Unable to parse downloaded data. "\
                                                     "Downloaded data: %1")).arg(QString(data));
        model->setError(SocialNetworkInterface::DataError, errorMessage);
        return false;
    }

    // dataMap should contain information like this
    //
    // {
    //     "id": _yourid_,
    //     _path_: {
    //         "data": [
    //             {
    //                 "id": "someid"
    //                 "name": "Someone",
    //                 ...
    //             },
    //             ...
    //         ],
    //         "paging": {...}
    //     }
    // }

    FacebookInterface::ContentItemType contentType = static_cast<FacebookInterface::ContentItemType>(properties.value(CONTENTTYPE_KEY).toInt());
    QString path = properties.value(PATH_KEY).toString();
    QVariantMap pathDataMap = dataMap.value(path).toMap();
    QVariantList dataList = pathDataMap.value(FACEBOOK_ONTOLOGY_METADATA_DATA).toList();
    QList<ContentItemInterface *> modelData;

    foreach (const QVariant &dataEntry, dataList) {
        QVariantMap dataMap = dataEntry.toMap();
        if (isDataAcceptable(dataMap)) {
            modelData.append(FacebookInterfacePrivate::createItem(contentType, dataMap,
                                                                  socialNetwork, model));
        }
    }


    // Extract paging informations
    // We deliver paging informations to the node extra using the following scheme
    // {
    //     "next": {
    //         "key1": value1
    //         "key2": value2
    //     },
    //     "previous": {...}
    // }
    // Next and previous are the conventionnal way to handle paging cursors,
    // but we need to know if, for a specific type, we need to display the
    // next / previous button. The "have" key is used to display this button.
    //
    // It is usually simple to detect if there is a need of having a next / previous
    // button if Facebook uses cursors. But if it don't use cursors, it is harder.
    //
    // We guess the presence of this button with the following idea:
    // if the next or previous URL that Facebook API provide do not exist then
    // there is no next or previous. If it is provided, there is a next (resp. previous)
    // if and only if we are not appending (resp. prepending) or if we are appending
    // (resp. prepending) and that the data that is retrieved is not empty.
    QVariantMap previousExtra;
    QVariantMap nextExtra;

    QVariantMap pagingMap = pathDataMap.value(FACEBOOK_ONTOLOGY_METADATA_PAGING).toMap();

    // Manage cursors: easy
    if (pagingMap.contains(FACEBOOK_ONTOLOGY_METADATA_PAGING_CURSORS)) {
        QVariantMap cursorsMap = pagingMap.value(FACEBOOK_ONTOLOGY_METADATA_PAGING_CURSORS).toMap();

        // Data for cursors
        if (cursorsMap.contains(FACEBOOK_ONTOLOGY_METADATA_PAGING_CURSORS_AFTER)) {
            nextExtra.insert(FACEBOOK_ONTOLOGY_METADATA_PAGING_CURSORS_AFTER,
                             cursorsMap.value(FACEBOOK_ONTOLOGY_METADATA_PAGING_CURSORS_AFTER).toString());
        }

        if (cursorsMap.contains(FACEBOOK_ONTOLOGY_METADATA_PAGING_CURSORS_BEFORE)) {
            previousExtra.insert(FACEBOOK_ONTOLOGY_METADATA_PAGING_CURSORS_BEFORE,
                                 cursorsMap.value(FACEBOOK_ONTOLOGY_METADATA_PAGING_CURSORS_BEFORE).toString());
        }
    } else {
        // If not, we should parse the next and previous url and extract relevant data
        QUrl previousUrl = QUrl(pagingMap.value(FACEBOOK_ONTOLOGY_METADATA_PAGING_PREVIOUS).toString());

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
        QList<QPair<QString, QString> > previousQueries = previousUrl.queryItems();
#else
        QUrlQuery query (previousUrl);
        QList<QPair<QString, QString> > previousQueries = query.queryItems();
#endif
        QList<QPair<QString, QString> >::const_iterator i;

        for (i = previousQueries.begin(); i != previousQueries.end(); ++ i) {
            if (i->first == FACEBOOK_ONTOLOGY_METADATA_PAGING_OFFSET
                || i->first == FACEBOOK_ONTOLOGY_METADATA_PAGING_SINCE) {
                previousExtra.insert(i->first, i->second);
            }
        }

        QUrl nextUrl = QUrl(pagingMap.value(FACEBOOK_ONTOLOGY_METADATA_PAGING_NEXT).toString());
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
        QList<QPair<QString, QString> > nextQueries = nextUrl.queryItems();
#else
        query = QUrlQuery(nextUrl);
        QList<QPair<QString, QString> > nextQueries = query.queryItems();
#endif

        for (i = nextQueries.begin(); i != nextQueries.end(); ++ i) {
            if (i->first == FACEBOOK_ONTOLOGY_METADATA_PAGING_OFFSET
                || i->first == FACEBOOK_ONTOLOGY_METADATA_PAGING_UNTIL) {
                nextExtra.insert(i->first, i->second);
            }
        }
    }

    if (!previousExtra.isEmpty()) {
        previousExtra.insert(FACEBOOK_ONTOLOGY_METADATA_PAGING_LIMIT,
                             properties.value(FACEBOOK_ONTOLOGY_METADATA_PAGING_LIMIT).toInt());
    }

    if (!nextExtra.isEmpty()) {
        nextExtra.insert(FACEBOOK_ONTOLOGY_METADATA_PAGING_LIMIT,
                         properties.value(FACEBOOK_ONTOLOGY_METADATA_PAGING_LIMIT).toInt());
    }

    // If we have cursors, next and previous are easy, except if the
    // type is comments. We need to workaround Facebook comments,
    // since the previous field won't exist in returned replies (sadly)
    bool hasPrevious = false;
    bool hasNext = false;

    if (pagingMap.contains(FACEBOOK_ONTOLOGY_METADATA_PAGING_CURSORS)
        && contentType != FacebookInterface::Comment) {
        hasPrevious = pagingMap.contains(FACEBOOK_ONTOLOGY_METADATA_PAGING_PREVIOUS);
        hasNext = pagingMap.contains(FACEBOOK_ONTOLOGY_METADATA_PAGING_NEXT);
    } else if (pagingMap.contains(FACEBOOK_ONTOLOGY_METADATA_PAGING_CURSORS)
               && contentType == FacebookInterface::Comment) {
        // If we have comments, we hack and say that we always have previous and next
        // This is really hacking and TODO
        hasPrevious = true;
        hasNext = true;
    } else {
        // Otherwise, let's guess previous and next in paging
        if (pagingMap.contains(FACEBOOK_ONTOLOGY_METADATA_PAGING_PREVIOUS)) {
            hasPrevious = true;
            if (loadType == FilterInterface::LoadPrevious && dataList.isEmpty()) {
                hasPrevious = false;
            }
        }

        if (pagingMap.contains(FACEBOOK_ONTOLOGY_METADATA_PAGING_NEXT)) {
            hasNext = true;
            if (loadType == FilterInterface::LoadNext && dataList.isEmpty()) {
                hasNext = false;
            }
        }
    }

    QVariantMap extraData;
    extraData.insert(FACEBOOK_ONTOLOGY_METADATA_PAGING_PREVIOUS, previousExtra);
    extraData.insert(FACEBOOK_ONTOLOGY_METADATA_PAGING_NEXT, nextExtra);

    // Set some information about the model (pagination)
    switch (loadType) {
    case FilterInterface::LoadPrevious:
        hasNext = model->hasNext();
        extraData.insert(FACEBOOK_ONTOLOGY_METADATA_PAGING_NEXT,
                         model->extraData().value(FACEBOOK_ONTOLOGY_METADATA_PAGING_NEXT));
        break;
    case FilterInterface::LoadNext:
        hasPrevious = model->hasPrevious();
        extraData.insert(FACEBOOK_ONTOLOGY_METADATA_PAGING_PREVIOUS,
                         model->extraData().value(FACEBOOK_ONTOLOGY_METADATA_PAGING_PREVIOUS));
        break;
    default:
        break;
    }
    model->setPagination(hasPrevious, hasNext);
    model->setExtraData(extraData);

    // Populate model depending on the type of load
    switch (loadType) {
    case FilterInterface::Load:
        model->setModelData(modelData);
        break;
    case FilterInterface::LoadPrevious:
        model->prependModelData(modelData);
        break;
    case FilterInterface::LoadNext:
        model->appendModelData(modelData);
        break;
    default:
        break;
    }

    return true;
}

bool FacebookRelatedDataFilterInterface::isDataAcceptable(const QVariantMap &data)
{
    Q_UNUSED(data)
    return true;
}
