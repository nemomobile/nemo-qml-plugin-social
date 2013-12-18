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
            var component1 = Qt.createComponent(Qt.resolvedUrl("FacebookModelDestructionTestPage.qml"));
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
            var component2 = Qt.createComponent(Qt.resolvedUrl("FacebookFilterDestructionTestPage.qml"));
            if (component2.status == Component.Ready) {
                var page2 = component2.createObject(root);
                page2.populate(nodeId)
                page2.backClicked.connect(root.backHome)
            }
            break
        case 11:
            homeList.populate(nodeId)
            break
        case 12:
            offsetedCommentsList.populate()
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

    FacebookRelatedDataFilter {
        id: commentsFilter
        connection: Facebook.Comments
    }

    FacebookRelatedDataFilter {
        id: friendsFilter
        connection: Facebook.Friends
    }

    ListView {
        id: main
        visible: whichActive == 0
        anchors.fill: parent
        model: ListModel {
            ListElement {
                text: "Show home feed"
                which: 11
            }
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
                text: "Test getting offseted comments"
                which: 12
            }

            ListElement {
                text: "Quit"
                which: -1
            }
        }

        delegate: Item {
            width: main.width
            height: childrenRect.height
            FacebookButton {
                anchors.horizontalCenter: parent.horizontalCenter
                text: model.text
                onClicked: {
                    if (model.which == -1) {
                        Qt.quit()
                    } else if (model.which == -2) {
                        if(!portraitUser.asked) {
                            portraitUser.asked = true
                            portraitUser.load()
                            root.whichActive = -1
                        } else {
                            portraitUser.displayPortraitUrl()
                        }
                    } else if(model.which == -3) {
                        if (userFilter.identifier == "me") {
                            user.displayUser()
                            return
                        }

                        root.whichActive = -3
                        userFilter.identifier = "me"
                        user.load()
                    } else if(model.which == -4) {
                        if (!nsuffysUser.asked) {
                            nsuffysUser.asked = true
                            root.whichActive = -4
                            nsuffysUser.load()
                        }
                    } else {
                        makeActive(model.which, facebook.currentUserIdentifier)
                    }
                }
            }
        }
    }

    FacebookPostList {
        id: postList
        visible: whichActive == 7
        onBackClicked: back(0)
        onPostClicked: makeActive(8, postId)
        filter: FacebookRelatedDataFilter {
            connection: Facebook.Feed
        }
    }

    FacebookPostList {
        id: homeList
        visible: whichActive == 11
        onBackClicked: back(0)
        filter: FacebookRelatedDataFilter {
            connection: Facebook.Home
        }
    }

    FacebookPostCommentsList {
        id: postCommmentList
        visible: whichActive == 8
        onBackClicked: back(7)
    }

    FacebookNotificationsList {
        id: notificationsList
        visible: whichActive == 1
        onBackClicked: back(0)
    }

    FacebookFriendsList {
        id: friendsList
        visible: whichActive == 2
        onBackClicked: back(0)
    }

    FacebookAlbumsList {
        id: albumsList
        visible: whichActive == 3
        onBackClicked: back(0)
        onAlbumClicked: makeActive(4, albumId)
    }

    FacebookPhotosGrid {
        id: photosGrid
        visible: whichActive == 4
        onBackClicked: back(3)
        onPhotoClicked: {
            photoCommentsList.photoId = photoId
            makeActive(5, photoId)
        }
    }

    FacebookPhotoCommentsList {
        id: photoCommentsList
        property string photoId
        visible: whichActive == 5
        onBackClicked: back(4)
        onShowLikesClicked: makeActive(9, photoId)
    }

    FacebookLikesList {
        id: likesList
        visible: whichActive == 9
        onBackClicked: back(5)
    }

//    FacebookPostOffsetedCommentsList {
//        id: offsetedCommentsList
//        visible: whichActive == 12
//        onBackClicked: back(0)
//    }

    SocialNetworkModel {
        id: filterDestructionTestModel
        socialNetwork: facebook
        onFilterChanged: console.debug("Current filter: " + filter)
    }

    FacebookUser {
        id: portraitUser
        socialNetwork: facebook
        filter: FacebookItemFilter {
            identifier: facebook.currentUserIdentifier
            fields: "id,picture"
        }

        property bool asked: false
        function displayPortraitUrl() {
            console.debug("User picture: " + picture.url)
        }

        onStatusChanged: {
            if (status == Facebook.Idle && asked) {
                displayPortraitUrl()
                root.whichActive = 0
            }
        }
    }

    FacebookUser {
        id: user
        socialNetwork: facebook
        filter: FacebookItemFilter {
            id: userFilter
            fields: "id,name"
        }

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

    FacebookUser {
        id: nsuffysUser
        socialNetwork: facebook
        filter: FacebookItemFilter {
            identifier: "1069515276"
            fields: "id,name"
        }

        property bool asked: false

        onStatusChanged: {
            if (status == Facebook.Idle && asked) {
                console.debug("Test user retrieved: " + nsuffysUser.name)
                root.whichActive = 0
            }
        }
    }
}
