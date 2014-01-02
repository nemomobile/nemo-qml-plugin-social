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
    function populate() {
        // Jolla announce that the 1st batch is booked
        post.load()
        model.clear()
        view.positionViewAtBeginning()
    }

    FacebookPost {
        id: post
        socialNetwork: facebook
        filter: FacebookItemFilter {
            fields: "id,likes,comments"
            identifier: "324048634313353_603842659667281"
        }
        onLoaded: {
            if (status == Facebook.Idle) {
                // Compute the offset
                var offset = Math.floor(post.commentsCount / model.filter.limit) * model.filter.limit
                model.filter.offset = offset
                model.load()
            }
        }
    }

    SocialNetworkModel {
        id: model
        socialNetwork: facebook
        filter: FacebookRelatedDataFilter {
            identifier: "324048634313353_603842659667281"
            connection: Facebook.Comments
            limit: 5
        }

//        filters: [
//            FacebookCommentFilter {
//                retrieveMode: FacebookCommentFilter.RetrieveLatest
//                limit: 5
//            }
//        ]
    }

    Text {
        id: topLabel
        anchors.top: parent.top
        anchors.horizontalCenter: parent.horizontalCenter
        text: post.status == Facebook.Busy || model.status == Facebook.Busy
              ? "... Loading ..."
              : (post.status == Facebook.Error || model.status == Facebook.Error
                 ? "Error"
                 : "Comments: " + post.commentsCount + "\nLikes: " + post.likesCount)
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
        header: Item {
            width: view.width
            height: childrenRect.height
            FacebookButton {
                anchors.horizontalCenter: parent.horizontalCenter
                text: model.hasPrevious ? "Load previous" : "Cannot load more"
                onClicked: model.loadPrevious()
            }
        }
        footer: Item {
            width: view.width
            height: childrenRect.height
            FacebookButton {
                anchors.horizontalCenter: parent.horizontalCenter
                text: model.hasNext ? "Load next" : "Cannot load more"
                onClicked: model.loadNext()
            }
        }
    }
}
