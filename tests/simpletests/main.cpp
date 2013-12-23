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

#include <QtTest/QtTest>
#include <QtTest/QSignalSpy>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkRequest>
#include <socialnetworkinterface.h>
#include <socialnetworkinterface_p.h>
#include <socialnetworkmodelinterface.h>
#include <contentiteminterface.h>
#include <contentiteminterface_p.h>
#include <identifiablecontentiteminterface.h>
#include <identifiablecontentiteminterface_p.h>
#include <filterinterface.h>
#include <filterinterface_p.h>

static const char *DATA_KEY = "data";
static const int DATA_VALUE = 42;

// A set of simple tests that covers tricky areas, like initialization
// flow of components, and destruction of one of them when loading.

// TODO write tests for sorters

class TestSocialNetworkInterface: public SocialNetworkInterface
{
    Q_OBJECT
public:
    explicit TestSocialNetworkInterface(QObject *parent = 0);
    QObject * get(FilterInterface *filter);
private:
    Q_DECLARE_PRIVATE(SocialNetworkInterface)
};

TestSocialNetworkInterface::TestSocialNetworkInterface(QObject *parent)
    : SocialNetworkInterface(parent)
{
}

QObject *TestSocialNetworkInterface::get(FilterInterface *filter)
{
    Q_D(SocialNetworkInterface);
    QNetworkReply *reply = d->networkAccessManager->get(QNetworkRequest(QUrl("http://jolla.com")));
    d->runReply(reply, filter);
    return reply;
}

class TestFilterInterface: public FilterInterface
{
    Q_OBJECT
public:
    explicit TestFilterInterface(QObject *parent = 0);
    bool isAcceptable(QObject *item, SocialNetworkInterface *socialNetwork) const;
protected:
    bool performLoadRequestImpl(QObject *item, SocialNetworkInterface *socialNetwork,
                                LoadType loadType);
    bool performSetItemDataImpl(IdentifiableContentItemInterface *item,
                                SocialNetworkInterface *socialNetwork,
                                const QByteArray &data, LoadType loadType,
                                const QVariantMap &properties);
    bool performSetModelDataImpl(SocialNetworkModelInterface *model,
                                 SocialNetworkInterface *socialNetwork, const QByteArray &data,
                                 LoadType loadType, const QVariantMap &properties);
private:
    Q_DECLARE_PRIVATE(FilterInterface)
};

TestFilterInterface::TestFilterInterface(QObject *parent)
    : FilterInterface(parent)
{
}

bool TestFilterInterface::isAcceptable(QObject *item, SocialNetworkInterface *socialNetwork) const
{
    Q_UNUSED(socialNetwork)
    return qobject_cast<IdentifiableContentItemInterface *>(item)
           || qobject_cast<SocialNetworkModelInterface *>(item);
}

bool TestFilterInterface::performLoadRequestImpl(QObject *item,
                                                 SocialNetworkInterface *socialNetwork,
                                                 LoadType loadType)
{
    Q_D(FilterInterface);
    TestSocialNetworkInterface *testSni = qobject_cast<TestSocialNetworkInterface *>(socialNetwork);
    if (!testSni) {
        return false;
    }

    return d->addHandle(testSni->get(this), item, socialNetwork, loadType);
}

bool TestFilterInterface::performSetItemDataImpl(IdentifiableContentItemInterface *item,
                                                 SocialNetworkInterface *socialNetwork,
                                                 const QByteArray &data,
                                                 LoadType loadType,
                                                 const QVariantMap &properties)
{
    Q_UNUSED(socialNetwork)
    Q_UNUSED(data)
    Q_UNUSED(loadType)
    Q_UNUSED(properties)
    QVariantMap dataMap;
    dataMap.insert(NEMOQMLPLUGINS_SOCIAL_CONTENTITEMID, 0);
    dataMap.insert(DATA_KEY, DATA_VALUE);
    item->setData(dataMap);
    return true;
}

