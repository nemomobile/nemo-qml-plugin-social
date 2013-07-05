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
    width: 300
    height: 500
    property string consumerKey     // provided by main.cpp
    property string consumerSecret  // provided by main.cpp
    property string token           // provided by main.cpp
    property string tokenSecret     // provided by main.cpp
    property string identifier      // provided by main.cpp
    property int whichActive: 0

    function back(which) {
        whichActive = which
    }

    function makeActive(which, nodeId) {
        switch (which) {
        case 1:
            followingList.populate(nodeId)
            break
        case 2:
            followersList.populate(nodeId)
            break
        case 3:
            feedList.populate(nodeId)
            break
        case 4:
            homeList.populate(nodeId)
            break
        case 5:
            userColorPage.populate(nodeId)
            break
        }
        whichActive = which
    }

    Twitter {
        id: twitter
        consumerKey: root.consumerKey
        consumerSecret: root.consumerSecret
        oauthToken: root.token
        oauthTokenSecret: root.tokenSecret
        currentUserIdentifier: root.identifier

        property QtObject friendsFilter:      ContentItemTypeFilter { type: Twitter.Friends }
        property QtObject followersFilter:    ContentItemTypeFilter { type: Twitter.Followers }
        property QtObject feedFilter:         ContentItemTypeFilter { type: Twitter.Tweet }
        property QtObject homeFilter:         ContentItemTypeFilter { type: Twitter.Home }
    }

    TwitterUser {
        id: user
        socialNetwork: twitter
        onStatusChanged: {
            if (status == Twitter.Idle && whichActive == -2) {
                console.debug("=== Current user ===")
                console.debug("Screen name :  " + user.screenName)
                console.debug("Description :  " + user.description)
                console.debug("Url :          " + user.url)
                console.debug("Tweets:        " + user.statusesCount)
                console.debug("Followers :    " + user.followersCount)
                console.debug("Following:     " + user.friendsCount)
                whichActive = 0
            } else if (status == Twitter.Idle && whichActive == -4) {
                whichActive = 0
            }
        }
    }

    TwitterTweet {
        id: tweet
        socialNetwork: twitter
        onStatusChanged: {
            if (status == Twitter.Idle && whichActive == -3) {
                console.debug("=== One tweet (Joona's about his slides) ===")
                console.debug("Text:          " + tweet.text)
                console.debug("Screen name:   " + tweet.user.screenName)
                console.debug("User:          " + tweet.user.name)
                console.debug("#favourites:   " + tweet.favoriteCount)
                console.debug("#RT:           " + tweet.retweetCount)
                whichActive = 0
            }
        }
    }

    ListView {
        id: main
        visible: whichActive == 0
        anchors.fill: parent
        model: ListModel {
            ListElement {
                text: "Get current user in terminal"
                which: -2
            }
            ListElement {
                text: "Get a tweet in terminal"
                which: -3
            }
            ListElement {
                text: "Show following"
                which: 1
            }
            ListElement {
                text: "Show followers"
                which: 2
            }
            ListElement {
                text: "Show feed"
                which: 3
            }
            ListElement {
                text: "Show home"
                which: 4
            }
            ListElement {
                text: "Upload a random tweet"
                which: -4
            }
            ListElement {
                text: "Show colors of the user"
                which: 5
            }
            ListElement {
                text: "Quit"
                which: -1
            }
        }

        delegate: Item {
            width: main.width
            height: childrenRect.height
            TwitterButton {
                anchors.horizontalCenter: parent.horizontalCenter
                text: model.text
                onClicked: {
                    if (model.which == -1) {
                        Qt.quit()
                    } else if (model.which == -2) {
                        whichActive = model.which
                        user.identifier = root.identifier
                    } else if (model.which == -3) {
                        whichActive = model.which
                        tweet.identifier = "349499121868095490"
                    } else if (model.which == -4) {
                        if (user.identifier != root.identifier) {
                            console.debug("Current user not loaded, please load it using Get current user in Terminal")
                            return
                        }
                        user.uploadTweet("Here is a random number: " + Math.random())
                        whichActive = model.which

                    } else {
                        makeActive(model.which, root.identifier)
                    }
                }
            }
        }
    }

    TwitterUserList {
        id: followingList
        visible: whichActive == 1
        filters: [twitter.friendsFilter]
        onBackClicked: back(0)
    }
    TwitterUserList {
        id: followersList
        visible: whichActive == 2
        filters: [twitter.followersFilter]
        onBackClicked: back(0)
    }

    TwitterTweetList {
        id: feedList
        visible: whichActive == 3
        filters: [twitter.feedFilter]
        onBackClicked: back(0)
    }

    TwitterTweetList {
        id: homeList
        visible: whichActive == 4
        filters: [twitter.homeFilter]
        onBackClicked: back(0)
    }

    TwitterUserColorPage {
        id: userColorPage
        visible: whichActive == 5
        filters: []
        onBackClicked: back(0)
    }
}
