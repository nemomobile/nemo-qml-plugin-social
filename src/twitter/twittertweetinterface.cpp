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

#include "twittertweetinterface_p.h"
#include "twitterinterface.h"
#include "twitterontology_p.h"
// <<< include
#include <QtCore/QtDebug>


#define TWITTER_ONTOLOGY_TWEET_RETWEETED_STATUS QLatin1String("retweeted_status")
// >>> include

TwitterTweetInterfacePrivate::TwitterTweetInterfacePrivate(TwitterTweetInterface *q)
    : IdentifiableContentItemInterfacePrivate(q)
    , action(TwitterInterfacePrivate::NoAction)
// <<< custom
    , user(0)
// >>> custom
{
}

void TwitterTweetInterfacePrivate::finishedHandler()
{
// <<< finishedHandler
    Q_Q(TwitterTweetInterface);

    if (!reply()) {
        // if an error occurred, it might have been deleted by the error handler.
        qWarning() << Q_FUNC_INFO << "network request finished but no reply";
        return;
    }

    QByteArray replyData = reply()->readAll();
    deleteReply();
    bool ok = false;
    QVariant responseDataVariant = ContentItemInterfacePrivate::parseReplyDataVariant(replyData, &ok);
    QVariantMap responseData;
    if (responseDataVariant.type() == QVariant::Map) {
        responseData = responseDataVariant.toMap();
    }
    if (!ok)
        responseData.insert("response", replyData);

    switch (action) {
        case TwitterInterfacePrivate::TweetAction: {
            status = SocialNetworkInterface::Idle;
            emit q->statusChanged();
        }
        break;
        case TwitterInterfacePrivate::RetweetAction: {
            // Update the retweet count
            bool retweeted = responseData.value(TWITTER_ONTOLOGY_TWEET_RETWEETED).toBool();
            int retweetCount = responseData.value(TWITTER_ONTOLOGY_TWEET_RETWEETCOUNT).toInt();
            QVariantMap currentData = data();
            currentData.insert(TWITTER_ONTOLOGY_TWEET_RETWEETED, QVariant::fromValue(retweeted));
            currentData.insert(TWITTER_ONTOLOGY_TWEET_RETWEETCOUNT, retweetCount);
            setData(currentData);
            emit q->retweetedChanged();
            emit q->retweetCountChanged();

            status = SocialNetworkInterface::Idle;
            emit q->statusChanged();
            emit q->responseReceived(responseData);
        }
        break;
        case TwitterInterfacePrivate::UnretweetGetTweetIdToRemoveAction: {
            // We parse the response data, searching for the tweet that corresponds
            QVariantList tweets = responseDataVariant.toList();
            QString retweetIdentifier;

            foreach (const QVariant &tweetVariant, tweets) {
                QVariantMap tweet = tweetVariant.toMap();
                if (!tweet.contains(TWITTER_ONTOLOGY_TWEET_RETWEETED_STATUS)) {
                    continue;
                }

                QVariantMap retweetedTweet = tweet.value(TWITTER_ONTOLOGY_TWEET_RETWEETED_STATUS).toMap();
                QString originalTweetId = retweetedTweet.value(TWITTER_ONTOLOGY_METADATA_ID).toString();

                if (originalTweetId == identifier) {
                    retweetIdentifier = tweet.value(TWITTER_ONTOLOGY_METADATA_ID).toString();
                    break;
                }
            }

            if (!retweetIdentifier.isEmpty()) {
                // We got the identifier to remove,
                // now, perform tweet deletion
                QVariantMap postData;
                postData.insert(TWITTER_ONTOLOGY_CONNECTION_TRIM_USER_KEY, QLatin1String("true"));
                QString path = QString(TWITTER_ONTOLOGY_CONNECTION_STATUS_DESTROY).arg(retweetIdentifier);

                // Slight hack to overcome the fact that request don't want to
                // work when Busy
                status = SocialNetworkInterface::Idle;
                bool requestMade = request(IdentifiableContentItemInterfacePrivate::Post,
                                              QString(), path, QStringList(), postData);
                status = SocialNetworkInterface::Busy;

                if (requestMade) {
                    action = TwitterInterfacePrivate::UnretweetRemoveTweetAction;
                    connectFinishedAndErrors();
                    return;
                }
            }

            error = SocialNetworkInterface::DataUpdateError;
            errorMessage = QLatin1String("Unable to cancel retweet");
            status = SocialNetworkInterface::Error;

            emit q->statusChanged();
            emit q->errorChanged();
            emit q->errorMessageChanged();
            emit q->responseReceived(responseData);
        }
        break;
        case TwitterInterfacePrivate::UnretweetRemoveTweetAction: {
            bool retweeted = responseData.value(TWITTER_ONTOLOGY_TWEET_RETWEETED).toBool();
            int retweetCount = responseData.value(TWITTER_ONTOLOGY_TWEET_RETWEETCOUNT).toInt();
            QVariantMap currentData = data();
            currentData.insert(TWITTER_ONTOLOGY_TWEET_RETWEETED, QVariant::fromValue(retweeted));
            currentData.insert(TWITTER_ONTOLOGY_TWEET_RETWEETCOUNT, retweetCount);
            setData(currentData);
            emit q->retweetedChanged();
            emit q->retweetCountChanged();

            status = SocialNetworkInterface::Idle;
            emit q->statusChanged();
            emit q->responseReceived(responseData);
        }
        break;
        case TwitterInterfacePrivate::UnfavoriteAction:
        case TwitterInterfacePrivate::FavoriteAction: {

            // Update the favorited count
            bool favorited = responseData.value(TWITTER_ONTOLOGY_TWEET_FAVORITED).toBool();
            QVariantMap currentData = data();
            int favoriteCount = currentData.value(TWITTER_ONTOLOGY_TWEET_FAVORITECOUNT).toInt();
            // We need to overcome a Twitter bug (or feature) that when returning from
            // favorites/create, the "favorite_count" field is null. So, as a hack, we
            // just increment (or decrement) the current favorite count.
            if (action == TwitterInterfacePrivate::UnfavoriteAction) {
                favoriteCount = qMax(0, favoriteCount - 1);
            } else if (action == TwitterInterfacePrivate::FavoriteAction) {
                ++ favoriteCount;
            }
            currentData.insert(TWITTER_ONTOLOGY_TWEET_FAVORITECOUNT, favoriteCount);
            currentData.insert(TWITTER_ONTOLOGY_TWEET_FAVORITED, QVariant::fromValue(favorited));
            setData(currentData);
            emit q->favoritedChanged();
            emit q->favoriteCountChanged();

            status = SocialNetworkInterface::Idle;
            emit q->statusChanged();
            emit q->responseReceived(responseData);
        }
        break;
        default: {
            error = SocialNetworkInterface::OtherError;
            errorMessage = QLatin1String("Request finished but no action currently in progress");
            status = SocialNetworkInterface::Error;
            emit q->statusChanged();
            emit q->errorChanged();
            emit q->errorMessageChanged();
            emit q->responseReceived(responseData);
        }
        break;
    }

// >>> finishedHandler
}
void TwitterTweetInterfacePrivate::emitPropertyChangeSignals(const QVariantMap &oldData,
                                                             const QVariantMap &newData)
{
    Q_Q(TwitterTweetInterface);
    QVariant oldFavoriteCount = oldData.value(TWITTER_ONTOLOGY_TWEET_FAVORITECOUNT);
    QVariant newFavoriteCount = newData.value(TWITTER_ONTOLOGY_TWEET_FAVORITECOUNT);
    QVariant oldFavorited = oldData.value(TWITTER_ONTOLOGY_TWEET_FAVORITED);
    QVariant newFavorited = newData.value(TWITTER_ONTOLOGY_TWEET_FAVORITED);
    QVariant oldFilterLevel = oldData.value(TWITTER_ONTOLOGY_TWEET_FILTERLEVEL);
    QVariant newFilterLevel = newData.value(TWITTER_ONTOLOGY_TWEET_FILTERLEVEL);
    QVariant oldInReplyToScreenName = oldData.value(TWITTER_ONTOLOGY_TWEET_INREPLYTOSCREENNAME);
    QVariant newInReplyToScreenName = newData.value(TWITTER_ONTOLOGY_TWEET_INREPLYTOSCREENNAME);
    QVariant oldInReplyToStatusIdentifier = oldData.value(TWITTER_ONTOLOGY_TWEET_INREPLYTOSTATUSIDENTIFIER);
    QVariant newInReplyToStatusIdentifier = newData.value(TWITTER_ONTOLOGY_TWEET_INREPLYTOSTATUSIDENTIFIER);
    QVariant oldInReplyToUserIdentifier = oldData.value(TWITTER_ONTOLOGY_TWEET_INREPLYTOUSERIDENTIFIER);
    QVariant newInReplyToUserIdentifier = newData.value(TWITTER_ONTOLOGY_TWEET_INREPLYTOUSERIDENTIFIER);
    QVariant oldLang = oldData.value(TWITTER_ONTOLOGY_TWEET_LANG);
    QVariant newLang = newData.value(TWITTER_ONTOLOGY_TWEET_LANG);
    QVariant oldPossiblySensitive = oldData.value(TWITTER_ONTOLOGY_TWEET_POSSIBLYSENSITIVE);
    QVariant newPossiblySensitive = newData.value(TWITTER_ONTOLOGY_TWEET_POSSIBLYSENSITIVE);
    QVariant oldRetweetCount = oldData.value(TWITTER_ONTOLOGY_TWEET_RETWEETCOUNT);
    QVariant newRetweetCount = newData.value(TWITTER_ONTOLOGY_TWEET_RETWEETCOUNT);
    QVariant oldRetweeted = oldData.value(TWITTER_ONTOLOGY_TWEET_RETWEETED);
    QVariant newRetweeted = newData.value(TWITTER_ONTOLOGY_TWEET_RETWEETED);
    QVariant oldSource = oldData.value(TWITTER_ONTOLOGY_TWEET_SOURCE);
    QVariant newSource = newData.value(TWITTER_ONTOLOGY_TWEET_SOURCE);
    QVariant oldText = oldData.value(TWITTER_ONTOLOGY_TWEET_TEXT);
    QVariant newText = newData.value(TWITTER_ONTOLOGY_TWEET_TEXT);
    QVariant oldTruncated = oldData.value(TWITTER_ONTOLOGY_TWEET_TRUNCATED);
    QVariant newTruncated = newData.value(TWITTER_ONTOLOGY_TWEET_TRUNCATED);
    QVariant oldWithheldCopyright = oldData.value(TWITTER_ONTOLOGY_TWEET_WITHHELDCOPYRIGHT);
    QVariant newWithheldCopyright = newData.value(TWITTER_ONTOLOGY_TWEET_WITHHELDCOPYRIGHT);
    QVariant oldWithheldScope = oldData.value(TWITTER_ONTOLOGY_TWEET_WITHHELDSCOPE);
    QVariant newWithheldScope = newData.value(TWITTER_ONTOLOGY_TWEET_WITHHELDSCOPE);

    if (newFavoriteCount != oldFavoriteCount)
        emit q->favoriteCountChanged();
    if (newFavorited != oldFavorited)
        emit q->favoritedChanged();
    if (newFilterLevel != oldFilterLevel)
        emit q->filterLevelChanged();
    if (newInReplyToScreenName != oldInReplyToScreenName)
        emit q->inReplyToScreenNameChanged();
    if (newInReplyToStatusIdentifier != oldInReplyToStatusIdentifier)
        emit q->inReplyToStatusIdentifierChanged();
    if (newInReplyToUserIdentifier != oldInReplyToUserIdentifier)
        emit q->inReplyToUserIdentifierChanged();
    if (newLang != oldLang)
        emit q->langChanged();
    if (newPossiblySensitive != oldPossiblySensitive)
        emit q->possiblySensitiveChanged();
    if (newRetweetCount != oldRetweetCount)
        emit q->retweetCountChanged();
    if (newRetweeted != oldRetweeted)
        emit q->retweetedChanged();
    if (newSource != oldSource)
        emit q->sourceChanged();
    if (newText != oldText)
        emit q->textChanged();
    if (newTruncated != oldTruncated)
        emit q->truncatedChanged();
    if (newWithheldCopyright != oldWithheldCopyright)
        emit q->withheldCopyrightChanged();
    if (newWithheldScope != oldWithheldScope)
        emit q->withheldScopeChanged();

// <<< emitPropertyChangeSignals
    // Update the user object if needed
    QVariantMap oldUserMap = oldData.value(TWITTER_ONTOLOGY_TWEET_USER).toMap();
    QVariantMap newUserMap = newData.value(TWITTER_ONTOLOGY_TWEET_USER).toMap();

    if (oldUserMap != newUserMap) {
        qobject_cast<TwitterInterface*>(q->socialNetwork())->setTwitterContentItemData(user, newUserMap);
    }

    // Update created_at
    QString createdAtString = newData.value(TWITTER_ONTOLOGY_TWEET_CREATEDAT).toString();
    // We cast the created at string
    // According to Twitter documentation
    // (https://dev.twitter.com/docs/platform-objects/tweets)
    // the created_at time is returned as an UTC time
    // with the format "ddd MMM dd HH:mm:ss +0000 yyyy",
    // in english language

    QLocale locale (QLocale::English, QLocale::UnitedStates);
    QDateTime newCreatedAt = locale.toDateTime(createdAtString,
                                                    "ddd MMM dd HH:mm:ss +0000 yyyy");
    newCreatedAt.setTimeSpec(Qt::UTC);
    newCreatedAt = newCreatedAt.toLocalTime();
    if (createdAt != newCreatedAt) {
        createdAt =  newCreatedAt;
        emit q->createdAtChanged();
    }

    // Update the current user retweet identifier if needed
    // TODO^
// >>> emitPropertyChangeSignals

    // Call super class implementation
    QVariantMap oldDataWithId = oldData;
    oldDataWithId.insert(NEMOQMLPLUGINS_SOCIAL_CONTENTITEMID,
                         oldData.value(TWITTER_ONTOLOGY_METADATA_ID));
    QVariantMap newDataWithId = newData;
    newDataWithId.insert(NEMOQMLPLUGINS_SOCIAL_CONTENTITEMID,
                         newData.value(TWITTER_ONTOLOGY_METADATA_ID));
    IdentifiableContentItemInterfacePrivate::emitPropertyChangeSignals(oldDataWithId, newDataWithId);
}