bool TestFilterInterface::performSetModelDataImpl(SocialNetworkModelInterface *model,
                                                  SocialNetworkInterface *socialNetwork,
                                                  const QByteArray &data, LoadType loadType,
                                                  const QVariantMap &properties)
{
    Q_UNUSED(model)
    Q_UNUSED(socialNetwork)
    Q_UNUSED(data)
    Q_UNUSED(loadType)
    Q_UNUSED(properties)
    return true;
}

class TestContentItemInterfacePrivate;
class TestContentItemInterface: public ContentItemInterface
{
    Q_OBJECT
public:
    explicit TestContentItemInterface(QObject *parent = 0);
    bool isComplete() const;
private:
    Q_DECLARE_PRIVATE(TestContentItemInterface)
};

class TestContentItemInterfacePrivate: public ContentItemInterfacePrivate
{
public:
    explicit TestContentItemInterfacePrivate(TestContentItemInterface * q);
private:
    void initializationIncomplete();
    void initializationComplete();
    bool initializationCompleteValue;
    Q_DECLARE_PUBLIC(TestContentItemInterface)
};

TestContentItemInterfacePrivate::TestContentItemInterfacePrivate(TestContentItemInterface *q)
    : ContentItemInterfacePrivate(q), initializationCompleteValue(false)
{
}

void TestContentItemInterfacePrivate::initializationIncomplete()
{
    initializationCompleteValue = false;
    ContentItemInterfacePrivate::initializationIncomplete();
}

void TestContentItemInterfacePrivate::initializationComplete()
{
    initializationCompleteValue = true;
    ContentItemInterfacePrivate::initializationComplete();
}

TestContentItemInterface::TestContentItemInterface(QObject *parent)
    : ContentItemInterface(*(new TestContentItemInterfacePrivate(this)), parent)
{
}

bool TestContentItemInterface::isComplete() const
{
    Q_D(const TestContentItemInterface);
    return d->initializationCompleteValue;
}


class SimpleTests: public QObject
{
    Q_OBJECT
private slots:
    void testSniInit()
    {
        // Initialization tests for SNI
        SocialNetworkInterface *sni = new SocialNetworkInterface(this);
        QSignalSpy sniSpy(sni, SIGNAL(initializedChanged()));
        QVERIFY(!sni->isInitialized());
        QCOMPARE(sniSpy.count(), 0);
        sni->classBegin();
        QVERIFY(!sni->isInitialized());
        QCOMPARE(sniSpy.count(), 0);
        sni->componentComplete();
        QVERIFY(sni->isInitialized());
        QCOMPARE(sniSpy.count(), 1);
        QVERIFY(sniSpy.takeFirst().isEmpty());
        sni->deleteLater();
    }

