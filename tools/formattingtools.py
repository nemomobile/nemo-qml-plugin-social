# Copyright (C) 2013 Jolla Ltd. <chris.adams@jollamobile.com>
#
# You may use this file under the terms of the BSD license as follows:
#
# "Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are
# met:
#   * Redistributions of source code must retain the above copyright
#     notice, this list of conditions and the following disclaimer.
#   * Redistributions in binary form must reproduce the above copyright
#     notice, this list of conditions and the following disclaimer in
#     the documentation and/or other materials provided with the
#     distribution.
#   * Neither the name of Nemo Mobile nor the names of its contributors
#     may be used to endorse or promote products derived from this
#     software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
# OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."

def upperCamelCase(splitted):
    camelCased = camelCase(splitted)
    return camelCased[0].upper() + camelCased[1:]

def isPointer(name):
    if name.strip()[-1:] == "*":
        return True
    else:
        return False
        
def removePointer(name):
    return name.replace("*", "").strip()

def split(name):
    return name.split("_")

def camelCase(splitted):
    newSplitted = []
    for splittedWord in splitted:
        splittedWord = splittedWord.lower()
        splittedWord = splittedWord[0].upper() + splittedWord[1:]
        newSplitted.append(splittedWord)
    camelCase = "".join(newSplitted)
    camelCase = camelCase[0].lower() + camelCase[1:]
    return camelCase

def ontologyKey(splitted, ontologyPrefix):
    key = "FACEBOOK_ONTOLOGY_"
    if len(splitted) == 0:
        key += ontologyPrefix
        return key
    key += ontologyPrefix + "_"
    for splittedWord in splitted:
        key += splittedWord.upper()
    return key

def addSpaces(string, size):
    spacesToAdd = max(size - len(string), 0)
    return string + (" " * spacesToAdd)