//-------------------------------

/*!
    \qmltype TwitterTweet
    \instantiates TwitterTweetInterface
    
*/
TwitterTweetInterface::TwitterTweetInterface(QObject *parent)
    : IdentifiableContentItemInterface(*(new TwitterTweetInterfacePrivate(this)), parent)
{
// <<< constructor
    Q_D(TwitterTweetInterface);
    d->user = new TwitterUserInterface(this);
// >>> constructor
}

/*! \reimp */
int TwitterTweetInterface::type() const
{
    return TwitterInterface::Tweet;
}

/*! \reimp */
bool TwitterTweetInterface::remove()
{
// <<< remove
    // TODO Implement remove if needed
    return IdentifiableContentItemInterface::remove();
// >>> remove
}

/*! \reimp */
bool TwitterTweetInterface::reload(const QStringList &whichFields)
{
// <<< reload
    Q_D(TwitterTweetInterface);
    Q_UNUSED(whichFields)

    RequestInfo requestInfo = TwitterInterfacePrivate::requestTweetInfo(d->identifier);
    if (!d->request(IdentifiableContentItemInterfacePrivate::Get, requestInfo.objectIdentifier,
                    requestInfo.extraPath, requestInfo.whichFields, requestInfo.postedData,
                    requestInfo.extraData)) {
        return false;
    }

    connect(d->reply(), SIGNAL(finished()), this, SLOT(reloadHandler()));
    d->connectErrors();
    return true;
// >>> reload
}

