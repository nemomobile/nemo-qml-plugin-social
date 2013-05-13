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

#ifndef TWITTERONTOLOGY_P_H
#define TWITTERONTOLOGY_P_H

// <<< default
// Have to be purged
/* Type names */

// identifiable types
#define TWITTER_ONTOLOGY_TYPE_USER                                  QLatin1String("user")
#define TWITTER_ONTOLOGY_TYPE_USER_ME                               QLatin1String("me")
#define TWITTER_ONTOLOGY_TYPE_TWEET                                 QLatin1String("tweet")
#define TWITTER_ONTOLOGY_TYPE_PLACE                                 QLatin1String("place")
#define TWITTER_ONTOLOGY_TYPE_DIRECTMESSAGE                         QLatin1String("direct_message")

// non-identifiable types
#define TWITTER_ONTOLOGY_TYPE_PLACE_BOUNDINGBOX                     QLatin1String("bounding_box")
#define TWITTER_ONTOLOGY_TYPE_PLACE_GEOMETRY                        QLatin1String("geometry")
#define TWITTER_ONTOLOGY_TYPE_PLACE_COORDINATES                     QLatin1String("coordinates")
#define TWITTER_ONTOLOGY_TYPE_TREND                                 QLatin1String("trend")
#define TWITTER_ONTOLOGY_TYPE_LIST                                  QLatin1String("list")


/* Properties of various types */

// Properties of the Tweet type
#define TWITTER_ONTOLOGY_PROPERTY_TWEET_COORDINATES                 QLatin1String("coordinates")
#define TWITTER_ONTOLOGY_PROPERTY_TWEET_FAVORITED                   QLatin1String("favorited") /* this one has no u */
#define TWITTER_ONTOLOGY_PROPERTY_TWEET_TRUNCATED                   QLatin1String("truncated")
#define TWITTER_ONTOLOGY_PROPERTY_TWEET_CREATEDAT                   QLatin1String("created_at")
#define TWITTER_ONTOLOGY_PROPERTY_TWEET_ID                          QLatin1String("id")
#define TWITTER_ONTOLOGY_PROPERTY_TWEET_IDSTR                       QLatin1String("id_str")
#define TWITTER_ONTOLOGY_PROPERTY_TWEET_ENTITIES                    QLatin1String("entities")
#define TWITTER_ONTOLOGY_PROPERTY_TWEET_INREPLYTOSCREENNAME         QLatin1String("in_reply_to_screen_name")
#define TWITTER_ONTOLOGY_PROPERTY_TWEET_INREPLYTOUSERID             QLatin1String("in_reply_to_user_id")
#define TWITTER_ONTOLOGY_PROPERTY_TWEET_INREPLYTOUSERIDSTR          QLatin1String("in_reply_to_user_id_str")
#define TWITTER_ONTOLOGY_PROPERTY_TWEET_INREPLYTOSTATUSID           QLatin1String("in_reply_to_status_id")
#define TWITTER_ONTOLOGY_PROPERTY_TWEET_INREPLYTOSTATUSIDSTR        QLatin1String("in_reply_to_status_id_str")
#define TWITTER_ONTOLOGY_PROPERTY_TWEET_CONTRIBUTORS                QLatin1String("contributors")
#define TWITTER_ONTOLOGY_PROPERTY_TWEET_TEXT                        QLatin1String("text")
#define TWITTER_ONTOLOGY_PROPERTY_TWEET_RETWEETCOUNT                QLatin1String("retweet_count")
#define TWITTER_ONTOLOGY_PROPERTY_TWEET_GEO                         QLatin1String("geo")
#define TWITTER_ONTOLOGY_PROPERTY_TWEET_RETWEETED                   QLatin1String("retweeted")
#define TWITTER_ONTOLOGY_PROPERTY_TWEET_POSSIBLYSENSITIVE           QLatin1String("possibly_sensitive")
#define TWITTER_ONTOLOGY_PROPERTY_TWEET_PLACE                       QLatin1String("place")
#define TWITTER_ONTOLOGY_PROPERTY_TWEET_USER                        QLatin1String("user")
#define TWITTER_ONTOLOGY_PROPERTY_TWEET_SOURCE                      QLatin1String("source")

