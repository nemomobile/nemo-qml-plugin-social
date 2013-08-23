/*
 * Copyright (C) 2013 Lucien XU <sfietkonstantin@free.fr>
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
 *   * The names of its contributors may not be used to endorse or promote 
 *     products derived from this software without specific prior written 
 *     permission.
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

#ifndef FACEBOOKCOMMENTFILTERINTERFACE_H
#define FACEBOOKCOMMENTFILTERINTERFACE_H

#include "filterinterface.h"

class FacebookCommentFilterInterfacePrivate;
class FacebookCommentFilterInterface: public FilterInterface
{
    Q_OBJECT
    Q_ENUMS(RetrieveMode)
    Q_PROPERTY(int limit READ limit WRITE setLimit NOTIFY limitChanged)
    Q_PROPERTY(int offset READ offset WRITE setOffset NOTIFY offsetChanged)
    Q_PROPERTY(RetrieveMode retrieveMode READ retrieveMode WRITE setRetrieveMode
               NOTIFY retrieveModeChanged)
public:
    enum RetrieveMode {
        RetrieveOffset,
        RetrieveFirst,
        RetrieveLatest
    };
    explicit FacebookCommentFilterInterface(QObject *parent = 0);
    virtual ~FacebookCommentFilterInterface();
    int limit() const;
    void setLimit(int limit);
    int offset() const;
    void setOffset(int offset);
    RetrieveMode retrieveMode() const;
    void setRetrieveMode(RetrieveMode retrieveMode);
Q_SIGNALS:
    void limitChanged();
    void offsetChanged();
    void retrieveModeChanged();
protected:
    QScopedPointer<FacebookCommentFilterInterfacePrivate> d_ptr;
private:
    Q_DECLARE_PRIVATE(FacebookCommentFilterInterface)
};

#endif // FACEBOOKCOMMENTFILTERINTERFACE_H
