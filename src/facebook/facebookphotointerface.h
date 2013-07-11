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

#ifndef FACEBOOKPHOTOINTERFACE_H
#define FACEBOOKPHOTOINTERFACE_H

#include "identifiablecontentiteminterface.h"

#if QT_VERSION_5
#include <QtQml/QQmlListProperty>
#define QDeclarativeListProperty QQmlListProperty
#else
#include <QtDeclarative/QDeclarativeListProperty>
#endif
#include <QtCore/QString>
#include "facebookobjectreferenceinterface.h"
#include "facebookphototaginterface.h"
#include "facebooknametaginterface.h"
#include <QtCore/QUrl>
#include "facebookphotoimageinterface.h"
#include <QtCore/QVariantMap>

/*
 * NOTE: if you construct one of these in C++ directly,
 * you MUST call classBegin() and componentCompleted()
 * directly after construction.
 */

class FacebookPhotoInterfacePrivate;
class FacebookPhotoInterface: public IdentifiableContentItemInterface
{
    Q_OBJECT
    Q_PROPERTY(QString albumIdentifier READ albumIdentifier NOTIFY albumIdentifierChanged)
    Q_PROPERTY(FacebookObjectReferenceInterface * from READ from NOTIFY fromChanged)
    Q_PROPERTY(QDeclarativeListProperty<FacebookPhotoTagInterface> tags READ tags NOTIFY tagsChanged)
    Q_PROPERTY(QString name READ name NOTIFY nameChanged)
    Q_PROPERTY(QDeclarativeListProperty<FacebookNameTagInterface> nameTags READ nameTags NOTIFY nameTagsChanged)
    Q_PROPERTY(QUrl icon READ icon NOTIFY iconChanged)
    Q_PROPERTY(QUrl picture READ picture NOTIFY pictureChanged)
    Q_PROPERTY(QUrl source READ source NOTIFY sourceChanged)
    Q_PROPERTY(int height READ height NOTIFY heightChanged)
    Q_PROPERTY(int width READ width NOTIFY widthChanged)
    Q_PROPERTY(QDeclarativeListProperty<FacebookPhotoImageInterface> images READ images NOTIFY imagesChanged)
    Q_PROPERTY(QUrl link READ link NOTIFY linkChanged)
    Q_PROPERTY(QVariantMap place READ place NOTIFY placeChanged)
    Q_PROPERTY(QString createdTime READ createdTime NOTIFY createdTimeChanged)
    Q_PROPERTY(QString updatedTime READ updatedTime NOTIFY updatedTimeChanged)
    Q_PROPERTY(int position READ position NOTIFY positionChanged)
    Q_PROPERTY(bool liked READ liked NOTIFY likedChanged)
    Q_PROPERTY(int likesCount READ likesCount NOTIFY likesCountChanged)
    Q_PROPERTY(int commentsCount READ commentsCount NOTIFY commentsCountChanged)
public:
    explicit FacebookPhotoInterface(QObject *parent = 0);

    // Overrides.
    int type() const;
    Q_INVOKABLE bool remove();
    Q_INVOKABLE bool reload(const QStringList &whichFields = QStringList());

    // Invokable API.
    Q_INVOKABLE bool like();
    Q_INVOKABLE bool unlike();
    Q_INVOKABLE bool tagUser(const QString &userId, float xOffset, float yOffset);
    Q_INVOKABLE bool untagUser(const QString &userId);
    Q_INVOKABLE bool tagText(const QString &text, float xOffset, float yOffset);
    Q_INVOKABLE bool untagText(const QString &text);
    Q_INVOKABLE bool uploadComment(const QString &message);
    Q_INVOKABLE bool removeComment(const QString &commentIdentifier);

    // Accessors
    QString albumIdentifier() const;
    FacebookObjectReferenceInterface * from() const;
    QDeclarativeListProperty<FacebookPhotoTagInterface> tags();
    QString name() const;
    QDeclarativeListProperty<FacebookNameTagInterface> nameTags();
    QUrl icon() const;
    QUrl picture() const;
    QUrl source() const;
    int height() const;
    int width() const;
    QDeclarativeListProperty<FacebookPhotoImageInterface> images();
    QUrl link() const;
    QVariantMap place() const;
    QString createdTime() const;
    QString updatedTime() const;
    int position() const;
    bool liked() const;
    int likesCount() const;
    int commentsCount() const;
Q_SIGNALS:
    void albumIdentifierChanged();
    void fromChanged();
    void tagsChanged();
    void nameChanged();
    void nameTagsChanged();
    void iconChanged();
    void pictureChanged();
    void sourceChanged();
    void heightChanged();
    void widthChanged();
    void imagesChanged();
    void linkChanged();
    void placeChanged();
    void createdTimeChanged();
    void updatedTimeChanged();
    void positionChanged();
    void likedChanged();
    void likesCountChanged();
    void commentsCountChanged();
private:
    Q_DECLARE_PRIVATE(FacebookPhotoInterface)
};

#endif // FACEBOOKPHOTOINTERFACE_H