/*!
    \qmlmethod bool TwitterTweet::uploadRetweet()
    */

bool TwitterTweetInterface::uploadRetweet()
{
// <<< uploadRetweet
    Q_D(TwitterTweetInterface);
    QString path = QString(TWITTER_ONTOLOGY_CONNECTION_STATUS_RETWEET).arg(identifier());
    bool requestMade = d->request(IdentifiableContentItemInterfacePrivate::Post,
                                  QString(), path);

    if (!requestMade) {
        return false;
    }

    d->action = TwitterInterfacePrivate::RetweetAction;
    d->connectFinishedAndErrors();
    return true;
// >>> uploadRetweet
}
/*!
    \qmlmethod bool TwitterTweet::removeRetweet()
    */

bool TwitterTweetInterface::removeRetweet()
{
// <<< removeRetweet
    Q_D(TwitterTweetInterface);
    TwitterInterface *twitter = qobject_cast<TwitterInterface*>(socialNetwork());
    QString userIdentifier = twitter->currentUserIdentifier();

    QString path = QString(TWITTER_ONTOLOGY_CONNECTION_STATUSES_USER_TIMELINE);
    QVariantMap extraData;
    extraData.insert(TWITTER_ONTOLOGY_CONNECTION_USER_ID_KEY, userIdentifier);
    extraData.insert(TWITTER_ONTOLOGY_CONNECTION_SINCE_ID_KEY, identifier());
    extraData.insert(TWITTER_ONTOLOGY_CONNECTION_COUNT_KEY, 200);
    extraData.insert(TWITTER_ONTOLOGY_CONNECTION_EXCLUDE_REPLIES_KEY, QLatin1String("true"));
    extraData.insert(TWITTER_ONTOLOGY_CONNECTION_INCLUDE_RTS_KEY, QLatin1String("true"));


    bool requestMade = d->request(IdentifiableContentItemInterfacePrivate::Get,
                                  QString(), path, QStringList(), QVariantMap(), extraData);

    if (!requestMade) {
        return false;
    }

    d->action = TwitterInterfacePrivate::UnretweetGetTweetIdToRemoveAction;
    d->connectFinishedAndErrors();
    return true;
// >>> removeRetweet
}
/*!
    \qmlmethod bool TwitterTweet::favorite()
    */