// Properties of the User type
#define TWITTER_ONTOLOGY_PROPERTY_USER_PROFILESIDEBARFILLCOLOR      QLatin1String("profile_sidebar_fill_color")
#define TWITTER_ONTOLOGY_PROPERTY_USER_PROFILESIDEBARBORDERCOLOR    QLatin1String("profile_sidebar_border_color")
#define TWITTER_ONTOLOGY_PROPERTY_USER_PROFILEBACKGROUNDCOLOR       QLatin1String("profile_background_color")
#define TWITTER_ONTOLOGY_PROPERTY_USER_PROFILEBACKGROUNDTILE        QLatin1String("profile_background_tile")
#define TWITTER_ONTOLOGY_PROPERTY_USER_PROFILEBACKGROUNDIMAGEURL    QLatin1String("profile_background_image_url")
#define TWITTER_ONTOLOGY_PROPERTY_USER_PROFILEBACKGROUNDIMAGEURLHTTPS   QLatin1String("profile_background_image_url_https")
#define TWITTER_ONTOLOGY_PROPERTY_USER_PROFILEIMAGEURL              QLatin1String("profile_image_url")
#define TWITTER_ONTOLOGY_PROPERTY_USER_PROFILEIMAGEURLHTTPS         QLatin1String("profile_image_url_https")
#define TWITTER_ONTOLOGY_PROPERTY_USER_PROFILELINKCOLOR             QLatin1String("profile_link_color")
#define TWITTER_ONTOLOGY_PROPERTY_USER_PROFILEUSEBACKGROUNDIMAGE    QLatin1String("profile_use_background_image")
#define TWITTER_ONTOLOGY_PROPERTY_USER_PROFILETEXTCOLOR             QLatin1String("profile_text_color")
#define TWITTER_ONTOLOGY_PROPERTY_USER_DEFAULTPROFILEIMAGE          QLatin1String("default_profile_image")
#define TWITTER_ONTOLOGY_PROPERTY_USER_NAME                         QLatin1String("name")
#define TWITTER_ONTOLOGY_PROPERTY_USER_CREATEDAT                    QLatin1String("created_at")
#define TWITTER_ONTOLOGY_PROPERTY_USER_LOCATION                     QLatin1String("location")
#define TWITTER_ONTOLOGY_PROPERTY_USER_FOLLOWREQUESTSENT            QLatin1String("follow_request_sent")
#define TWITTER_ONTOLOGY_PROPERTY_USER_ISTRANSLATOR                 QLatin1String("is_translator")
#define TWITTER_ONTOLOGY_PROPERTY_USER_IDSTR                        QLatin1String("id_str")
#define TWITTER_ONTOLOGY_PROPERTY_USER_ID                           QLatin1String("id")
#define TWITTER_ONTOLOGY_PROPERTY_USER_DEFAULTPROFILE               QLatin1String("default_profile")
#define TWITTER_ONTOLOGY_PROPERTY_USER_CONTRIBUTORSENABLED          QLatin1String("contributors_enabled")
#define TWITTER_ONTOLOGY_PROPERTY_USER_FAVOURITESCOUNT              QLatin1String("favourites_count") /* not a typo, u. */
#define TWITTER_ONTOLOGY_PROPERTY_USER_URL                          QLatin1String("url")
#define TWITTER_ONTOLOGY_PROPERTY_USER_UTCOFFSET                    QLatin1String("utc_offset")
#define TWITTER_ONTOLOGY_PROPERTY_USER_LISTEDCOUNT                  QLatin1String("listed_count")
#define TWITTER_ONTOLOGY_PROPERTY_USER_LANG                         QLatin1String("lang")
#define TWITTER_ONTOLOGY_PROPERTY_USER_FOLLOWERSCOUNT               QLatin1String("followers_count")
#define TWITTER_ONTOLOGY_PROPERTY_USER_PROTECTED                    QLatin1String("protected")
#define TWITTER_ONTOLOGY_PROPERTY_USER_NOTIFICATIONS                QLatin1String("notifications")
#define TWITTER_ONTOLOGY_PROPERTY_USER_VERIFIED                     QLatin1String("verified")
#define TWITTER_ONTOLOGY_PROPERTY_USER_GEOENABLED                   QLatin1String("geo_enabled")
#define TWITTER_ONTOLOGY_PROPERTY_USER_TIMEZONE                     QLatin1String("time_zone")
#define TWITTER_ONTOLOGY_PROPERTY_USER_DESCRIPTION                  QLatin1String("description")
#define TWITTER_ONTOLOGY_PROPERTY_USER_STATUSESCOUNT                QLatin1String("statuses_count")
#define TWITTER_ONTOLOGY_PROPERTY_USER_FRIENDSCOUNT                 QLatin1String("friends_count")
#define TWITTER_ONTOLOGY_PROPERTY_USER_FOLLOWING                    QLatin1String("following")
#define TWITTER_ONTOLOGY_PROPERTY_USER_SHOWALLINLINEMEDIA           QLatin1String("show_all_inline_media")
#define TWITTER_ONTOLOGY_PROPERTY_USER_SCREENNAME                   QLatin1String("screen_name")
#define TWITTER_ONTOLOGY_PROPERTY_USER_USERID                       QLatin1String("user_id")

