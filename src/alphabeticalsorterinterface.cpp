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

#include "alphabeticalsorterinterface.h"
#include "contentiteminterface.h"
#include <QtCore/QMetaProperty>
#include <QtCore/QMetaObject>
#include "sorterinterface_p.h"

class AlphabeticalSorterInterfacePrivate: public SorterInterfacePrivate
{
public:
    AlphabeticalSorterInterfacePrivate();
    QString field;
};

AlphabeticalSorterInterfacePrivate::AlphabeticalSorterInterfacePrivate()
{
}

AlphabeticalSorterInterface::AlphabeticalSorterInterface(QObject *parent):
    SorterInterface(*(new AlphabeticalSorterInterfacePrivate()), parent)
{
}

QString AlphabeticalSorterInterface::field() const
{
    Q_D(const AlphabeticalSorterInterface);
    return d->field;
}

void AlphabeticalSorterInterface::setField(const QString &field)
{
    Q_D(AlphabeticalSorterInterface);
    if (d->field != field) {
        d->field = field;
        emit fieldChanged();
    }
}

bool AlphabeticalSorterInterface::firstLessThanSecond(const QVariantMap &first,
                                              const QVariantMap &second) const
{
    Q_D(const AlphabeticalSorterInterface);
    if (d->field.isEmpty()) {
        return SorterInterface::firstLessThanSecond(first, second);
    }

    if (first.empty() && !second.empty())
        return true;

    if (second.empty())
        return false;

    // We first order by type
    int firstType = first.value(NEMOQMLPLUGINS_SOCIAL_CONTENTITEMTYPE).toInt();
    int secondType = second.value(NEMOQMLPLUGINS_SOCIAL_CONTENTITEMTYPE).toInt();
    if (firstType < secondType)
        return true;
    if (firstType > secondType)
        return false;



    return first.value(d->field).toString()
           < second.value(d->field).toString();

}
