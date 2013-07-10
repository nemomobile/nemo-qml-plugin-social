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

/*

    This is a manual test application.

    It consists of various views which make use of functionality
    which is provided by the social plugin.

    The first page consists of various options which may be selected.
    The options include:
        - show recent notifications
        - show friends
        - show all albums

    If the "notifications" option is selected, a listview with recent
    notifications is shown.

    If the "friends" option is selected, a listview with all of the
    user's friends is shown.

    If the "albums" option is selected, the user may then select an
    album whose photos can be shown, in a gridview.  Then, if the
    user selects a photo, the comments on that photo will be shown
    in a listview.
*/

Item {
    id: root
    width: 300
    height: 600
    property string accessToken // provided by main.cpp
    property int whichActive: 0

    function backHome() {
        back(0)
    }

    function back(which) {
        whichActive = which
    }

    function makeActive(which, nodeId) {
        switch (which) {
        case 1:
            notificationsList.populate(nodeId)
            break
        case 2:
            friendsList.populate(nodeId)
            break
        case 3:
            albumsList.populate(nodeId)
            break
        case 4:
            photosGrid.populate(nodeId)
            break
        case 5:
            photoCommentsList.populate(nodeId)
            break
        case 6:
            var component1 = Qt.createComponent(Qt.resolvedUrl("ModelDestructionTestPage.qml"));
            if (component1.status == Component.Ready) {
                var page1 = component1.createObject(root);
                page1.populate(nodeId)
                page1.backClicked.connect(root.backHome)
            }
            break
        case 7:
            postList.populate(nodeId)
            break
        case 8:
            postCommmentList.populate(nodeId)
            break
        case 9:
            likesList.populate(nodeId)
            break
        case 10:
            var component2 = Qt.createComponent(Qt.resolvedUrl("FilterDestructionTestPage.qml"));
            if (component2.status == Component.Ready) {
                var page2 = component2.createObject(root);
                page2.populate(nodeId)
                page2.backClicked.connect(root.backHome)
            }
            break
        }
        whichActive = which
    }

    Facebook {
        id: facebook
        accessToken: root.accessToken
        onCurrentUserIdentifierChanged: {
            console.debug("Current user identifier: " + currentUserIdentifier)
        }
    }

    ContentItemTypeFilter {
        id: commentsFilter
        type: Facebook.Comment
    }

    ContentItemTypeFilter {
        id: friendsFilter
        type: Facebook.User
    }

    ListView {
        id: main
        visible: whichActive == 0
        anchors.fill: parent
        model: ListModel {
            ListElement {
                text: "Show posts"
                which: 7
            }
            ListElement {
                text: "Show notifications"
                which: 1
            }
            ListElement {
                text: "Show friends"
                which: 2
            }
            ListElement {
                text: "Show albums"
                which: 3
            }
            ListElement {
                text: "Test model destruction"
                which: 6
            }
            ListElement {
                text: "Test filter destruction"
                which: 10
            }
            ListElement {
                text: "Show URL to user's picture"
                which: -2
            }
            ListElement {
                text: "Test item loading"
                which: -3
            }
            ListElement {
                text: "Test getting \"me\""
                which: -4
            }

            ListElement {
                text: "Quit"
                which: -1
            }
        }

        delegate: Item {
            width: main.width
            height: childrenRect.height
            Button {
                anchors.horizontalCenter: parent.horizontalCenter
                text: model.text
                onClicked: {
                    if (model.which == -1) {
                        Qt.quit()
                    } else if (model.which == -2) {
                        if(portraitModel.status == Facebook.Initializing) {
                            portraitModel.populate()
                            root.whichActive = -1
                        } else {
                            portraitModel.displayPortraitUrl()
                        }
                    } else if(model.which == -3) {
                        if (user.identifier == "me") {
                            user.displayUser()
                            return
                        }

                        root.whichActive = -3
                        user.identifier = "me"
                    } else if(model.which == -4) {
                        if (!nsuffysModel.retrieved) {
                            root.whichActive = -4
                            nsuffysModel.populate()
                        }
                    } else {
                        makeActive(model.which, facebook.currentUserIdentifier)
                    }
                }
            }
        }
    }

    PostList {
        id: postList
        visible: whichActive == 7
        onBackClicked: back(0)
        onPostClicked: makeActive(8, postId)
    }

    PostCommentsList {
        id: postCommmentList
        visible: whichActive == 8
        onBackClicked: back(7)
    }

    NotificationsList {
        id: notificationsList
        visible: whichActive == 1
        onBackClicked: back(0)
    }

    FriendsList {
        id: friendsList
        visible: whichActive == 2
        onBackClicked: back(0)
    }

    AlbumsList {
        id: albumsList
        visible: whichActive == 3
        onBackClicked: back(0)
        onAlbumClicked: makeActive(4, albumId)
    }

    PhotosGrid {
        id: photosGrid
        visible: whichActive == 4
        onBackClicked: back(3)
        onPhotoClicked: {
            photoCommentsList.photoId = photoId
            makeActive(5, photoId)
        }
    }

    PhotoCommentsList {
        id: photoCommentsList
        property string photoId
        visible: whichActive == 5
        onBackClicked: back(4)
        onShowLikesClicked: makeActive(9, photoId)
    }

    LikesList {
        id: likesList
        visible: whichActive == 9
        onBackClicked: back(5)
    }

    SocialNetworkModel {
        id: filterDestructionTestModel
        socialNetwork: facebook

        onStatusChanged: {
            if (status == SocialNetwork.Invalid) {
                console.debug("The status of the filter destruction test model is now Invalid")
            }
        }
    }

    SocialNetworkModel {
        id: portraitModel
        function displayPortraitUrl() {
            console.debug("User picture: " + portraitModel.node.picture.url)
        }

        socialNetwork: facebook
        nodeIdentifier: facebook.currentUserIdentifier
        filters: [ ContentItemTypeFilter { type: Facebook.UserPicture } ]
        onStatusChanged: {
            if (status == Facebook.Idle) {
                displayPortraitUrl()
                root.whichActive = 0
            }
        }
    }

    FacebookUser {
        id: user
        socialNetwork: facebook
        function displayUser() {
            if (user.name != "") {
                console.debug("Current user name: " + user.name)
            }
        }

        onStatusChanged: {
            if (status == Facebook.Idle) {
                displayUser()
                root.whichActive = 0
            }
        }
    }

    SocialNetworkModel {
        id: nsuffysModel
        property bool retrieved: false
        socialNetwork: facebook
        nodeIdentifier: "1069515276"
        onNodeChanged: {
            if (node != null) {
                console.debug("Test user retrieved: " + nsuffysModel.node.name)
                nsuffysModel.retrieved = true
                root.whichActive = 0
            }
        }
    }

}
