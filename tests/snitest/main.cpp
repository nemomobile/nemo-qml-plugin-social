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

#define ENABLE_TESTS

#include <QtCore/QTimer>
#include <QtTest/QtTest>
#include <QtTest/QSignalSpy>
#include <socialnetworkinterface.h>
#include <socialnetworkinterface_p.h>
#include <contentitemtypefilterinterface.h>

class TestSocialNetworkInterface: public SocialNetworkInterface
{
    Q_OBJECT
public:
    enum InternalState
    {
        Idle,
        PopulateData,
        PopulateModelData
    };

    explicit TestSocialNetworkInterface(QObject *parent = 0):
        SocialNetworkInterface(parent)
    {
        m_internalState = Idle;
        m_timer = new QTimer(this);
        m_timer->setSingleShot(true);
        connect(m_timer, SIGNAL(timeout()), this, SLOT(finishedHandler()));
    }
    void publicSetFilters(const QList<FilterInterface *> &filters)
    {
        Q_D(SocialNetworkInterface);
        d->publicSetFilters(filters);
    }
    QList<FilterInterface *> publicFilters() const
    {
        Q_D(const SocialNetworkInterface);
        return d->publicFilters();
    }
    QStack<Node> publicNodeStack() const
    {
        Q_D(const SocialNetworkInterface);
        return d->publicNodeStack();
    }
    QHash<QString, CacheEntry> publicCache() const
    {
        Q_D(const SocialNetworkInterface);
        return d->publicCache();
    }
    virtual void populateDataForLastNode()
    {
        m_internalState = PopulateData;
        m_timer->start(500);
    }
    virtual void populateModelDataforLastNode()
    {
        m_internalState = PopulateModelData;
        m_timer->start(500);
    }
    bool publicDeleteLastNode()
    {
        Q_D(SocialNetworkInterface);
        return d->publicDeleteLastNode();
    }

private:
    Q_DECLARE_PRIVATE(SocialNetworkInterface)
    InternalState m_internalState;
    QTimer *m_timer;
private slots:
    void finishedHandler()
    {
        Q_D(SocialNetworkInterface);
        switch (m_internalState) {
        case PopulateData:
            populateData();
            break;
        case PopulateModelData:
            populateModelData();
            break;
        default:
            break;
        }
        d->updateNodeAndContent();
    }
    void populateData()
    {
        Q_D(SocialNetworkInterface);
        QVariantMap data;
        data.insert("test", 123456);

        const Node &lastNode = d->publicNodeStack().top();

        d->setLastNodeCacheEntry(d->createCacheEntry(data, lastNode.identifier()));

        populateModelDataforLastNode();
    }
    void populateModelData()
    {
        Q_D(SocialNetworkInterface);
        QList<CacheEntry> modelData;
        QVariantMap data;
        for (int i = 0; i < 10; i++) {
            data.insert("test", i);
            CacheEntry cacheEntry = d->createCacheEntry(data, QString::number(i));
            modelData.append(cacheEntry);
        }

        d->setLastNodeData(modelData);

        m_internalState = Idle;
        d->setStatus(SocialNetworkInterface::Idle);
    }
};

