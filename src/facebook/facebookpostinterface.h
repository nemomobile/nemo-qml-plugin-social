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

#ifndef FACEBOOKPOSTINTERFACE_H
#define FACEBOOKPOSTINTERFACE_H

#include "identifiablecontentiteminterface.h"

#if QT_VERSION_5
#include <QtQml/QQmlListProperty>
#define QDeclarativeListProperty QQmlListProperty
#else
#include <QtDeclarative/QDeclarativeListProperty>
#endif
#include "facebookobjectreferenceinterface.h"
#include <QtCore/QString>
#include "facebooknametaginterface.h"
#include <QtCore/QUrl>
#include "facebookpostpropertyinterface.h"
#include "facebookpostactioninterface.h"

/*
 * NOTE: if you construct one of these in C++ directly,
 * you MUST call classBegin() and componentCompleted()
 * directly after construction.
 */

class FacebookPostInterfacePrivate;
class FacebookPostInterface: public IdentifiableContentItemInterface
{
    Q_OBJECT
    Q_PROPERTY(FacebookObjectReferenceInterface * from READ from NOTIFY fromChanged)
    Q_PROPERTY(QDeclarativeListProperty<FacebookObjectReferenceInterface> to READ to NOTIFY toChanged)
    Q_PROPERTY(QString message READ message NOTIFY messageChanged)
    Q_PROPERTY(QDeclarativeListProperty<FacebookNameTagInterface> messageTags READ messageTags NOTIFY messageTagsChanged)
    Q_PROPERTY(QUrl picture READ picture NOTIFY pictureChanged)
    Q_PROPERTY(QUrl link READ link NOTIFY linkChanged)
    Q_PROPERTY(QString name READ name NOTIFY nameChanged)
    Q_PROPERTY(QString caption READ caption NOTIFY captionChanged)
    Q_PROPERTY(QString description READ description NOTIFY descriptionChanged)
    Q_PROPERTY(QUrl source READ source NOTIFY sourceChanged)
    Q_PROPERTY(QDeclarativeListProperty<FacebookPostPropertyInterface> properties READ properties NOTIFY propertiesChanged)
    Q_PROPERTY(QUrl icon READ icon NOTIFY iconChanged)
    Q_PROPERTY(QDeclarativeListProperty<FacebookPostActionInterface> actions READ actions NOTIFY actionsChanged)
    Q_PROPERTY(QString postType READ postType NOTIFY postTypeChanged)
    Q_PROPERTY(QString story READ story NOTIFY storyChanged)
    Q_PROPERTY(QDeclarativeListProperty<FacebookNameTagInterface> storyTags READ storyTags NOTIFY storyTagsChanged)
    Q_PROPERTY(QDeclarativeListProperty<FacebookObjectReferenceInterface> withTags READ withTags NOTIFY withTagsChanged)
    Q_PROPERTY(QString objectIdentifier READ objectIdentifier NOTIFY objectIdentifierChanged)
    Q_PROPERTY(FacebookObjectReferenceInterface * application READ application NOTIFY applicationChanged)
    Q_PROPERTY(QString createdTime READ createdTime NOTIFY createdTimeChanged)
    Q_PROPERTY(QString updatedTime READ updatedTime NOTIFY updatedTimeChanged)
    Q_PROPERTY(int shares READ shares NOTIFY sharesChanged)
    Q_PROPERTY(bool hidden READ hidden NOTIFY hiddenChanged)
    Q_PROPERTY(QString statusType READ statusType NOTIFY statusTypeChanged)
    Q_PROPERTY(bool liked READ liked NOTIFY likedChanged)
    Q_PROPERTY(int likesCount READ likesCount NOTIFY likesCountChanged)
    Q_PROPERTY(int commentsCount READ commentsCount NOTIFY commentsCountChanged)
public:
    explicit FacebookPostInterface(QObject *parent = 0);

    // Overrides.
    int type() const;
#if 0
    Q_INVOKABLE bool remove();
    Q_INVOKABLE bool reload(const QStringList &whichFields = QStringList());

    // Invokable API.
    Q_INVOKABLE bool like();
    Q_INVOKABLE bool unlike();
    Q_INVOKABLE bool uploadComment(const QString &message);
    Q_INVOKABLE bool removeComment(const QString &commentIdentifier);
#endif

    // Accessors
    FacebookObjectReferenceInterface * from() const;
    QDeclarativeListProperty<FacebookObjectReferenceInterface> to();
    QString message() const;
    QDeclarativeListProperty<FacebookNameTagInterface> messageTags();
    QUrl picture() const;
    QUrl link() const;
    QString name() const;
    QString caption() const;
    QString description() const;
    QUrl source() const;
    QDeclarativeListProperty<FacebookPostPropertyInterface> properties();
    QUrl icon() const;
    QDeclarativeListProperty<FacebookPostActionInterface> actions();
    QString postType() const;
    QString story() const;
    QDeclarativeListProperty<FacebookNameTagInterface> storyTags();
    QDeclarativeListProperty<FacebookObjectReferenceInterface> withTags();
    QString objectIdentifier() const;
    FacebookObjectReferenceInterface * application() const;
    QString createdTime() const;
    QString updatedTime() const;
    int shares() const;
    bool hidden() const;
    QString statusType() const;
    bool liked() const;
    int likesCount() const;
    int commentsCount() const;
Q_SIGNALS:
    void fromChanged();
    void toChanged();
    void messageChanged();
    void messageTagsChanged();
    void pictureChanged();
    void linkChanged();
    void nameChanged();
    void captionChanged();
    void descriptionChanged();
    void sourceChanged();
    void propertiesChanged();
    void iconChanged();
    void actionsChanged();
    void postTypeChanged();
    void storyChanged();
    void storyTagsChanged();
    void withTagsChanged();
    void objectIdentifierChanged();
    void applicationChanged();
    void createdTimeChanged();
    void updatedTimeChanged();
    void sharesChanged();
    void hiddenChanged();
    void statusTypeChanged();
    void likedChanged();
    void likesCountChanged();
    void commentsCountChanged();
protected:
    explicit FacebookPostInterface(FacebookPostInterfacePrivate &dd, QObject *parent = 0);
private:
    Q_DECLARE_PRIVATE(FacebookPostInterface)
};

#endif // FACEBOOKPOSTINTERFACE_H
