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

#ifndef FACEBOOKPOSTINTERFACE_P_H
#define FACEBOOKPOSTINTERFACE_P_H

#include "facebookpostinterface.h"
#include "facebookinterface_p.h"
#include "identifiablecontentiteminterface_p.h"
#include <QtCore/QList>

class FacebookPostInterfacePrivate: public IdentifiableContentItemInterfacePrivate
{
public:
    explicit FacebookPostInterfacePrivate(FacebookPostInterface *q);
    void finishedHandler();
    void emitPropertyChangeSignals(const QVariantMap &oldData, const QVariantMap &newData);
    FacebookInterfacePrivate::FacebookAction action;
    FacebookObjectReferenceInterface *from;
    QList<FacebookObjectReferenceInterface *> to;
    QList<FacebookNameTagInterface *> messageTags;
    QList<FacebookPostPropertyInterface *> properties;
    QList<FacebookPostActionInterface *> actions;
    QList<FacebookNameTagInterface *> storyTags;
    QList<FacebookObjectReferenceInterface *> withTags;
    FacebookObjectReferenceInterface *application;
    bool liked;
    int likesCount;
    int commentsCount;
private:
    Q_DECLARE_PUBLIC(FacebookPostInterface)
    static void to_append(QDeclarativeListProperty<FacebookObjectReferenceInterface> *list,
                          FacebookObjectReferenceInterface *data);
    static FacebookObjectReferenceInterface * to_at(QDeclarativeListProperty<FacebookObjectReferenceInterface> *list,
                                                    int index);
    static void to_clear(QDeclarativeListProperty<FacebookObjectReferenceInterface> *list);
    static int to_count(QDeclarativeListProperty<FacebookObjectReferenceInterface> *list);
    static void message_tags_append(QDeclarativeListProperty<FacebookNameTagInterface> *list,
                                    FacebookNameTagInterface *data);
    static FacebookNameTagInterface * message_tags_at(QDeclarativeListProperty<FacebookNameTagInterface> *list,
                                                      int index);
    static void message_tags_clear(QDeclarativeListProperty<FacebookNameTagInterface> *list);
    static int message_tags_count(QDeclarativeListProperty<FacebookNameTagInterface> *list);
    static void properties_append(QDeclarativeListProperty<FacebookPostPropertyInterface> *list,
                                  FacebookPostPropertyInterface *data);
    static FacebookPostPropertyInterface * properties_at(QDeclarativeListProperty<FacebookPostPropertyInterface> *list,
                                                         int index);
    static void properties_clear(QDeclarativeListProperty<FacebookPostPropertyInterface> *list);
    static int properties_count(QDeclarativeListProperty<FacebookPostPropertyInterface> *list);
    static void actions_append(QDeclarativeListProperty<FacebookPostActionInterface> *list,
                               FacebookPostActionInterface *data);
    static FacebookPostActionInterface * actions_at(QDeclarativeListProperty<FacebookPostActionInterface> *list,
                                                    int index);
    static void actions_clear(QDeclarativeListProperty<FacebookPostActionInterface> *list);
    static int actions_count(QDeclarativeListProperty<FacebookPostActionInterface> *list);
    static void story_tags_append(QDeclarativeListProperty<FacebookNameTagInterface> *list,
                                  FacebookNameTagInterface *data);
    static FacebookNameTagInterface * story_tags_at(QDeclarativeListProperty<FacebookNameTagInterface> *list,
                                                    int index);
    static void story_tags_clear(QDeclarativeListProperty<FacebookNameTagInterface> *list);
    static int story_tags_count(QDeclarativeListProperty<FacebookNameTagInterface> *list);
    static void with_tags_append(QDeclarativeListProperty<FacebookObjectReferenceInterface> *list,
                                 FacebookObjectReferenceInterface *data);
    static FacebookObjectReferenceInterface * with_tags_at(QDeclarativeListProperty<FacebookObjectReferenceInterface> *list,
                                                           int index);
    static void with_tags_clear(QDeclarativeListProperty<FacebookObjectReferenceInterface> *list);
    static int with_tags_count(QDeclarativeListProperty<FacebookObjectReferenceInterface> *list);
};

#endif // FACEBOOKPOSTINTERFACE_P_H