class SniTest: public QObject
{
    Q_OBJECT
private slots:
    void testSocialNetworkInterfaceSingleNode()
    {
        // Check node identifier settings
        QString identifier = "Test_node_identifier";
        TestSocialNetworkInterface *sni = new TestSocialNetworkInterface(this);
        sni->classBegin();
        sni->componentComplete();
        QSignalSpy nodeIdentifierSpy (sni, SIGNAL(nodeIdentifierChanged()));
        sni->setNodeIdentifier(identifier);
        QCOMPARE(sni->nodeIdentifier(), identifier);
        QCOMPARE(nodeIdentifierSpy.count(), 1);
        QCOMPARE(nodeIdentifierSpy.first().isEmpty() == true, true);


        ContentItemTypeFilterInterface *filter = new ContentItemTypeFilterInterface(this);
        filter->setType(3);

        QList<FilterInterface *> filters;
        filters.append(filter);

        sni->publicSetFilters(filters);

        // Check if (when initialized), the SNI do not contain anything
        // and that the node returns a null pointer
        QCOMPARE(sni->publicNodeStack().count(), 0);
        QCOMPARE(sni->node(), static_cast<IdentifiableContentItemInterface *>(0));


        // Now let's do a request (so we can push a node)
        sni->populate();

        // Check if the newly created node got pushed in the stack
        // It should not have any information though
        QCOMPARE(sni->publicNodeStack().count(), 1);
        QCOMPARE(sni->publicNodeStack().top().isNull() == false, true);
        QCOMPARE(sni->publicNodeStack().top().identifier(), identifier);
        QCOMPARE(sni->publicNodeStack().top().filters().contains(filter) == true, true);
        QCOMPARE(sni->publicNodeStack().top().filters().count(), 1);

        // After 500 msecs, the node should have the CacheEntry populated
        // A new entry in the cache should have been created, associated
        // to the same identifier as the node, and the cache entry of the
        // node should be the same
        QTest::qWait(750);

        QCOMPARE(sni->publicCache().count(), 1);
        QCOMPARE(sni->publicCache().contains(identifier) == true, true);
        QCOMPARE(sni->publicNodeStack().top().cacheEntry(), sni->publicCache().value(identifier));

        // Wait for the end of the other request
        QTest::qWait(500);

        // Perform a test about the explicitely shared status
        QVariantMap data = sni->publicCache().value(identifier).data();
        data.insert("test2", "abcdef");
        sni->publicCache()[identifier].setData(data);

        QCOMPARE(sni->publicNodeStack().top().cacheEntry().data().value("test").toInt(), 123456);
        QCOMPARE(sni->publicNodeStack().top().cacheEntry().data().value("test2").toString(),
                 QString("abcdef"));

        // We remove the last node to test if the cache got
        // cleared as well
        QCOMPARE(sni->publicDeleteLastNode() == true, true);
        QCOMPARE(sni->publicNodeStack().isEmpty() == true, true);
        QCOMPARE(sni->publicCache().isEmpty() == true, true);

        sni->deleteLater();
    }

    void testSocialNetworkInterfaceSeveralNodes()
    {
        // Let's create a new node
        QString identifier1 = "Test_node_identifier";
        TestSocialNetworkInterface *sni = new TestSocialNetworkInterface(this);
        sni->classBegin();
        sni->componentComplete();
        sni->setNodeIdentifier(identifier1);


        ContentItemTypeFilterInterface *filter1 = new ContentItemTypeFilterInterface(this);
        filter1->setType(3);
        QList<FilterInterface *> filters1;
        filters1.append(filter1);
        sni->publicSetFilters(filters1);
        sni->populate();
        sni->nextNode();

        QSignalSpy spy (sni, SIGNAL(statusChanged()));

        QTest::qWait(750);
        QCOMPARE(sni->publicNodeStack().count(), 1);
        QTest::qWait(500);

        // After 400 msec, the "whole node", with the model data, should be loaded.
        QCOMPARE(spy.count(), 1);
        QCOMPARE(spy.first().isEmpty() == true, true);

        QString identifier2 = "Other_test_node_identifier";
        sni->setNodeIdentifier(identifier2);

        ContentItemTypeFilterInterface *filter2 = new ContentItemTypeFilterInterface(this);
        filter2->setType(5);
        QList<FilterInterface *> filters2;
        filters2.append(filter2);
        sni->publicSetFilters(filters2);
        sni->populate();
        sni->nextNode();

        QCOMPARE(spy.count(), 2);
        QCOMPARE(spy.last().isEmpty() == true, true);

        QTest::qWait(750);
        QCOMPARE(sni->publicNodeStack().count(), 2);

        QTest::qWait(500);

        // Now we should have 2 nodes, with identical model data
        // Check the refcount of these model data
        const Node &node1 = sni->publicNodeStack().at(0);
        foreach (CacheEntry entry, node1.data()) {
            QCOMPARE(entry.refcount(), 2);
        }
        const Node &node2 = sni->publicNodeStack().at(1);
        foreach (CacheEntry entry, node2.data()) {
            QCOMPARE(entry.refcount(), 2);
        }

        // Remove one node and check if the cache entry associated to the second
        // node got derefed
        QCOMPARE(sni->publicDeleteLastNode() == true, true);
        QCOMPARE(sni->publicCache().contains(identifier2) == false, true);

        // Remove the second node, and everything should be removed
        QCOMPARE(sni->publicDeleteLastNode() == true, true);
        QCOMPARE(sni->publicNodeStack().isEmpty() == true, true);
        QCOMPARE(sni->publicCache().isEmpty() == true, true);
        sni->deleteLater();
    }