bool TwitterTweetInterface::favorite()
{
// <<< favorite
    Q_D(TwitterTweetInterface);
    QVariantMap postData;
    postData.insert(TWITTER_ONTOLOGY_CONNECTION_ID_KEY, identifier());
    postData.insert(TWITTER_ONTOLOGY_CONNECTION_INCLUDE_ENTITIES_KEY, QLatin1String("false"));
    QString path = QString(TWITTER_ONTOLOGY_CONNECTION_FAVORITES_CREATE);
    bool requestMade = d->request(IdentifiableContentItemInterfacePrivate::Post,
                                  QString(), path, QStringList(), postData);

    if (!requestMade) {
        return false;
    }

    d->action = TwitterInterfacePrivate::FavoriteAction;
    d->connectFinishedAndErrors();
    return true;
// >>> favorite
}
/*!
    \qmlmethod bool TwitterTweet::unfavorite()
    */

bool TwitterTweetInterface::unfavorite()
{
// <<< unfavorite
    Q_D(TwitterTweetInterface);
    QVariantMap postData;
    postData.insert(TWITTER_ONTOLOGY_CONNECTION_ID_KEY, identifier());
    postData.insert(TWITTER_ONTOLOGY_CONNECTION_INCLUDE_ENTITIES_KEY, QLatin1String("false"));
    QString path = QString(TWITTER_ONTOLOGY_CONNECTION_FAVORITES_DESTROY);
    bool requestMade = d->request(IdentifiableContentItemInterfacePrivate::Post,
                                  QString(), path, QStringList(), postData);

    if (!requestMade) {
        return false;
    }

    d->action = TwitterInterfacePrivate::UnfavoriteAction;
    d->connectFinishedAndErrors();
    return true;
// >>> unfavorite
}
/*!
    \qmlmethod bool TwitterTweet::uploadReply(const QString &message, const QStringList &pathToMedias)
    */