    void testCiiInit()
    {
        // Initialization tests for CII
        // Four cases to test
        // 1. sni not complete + cii not complete : not complete
        // 2. sni not complete + cii complete : not complete
        // 3. sni complete + cii not complete : not complete
        // 4. sni complete + cii complete : complete
        //
        // for 1. we need to test after completing sni and cii, cii and sni
        // for 2. we need to test after completing sni
        // for 3. we need to test after completing cii
        TestContentItemInterface *cii = new TestContentItemInterface(this);
        QVERIFY(!cii->isComplete());
        cii->classBegin();
        QVERIFY(!cii->isComplete());
        cii->componentComplete();
        QVERIFY(!cii->isComplete());
        cii->deleteLater();

        SocialNetworkInterface *sni1a = new SocialNetworkInterface(this);
        TestContentItemInterface *cii1a = new TestContentItemInterface(this);
        cii1a->setSocialNetwork(sni1a);
        QVERIFY(!cii1a->isComplete());
        sni1a->classBegin();
        QVERIFY(!cii1a->isComplete());
        sni1a->componentComplete();
        QVERIFY(!cii1a->isComplete());
        cii1a->classBegin();
        QVERIFY(!cii1a->isComplete());
        cii1a->componentComplete();
        QVERIFY(cii1a->isComplete());
        cii1a->deleteLater();
        sni1a->deleteLater();

        SocialNetworkInterface *sni1b = new SocialNetworkInterface(this);
        TestContentItemInterface *cii1b = new TestContentItemInterface(this);
        cii1b->setSocialNetwork(sni1b);
        QVERIFY(!cii1b->isComplete());
        cii1b->classBegin();
        QVERIFY(!cii1b->isComplete());
        cii1b->componentComplete();
        QVERIFY(!cii1b->isComplete());
        sni1b->classBegin();
        QVERIFY(!cii1b->isComplete());
        sni1b->componentComplete();
        QVERIFY(cii1b->isComplete());
        cii1b->deleteLater();
        sni1b->deleteLater();


        SocialNetworkInterface *sni2 = new SocialNetworkInterface(this);
        TestContentItemInterface *cii2 = new TestContentItemInterface(this);
        cii2->classBegin();
        cii2->componentComplete();
        cii2->setSocialNetwork(sni2);
        QVERIFY(!cii2->isComplete());
        sni2->classBegin();
        QVERIFY(!cii2->isComplete());
        sni2->componentComplete();
        QVERIFY(cii2->isComplete());
        cii2->deleteLater();
        sni2->deleteLater();

        SocialNetworkInterface *sni3 = new SocialNetworkInterface(this);
        TestContentItemInterface *cii3 = new TestContentItemInterface(this);
        sni3->classBegin();
        sni3->componentComplete();
        cii3->setSocialNetwork(sni3);
        QVERIFY(!cii3->isComplete());
        cii3->classBegin();
        QVERIFY(!cii3->isComplete());
        cii3->componentComplete();
        QVERIFY(cii3->isComplete());
        cii3->deleteLater();
        sni3->deleteLater();

        SocialNetworkInterface *sni4 = new SocialNetworkInterface(this);
        TestContentItemInterface *cii4 = new TestContentItemInterface(this);
        sni4->classBegin();
        sni4->componentComplete();
        cii4->classBegin();
        cii4->componentComplete();
        cii4->setSocialNetwork(sni4);
        QVERIFY(cii4->isComplete());
        cii4->deleteLater();
        sni4->deleteLater();

        // Test that we goes back to init state when setting a new SNI
        SocialNetworkInterface * sni1 = new SocialNetworkInterface(this);
        sni2 = new SocialNetworkInterface(this);
        TestContentItemInterface * cii1 = new TestContentItemInterface(this);
        sni1->classBegin();
        sni1->componentComplete();
        cii1->classBegin();
        cii1->componentComplete();
        cii1->setSocialNetwork(sni1);
        QVERIFY(cii1->isComplete());
        QCOMPARE(cii1->socialNetwork(), sni1);
        cii1->setSocialNetwork(sni2);
        QCOMPARE(cii1->socialNetwork(), sni2);
        QVERIFY(!cii1->isComplete());
        sni2->classBegin();
        sni2->componentComplete();
        QVERIFY(cii1->isComplete());
        cii1->deleteLater();
        sni1->deleteLater();
        sni2->deleteLater();
    }

