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

#include <QtCore/QCoreApplication>
#include <QtCore/QDir>
#include <QtCore/QSettings>
#include <QtTest/QtTest>
#include <QtTest/QSignalSpy>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkRequest>
#include "socialnetworkmodelinterface.h"
#include <facebook/facebookontology_p.h>
#include <facebook/facebookinterface.h>
#include <facebook/facebookitemfilterinterface.h>
#include <facebook/facebookrelateddatafilterinterface.h>
#include <facebook/facebookalbuminterface.h>
#include <facebook/facebookuserinterface.h>
#include <facebook/facebookrelateddatafilterinterface.h>
#include <facebook/facebookinterface_p.h>

class SimpleFacebookTests: public QObject
{
    Q_OBJECT
private:
    QString m_token;
    QString m_firstName;
    QString m_lastName;
    QString m_identifier;
private slots:

    void initTestCase()
    {
        QString path = QDir(QCoreApplication::applicationDirPath()).absoluteFilePath("config.ini");
        QSettings settings (path, QSettings::IniFormat);
        QVERIFY(settings.contains("facebook/token"));
        QVERIFY(settings.contains("facebook/first_name"));
        QVERIFY(settings.contains("facebook/last_name"));
        m_token = settings.value("facebook/token").toString();
        m_firstName = settings.value("facebook/first_name").toString();
        m_lastName = settings.value("facebook/last_name").toString();
        QVERIFY(!m_token.isEmpty());
        QVERIFY(!m_firstName.isEmpty());
        QVERIFY(!m_lastName.isEmpty());
    }


    void testQueryMe()
    {
        FacebookInterface *facebook = new FacebookInterface(this);
        facebook->setAccessToken(m_token);
        facebook->classBegin();
        facebook->componentComplete();
        QSignalSpy spy(facebook, SIGNAL(currentUserIdentifierChanged()));

        FacebookItemFilterInterface *filter = new FacebookItemFilterInterface(this);
        filter->setIdentifier("me");
        filter->setFields("id,first_name,last_name");

        FacebookUserInterface *item = new FacebookUserInterface(this);
        item->classBegin();
        item->componentComplete();

        item->setSocialNetwork(facebook);
        item->setFilter(filter);
        QCOMPARE(item->status(), SocialNetworkInterface::Idle);


        QCOMPARE(spy.count(), 0);
        item->load();
        while (item->status() == SocialNetworkInterface::Busy) {
            QTest::qWait(500);
        }

        if (item->status() == SocialNetworkInterface::Error) {
            qWarning() << item->errorMessage();
        }
        QCOMPARE(item->status(), SocialNetworkInterface::Idle); // If it fails here, don't forget to renew the token !
        QCOMPARE(item->firstName(), m_firstName);
        QCOMPARE(item->lastName(), m_lastName);

        QCOMPARE(spy.count(), 1);
        QCOMPARE(facebook->currentUserIdentifier(), item->identifier()); // "me" is now the current user

        m_identifier = facebook->currentUserIdentifier();

        facebook->deleteLater();
        filter->deleteLater();
        item->deleteLater();
    }

    void testQueryOther()
    {
        FacebookInterface *facebook = new FacebookInterface(this);
        facebook->setAccessToken(m_token);
        facebook->classBegin();
        facebook->componentComplete();
        QSignalSpy spy(facebook, SIGNAL(currentUserIdentifierChanged()));

        FacebookItemFilterInterface *filter = new FacebookItemFilterInterface(this);
        filter->setIdentifier("324048634313353"); // This is Jolla's page
        filter->setFields("id,name");

        IdentifiableContentItemInterface *item = new IdentifiableContentItemInterface(this);
        item->classBegin();
        item->componentComplete();

        item->setSocialNetwork(facebook);
        item->setFilter(filter);
        QCOMPARE(item->status(), SocialNetworkInterface::Idle);

        QCOMPARE(spy.count(), 0);
        item->load();
        while (item->status() == SocialNetworkInterface::Busy) {
            QTest::qWait(500);
        }

        if (item->status() == SocialNetworkInterface::Error) {
            qWarning() << item->errorMessage();
        }
        QCOMPARE(item->status(), SocialNetworkInterface::Idle); // Fails with Qt 4.8.5 (Fedora)
        // This test might fail if Jolla changes name
        QCOMPARE(item->data().value("name").toString(), QString("Jolla"));

        QCOMPARE(spy.count(), 1);
        QCOMPARE(facebook->currentUserIdentifier(), m_identifier); // "me" is now the current user

        facebook->deleteLater();
        filter->deleteLater();
        item->deleteLater();
    }