bool TwitterTweetInterface::uploadReply(const QString &message, const QStringList &pathToMedias)
{
// <<< uploadReply
    Q_D(TwitterTweetInterface);
    QString path;
    if (pathToMedias.isEmpty()) {
        path = TWITTER_ONTOLOGY_CONNECTION_STATUS_UPDATE;
    } else {
        qWarning() << Q_FUNC_INFO << "Uploading media is not supported yet";
        return false;
    }

    QVariantMap postData;
    postData.insert(TWITTER_ONTOLOGY_CONNECTION_STATUS_UPDATE_STATUS_KEY, message);
    postData.insert(TWITTER_ONTOLOGY_CONNECTION_STATUS_UPDATE_IN_REPLY_TO_STATUS_ID_KEY,
                    identifier());
    bool requestMade = d->request(IdentifiableContentItemInterfacePrivate::Post,
                                  QString(), path, QStringList(), postData);

    if (!requestMade) {
        return false;
    }

    d->action = TwitterInterfacePrivate::TweetAction;
    d->connectFinishedAndErrors();
    return true;
// >>> uploadReply
}

/*!
    \qmlproperty QDateTime TwitterTweet::createdAt
    
*/
QDateTime TwitterTweetInterface::createdAt() const
{
    Q_D(const TwitterTweetInterface);
    return d->createdAt;
}

/*!
    \qmlproperty int TwitterTweet::favoriteCount
    
*/
int TwitterTweetInterface::favoriteCount() const
{
    Q_D(const TwitterTweetInterface);
    QString numberString = d->data().value(TWITTER_ONTOLOGY_TWEET_FAVORITECOUNT).toString();
    bool ok;
    int number = numberString.toInt(&ok);
    if (ok) {
        return number;
    }
    return -1;
}

/*!
    \qmlproperty bool TwitterTweet::favorited
    
*/
bool TwitterTweetInterface::favorited() const
{
    Q_D(const TwitterTweetInterface);
    return d->data().value(TWITTER_ONTOLOGY_TWEET_FAVORITED).toString() == QLatin1String("true");
}

