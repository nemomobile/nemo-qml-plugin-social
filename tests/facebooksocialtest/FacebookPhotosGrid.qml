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
    signal photoClicked(string photoId)

    function populate(nodeId) {
        model.clear()
        itemFilter.identifier = nodeId
        modelFilter.identifier = nodeId
        album.load()
        model.load()
        view.positionViewAtBeginning()
    }

    SocialNetworkModel {
        id: model
        socialNetwork: facebook
        filter: FacebookRelatedDataFilter {
            id: modelFilter
            connection: Facebook.Photos
        }
    }

    FacebookAlbum {
        id: album
        socialNetwork: facebook
        filter: FacebookItemFilter {
            id: itemFilter
            fields: "id,likes,comments"
        }
    }

    Text {
        id: topLabel
        anchors.top: parent.top
        anchors.horizontalCenter: parent.horizontalCenter
        text: model.status != SocialNetwork.Idle || album.status != SocialNetwork.Idle
              ? "... Loading ..."
              : "There are "  + model.count + " photos in this album"
              + "\nComments: " +  album.commentsCount
              + "\nLikes: " + album.likesCount
    }

    FacebookButton {
        id: backButton
        anchors.bottom: parent.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        text: "Back"
        onClicked: container.backClicked()
    }

    GridView {
        id: view
        clip: true
        anchors.top: topLabel.bottom
        anchors.bottom: backButton.top
        anchors.left: parent.left
        anchors.right: parent.right
        cellWidth: width / 4
        cellHeight: cellWidth
        model: model
        delegate: MouseArea {
            id: photoDelegate
            onClicked: container.photoClicked(model.contentItem.identifier)
            width: view.cellWidth
            height: view.cellHeight
            clip: true
            Image {
                anchors.fill: parent
                fillMode: Image.PreserveAspectCrop
                source: model.contentItem.picture
            }
        }
    }
}
