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

#include "contentitemtypefilterinterface.h"
#include "contentiteminterface.h"

class ContentItemTypeFilterInterfacePrivate
{
public:
    ContentItemTypeFilterInterfacePrivate();
    int type;
    int limit;
    QStringList whichFields;
};

ContentItemTypeFilterInterfacePrivate::ContentItemTypeFilterInterfacePrivate()
    : type(0), limit(0)
{
}

// ------------------------------


/*!
    \qmltype ContentItemTypeFilter
    \instantiates ContentItemTypeFilterInterface
    \inqmlmodule org.nemomobile.social 1
    \brief A filter used to query related data based on their types

    The ContentItemTypeFilter is used to perform queries based on
    the type of the related data to get using the \l type property.
    It also provides the capability to only select some given fields
    using \l whichFields and a limited number of entries using \l limit.

    \note Twitter does not support \l whichFields.

    The following example shows how to select a list of friends for the current
    user in Facebook retrieved using ContentItemTypeFilter.

    \qml
    import QtQuick 1.1
    import org.nemomobile.social 1.0

    Item {
        Facebook {
            id: facebook
            accessToken: "..." // Need a valid access token
        }

        SocialNetworkModel {
            id: friendsModel
            nodeIdentifier: "me"
            socialNetwork: facebook
            filters: ContentItemTypeFilter { type: Facebook.User; limit: 30 }
        }

        ListView {
            model: friendsModel
            delegate: Text { text: contentItem.name }
        }

        Component.onCompleted: friendsModel.populate()
    }
    \endqml


    \section1 Selected data using this filter

    \section2 Facebook

    The following tables describes the Facebook entities that are selected
    given a type passed in the filter and a node type. Types that are not
    listed should not be used to filtering or filtering will fail.

    \section3 User

    \table
    \header
        \li Filter type
        \li
    \row
        \li Album
        \li Albums of the user
    \row
        \li Notification
        \li Notifications for the current user (only works if the node corresponds to the
            logged user)
    \row
        \li Photo
        \li Photos of the user
    \row
        \li Post
        \li The feed posted on the user's wall
    \row
        \li User
        \li Friends of the current user (only works if the node corresponds to the  logged user)
    \endtable

    \section3 Album

    \table
    \header
        \li Filter type
        \li
    \row
        \li Comment
        \li The comments posted on the album
    \row
        \li Photo
        \li The photos in the album
    \row
        \li Like
        \li The likes posted on the album
    \endtable

    \section3 Photo

    \table
    \header
        \li Filter type
        \li
    \row
        \li Comment
        \li The comments posted on the photo
    \row
        \li Like
        \li The likes posted on the photo
    \endtable

    \section3 Post

    \table
    \header
        \li Filter type
        \li
    \row
        \li Comment
        \li The comments posted on the post
    \row
        \li Like
        \li The likes posted on the post
    \endtable

    \section2 Twitter

    TODO


 */

ContentItemTypeFilterInterface::ContentItemTypeFilterInterface(QObject *parent)
    : FilterInterface(parent), d_ptr(new ContentItemTypeFilterInterfacePrivate)
{
}

ContentItemTypeFilterInterface::~ContentItemTypeFilterInterface()
{
}


/*!
    \qmlproperty enum ContentItemTypeFilter::type

    Hold the type that is used by this ContentItemTypeFilter. Be sure
    to use types that are exposed by the SocialNetwork subclass you
    intend to use.
*/
int ContentItemTypeFilterInterface::type() const
{
    Q_D(const ContentItemTypeFilterInterface);
    return d->type;
}

void ContentItemTypeFilterInterface::setType(int type)
{
    Q_D(ContentItemTypeFilterInterface);
    if (d->type != type) {
        d->type = type;
        emit typeChanged();
    }
}

/*!
    \qmlproperty int ContentItemTypeFilter::limit

    Hold the limit that is used by this ContentItemTypeFilter. The
    limit property is used to determine how many related items should
    be queried from the social network.
*/

int ContentItemTypeFilterInterface::limit() const
{
    Q_D(const ContentItemTypeFilterInterface);
    return d->limit;
}

void ContentItemTypeFilterInterface::setLimit(int limit)
{
    Q_D(ContentItemTypeFilterInterface);
    if (d->limit != limit) {
        d->limit = limit;
        emit limitChanged();
    }
}

/*!
    \qmlproperty list<string> ContentItemTypeFilter::whichFields

    Hold a list of fields that should be queried using this
    ContentItemTypeFilter.
*/

QStringList ContentItemTypeFilterInterface::whichFields() const
{
    Q_D(const ContentItemTypeFilterInterface);
    return d->whichFields;
}

void ContentItemTypeFilterInterface::setWhichFields(const QStringList &whichFields)
{
    Q_D(ContentItemTypeFilterInterface);
    if (d->whichFields != whichFields) {
        d->whichFields = whichFields;
        emit whichFieldsChanged();
    }
}
