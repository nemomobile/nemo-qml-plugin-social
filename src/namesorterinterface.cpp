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

#include "namesorterinterface.h"
#include "contentiteminterface.h"
#include <QtCore/QMetaProperty>
#include <QtCore/QMetaObject>
#include "facebook/facebookontology_p.h"

NameSorterInterface::NameSorterInterface(QObject *parent):
    SorterInterface(parent)
{
}

bool NameSorterInterface::firstLessThanSecond(const QVariantMap &first,
                                              const QVariantMap &second) const
{
    if (first.empty() && !second.empty())
        return true;

    if (second.empty())
        return false;

//    // We first order by type
//    if (first->type() < second->type())
//        return true;
//    if (first->type() > second->type())
//        return false;

//    // Checks
//    int firstPropertyIndex = first->metaObject()->indexOfProperty("name");
//    int secondPropertyIndex = second->metaObject()->indexOfProperty("name");

//    if (firstPropertyIndex == -1 || secondPropertyIndex == -1) {
//        return SorterInterface::firstLessThanSecond(first, second);
//    }

//    QMetaProperty firstProperty = first->metaObject()->property(firstPropertyIndex);
//    QMetaProperty secondProperty = first->metaObject()->property(secondPropertyIndex);

//    if (firstProperty.type() != QVariant::String || secondProperty.type() != QVariant::String) {
//        return SorterInterface::firstLessThanSecond(first, second);
//    }

//    QString firstName = firstProperty.read(first).toString();
//    QString secondName = firstProperty.read(first).toString();

//    return firstName < secondName;

    return first.value(FACEBOOK_ONTOLOGY_OBJECTREFERENCE_OBJECTNAME).toString()
           < second.value(FACEBOOK_ONTOLOGY_OBJECTREFERENCE_OBJECTNAME).toString();

}
