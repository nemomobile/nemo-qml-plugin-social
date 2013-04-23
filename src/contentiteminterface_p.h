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

#ifndef CONTENTITEMINTERFACE_P_H
#define CONTENTITEMINTERFACE_P_H

#include <QtCore/QVariantMap>

class SocialNetworkInterface;
class ContentItemInterface;
class ContentItemInterfacePrivate
{
public:
    explicit ContentItemInterfacePrivate(ContentItemInterface *q);
    virtual ~ContentItemInterfacePrivate();

    QVariantMap data() const;
    void setData(const QVariantMap &data);

    virtual void emitPropertyChangeSignals(const QVariantMap &oldData, const QVariantMap &newData);
    virtual void initializationComplete();

    // helper api - parse network reply data into QVariantMap
    // TODO: This method should be put in a header containing useful functions, and maybe inlined
    static QVariantMap parseReplyData(const QByteArray &replyData, bool *ok);

    SocialNetworkInterface *socialNetworkInterface;
    bool isInitialized;
protected:
    ContentItemInterface * const q_ptr;
private:
    // Slots
    void socialNetworkStatusChangedHandler();
    Q_DECLARE_PUBLIC(ContentItemInterface)
    QVariantMap m_data;
};

#endif // CONTENTITEMINTERFACE_P_H
