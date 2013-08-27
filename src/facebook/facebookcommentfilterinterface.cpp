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

#include "facebookcommentfilterinterface.h"

class FacebookCommentFilterInterfacePrivate
{
public:
    FacebookCommentFilterInterfacePrivate();
    int limit;
    int offset;
    FacebookCommentFilterInterface::RetrieveMode retrieveMode;
};

FacebookCommentFilterInterfacePrivate::FacebookCommentFilterInterfacePrivate():
    limit(20), offset(0), retrieveMode(FacebookCommentFilterInterface::RetrieveOffset)
{
}

// ------------------------------

FacebookCommentFilterInterface::FacebookCommentFilterInterface(QObject *parent):
    FilterInterface(parent), d_ptr(new FacebookCommentFilterInterfacePrivate())
{
}

FacebookCommentFilterInterface::~FacebookCommentFilterInterface()
{
}

int FacebookCommentFilterInterface::limit() const
{
    Q_D(const FacebookCommentFilterInterface);
    return d->limit;
}

void FacebookCommentFilterInterface::setLimit(int limit)
{
    Q_D(FacebookCommentFilterInterface);
    if (d->limit != limit) {
        d->limit = limit;
        emit limitChanged();
    }
}

int FacebookCommentFilterInterface::offset() const
{
    Q_D(const FacebookCommentFilterInterface);
    return d->offset;
}

void FacebookCommentFilterInterface::setOffset(int offset)
{
    Q_D(FacebookCommentFilterInterface);
    if (d->offset != offset) {
        d->offset = offset;
        emit offsetChanged();
    }
}

// Retrieve mode override offset
// If you use RetrieveOffset it will follow the value
// entered in offset, otherwise it will retrieve either
// the first entries, or the last, using count as the
// batch size.
FacebookCommentFilterInterface::RetrieveMode FacebookCommentFilterInterface::retrieveMode() const
{
    Q_D(const FacebookCommentFilterInterface);
    return d->retrieveMode;
}

void FacebookCommentFilterInterface::setRetrieveMode(RetrieveMode retrieveMode)
{
    Q_D(FacebookCommentFilterInterface);
    if (d->retrieveMode != retrieveMode) {
        d->retrieveMode = retrieveMode;
        emit retrieveModeChanged();
    }
}