// Properties of the Place type
#define TWITTER_ONTOLOGY_PROPERTY_PLACE_ATTRIBUTES                  QLatin1String("attributes")
#define TWITTER_ONTOLOGY_PROPERTY_PLACE_BOUNDINGBOX                 QLatin1String("bounding_box")
#define TWITTER_ONTOLOGY_PROPERTY_PLACE_BOUNDINGBOX_COORDINATES     QLatin1String("coordinates")
#define TWITTER_ONTOLOGY_PROPERTY_PLACE_BOUNDINGBOX_TYPE            QLatin1String("type")
#define TWITTER_ONTOLOGY_PROPERTY_PLACE_CONTAINEDWITHIN             QLatin1String("contained_within")
#define TWITTER_ONTOLOGY_PROPERTY_PLACE_COUNTRY                     QLatin1String("country")
#define TWITTER_ONTOLOGY_PROPERTY_PLACE_COUNTRY_CODE                QLatin1String("country_code")
#define TWITTER_ONTOLOGY_PROPERTY_PLACE_FULLNAME                    QLatin1String("full_name")
#define TWITTER_ONTOLOGY_PROPERTY_PLACE_GEOMETRY                    QLatin1String("geometry")
#define TWITTER_ONTOLOGY_PROPERTY_PLACE_GEOMETRY_COORDINATES        QLatin1String("coordinates")
#define TWITTER_ONTOLOGY_PROPERTY_PLACE_GEOMETRY_TYPE               QLatin1String("type")
#define TWITTER_ONTOLOGY_PROPERTY_PLACE_ID                          QLatin1String("id")
#define TWITTER_ONTOLOGY_PROPERTY_PLACE_NAME                        QLatin1String("name")
#define TWITTER_ONTOLOGY_PROPERTY_PLACE_PLACETYPE                   QLatin1String("place_type")
#define TWITTER_ONTOLOGY_PROPERTY_PLACE_POLYLINES                   QLatin1String("polylines")
#define TWITTER_ONTOLOGY_PROPERTY_PLACE_URL                         QLatin1String("url")

// Property values for various Place properties
#define TWITTER_ONTOLOGY_PROPERTY_PLACE_GEOMETRY_TYPE_POLYGON       QLatin1String("polygon")
#define TWITTER_ONTOLOGY_PROPERTY_PLACE_PLACETYPE_NEIGHBORHOOD      QLatin1String("neighborhood")
#define TWITTER_ONTOLOGY_PROPERTY_PLACE_PLACETYPE_CITY              QLatin1String("city")

#define TWITTER_ONTOLOGY_PROPERTY_DIRECTMESSAGE_ID                  QLatin1String("id")

#define TWITTER_ONTOLOGY_PROPERTY_GENERIC_IDSTR                     QLatin1String("id_str")