    void testChangeId()
    {
        FacebookInterface *facebook = new FacebookInterface(this);
        facebook->setAccessToken(m_token);
        facebook->classBegin();
        facebook->componentComplete();

        FacebookItemFilterInterface *filter = new FacebookItemFilterInterface(this);
        filter->setIdentifier("100005045024027"); // "Fake Chris"
        filter->setFields("id,name");

        FacebookUserInterface *item = new FacebookUserInterface(this);
        item->classBegin();
        item->componentComplete();

        item->setSocialNetwork(facebook);
        item->setFilter(filter);
        item->load();
        while (item->status() == SocialNetworkInterface::Busy) {
            QTest::qWait(500);
        }

        if (item->status() == SocialNetworkInterface::Error) {
            qWarning() << item->errorMessage();
        }
        QCOMPARE(item->status(), SocialNetworkInterface::Idle);
        // This test might fail if Chris changes name
        QCOMPARE(item->data().value("name").toString(), QString("Chris Adams"));

        // Now changes to another user
        filter->setIdentifier("100004850591232"); // "Fake Lucien"

        item->load();
        while (item->status() == SocialNetworkInterface::Busy) {
            QTest::qWait(500);
        }

        if (item->status() == SocialNetworkInterface::Error) {
            qWarning() << item->errorMessage();
        }
        QCOMPARE(item->status(), SocialNetworkInterface::Idle);
        // This test might fail if Lucien changes name
        QCOMPARE(item->data().value("name").toString(), QString("Lucien El-test Xu"));

        facebook->deleteLater();
        filter->deleteLater();
        item->deleteLater();
    }

    void testSimpleModel()
    {
        FacebookInterface *facebook = new FacebookInterface(this);
        facebook->setAccessToken(m_token);
        facebook->classBegin();
        facebook->componentComplete();

        FacebookRelatedDataFilterInterface *filter = new FacebookRelatedDataFilterInterface(this);
        filter->setIdentifier(m_identifier);
        filter->setFields("id,name,username");
        filter->setConnection(FacebookInterface::Friends);

        SocialNetworkModelInterface *model = new SocialNetworkModelInterface(this);
        model->classBegin();
        model->componentComplete();
        model->setSocialNetwork(facebook);

        model->setFilter(filter);
        QVERIFY(model->load());

        while (model->status() == SocialNetworkInterface::Busy) {
            QTest::qWait(500);
        }

        if (model->status() == SocialNetworkInterface::Error) {
            qWarning() << model->errorMessage();
        }
        QCOMPARE(model->status(), SocialNetworkInterface::Idle);
        if (model->count() > 0) {
            QObject *first = model->relatedItem(0);
            FacebookUserInterface *item = qobject_cast<FacebookUserInterface *>(first);
            QVERIFY(item);

            if (item) {
                qWarning() << "Some friends:";
                for (int i = 0; i < qMin(model->count(), 5); i++) {
                    QObject *itemObject = model->relatedItem(i);
                    item = qobject_cast<FacebookUserInterface *>(itemObject);
                    qWarning() << item->name() << item->username();
                }
            }
        }

        facebook->deleteLater();
        filter->deleteLater();
        model->deleteLater();
    }

