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

import QtQuick 1.1
import org.nemomobile.social 1.0

Item {
    id: container
    anchors.fill: parent
    signal backClicked

    Connections {
        target: root
        onWhichActiveChanged: {
            container.visible = (root.whichActive == 10)
        }
    }

    function populate(nodeId) {
        filterDestructionTestModel.nodeIdentifier = nodeId
        filterDestructionTestModel.filters = [testFilter]
        filterDestructionTestModel.populate()
        view.positionViewAtBeginning()
    }

    ContentItemTypeFilter {
        id: testFilter
        type: Facebook.User
    }

    Text {
        id: topLabel
        anchors.top: parent.top
        anchors.horizontalCenter: parent.horizontalCenter
        text: "You have " + filterDestructionTestModel.count + " friends"
    }

    Button {
        id: backButton
        anchors.bottom: parent.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        text: "Back"
        onClicked: {
            container.backClicked()
            // If we do not destroy the filter first, we might go into a race
            // conditin in Qt 5 where the delegates are destroyed twice, and
            // leads into a segfault :(
            testFilter.destroy()
            container.destroy()
        }
    }

    ListView {
        id: view
        clip: true
        anchors.top: topLabel.bottom
        anchors.bottom: backButton.top
        anchors.left: parent.left
        anchors.right: parent.right
        model: filterDestructionTestModel
        footer: Item {
            width: view.width
            height: childrenRect.height
            Button {
                anchors.horizontalCenter: parent.horizontalCenter
                text: filterDestructionTestModel.hasNext ? "Load more" : "Cannot load more"
                onClicked: filterDestructionTestModel.loadNext()
            }
        }

        delegate: Item {
            width: view.width
            height: 50

            Text {
                anchors.centerIn: parent
                text: model.contentItem.name
            }
        }
    }
}
