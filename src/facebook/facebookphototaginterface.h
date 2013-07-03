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

#ifndef FACEBOOKPHOTOTAGINTERFACE_H
#define FACEBOOKPHOTOTAGINTERFACE_H

#include "contentiteminterface.h"

#include <QtCore/QString>

/*
 * NOTE: if you construct one of these in C++ directly,
 * you MUST call classBegin() and componentCompleted()
 * directly after construction.
 */

/*
 * NOTE: this is an unidentifiable content item which
 * is read only and only creatable by the top level
 * FacebookInterface.
 */

class FacebookPhotoTagInterfacePrivate;
class FacebookPhotoTagInterface: public ContentItemInterface
{
    Q_OBJECT
    Q_PROPERTY(QString userIdentifier READ userIdentifier NOTIFY userIdentifierChanged)
    Q_PROPERTY(QString userName READ userName NOTIFY userNameChanged)
    Q_PROPERTY(QString text READ text NOTIFY textChanged)
    Q_PROPERTY(float x READ x NOTIFY xChanged)
    Q_PROPERTY(float y READ y NOTIFY yChanged)
    Q_PROPERTY(QString createdTime READ createdTime NOTIFY createdTimeChanged)
public:
    explicit FacebookPhotoTagInterface(QObject *parent = 0);

    // Overrides.
    int type() const;
    // Accessors
    QString userIdentifier() const;
    QString userName() const;
    QString text() const;
    float x() const;
    float y() const;
    QString createdTime() const;
Q_SIGNALS:
    void userIdentifierChanged();
    void userNameChanged();
    void textChanged();
    void xChanged();
    void yChanged();
    void createdTimeChanged();
private:
    Q_DECLARE_PRIVATE(FacebookPhotoTagInterface)
};

#endif // FACEBOOKPHOTOTAGINTERFACE_H