    void testQuerySelf()
    {
        // We need to test if we can get self with those four queries
        // ICII, id = self / other
        // SNMI, id = self / other

        // First, let's try to load a simple ICII
        FacebookInterface *facebook = new FacebookInterface(this);
        facebook->setAccessToken(m_token);
        facebook->classBegin();
        facebook->componentComplete();

        FacebookItemFilterInterface *filter1 = new FacebookItemFilterInterface(this);
        filter1->setIdentifier("me");
        filter1->setFields("name"); // We are forcing the non retrieving of the id but it should work

        FacebookUserInterface *item = new FacebookUserInterface(this);
        item->classBegin();
        item->componentComplete();

        item->setSocialNetwork(facebook);
        item->setFilter(filter1);


        item->load();
        while (item->status() == SocialNetworkInterface::Busy) {
            QTest::qWait(500);
        }

        if (item->status() == SocialNetworkInterface::Error) {
            qWarning() << item->errorMessage();
        }
        QCOMPARE(item->status(), SocialNetworkInterface::Idle);
        QCOMPARE(facebook->currentUserIdentifier(), m_identifier);
        QCOMPARE(item->identifier(), m_identifier);

        facebook->deleteLater();
        filter1->deleteLater();
        item->deleteLater();

        // Let's load Fake Chris now
        facebook = new FacebookInterface(this);
        facebook->setAccessToken(m_token);
        facebook->classBegin();
        facebook->componentComplete();

        filter1 = new FacebookItemFilterInterface(this);
        filter1->setIdentifier("100005045024027");
        filter1->setFields("name");

        item = new FacebookUserInterface(this);
        item->classBegin();
        item->componentComplete();

        item->setSocialNetwork(facebook);
        item->setFilter(filter1);

        item->load();
        while (item->status() == SocialNetworkInterface::Busy) {
            QTest::qWait(500);
        }

        if (item->status() == SocialNetworkInterface::Error) {
            qWarning() << item->errorMessage();
        }
        QCOMPARE(item->status(), SocialNetworkInterface::Idle);
        QCOMPARE(facebook->currentUserIdentifier(), m_identifier);

        facebook->deleteLater();
        filter1->deleteLater();
        item->deleteLater();

        // Let's load my friends
        facebook = new FacebookInterface(this);
        facebook->setAccessToken(m_token);
        facebook->classBegin();
        facebook->componentComplete();

        FacebookRelatedDataFilterInterface *filter2 = new FacebookRelatedDataFilterInterface(this);
        filter2->setIdentifier("me");
        filter2->setFields("id,name");
        filter2->setConnection(FacebookInterface::Friends);

        SocialNetworkModelInterface *model = new SocialNetworkModelInterface(this);
        model->classBegin();
        model->componentComplete();
        model->setSocialNetwork(facebook);

        model->setFilter(filter2);
        QVERIFY(model->load());

        while (model->status() == SocialNetworkInterface::Busy) {
            QTest::qWait(500);
        }

        if (model->status() == SocialNetworkInterface::Error) {
            qWarning() << model->errorMessage();
        }
        QCOMPARE(model->status(), SocialNetworkInterface::Idle);
        QCOMPARE(facebook->currentUserIdentifier(), m_identifier);

        facebook->deleteLater();
        filter2->deleteLater();
        model->deleteLater();

        // Let's load Fake Chris's albums
        facebook = new FacebookInterface(this);
        facebook->setAccessToken(m_token);
        facebook->classBegin();
        facebook->componentComplete();

        filter2 = new FacebookRelatedDataFilterInterface(this);
        filter2->setIdentifier("100005045024027");
        filter2->setFields("id,name");
        filter2->setConnection(FacebookInterface::Albums);

        model = new SocialNetworkModelInterface(this);
        model->classBegin();
        model->componentComplete();
        model->setSocialNetwork(facebook);

        model->setFilter(filter2);
        QVERIFY(model->load());

        while (model->status() == SocialNetworkInterface::Busy) {
            QTest::qWait(500);
        }

        if (model->status() == SocialNetworkInterface::Error) {
            qWarning() << model->errorMessage();
        }
        QCOMPARE(model->status(), SocialNetworkInterface::Idle);
        QCOMPARE(facebook->currentUserIdentifier(), m_identifier);

        facebook->deleteLater();
        filter2->deleteLater();
        model->deleteLater();
    }