/*!
    \qmlproperty QString TwitterTweet::filterLevel
    
*/
QString TwitterTweetInterface::filterLevel() const
{
    Q_D(const TwitterTweetInterface);
    return d->data().value(TWITTER_ONTOLOGY_TWEET_FILTERLEVEL).toString();
}

/*!
    \qmlproperty QString TwitterTweet::inReplyToScreenName
    
*/
QString TwitterTweetInterface::inReplyToScreenName() const
{
    Q_D(const TwitterTweetInterface);
    return d->data().value(TWITTER_ONTOLOGY_TWEET_INREPLYTOSCREENNAME).toString();
}

/*!
    \qmlproperty QString TwitterTweet::inReplyToStatusIdentifier
    
*/
QString TwitterTweetInterface::inReplyToStatusIdentifier() const
{
    Q_D(const TwitterTweetInterface);
    return d->data().value(TWITTER_ONTOLOGY_TWEET_INREPLYTOSTATUSIDENTIFIER).toString();
}

/*!
    \qmlproperty QString TwitterTweet::inReplyToUserIdentifier
    
*/
QString TwitterTweetInterface::inReplyToUserIdentifier() const
{
    Q_D(const TwitterTweetInterface);
    return d->data().value(TWITTER_ONTOLOGY_TWEET_INREPLYTOUSERIDENTIFIER).toString();
}

/*!
    \qmlproperty QString TwitterTweet::lang
    
*/
QString TwitterTweetInterface::lang() const
{
    Q_D(const TwitterTweetInterface);
    return d->data().value(TWITTER_ONTOLOGY_TWEET_LANG).toString();
}

/*!
    \qmlproperty bool TwitterTweet::possiblySensitive
    
*/
bool TwitterTweetInterface::possiblySensitive() const
{
    Q_D(const TwitterTweetInterface);
    return d->data().value(TWITTER_ONTOLOGY_TWEET_POSSIBLYSENSITIVE).toString() == QLatin1String("true");
}

/*!
    \qmlproperty int TwitterTweet::retweetCount
    
*/
int TwitterTweetInterface::retweetCount() const
{
    Q_D(const TwitterTweetInterface);
    QString numberString = d->data().value(TWITTER_ONTOLOGY_TWEET_RETWEETCOUNT).toString();
    bool ok;
    int number = numberString.toInt(&ok);
    if (ok) {
        return number;
    }
    return -1;
}

/*!
    \qmlproperty bool TwitterTweet::retweeted
    
*/
bool TwitterTweetInterface::retweeted() const
{
    Q_D(const TwitterTweetInterface);
    return d->data().value(TWITTER_ONTOLOGY_TWEET_RETWEETED).toString() == QLatin1String("true");
}

/*!
    \qmlproperty QString TwitterTweet::source
    
*/
QString TwitterTweetInterface::source() const
{
    Q_D(const TwitterTweetInterface);
    return d->data().value(TWITTER_ONTOLOGY_TWEET_SOURCE).toString();
}

/*!
    \qmlproperty QString TwitterTweet::text
    
*/
QString TwitterTweetInterface::text() const
{
    Q_D(const TwitterTweetInterface);
    return d->data().value(TWITTER_ONTOLOGY_TWEET_TEXT).toString();
}

/*!
    \qmlproperty bool TwitterTweet::truncated
    
*/
bool TwitterTweetInterface::truncated() const
{
    Q_D(const TwitterTweetInterface);
    return d->data().value(TWITTER_ONTOLOGY_TWEET_TRUNCATED).toString() == QLatin1String("true");
}

/*!
    \qmlproperty TwitterUserInterface * TwitterTweet::user
    
*/
TwitterUserInterface * TwitterTweetInterface::user() const
{
    Q_D(const TwitterTweetInterface);
    return d->user;
}

/*!
    \qmlproperty bool TwitterTweet::withheldCopyright
    
*/
bool TwitterTweetInterface::withheldCopyright() const
{
    Q_D(const TwitterTweetInterface);
    return d->data().value(TWITTER_ONTOLOGY_TWEET_WITHHELDCOPYRIGHT).toString() == QLatin1String("true");
}

/*!
    \qmlproperty QString TwitterTweet::withheldScope
    
*/
QString TwitterTweetInterface::withheldScope() const
{
    Q_D(const TwitterTweetInterface);
    return d->data().value(TWITTER_ONTOLOGY_TWEET_WITHHELDSCOPE).toString();
}

