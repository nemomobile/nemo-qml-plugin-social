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

#include "twitterdatautil_p.h"

#include <QtCore/QVariantMap>
#include <QtCore/QObject>
#include <QtCore/QList>
#include <QtCore/QString>
#include <QtCore/QByteArray>
#include <QtCore/QUuid>
#include <QtCore/qmath.h>
#include <QtCore/QDateTime>
#include <QtCore/QUrl>
#include <QtCore/QStringList>

#include <QCryptographicHash>
#include <QtCore/QDebug>

//#if (QT_VERSION < QT_VERSION_CHECK(5, 0, 0))
//#include <qjson/parser.h>
//#else
//#include <QJsonDocument>
//#endif

static const char *OAUTH_SIGNATURE_METHOD = "HMAC-SHA1";
static const char *OAUTH_VERSION = "1.0";


// This function taken from http://qt-project.org/wiki/HMAC-SHA1 which is in the public domain
// and carries no licensing requirements (as at 2013-05-09)

// TODO XXX: investigate QMessageAuthenticationCode when porting to qt 5.1
// that should make that thing easier
static QByteArray hmacSha1(const QByteArray &signingKey, const QByteArray &baseArray)
{
    QByteArray key = signingKey;

    int blockSize = 64; // HMAC-SHA-1 block size, defined in SHA-1 standard
    if (key.length() > blockSize) { // if key is longer than block size (64), reduce key length with SHA-1 compression
        key = QCryptographicHash::hash(key, QCryptographicHash::Sha1);
    }
 
    QByteArray innerPadding(blockSize, char(0x36)); // initialize inner padding with char "6"
    QByteArray outerPadding(blockSize, char(0x5c)); // initialize outer padding with char "\"
    // ascii characters 0x36 ("6") and 0x5c ("\") are selected because they have large
    // Hamming distance (http://en.wikipedia.org/wiki/Hamming_distance)
 
    for (int i = 0; i < key.length(); i++) {
        innerPadding[i] = innerPadding[i] ^ key.at(i); // XOR operation between every byte in key and innerpadding, of key length
        outerPadding[i] = outerPadding[i] ^ key.at(i); // XOR operation between every byte in key and outerpadding, of key length
    }
 
    // result = hash ( outerPadding CONCAT hash ( innerPadding CONCAT baseArray ) ).toBase64
    QByteArray total = outerPadding;
    QByteArray part = innerPadding;
    part.append(baseArray);
    total.append(QCryptographicHash::hash(part, QCryptographicHash::Sha1));
    QByteArray hashed = QCryptographicHash::hash(total, QCryptographicHash::Sha1);
    return hashed.toBase64();
}

QByteArray TwitterDataUtil::authorizationHeader(const QByteArray &oauthConsumerKey,
                                                const QByteArray &oauthConsumerSecret,
                                                const QByteArray &requestMethod,
                                                const QByteArray &requestUrl,
                                                const QList<QPair<QByteArray, QByteArray> > &parameters,
                                                const QByteArray &oauthToken,
                                                const QByteArray &oauthTokenSecret,
                                                const QByteArray &oauthNonce,
                                                const QByteArray &oauthTimestamp)
{
    // Twitter requires all requests to be signed with an authorization header.
    QByteArray nonce = oauthNonce;
    if (nonce.isEmpty()) {
        nonce = QUuid::createUuid().toByteArray().toBase64();
    }
    QByteArray oauthSignature;
    QByteArray timestamp = oauthTimestamp;
    if (timestamp.isEmpty()) {
        timestamp = QByteArray::number(qFloor(QDateTime::currentMSecsSinceEpoch() / 1000.0));
    }

    // now build up the encoded parameters map.  We use a map to perform alphabetical sorting.
    QMap<QByteArray, QByteArray> encodedParams;
    encodedParams.insert(QUrl::toPercentEncoding(QByteArray("oauth_consumer_key")),
                         QUrl::toPercentEncoding(oauthConsumerKey));
    encodedParams.insert(QUrl::toPercentEncoding(QByteArray("oauth_nonce")),
                         QUrl::toPercentEncoding(nonce));
    encodedParams.insert(QUrl::toPercentEncoding(QByteArray("oauth_signature_method")),
                         QUrl::toPercentEncoding(QByteArray(OAUTH_SIGNATURE_METHOD)));
    encodedParams.insert(QUrl::toPercentEncoding(QByteArray("oauth_timestamp")),
                         QUrl::toPercentEncoding(timestamp));
    encodedParams.insert(QUrl::toPercentEncoding(QByteArray("oauth_version")),
                         QUrl::toPercentEncoding(QByteArray(OAUTH_VERSION)));
    if (!oauthToken.isEmpty()) {
        encodedParams.insert(QUrl::toPercentEncoding(QByteArray("oauth_token")),
                             QUrl::toPercentEncoding(oauthToken));
    }
    for (int i = 0; i < parameters.size(); ++i) {
        QPair<QByteArray, QByteArray> param = parameters.at(i);
        encodedParams.insert(QUrl::toPercentEncoding(param.first),
                             QUrl::toPercentEncoding(param.second));
    }

    QByteArray parametersByteArray;
    QList<QByteArray> keys = encodedParams.keys();
    foreach (const QByteArray &key, keys) {
        parametersByteArray += key + QByteArray("=") + encodedParams.value(key)
                            + QByteArray("&");
    } 
    parametersByteArray.chop(1);

    QByteArray signatureBaseString = requestMethod.toUpper() + QByteArray("&")
                                   + QUrl::toPercentEncoding(requestUrl)
                                   + QByteArray("&") + QUrl::toPercentEncoding(parametersByteArray);


    QByteArray signingKey = QUrl::toPercentEncoding(oauthConsumerSecret) + QByteArray("&")
                          + QUrl::toPercentEncoding(oauthTokenSecret);

    oauthSignature = hmacSha1(signingKey, signatureBaseString);
    encodedParams.insert(QUrl::toPercentEncoding(QByteArray("oauth_signature")),
                         QUrl::toPercentEncoding(oauthSignature));

    // now generate the Authorization header from the encoded parameters map.
    // we need to remove the query items from the encoded parameters map first.
    QByteArray authHeader = QByteArray("OAuth ");


    QList<QPair<QByteArray, QByteArray> >::const_iterator i;
    for (i = parameters.begin(); i != parameters.end(); ++i) {
        encodedParams.remove(QUrl::toPercentEncoding((*i).first));
    }
    keys = encodedParams.keys();
    foreach (const QByteArray &key, keys) {
        authHeader += key + "=\"" + encodedParams.value(key) + "\", ";
    } 
    authHeader.chop(2);

    return authHeader;
}


//QVariant TwitterDataUtil::parseReplyData(const QByteArray &replyData, bool *ok)
//{
//    QVariant parsed;

//#if (QT_VERSION < QT_VERSION_CHECK(5, 0, 0))
//    QJson::Parser jsonParser;
//    parsed = jsonParser.parse(replyData, ok);
//#else
//    QJsonDocument jsonDocument = QJsonDocument::fromJson(replyData);
//    *ok = !doc.isEmpty();
//    parsed = doc.toVariant();
//#endif

//    if (*ok && parsed.type() == QVariant::Map) {
//        return parsed.toMap();
//    } else if (*ok && parsed.type() == QVariant::List) {
//        return parsed.toList();
//    }

//    *ok = false;
//    return QVariantMap();
//}