    void testFieldMaker()
    {
        // 0 = empty argument
        // 1 = non-empty

        // 0 0 0
        QString str = FacebookInterfacePrivate::makeFields(QString(), QString(),
                                                           QMap<QString, QString>());
        QVERIFY(str.isEmpty());

        // 1 0 0
        str = FacebookInterfacePrivate::makeFields("friends", QString(), QMap<QString, QString>());
        QCOMPARE(str, QLatin1String("id,friends"));

        // 0 1 0
        str = FacebookInterfacePrivate::makeFields(QString(), "id,name", QMap<QString, QString>());
        QCOMPARE(str, QLatin1String("id,name"));

        // 0 0 1
        QMap<QString, QString> arguments;
        arguments.insert("key", "value");
        str = FacebookInterfacePrivate::makeFields(QString(), QString(), arguments);
        QVERIFY(str.isEmpty());

        // 1 1 0
        str = FacebookInterfacePrivate::makeFields("friends", "id,name", QMap<QString, QString>());
        QCOMPARE(str, QLatin1String("id,friends.fields(id,name)"));

        // 1 0 1
        str = FacebookInterfacePrivate::makeFields("friends", QString(), arguments);
        QCOMPARE(str, QLatin1String("id,friends.key(value)"));

        // 0 1 1
        str = FacebookInterfacePrivate::makeFields(QString(), "id,name", arguments);
        QCOMPARE(str, QLatin1String("id,name"));

        // 1 1 1
        str = FacebookInterfacePrivate::makeFields("friends", "id,name", arguments);
        QCOMPARE(str, QLatin1String("id,friends.fields(id,name).key(value)"));

        // Test the fields parser
        // Spaces
        str = FacebookInterfacePrivate::makeFields(QString(), "     id    ,    name  ",
                                                   QMap<QString, QString>());
        QCOMPARE(str, QLatin1String("id,name"));

        // Empty values
        str = FacebookInterfacePrivate::makeFields(QString(), ",,id,,,name,,",
                                                   QMap<QString, QString>());
        QCOMPARE(str, QLatin1String("id,name"));

        // Likes and comments summary
        str = FacebookInterfacePrivate::makeFields(QString(), "likes",
                                                   QMap<QString, QString>());
        QCOMPARE(str, QLatin1String("likes.summary(1)"));

        str = FacebookInterfacePrivate::makeFields(QString(), "comments",
                                                   QMap<QString, QString>());
        QCOMPARE(str, QLatin1String("comments.summary(1)"));

        str = FacebookInterfacePrivate::makeFields(QString(), "something,likes,somethingelse,comments,likestest,commentstest",
                                                   QMap<QString, QString>());
        QCOMPARE(str, QLatin1String("something,likes.summary(1),somethingelse,comments.summary(1),likestest,commentstest"));
    }

    void testLikeAction()
    {
        // Let's load a post
        // (Jolla is prebooked)
        FacebookInterface *facebook = new FacebookInterface(this);
        facebook->setAccessToken(m_token);
        facebook->classBegin();
        facebook->componentComplete();

        FacebookItemFilterInterface *filter = new FacebookItemFilterInterface(this);
        filter->setIdentifier("324048634313353_603842659667281");
        filter->setFields("id,likes");

        FacebookPostInterface *item = new FacebookPostInterface(this);
        item->classBegin();
        item->componentComplete();

        item->setSocialNetwork(facebook);
        item->setFilter(filter);
        item->load();
        while (item->status() == SocialNetworkInterface::Busy) {
            QTest::qWait(500);
        }
        QCOMPARE(item->status(), SocialNetworkInterface::Idle);


        for (int i = 0; i < 2; ++i) {
            // Try to send a like / unlike
            bool liked = item->liked();
            qWarning() << "The post is currently liked (before operation):" << liked;
            int likesCount = item->likesCount();
            if (!liked) {
                QVERIFY(item->like());
            } else {
                QVERIFY(item->unlike());
            }

            while (item->actionStatus() == SocialNetworkInterface::Busy) {
                QTest::qWait(500);
            }
            QCOMPARE(item->actionStatus(), SocialNetworkInterface::Idle);

            if (!liked) {
                QCOMPARE(item->likesCount(), likesCount + 1);
                QVERIFY(item->liked());
            } else {
                QCOMPARE(item->likesCount(), likesCount - 1);
                QVERIFY(!item->liked());
            }
        }


        // Let's try to comment
        QString comment = "Test message please ignore";
        item->uploadComment(comment);
        while (item->actionStatus() == SocialNetworkInterface::Busy) {
            QTest::qWait(500);
        }
        QCOMPARE(item->actionStatus(), SocialNetworkInterface::Idle);

        // Fetch the comment list
        FacebookRelatedDataFilterInterface *filter2 = new FacebookRelatedDataFilterInterface(this);
        filter2->setIdentifier("324048634313353_603842659667281");
        filter2->setFields("id,message");
        filter2->setOffset(item->commentsCount());
        filter2->setConnection(FacebookInterface::Comments);


        SocialNetworkModelInterface *model = new SocialNetworkModelInterface(this);
        model->classBegin();
        model->componentComplete();
        model->setSocialNetwork(facebook);

        model->setFilter(filter2);
        QVERIFY(model->load());

        while (model->status() == SocialNetworkInterface::Busy) {
            QTest::qWait(500);
        }
        QCOMPARE(model->status(), SocialNetworkInterface::Idle);

        QString commentIdentifier;
        for (int i = 0; i < model->count(); i++) {
            QObject *itemObject = model->relatedItem(i);
            FacebookCommentInterface *item = qobject_cast<FacebookCommentInterface *>(itemObject);
            if (item->message() == comment) {
                commentIdentifier = item->identifier();
            }
        }

        QVERIFY(!commentIdentifier.isEmpty());
        qWarning() << "Found comment identifier:" << commentIdentifier;

        // Remove the posted comment
        item->removeComment(commentIdentifier);
        while (item->actionStatus() == SocialNetworkInterface::Busy) {
            QTest::qWait(500);
        }
        QCOMPARE(item->actionStatus(), SocialNetworkInterface::Idle);
    }