/* Connections of various types */

#define TWITTER_ONTOLOGY_CONNECTION_TWEET_RETWEETS                  QLatin1String("statuses/retweets/%1.json")
#define TWITTER_ONTOLOGY_CONNECTION_TWEET_RETWEETERS                QLatin1String("statuses/retweeters/ids.json")

#define TWITTER_ONTOLOGY_CONNECTION_USER_ME_TIMELINE_MENTIONS       QLatin1String("statuses/mentions_timeline.json")
#define TWITTER_ONTOLOGY_CONNECTION_USER_ME_TIMELINE_USER           QLatin1String("statuses/user_timeline.json")
#define TWITTER_ONTOLOGY_CONNECTION_USER_ME_TIMELINE_HOME           QLatin1String("statuses/home_timeline.json")
#define TWITTER_ONTOLOGY_CONNECTION_USER_ME_TIMELINE_RETWEETS       QLatin1String("statuses/retweets_of_me.json")
#define TWITTER_ONTOLOGY_CONNECTION_USER_ME_SEARCH                  QLatin1String("search/tweets.json")
#define TWITTER_ONTOLOGY_CONNECTION_USER_ME_DIRECTMESSAGES_TO       QLatin1String("direct_messages.json")
#define TWITTER_ONTOLOGY_CONNECTION_USER_ME_DIRECTMESSAGES_FROM     QLatin1String("direct_messages/sent.json")
#define TWITTER_ONTOLOGY_CONNECTION_USER_ME_FRIENDS_FOLLOWEES       QLatin1String("friends/ids.json")
#define TWITTER_ONTOLOGY_CONNECTION_USER_ME_FRIENDS_FOLLOWERS       QLatin1String("followers/ids.json")

#define TWITTER_ONTOLOGY_CONNECTION_PLACE_SIMILARPLACES             QLatin1String("geo/similar_places.json")
#define TWITTER_ONTOLOGY_CONNECTION_PLACE_TRENDS                    QLatin1String("trends/place.json")   /* woeid ... */
#define TWITTER_ONTOLOGY_CONNECTION_PLACE_CLOSESTTRENDING           QLatin1String("trends/closest.json") /* woeid ... */


/* Optional query items for connections of various types */

#define TWITTER_ONTOLOGY_CONNECTION_TWEET_COUNT                     QLatin1String("count")
#define TWITTER_ONTOLOGY_CONNECTION_USER_ME_COUNT                   QLatin1String("count")
#define TWITTER_ONTOLOGY_CONNECTION_PLACE_SIMILARPLACES_COUNT       QLatin1String("count")
#define TWITTER_ONTOLOGY_CONNECTION_PLACE_SIMILARPLACES_NAME        QLatin1String("name")


/* Queries for various objects */

#define TWITTER_ONTOLOGY_QUERY_USER                                 QLatin1String("users/show.json")
#define TWITTER_ONTOLOGY_QUERY_USER_ME                              QLatin1String("account/verify_credentials.json")
#define TWITTER_ONTOLOGY_QUERY_TWEET                                QLatin1String("statuses/show/%1.json")
#define TWITTER_ONTOLOGY_QUERY_DIRECTMESSAGE                        QLatin1String("direct_messages/show.json")
#define TWITTER_ONTOLOGY_QUERY_PLACE                                QLatin1String("geo/id/%1.json")
// >>> default

// <<< metadata
#define TWITTER_ONTOLOGY_METADATA_ID                       QLatin1String("id_str")
// >>> metadata