    void testSnmiInit()
    {
        // Initialization tests for CII
        // Same four cases to test
        SocialNetworkModelInterface *snmi = new SocialNetworkModelInterface(this);
        QCOMPARE(snmi->status(), SocialNetworkInterface::Initializing);
        snmi->classBegin();
        QCOMPARE(snmi->status(), SocialNetworkInterface::Initializing);
        snmi->componentComplete();
        QCOMPARE(snmi->status(), SocialNetworkInterface::Initializing);
        snmi->deleteLater();


        SocialNetworkInterface *sni1a = new SocialNetworkInterface(this);
        SocialNetworkModelInterface *snmi1a = new SocialNetworkModelInterface(this);
        QSignalSpy spy1a (snmi1a, SIGNAL(statusChanged()));
        snmi1a->setSocialNetwork(sni1a);
        QCOMPARE(snmi1a->status(), SocialNetworkInterface::Initializing);
        QCOMPARE(spy1a.count(), 0);
        sni1a->classBegin();
        QCOMPARE(snmi1a->status(), SocialNetworkInterface::Initializing);
        QCOMPARE(spy1a.count(), 0);
        sni1a->componentComplete();
        QCOMPARE(snmi1a->status(), SocialNetworkInterface::Initializing);
        QCOMPARE(spy1a.count(), 0);
        snmi1a->classBegin();
        QCOMPARE(snmi1a->status(), SocialNetworkInterface::Initializing);
        QCOMPARE(spy1a.count(), 0);
        snmi1a->componentComplete();
        QCOMPARE(snmi1a->status(), SocialNetworkInterface::Idle);
        QCOMPARE(spy1a.count(), 1);
        snmi1a->deleteLater();
        sni1a->deleteLater();

        SocialNetworkInterface *sni1b = new SocialNetworkInterface(this);
        SocialNetworkModelInterface *snmi1b = new SocialNetworkModelInterface(this);
        QSignalSpy spy1b (snmi1b, SIGNAL(statusChanged()));
        snmi1b->setSocialNetwork(sni1b);
        QCOMPARE(snmi1b->status(), SocialNetworkInterface::Initializing);
        QCOMPARE(spy1b.count(), 0);
        snmi1b->classBegin();
        QCOMPARE(snmi1b->status(), SocialNetworkInterface::Initializing);
        QCOMPARE(spy1b.count(), 0);
        snmi1b->componentComplete();
        QCOMPARE(snmi1b->status(), SocialNetworkInterface::Initializing);
        QCOMPARE(spy1b.count(), 0);
        sni1b->classBegin();
        QCOMPARE(snmi1b->status(), SocialNetworkInterface::Initializing);
        QCOMPARE(spy1b.count(), 0);
        sni1b->componentComplete();
        QCOMPARE(snmi1b->status(), SocialNetworkInterface::Idle);
        QCOMPARE(spy1b.count(), 1);
        snmi1b->deleteLater();
        sni1b->deleteLater();

        SocialNetworkInterface *sni2 = new SocialNetworkInterface(this);
        SocialNetworkModelInterface *snmi2 = new SocialNetworkModelInterface(this);
        QSignalSpy spy2 (snmi2, SIGNAL(statusChanged()));
        snmi2->classBegin();
        snmi2->componentComplete();
        snmi2->setSocialNetwork(sni2);
        QCOMPARE(snmi2->status(), SocialNetworkInterface::Initializing);
        QCOMPARE(spy2.count(), 0);
        sni2->classBegin();
        QCOMPARE(snmi2->status(), SocialNetworkInterface::Initializing);
        QCOMPARE(spy2.count(), 0);
        sni2->componentComplete();
        QCOMPARE(snmi2->status(), SocialNetworkInterface::Idle);
        QCOMPARE(spy2.count(), 1);
        snmi2->deleteLater();
        sni2->deleteLater();

        SocialNetworkInterface *sni3 = new SocialNetworkInterface(this);
        SocialNetworkModelInterface *snmi3 = new SocialNetworkModelInterface(this);
        QSignalSpy spy3 (snmi3, SIGNAL(statusChanged()));
        sni3->classBegin();
        sni3->componentComplete();
        snmi3->setSocialNetwork(sni3);
        QCOMPARE(snmi3->status(), SocialNetworkInterface::Initializing);
        QCOMPARE(spy3.count(), 0);
        snmi3->classBegin();
        QCOMPARE(snmi3->status(), SocialNetworkInterface::Initializing);
        QCOMPARE(spy3.count(), 0);
        snmi3->componentComplete();
        QCOMPARE(snmi3->status(), SocialNetworkInterface::Idle);
        QCOMPARE(spy3.count(), 1);
        snmi3->deleteLater();
        sni3->deleteLater();

        SocialNetworkInterface *sni4 = new SocialNetworkInterface(this);
        SocialNetworkModelInterface *snmi4 = new SocialNetworkModelInterface(this);
        QSignalSpy spy4 (snmi4, SIGNAL(statusChanged()));
        sni4->classBegin();
        sni4->componentComplete();
        snmi4->classBegin();
        snmi4->componentComplete();
        snmi4->setSocialNetwork(sni4);
        QCOMPARE(snmi4->status(), SocialNetworkInterface::Idle);
        QCOMPARE(spy4.count(), 1);
        snmi4->deleteLater();
        sni4->deleteLater();


        // Test that we goes back to init state when setting a new SNI
        SocialNetworkInterface * sni1 = new SocialNetworkInterface(this);
        sni2 = new SocialNetworkInterface(this);
        SocialNetworkModelInterface * snmi1 = new SocialNetworkModelInterface(this);
        sni1->classBegin();
        sni1->componentComplete();
        snmi1->classBegin();
        snmi1->componentComplete();
        snmi1->setSocialNetwork(sni1);
        QCOMPARE(snmi1->status(), SocialNetworkInterface::Idle);
        QCOMPARE(snmi1->socialNetwork(), sni1);
        snmi1->setSocialNetwork(sni2);
        QCOMPARE(snmi1->socialNetwork(), sni2);
        QCOMPARE(snmi1->status(), SocialNetworkInterface::Initializing);
        sni2->classBegin();
        sni2->componentComplete();
        QCOMPARE(snmi1->status(), SocialNetworkInterface::Idle);
        snmi1->deleteLater();
        sni1->deleteLater();
        sni2->deleteLater();
    }