    void testAlbumActions() // This do not test expected failures
    {
        // Flow is: create album, like and comment it
        // upload a photo
        // like and comment the photo
        // tag and untag the photo
        // remove the photo
        // remove album: not possible

        QString albumName = "Test album please ignore";
        qWarning() << "Be sure that you don't have an album called" << albumName;

        // 0. Get user
        FacebookInterface *facebook = new FacebookInterface(this);
        facebook->setAccessToken(m_token);
        facebook->classBegin();
        facebook->componentComplete();

        FacebookItemFilterInterface *filter = new FacebookItemFilterInterface(this);
        filter->setIdentifier(m_identifier);
        filter->setFields("id");

        FacebookUserInterface *item = new FacebookUserInterface(this);
        item->classBegin();
        item->componentComplete();

        item->setSocialNetwork(facebook);
        item->setFilter(filter);
        item->load();
        while (item->status() == SocialNetworkInterface::Busy) {
            QTest::qWait(500);
        }
        QCOMPARE(item->status(), SocialNetworkInterface::Idle);

        // 1. Create album
        QVERIFY(item->uploadAlbum(albumName));
        while (item->actionStatus() == SocialNetworkInterface::Busy) {
            QTest::qWait(500);
        }
        QCOMPARE(item->actionStatus(), SocialNetworkInterface::Idle);

        // 2. Like and unlike the album
        // Fetch the album list
        FacebookRelatedDataFilterInterface *albumListFilter = new FacebookRelatedDataFilterInterface(this);
        albumListFilter->setIdentifier(m_identifier);
        albumListFilter->setFields("id,name");
        albumListFilter->setConnection(FacebookInterface::Albums);


        SocialNetworkModelInterface *albumModel = new SocialNetworkModelInterface(this);
        albumModel->classBegin();
        albumModel->componentComplete();
        albumModel->setSocialNetwork(facebook);

        albumModel->setFilter(albumListFilter);
        QVERIFY(albumModel->load());

        while (albumModel->status() == SocialNetworkInterface::Busy) {
            QTest::qWait(500);
        }
        QCOMPARE(albumModel->status(), SocialNetworkInterface::Idle);

        FacebookAlbumInterface *album = 0;
        for (int i = 0; i < albumModel->count(); i++) {
            QObject *itemObject = albumModel->relatedItem(i);
            FacebookAlbumInterface *item = qobject_cast<FacebookAlbumInterface *>(itemObject);
            if (item->name() == albumName) {
                album = item;
            }
        }

        QVERIFY(album);
        qWarning() << "Found album identifier:" << album->identifier();

        // Likes check
        for (int i = 0; i < 2; ++i) {
            // Try to send a like / unlike
            bool liked = album->liked();
            qWarning() << "The album is currently liked (before operation):" << liked;
            int likesCount = album->likesCount();
            if (!liked) {
                QVERIFY(album->like());
            } else {
                QVERIFY(album->unlike());
            }

            while (album->actionStatus() == SocialNetworkInterface::Busy) {
                QTest::qWait(500);
            }
            QCOMPARE(album->actionStatus(), SocialNetworkInterface::Idle);

            if (!liked) {
                QCOMPARE(album->likesCount(), likesCount + 1);
                QVERIFY(album->liked());
            } else {
                QCOMPARE(album->likesCount(), likesCount - 1);
                QVERIFY(!album->liked());
            }
        }

        // 3. Comment and uncomment the album
        QString comment = "Test message please ignore";
        QVERIFY(album->uploadComment(comment));
        while (album->actionStatus() == SocialNetworkInterface::Busy) {
            QTest::qWait(500);
        }
        QCOMPARE(album->actionStatus(), SocialNetworkInterface::Idle);

        // Fetch the comment list
        FacebookRelatedDataFilterInterface *commentFilter = new FacebookRelatedDataFilterInterface(this);
        commentFilter->setIdentifier(album->identifier());
        commentFilter->setFields("id,message");
        commentFilter->setConnection(FacebookInterface::Comments);


        SocialNetworkModelInterface *commentModel = new SocialNetworkModelInterface(this);
        commentModel->classBegin();
        commentModel->componentComplete();
        commentModel->setSocialNetwork(facebook);

        commentModel->setFilter(commentFilter);
        QVERIFY(commentModel->load());

        while (commentModel->status() == SocialNetworkInterface::Busy) {
            QTest::qWait(500);
        }
        QCOMPARE(commentModel->status(), SocialNetworkInterface::Idle);

        QString commentIdentifier;
        for (int i = 0; i < commentModel->count(); i++) {
            QObject *itemObject = commentModel->relatedItem(i);
            FacebookCommentInterface *item = qobject_cast<FacebookCommentInterface *>(itemObject);
            if (item->message() == comment) {
                commentIdentifier = item->identifier();
            }
        }

        QVERIFY(!commentIdentifier.isEmpty());
        qWarning() << "Found comment identifier:" << commentIdentifier;

        // Remove the posted comment
        QVERIFY(album->removeComment(commentIdentifier));
        while (album->actionStatus() == SocialNetworkInterface::Busy) {
            QTest::qWait(500);
        }
        QCOMPARE(album->actionStatus(), SocialNetworkInterface::Idle);

        // 4. Upload a photo
        QVERIFY(album->uploadPhoto(":/test.jpg"));
        while (album->actionStatus() == SocialNetworkInterface::Busy) {
            QTest::qWait(500);
        }
        QCOMPARE(album->actionStatus(), SocialNetworkInterface::Idle);

        // 5. Like and unlike the photo
        // Fetch the photo list
        FacebookRelatedDataFilterInterface *photoListFilter = new FacebookRelatedDataFilterInterface(this);
        photoListFilter->setIdentifier(album->identifier());
        photoListFilter->setFields("id,name");
        photoListFilter->setConnection(FacebookInterface::Photos);


        SocialNetworkModelInterface *photoModel = new SocialNetworkModelInterface(this);
        photoModel->classBegin();
        photoModel->componentComplete();
        photoModel->setSocialNetwork(facebook);

        photoModel->setFilter(photoListFilter);
        QVERIFY(photoModel->load());

        while (photoModel->status() == SocialNetworkInterface::Busy) {
            QTest::qWait(500);
        }
        QCOMPARE(photoModel->status(), SocialNetworkInterface::Idle);

        FacebookPhotoInterface *photo = 0;
        QVERIFY(photoModel->count() > 0);
        photo = qobject_cast<FacebookPhotoInterface *>(photoModel->relatedItem(0));
        QVERIFY(photo);

        // Likes check
        for (int i = 0; i < 2; ++i) {
            // Try to send a like / unlike
            bool liked = photo->liked();
            qWarning() << "The photo is currently liked (before operation):" << liked;
            int likesCount = photo->likesCount();
            if (!liked) {
                QVERIFY(photo->like());
            } else {
                QVERIFY(photo->unlike());
            }

            while (photo->actionStatus() == SocialNetworkInterface::Busy) {
                QTest::qWait(500);
            }
            QCOMPARE(photo->actionStatus(), SocialNetworkInterface::Idle);

            if (!liked) {
                QCOMPARE(photo->likesCount(), likesCount + 1);
                QVERIFY(photo->liked());
            } else {
                QCOMPARE(photo->likesCount(), likesCount - 1);
                QVERIFY(!photo->liked());
            }
        }

        // 6. Comment and uncomment the photo
        QVERIFY(photo->uploadComment(comment));
        while (photo->actionStatus() == SocialNetworkInterface::Busy) {
            QTest::qWait(500);
        }
        QCOMPARE(photo->actionStatus(), SocialNetworkInterface::Idle);

        // Fetch the comment list
        commentFilter->setIdentifier(photo->identifier());
        commentFilter->setFields("id,message");
        commentFilter->setConnection(FacebookInterface::Comments);

        commentModel->setFilter(commentFilter);
        QVERIFY(commentModel->load());

        while (commentModel->status() == SocialNetworkInterface::Busy) {
            QTest::qWait(500);
        }
        QCOMPARE(commentModel->status(), SocialNetworkInterface::Idle);

        commentIdentifier.clear();
        for (int i = 0; i < commentModel->count(); i++) {
            QObject *itemObject = commentModel->relatedItem(i);
            FacebookCommentInterface *item = qobject_cast<FacebookCommentInterface *>(itemObject);
            if (item->message() == comment) {
                commentIdentifier = item->identifier();
            }
        }

        QVERIFY(!commentIdentifier.isEmpty());
        qWarning() << "Found comment identifier:" << commentIdentifier;

        // Remove the posted comment
        QVERIFY(photo->removeComment(commentIdentifier));
        while (photo->actionStatus() == SocialNetworkInterface::Busy) {
            QTest::qWait(500);
        }
        QCOMPARE(photo->actionStatus(), SocialNetworkInterface::Idle);

        // 7. Tag user on the photo
        // Let's tag Fake Chris. This will fail if Fake Chris is not your friend.
        QVERIFY(photo->tagUser("100005045024027", 0.5, 0.5));
        while (photo->actionStatus() == SocialNetworkInterface::Busy) {
            QTest::qWait(500);
        }
        QCOMPARE(photo->actionStatus(), SocialNetworkInterface::Idle);

        // Let's also tag something meaningless
        QString tagMessage = "Test tag";
        QVERIFY(photo->tagText(tagMessage, 0.25, 0.75));
        while (photo->actionStatus() == SocialNetworkInterface::Busy) {
            QTest::qWait(500);
        }
        QCOMPARE(photo->actionStatus(), SocialNetworkInterface::Idle);

        // 8. Delete tag
        QVERIFY(photo->untagUser("100005045024027"));
        while (photo->actionStatus() == SocialNetworkInterface::Busy) {
            QTest::qWait(500);
        }
        // Facebook API do not support this
        QEXPECT_FAIL("", "Facebook API limits this call", Continue);
        QCOMPARE(photo->actionStatus(), SocialNetworkInterface::Idle);

        QVERIFY(photo->untagText(tagMessage));
        while (photo->actionStatus() == SocialNetworkInterface::Busy) {
            QTest::qWait(500);
        }
        // Facebook API do not support this
        QEXPECT_FAIL("", "Facebook API limits this call", Continue);
        QCOMPARE(photo->actionStatus(), SocialNetworkInterface::Idle);

        // 9. Delete photo
        album->removePhoto(photo->identifier());
        while (item->actionStatus() == SocialNetworkInterface::Busy) {
            QTest::qWait(500);
        }
        QCOMPARE(item->actionStatus(), SocialNetworkInterface::Idle);

        // 10. Delete album
        item->removeAlbum(album->identifier());
        while (item->actionStatus() == SocialNetworkInterface::Busy) {
            QTest::qWait(500);
        }
        // Facebook API do not support this
        QEXPECT_FAIL("", "Facebook API limits this call", Continue);
        QCOMPARE(item->actionStatus(), SocialNetworkInterface::Idle);
    }

    void testAlbumActionsExpectedFailure()
    {
        // TODO
    }
};

QTEST_MAIN(SimpleFacebookTests)

#include "main.moc"