// <<< user
#define TWITTER_ONTOLOGY_USER                              QLatin1String("user")
#define TWITTER_ONTOLOGY_USER_CONTRIBUTORSENABLED          QLatin1String("contributors_enabled")
#define TWITTER_ONTOLOGY_USER_CREATEDAT                    QLatin1String("created_at")
#define TWITTER_ONTOLOGY_USER_DEFAULTPROFILE               QLatin1String("default_profile")
#define TWITTER_ONTOLOGY_USER_DEFAULTPROFILEIMAGE          QLatin1String("default_profile_image")
#define TWITTER_ONTOLOGY_USER_DESCRIPTION                  QLatin1String("description")
#define TWITTER_ONTOLOGY_USER_FAVOURITESCOUNT              QLatin1String("favourites_count")
#define TWITTER_ONTOLOGY_USER_FOLLOWREQUESTSENT            QLatin1String("follow_request_sent")
#define TWITTER_ONTOLOGY_USER_FOLLOWERSCOUNT               QLatin1String("followers_count")
#define TWITTER_ONTOLOGY_USER_FRIENDSCOUNT                 QLatin1String("friends_count")
#define TWITTER_ONTOLOGY_USER_GEOENABLED                   QLatin1String("geo_enabled")
#define TWITTER_ONTOLOGY_USER_ISTRANSLATOR                 QLatin1String("is_translator")
#define TWITTER_ONTOLOGY_USER_LANG                         QLatin1String("lang")
#define TWITTER_ONTOLOGY_USER_LISTEDCOUNT                  QLatin1String("listed_count")
#define TWITTER_ONTOLOGY_USER_LOCATION                     QLatin1String("location")
#define TWITTER_ONTOLOGY_USER_NAME                         QLatin1String("name")
#define TWITTER_ONTOLOGY_USER_PROFILEBACKGROUNDCOLOR       QLatin1String("profile_background_color")
#define TWITTER_ONTOLOGY_USER_PROFILEBACKGROUNDIMAGEURL    QLatin1String("profile_background_image_url")
#define TWITTER_ONTOLOGY_USER_PROFILEBACKGROUNDIMAGEURLHTTPS QLatin1String("profile_background_image_url_https")
#define TWITTER_ONTOLOGY_USER_PROFILEBACKGROUNDTILE        QLatin1String("profile_background_tile")
#define TWITTER_ONTOLOGY_USER_PROFILEBANNERURL             QLatin1String("profile_banner_url")
#define TWITTER_ONTOLOGY_USER_PROFILEIMAGEURL              QLatin1String("profile_image_url")
#define TWITTER_ONTOLOGY_USER_PROFILEIMAGEURLHTTPS         QLatin1String("profile_image_url_https")
#define TWITTER_ONTOLOGY_USER_PROFILELINKCOLOR             QLatin1String("profile_link_color")
#define TWITTER_ONTOLOGY_USER_PROFILESIDEBARBORDERCOLOR    QLatin1String("profile_sidebar_border_color")
#define TWITTER_ONTOLOGY_USER_PROFILESIDEBARFILLCOLOR      QLatin1String("profile_sidebar_fill_color")
#define TWITTER_ONTOLOGY_USER_PROFILETEXTCOLOR             QLatin1String("profile_text_color")
#define TWITTER_ONTOLOGY_USER_PROFILEUSEBACKGROUNDIMAGE    QLatin1String("profile_use_background_image")
#define TWITTER_ONTOLOGY_USER_ISPROTECTED                  QLatin1String("protected")
#define TWITTER_ONTOLOGY_USER_SCREENNAME                   QLatin1String("screen_name")
#define TWITTER_ONTOLOGY_USER_SHOWALLINLINEMEDIA           QLatin1String("show_all_inline_media")
#define TWITTER_ONTOLOGY_USER_STATUSESCOUNT                QLatin1String("statuses_count")
#define TWITTER_ONTOLOGY_USER_TIMEZONE                     QLatin1String("time_zone")
#define TWITTER_ONTOLOGY_USER_URL                          QLatin1String("url")
#define TWITTER_ONTOLOGY_USER_UTCOFFSET                    QLatin1String("utc_offset")
#define TWITTER_ONTOLOGY_USER_VERIFIED                     QLatin1String("verified")
#define TWITTER_ONTOLOGY_USER_WITHHELDINCOUNTRIES          QLatin1String("withheld_in_countries")
#define TWITTER_ONTOLOGY_USER_WITHHELDSCOPE                QLatin1String("withheld_scope")
// >>> user

#endif // TWITTERONTOLOGY_P_H
