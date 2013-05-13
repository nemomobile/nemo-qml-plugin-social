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

import sys

# Attributes
_socialNetwork = "" # Should be passed camel-cased
className = ""

def license():
    return """/*
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

"""

def setSocialNetwork(newSocialNetwork):
    global _socialNetwork
    if _socialNetwork != "":
        print "Cannot reset a social network"
        return
    
    if newSocialNetwork != "Facebook" and newSocialNetwork != "Twitter":
        print "Cannot set " + newSocialNetwork + " as a social network"
        sys.exit(0)
    _socialNetwork = newSocialNetwork

def socialNetwork():
    return _socialNetwork



def guardOpening(private = False):
    guard = "#ifndef " + className.upper()
    if private:
        guard += "_P"
    guard += "_H\n"
    guard += "#define " + className.upper()
    if private:
        guard += "_P"
    guard += "_H\n"
    return guard

def guardClosing(private = False):
    guard = "#endif // " + className.upper()
    if private:
        guard += "_P"
    guard += "_H\n"
    return guard

def ontologyKey(splitted, ontologyPrefix):
    key = _socialNetwork.upper() + "_ONTOLOGY_"
    if len(splitted) == 0:
        key += ontologyPrefix
        return key
    key += ontologyPrefix + "_"
    for splittedWord in splitted:
        key += splittedWord.upper()
    return key

def socialNetworkIncludes():
    includes = "#include \"" + _socialNetwork.lower() + "interface.h\"\n"
    includes += "#include \"" + _socialNetwork.lower() + "ontology_p.h\"\n"
    return includes
    
def socialNetworkIdentifiableIncludes():
    includes = "#include \"" + _socialNetwork.lower() + "interface_p.h\"\n"
    return includes
    
def socialNetworkActionAttribute():
    attribute = _socialNetwork + "InterfacePrivate::" + _socialNetwork + "Action action;\n"
    return attribute

def socialNetworkActionAttributeInitialization():
    attribute = "action(" + _socialNetwork +  "InterfacePrivate::NoAction)"
    return attribute

def socialNetworkIdMetadata():
    metadata = _socialNetwork.upper() + "_ONTOLOGY_METADATA_ID"
    return metadata

def socialNetworkType(name):
    type = _socialNetwork + "Interface::" + name
    return type