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
    signal postClicked(string postId)
    property alias filters: model.filters
    function populate(nodeId) {
        model.nodeIdentifier = nodeId
        model.populate()
        view.positionViewAtBeginning()
    }

    SocialNetworkModel {
        id: model
        socialNetwork: facebook
    }

    Text {
        id: topLabel
        anchors.top: parent.top
        anchors.horizontalCenter: parent.horizontalCenter
        text: "There are " + model.count + " posts"
    }

    Button {
        id: backButton
        anchors.bottom: parent.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        text: "Back"
        onClicked: container.backClicked()
    }

    ListView {
        id: view
        clip: true
        anchors.top: topLabel.bottom
        anchors.bottom: backButton.top
        anchors.left: parent.left
        anchors.right: parent.right
        model: model
        footer: Item {
            width: view.width
            height: childrenRect.height
            Button {
                anchors.horizontalCenter: parent.horizontalCenter
                text: model.hasNext ? "Load more" : "Cannot load more"
                onClicked: model.loadNext()
            }
        }
        delegate: ButtonBackground {
            width: view.width
            height: column.height + 20
            onClicked: container.postClicked(model.contentItem.identifier)
            onRightClicked: {
                menuContainer.postIdentifier = model.contentItem.identifier
                menuContainer.visible = true
            }
            Column {
                id: column
                property string message: model.contentItem.message
                property string story: model.contentItem.story
                anchors.left: parent.left; anchors.leftMargin: 10
                anchors.right: parent.right; anchors.rightMargin: 10
                anchors.verticalCenter: parent.verticalCenter

                Text {
                    text: "From: " + model.contentItem.from.objectName
                }
                Text {
                    anchors.left: parent.left; anchors.right: parent.right
                    text: column.message != "" ? column.message
                                               : (column.story != "" ? column.story : "<No text>")
                    wrapMode: Text.WordWrap
                }
            }
        }
    }

    Item {
        id: menuContainer
        anchors.fill: parent
        property string postIdentifier
        property bool loading: false
        visible: false

        Connections {
            target: model.node
            onStatusChanged: {
                if (model.node.status == Facebook.Idle) {
                    menuContainer.loading = false
                    menuContainer.visible = false
                    model.repopulate()
                }
            }
        }

        Rectangle {
            anchors.fill: parent
            color: "grey"
            opacity: 0.5
        }

        MouseArea {
            anchors.fill: parent
            enabled: menuContainer.visible
            onClicked: menuContainer.visible = false
        }

        Column {
            anchors.centerIn: parent
            Button {
                enabled: menuContainer.visible && !menuContainer.loading
                text: "Remove"
                onClicked: {
                    menuContainer.loading = true
                    model.node.removeStatus(menuContainer.postIdentifier)
                }
            }
        }
    } 

}
