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

import json
import formattingtools

class Variable:
    def __init__(self):
        self.name = ""
        self.key = ""
        self.type = ""
        self.isPointer = False
        self.isConst = False
        self.isReference = False

class Property(Variable):
    def __init__(self):
        Variable.__init__(self)
        self.doc = ""
        self.custom = False
        self.isOntology = True
        self.isList = False
    
class Parameter(Variable):
    def __init__(self):
        Variable.__init__(self)
        self.default = ""

class Method:
    def __init__(self):
        self.name = ""
        self.parameters = []
        self.doc = ""

class FacebookObject:
    def __init__(self):
        self.identifiable = False
        self.name = ""
        self.doc = ""
        self.properties = []
        self.methods = []
        self.extraPublic = ""
        self.extraProtected = ""
        self.extraPrivate = ""
        self.extraPublicP = ""
        self.extraProtectedP = ""
        self.extraPrivateP = ""
        self.extraSource = ""

def extract(file):
    try:
        f = open(file)
    except:
        print "Failed to open " + file
        return

    data = json.load(f)

    object = FacebookObject()
    object.name = data["name"]
    object.doc = data["doc"]
    object.identifiable = data["identifiable"]
    for propertyAttributes in data["properties"]:
        property = Property()
        property.name = propertyAttributes["name"]
        if property.name == "type":
            # Handle the specific "type"
            property.name = object.name + "_" + property.name
            property.key = "type"
        elif "id" in formattingtools.split(property.name):
            # Replace id with identifier
            realSplittedName = []
            for entry in formattingtools.split(property.name):
                if entry == "id":
                    realSplittedName.append("identifier")
                else:
                    realSplittedName.append(entry)
            property.key = property.name
            property.name = "_".join(realSplittedName)
        else:
            property.key = property.name
        property.type = propertyAttributes["type"]
        if "is_const" in propertyAttributes:
            property.isConst = propertyAttributes["is_const"]
        if "is_pointer" in propertyAttributes:
            property.isPointer = propertyAttributes["is_pointer"]
        if "is_reference" in propertyAttributes:
            property.isReference = propertyAttributes["is_reference"]
        if "custom" in propertyAttributes:
            property.custom = propertyAttributes["custom"]
        # A pointer should be stored in a specific attribute
        if property.isPointer:
            property.custom = True
        if "is_ontology" in propertyAttributes:
            property.isOntology = propertyAttributes["is_ontology"]
        if "is_list" in propertyAttributes:
            property.isList = propertyAttributes["is_list"]
        # A list should be stored in a specific attribute
        if property.isList:
            property.custom = True
            if property.isPointer or property.isReference or property.isConst:
                print "Warning: if a property is a list, it should not be a pointer, a reference or a constant"
                property.isPointer = False
                property.isReference = False
                property.isConst = False
        property.doc = propertyAttributes["doc"]
        object.properties.append(property)

    if "extra_public" in data:
        object.extraPublic = data["extra_public"]
    if "extra_protected" in data:
        object.extraProtected = data["extra_protected"]
    if "extra_private" in data:
        object.extraPrivate = data["extra_private"]
    if "extra_public_p" in data:
        object.extraPublicP = data["extra_public_p"]
    if "extra_protected_p" in data:
        object.extraProtectedP = data["extra_protected_p"]
    if "extra_private_p" in data:
        object.extraPrivateP = data["extra_private_p"]
    if "extra_source" in data:
        object.extraSource = data["extra_source"]

    if "methods" in data:
        for methodAttributes in data["methods"]:
            method = Method()
            method.name = methodAttributes["name"]
            method.doc = methodAttributes["doc"]
            
            for parametersAttributes in methodAttributes["parameters"]:
                parameter = Parameter()
                parameter.name = parametersAttributes["name"]
                parameter.type = parametersAttributes["type"]
                if "default" in parametersAttributes:
                    parameter.default = parametersAttributes["default"]
                if "is_const" in parametersAttributes:
                    parameter.isConst = parametersAttributes["is_const"]
                if "is_pointer" in parametersAttributes:
                    parameter.isPointer = parametersAttributes["is_pointer"]
                if "is_reference" in parametersAttributes:
                    parameter.isReference = parametersAttributes["is_reference"]
                method.parameters.append(parameter)
            
            object.methods.append(method)
    
    f.close()
    return object
