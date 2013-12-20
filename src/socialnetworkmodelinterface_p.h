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

#ifndef SOCIALNETWORKMODELINTERFACE_P_H
#define SOCIALNETWORKMODELINTERFACE_P_H

#include "socialnetworkmodelinterface.h"
#include "socialnetworkinterface_p.h"

class SocialNetworkModelInterfacePrivate
{
public:
    SocialNetworkModelInterfacePrivate(SocialNetworkModelInterface *q);
    virtual ~SocialNetworkModelInterfacePrivate();

    SocialNetworkInterface::Status status;
    SocialNetworkInterface::ErrorType error;
    QString errorMessage;

    SocialNetworkInterface *socialNetwork;
    FilterInterface *filter;
    bool hasPrevious;
    bool hasNext;
    QList<ContentItemInterface *> modelData;
    QVariantMap extraData;
//    bool resortUpdatePosted;
private:
    void init();
    static QHash<int, QByteArray> roleNames();
    bool load(FilterInterface::LoadType loadType);

    // Slots
    void socialNetworkInitializedChangedHandler();
    void socialNetworkDestroyedHandler();
    void filterDestroyedHandler();

    // Filters and sorters
//    static void filters_append(QDeclarativeListProperty<FilterInterface> *list, FilterInterface *filter);
//    static FilterInterface *filters_at(QDeclarativeListProperty<FilterInterface> *list, int index);
//    static void filters_clear(QDeclarativeListProperty<FilterInterface> *list);
//    static int filters_count(QDeclarativeListProperty<FilterInterface> *list);
//    QList<FilterInterface*> filters;

//    static void sorters_append(QDeclarativeListProperty<SorterInterface> *list, SorterInterface *sorter);
//    static SorterInterface *sorters_at(QDeclarativeListProperty<SorterInterface> *list, int index);
//    static void sorters_clear(QDeclarativeListProperty<SorterInterface> *list);
//    static int sorters_count(QDeclarativeListProperty<SorterInterface> *list);
//    QList<SorterInterface*> sorters;

//    void resort();

    // Methods for SNI
    void clear();
//    void setData(const CacheEntry::List &data);
//    void prependData(const CacheEntry::List &data);
//    void appendData(const CacheEntry::List &data);
//    void setStatus(SocialNetworkInterface::Status newStatus);
//    void setError(SocialNetworkInterface::ErrorType newError, const QString &newErrorMessage);
//    void setHavePreviousAndNext(bool newHasPrevious, bool newHasNext);

    // Slots
//    void sorterDestroyedHandler(QObject *object);
    bool initialized;
    SocialNetworkModelInterface *q_ptr;
    Q_DECLARE_PUBLIC(SocialNetworkModelInterface)
    friend class SocialNetworkInterfacePrivate;
};

#endif // SOCIALNETWORKMODELINTERFACE_P_H
