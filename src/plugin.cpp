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

#include <QtGlobal>
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
# include <QtQml>
# include <QQmlEngine>
# include <QQmlExtensionPlugin>
# define QDeclarativeEngine QQmlEngine
# define QDeclarativeExtensionPlugin QQmlExtensionPlugin
#else
# include <QtDeclarative>
# include <QDeclarativeEngine>
# include <QDeclarativeExtensionPlugin>
#endif

// social plugin headers
#include "socialnetworkinterface.h"
#include "socialnetworkmodelinterface.h"
#include "contentiteminterface.h"
#include "identifiablecontentiteminterface.h"
#include "filterinterface.h"
#include "sorterinterface.h"
#include "contentitemtypefilterinterface.h"

// facebook implementation headers
#include "facebook/facebookinterface.h"
#include "facebook/facebookobjectreferenceinterface.h"
#include "facebook/facebookalbuminterface.h"
#include "facebook/facebookcommentinterface.h"
#include "facebook/facebooknotificationinterface.h"
#include "facebook/facebookphotointerface.h"
#include "facebook/facebookpostinterface.h"
#include "facebook/facebookuserinterface.h"

#include "facebook/facebooklikeinterface.h"

// twitter implementation headers
#include "twitter/twitterinterface.h"
#include "twitter/twitteruserinterface.h"
#include "twitter/twittertweetinterface.h"
#include "twitter/twitterconversationfilterinterface.h"


class Q_DECL_EXPORT NemoSocialPlugin : public QDeclarativeExtensionPlugin
{
    Q_OBJECT
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    Q_PLUGIN_METADATA(IID "org.nemomobile.social")
#endif
public:
    virtual ~NemoSocialPlugin() { }

    void initializeEngine(QDeclarativeEngine *engine, const char *uri)
    {
        Q_ASSERT(uri == QLatin1String("org.nemomobile.social"));
        Q_UNUSED(engine)
        Q_UNUSED(uri)
    }

    void registerTypes(const char *uri)
    {
        Q_ASSERT(uri == QLatin1String("org.nemomobile.social"));

        // these types are actually uncreatable, but are not marked as such, in order
        // to allow users to specify properties of those types.
        // TODO: we will need to set them as uncreatable one day

        qmlRegisterType<SocialNetworkInterface>(uri, 1, 0, "SocialNetwork");
        qmlRegisterType<ContentItemInterface>(uri, 1, 0, "ContentItem");
        qmlRegisterType<IdentifiableContentItemInterface>(uri, 1, 0, "IdentifiableContentItem");
        qmlRegisterType<FilterInterface>(uri, 1, 0, "Filter");
        qmlRegisterType<SorterInterface>(uri, 1, 0, "Sorter");

        // creatable types from the social plugin
        qmlRegisterType<SocialNetworkModelInterface>(uri, 1, 0, "SocialNetworkModel");
        qmlRegisterType<ContentItemTypeFilterInterface>(uri, 1, 0, "ContentItemTypeFilter");

        // creatable types from the facebook implementation
        qmlRegisterType<FacebookInterface>(uri, 1, 0, "Facebook");
        qmlRegisterType<FacebookObjectReferenceInterface>(uri, 1, 0, "FacebookObjectReference");
        qmlRegisterType<FacebookAlbumInterface>(uri, 1, 0, "FacebookAlbum");
        qmlRegisterType<FacebookCommentInterface>(uri, 1, 0, "FacebookComment");
        qmlRegisterType<FacebookNotificationInterface>(uri, 1, 0, "FacebookNotification");
        qmlRegisterType<FacebookPhotoInterface>(uri, 1, 0, "FacebookPhoto");
        qmlRegisterType<FacebookPostInterface>(uri, 1, 0, "FacebookPost");
        qmlRegisterType<FacebookUserInterface>(uri, 1, 0, "FacebookUser");

        qmlRegisterType<FacebookLikeInterface>(uri, 1, 0, "FacebookLike");
        qmlRegisterType<FacebookNameTagInterface>(uri, 1, 0, "FacebookNameTag");
        qmlRegisterType<FacebookPhotoImageInterface>(uri, 1, 0, "FacebookPhotoImage");
        qmlRegisterType<FacebookPhotoTagInterface>(uri, 1, 0, "FacebookPhotoTag");
        qmlRegisterType<FacebookPostActionInterface>(uri, 1, 0, "FacebookPostAction");
        qmlRegisterType<FacebookPostPropertyInterface>(uri, 1, 0, "FacebookPostProperty");
        qmlRegisterType<FacebookUserCoverInterface>(uri, 1, 0, "FacebookUserCover");
        qmlRegisterType<FacebookUserPictureInterface>(uri, 1, 0, "FacebookUserPicture");

        // creatable types from the twitter implementation
        qmlRegisterType<TwitterInterface>(uri, 1, 0, "Twitter");
        qmlRegisterType<TwitterUserInterface>(uri, 1, 0, "TwitterUser");
        qmlRegisterType<TwitterTweetInterface>(uri, 1, 0, "TwitterTweet");
        qmlRegisterType<TwitterConversationFilterInterface>(uri, 1, 0, "TwitterConversationFilter");
    }
};

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
Q_EXPORT_PLUGIN2(nemosocial, NemoSocialPlugin)
#endif

#include "plugin.moc"
