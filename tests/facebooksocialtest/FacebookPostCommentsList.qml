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
    id: root
    anchors.fill: parent
    property string currentIdentifier
    signal backClicked
    function populate(nodeId) {
        model.clear()
        itemFilter.identifier = nodeId
        commentsFilter.identifier = nodeId
        post.load()
        model.load()
        view.positionViewAtBeginning()
    }

    FacebookPost {
        id: post
        socialNetwork: facebook
        filter: FacebookItemFilter {
            id: itemFilter
            fields: "id,likes,comments"
        }
    }

    SocialNetworkModel {
        id: model
        socialNetwork: facebook
        filter: commentsFilter
    }

    Text {
        id: topLabel
        anchors.top: parent.top
        anchors.horizontalCenter: parent.horizontalCenter
        text: model.status != Facebook.Idle || post.status != Facebook.Idle ? "... Loading ..."
              : "Comments: " + post.commentsCount
              + "\nLikes: " +  post.likesCount
    }

    FacebookButton {
        id: backButton
        anchors.bottom: parent.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        text: "Back"
        onClicked: root.backClicked()
    }

    ListView {
        id: view
        clip: true
        anchors.top: topLabel.bottom
        anchors.bottom: backButton.top
        anchors.left: parent.left
        anchors.right: parent.right
        model: model
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
                    text: "From: " + contentItem.from.objectName
                }
                Text {
                    anchors.left: parent.left; anchors.right: parent.right
                    text: "Message: " + contentItem.message
                    wrapMode: Text.WordWrap
                }
            }
        }
    }
}