    void testSimpleIciiLoad()
    {
        // A simple test to check if the loading flow for ICII is ok
        TestSocialNetworkInterface *sni = new TestSocialNetworkInterface(this);
        sni->classBegin();
        sni->componentComplete();

        IdentifiableContentItemInterface *icii = new IdentifiableContentItemInterface(this);
        icii->classBegin();
        icii->componentComplete();
        icii->setSocialNetwork(sni);

        TestFilterInterface *tfi = new TestFilterInterface(this);
        icii->setFilter(tfi);
        QVERIFY(icii->load());

        while (icii->status() == SocialNetworkInterface::Busy) {
            QTest::qWait(1000);
        }

        // This test won't pass without network connection
        QVERIFY(icii->status() == SocialNetworkInterface::Idle
                || icii->status() == SocialNetworkInterface::Error);
        if (icii->status() == SocialNetworkInterface::Idle) {
            QVERIFY(icii->data().contains(DATA_KEY));
            QCOMPARE(icii->data().value(DATA_KEY).toInt(), DATA_VALUE);
        }

        if (icii->status() == SocialNetworkInterface::Error) {
            QCOMPARE(icii->error(), SocialNetworkInterface::RequestError);
        }

        icii->deleteLater();
        tfi->deleteLater();
        sni->deleteLater();
    }

    void testIciiDestruction()
    {
        // Basically, we have 3 components:
        // - SNI
        // - FI
        // - ICII
        // We should try to destroy some of these components
        // when a request is running. Nothing should crash,
        // but ICII should report errors

        // Let's try to first destroy SNI
        TestSocialNetworkInterface *sni = new TestSocialNetworkInterface(this);
        sni->classBegin();
        sni->componentComplete();

        IdentifiableContentItemInterface *icii = new IdentifiableContentItemInterface(this);
        icii->classBegin();
        icii->componentComplete();
        icii->setSocialNetwork(sni);

        TestFilterInterface *tfi = new TestFilterInterface(this);
        icii->setFilter(tfi);
        QVERIFY(icii->load());

        QSignalSpy spy1 (sni, SIGNAL(destroyed()));
        sni->deleteLater();
        QTest::qWait(100); // Run the event loop
        QCOMPARE(spy1.count(), 1);

        while (icii->status() == SocialNetworkInterface::Busy) {
            QTest::qWait(1000);
        }

        QCOMPARE(icii->status(), SocialNetworkInterface::Error);
        tfi->deleteLater();
        icii->deleteLater();

        // Let's try to destroy FI
        sni = new TestSocialNetworkInterface(this);
        sni->classBegin();
        sni->componentComplete();

        icii = new IdentifiableContentItemInterface(this);
        icii->classBegin();
        icii->componentComplete();
        icii->setSocialNetwork(sni);

        tfi = new TestFilterInterface(this);
        icii->setFilter(tfi);
        QVERIFY(icii->load());

        QSignalSpy spy2 (tfi, SIGNAL(destroyed()));
        tfi->deleteLater();
        QTest::qWait(100); // Run the event loop
        QCOMPARE(spy2.count(), 1);

        while (icii->status() == SocialNetworkInterface::Busy) {
            QTest::qWait(1000);
        }

        QCOMPARE(icii->status(), SocialNetworkInterface::Error);
        sni->deleteLater();
        icii->deleteLater();

        // Let's try to destroy ICII
        sni = new TestSocialNetworkInterface(this);
        sni->classBegin();
        sni->componentComplete();

        icii = new IdentifiableContentItemInterface(this);
        icii->classBegin();
        icii->componentComplete();
        icii->setSocialNetwork(sni);

        tfi = new TestFilterInterface(this);
        icii->setFilter(tfi);
        QVERIFY(icii->load());

        QSignalSpy spy3 (icii, SIGNAL(destroyed()));
        icii->deleteLater();
        QTest::qWait(100); // Run the event loop
        QCOMPARE(spy3.count(), 1);

        // Wait for downloaded stuff, even if it won't be used
        // Check if there is no crash
        QTest::qWait(5000);

        tfi->deleteLater();
        sni->deleteLater();
    }

