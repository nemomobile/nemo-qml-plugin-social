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
    property alias filters: socialNetworkModel.filters
    property color backgroundColor
    property color linkColor
    property color sidebarBorderColor
    property color profileSidebarFillColor
    property color profileTextColor

    function populate(nodeId) {
        if (socialNetworkModel.nodeIdentifier != nodeId) {
            model.clear()
            socialNetworkModel.nodeIdentifier = nodeId
            socialNetworkModel.populate()
            return
        }

        if (model.count == 0) {
            socialNetworkModel.nodeIdentifier = nodeId
            socialNetworkModel.populate()
        }
    }

    SocialNetworkModel {
        id: socialNetworkModel
        socialNetwork: twitter
        nodeType: Twitter.User
        onStatusChanged: {
            if (status == SocialNetwork.Idle) {
                model.append({"name": "profile_background_color",
                              "color": socialNetworkModel.node.profileBackgroundColor})
                model.append({"name": "profile_link_color",
                              "color": socialNetworkModel.node.profileLinkColor})
                model.append({"name": "profile_sidebar_border_color",
                              "color": socialNetworkModel.node.profileSidebarBorderColor})
                model.append({"name": "profile_sidebar_fill_color",
                              "color": socialNetworkModel.node.profileSidebarFillColor})
                model.append({"name": "profile_text_color",
                              "color": socialNetworkModel.node.profileTextColor})
            }
        }
    }

    ListModel {
        id: model
    }

    ListView {
        id: view
        clip: true
        anchors.top: parent.top
        anchors.bottom: backButton.top
        anchors.left: parent.left
        anchors.right: parent.right
        model: model
        delegate: Rectangle {
            width: view.width
            height: 50
            color: model.color

            Rectangle {
                anchors.fill: text
                color: "white"
            }

            Text {
                id: text
                anchors.centerIn: parent
                text: model.name
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
}
