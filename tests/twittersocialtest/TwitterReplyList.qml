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
    property alias filters: model.filters

    // We don't use populate in that case
    function populate(nodeId) {}

    SocialNetworkModel {
        id: model
        socialNetwork: twitter
    }

    Column {
        id: header
        anchors.top: parent.top; anchors.left: parent.left; anchors.right: parent.right

        Item {
            anchors.left: parent.left; anchors.right: parent.right
            height: input.height

            Rectangle {
                anchors.fill: input
                color: "#CCCCCC"
            }
            TextInput {
                id: input
                height: 50
                anchors.left: parent.left; anchors.right: parent.right
                font.pixelSize: 50
            }
        }

        TwitterButton {
            anchors.horizontalCenter: parent.horizontalCenter
            text: "Load replies"
            onClicked: {
                model.nodeIdentifier = input.text
                model.populate()
                view.positionViewAtBeginning()
            }
        }
    }

    TwitterButton {
        id: backButton
        anchors.bottom: parent.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        text: "Back"
        onClicked: container.backClicked()
    }

    ListView {
        id: view
        clip: true
        anchors.top: header.bottom
        anchors.bottom: backButton.top
        anchors.left: parent.left
        anchors.right: parent.right
        model: model

        footer: Item {
            width: view.width
            height: childrenRect.height
            TwitterButton {
                anchors.horizontalCenter: parent.horizontalCenter
                text: model.hasNext ? "Load newer replies" : "Cannot load more"
                onClicked: model.loadNext()
            }
        }

        delegate: Item {
            id: commentDelegate
            width: parent.width
            height: column.height + 20
            Column {
                id: column
                anchors.left: parent.left; anchors.leftMargin: 10
                anchors.right: parent.right; anchors.rightMargin: 10
                anchors.verticalCenter: parent.verticalCenter

                Text {
                    text: "From: " + contentItem.user.screenName
                }

                Text {
                    text: "Posted at: " + Qt.formatDateTime(contentItem.createdAt)
                }

                Text {
                    anchors.left: parent.left; anchors.right: parent.right
                    text: "Message: " + contentItem.text
                    wrapMode: Text.WordWrap
                }
            }
        }
    }
}