    void testSnmiDestruction()
    {
        // Basically, we have 3 components:
        // - SNI
        // - FI
        // - SNMI
        // We should try to destroy some of these components
        // when a request is running. Nothing should crash,
        // but SNMI should report errors

        // Let's try to first destroy SNI
        TestSocialNetworkInterface *sni = new TestSocialNetworkInterface(this);
        sni->classBegin();
        sni->componentComplete();

        SocialNetworkModelInterface *snmi = new SocialNetworkModelInterface(this);
        snmi->classBegin();
        snmi->componentComplete();
        snmi->setSocialNetwork(sni);

        TestFilterInterface *tfi = new TestFilterInterface(this);
        snmi->setFilter(tfi);
        QVERIFY(snmi->load());

        QSignalSpy spy1 (sni, SIGNAL(destroyed()));
        sni->deleteLater();
        QTest::qWait(100); // Run the event loop
        QCOMPARE(spy1.count(), 1);

        while (snmi->status() == SocialNetworkInterface::Busy) {
            QTest::qWait(1000);
        }

        QCOMPARE(snmi->status(), SocialNetworkInterface::Error);
        tfi->deleteLater();
        snmi->deleteLater();

        // Let's try to destroy FI
        sni = new TestSocialNetworkInterface(this);
        sni->classBegin();
        sni->componentComplete();

        snmi = new SocialNetworkModelInterface(this);
        snmi->classBegin();
        snmi->componentComplete();
        snmi->setSocialNetwork(sni);

        tfi = new TestFilterInterface(this);
        snmi->setFilter(tfi);
        QVERIFY(snmi->load());

        QSignalSpy spy2 (tfi, SIGNAL(destroyed()));
        tfi->deleteLater();
        QTest::qWait(100); // Run the event loop
        QCOMPARE(spy2.count(), 1);

        while (snmi->status() == SocialNetworkInterface::Busy) {
            QTest::qWait(1000);
        }

        QCOMPARE(snmi->status(), SocialNetworkInterface::Error);
        sni->deleteLater();
        snmi->deleteLater();

        // Let's try to destroy SNMI
        sni = new TestSocialNetworkInterface(this);
        sni->classBegin();
        sni->componentComplete();

        snmi = new SocialNetworkModelInterface(this);
        snmi->classBegin();
        snmi->componentComplete();
        snmi->setSocialNetwork(sni);

        tfi = new TestFilterInterface(this);
        snmi->setFilter(tfi);
        QVERIFY(snmi->load());

        QSignalSpy spy3 (snmi, SIGNAL(destroyed()));
        snmi->deleteLater();
        QTest::qWait(100); // Run the event loop
        QCOMPARE(spy3.count(), 1);

        // Wait for downloaded stuff, even if it won't be used
        // Check if there is no crash
        QTest::qWait(5000);

        tfi->deleteLater();
        sni->deleteLater();
    }
};

QTEST_MAIN(SimpleTests)

#include "main.moc"