    void testSocialNetworkInterfacePreviousNext()
    {
        TestSocialNetworkInterface *sni = new TestSocialNetworkInterface(this);
        sni->classBegin();
        sni->componentComplete();

        // Test the current node
        QCOMPARE(sni->node(), static_cast<IdentifiableContentItemInterface *>(0));

        // Test changing to previous and next
        // should not do anything
        sni->previousNode();
        QCOMPARE(sni->node(), static_cast<IdentifiableContentItemInterface *>(0));
        sni->nextNode();
        QCOMPARE(sni->node(), static_cast<IdentifiableContentItemInterface *>(0));
        sni->nextNode();
        QCOMPARE(sni->node(), static_cast<IdentifiableContentItemInterface *>(0));
        sni->previousNode();
        QCOMPARE(sni->node(), static_cast<IdentifiableContentItemInterface *>(0));
        QCOMPARE(sni->hasPreviousNode() == false, true);
        QCOMPARE(sni->hasNextNode() == false, true);

        // Push a node and test again
        QString identifier = "Test_node_identifier";
        ContentItemTypeFilterInterface *filter = new ContentItemTypeFilterInterface(this);
        filter->setType(3);
        QList<FilterInterface *> filters;
        filters.append(filter);
        sni->setNodeIdentifier(identifier);
        sni->publicSetFilters(filters);
        sni->populate();

        QTest::qWait(1250);

        QCOMPARE(sni->hasPreviousNode() == false, true);
        QCOMPARE(sni->hasNextNode() == true, true);
        sni->nextNode();

        IdentifiableContentItemInterface *interface
                = sni->publicNodeStack().top().cacheEntry().identifiableItem();
        QCOMPARE(sni->node(), interface);
        QCOMPARE(sni->count(), 10);
        QCOMPARE(sni->hasPreviousNode() == true, true);
        QCOMPARE(sni->hasNextNode() == false, true);

        // Just test that we cannot go to the next node
        sni->nextNode();

        // Test if data behave well
        // TODO XXX do this

        // Go back now
        sni->previousNode();

        QCOMPARE(sni->node(), static_cast<IdentifiableContentItemInterface *>(0));
        QCOMPARE(sni->count(), 0);

    }

    void testSocialNetworkInterfaceDelayedCall()
    {
        TestSocialNetworkInterface *sni = new TestSocialNetworkInterface(this);
        sni->classBegin();

        // Queue some requests
        ContentItemTypeFilterInterface *filter = new ContentItemTypeFilterInterface(this);
        filter->setType(3);
        QList<FilterInterface *> filters;
        filters.append(filter);
        sni->publicSetFilters(filters);

        QString identifier1 = "Test_node_identifier";
        QString identifier2 = "Other_test_node_identifier";
        QString identifier3 = "Yet_another_test_node_identifier";

        sni->setNodeIdentifier(identifier1);
        sni->populate();
        sni->setNodeIdentifier(identifier2);
        sni->populate();
        sni->setNodeIdentifier(identifier3);
        sni->populate();
        sni->componentComplete();

        // Only the last identifier should be taken in account
        QTest::qWait(1250);

        QCOMPARE(sni->publicNodeStack().count(), 1);
        QCOMPARE(sni->publicNodeStack().top().identifier(), identifier3);
    }
};

QTEST_MAIN(SniTest)

#include "main.moc"
