#!/bin/python
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

import argparse
import minipatcher
import structure
import formattingtools
import typehelper

license = """/*
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

def indent(code, count):
    splitted = code.split("\n")
    finalSplitted = []
    for line in splitted:
        finalSplitted.append(("    " * count) + line)
    return "\n".join(finalSplitted)

def getType(property):
    type = property.type
    if property.isPointer:
        type += " *"
    elif property.isReference:
        type += " &"
    
    if property.isReference:
        type = "const " + type
    
    return type

def generate(structure_file):
    struct = structure.extract(structure_file)

    name = formattingtools.upperCamelCase(formattingtools.split(struct.name))
    className = "Facebook" + name + "Interface"
    shortClassName = "Facebook" + name
    prefix = struct.name.upper()

    # HEADER
    header = license
    header += "\n"
    header += "#ifndef " + className.upper() + "_H\n"
    header += "#define " + className.upper() + "_H\n"
    header += "\n"

    if struct.identifiable:
        header += "#include \"identifiablecontentiteminterface.h\"\n"
    else:
        header += "#include \"contentiteminterface.h\"\n"
    header += "\n"

    # Get a list of the includes that will be used
    includeList = []
    for property in struct.properties:
        include = typehelper.include(property)
        if not include is None and not include in includeList:
            includeList.append(include)
    for include in includeList:
        header += "#include " + include + "\n"

    header += "\n"
    header += "/*\n"
    header += " * NOTE: if you construct one of these in C++ directly,\n"
    header += " * you MUST call classBegin() and componentCompleted()\n"
    header += " * directly after construction.\n"
    header += " */\n"
    header += "\n"
    if not struct.identifiable:
        header += "/*\n"
        header += " * NOTE: this is an unidentifiable content item which\n"
        header += " * is read only and only creatable by the top level\n"
        header += " * FacebookInterface.\n"
        header += " */\n"
        header += "\n"


    header += "class " + className + "Private;\n"
    header += "class " + className + ": public "
    if struct.identifiable:
        header += "IdentifiableContentItemInterface"
    else:
        header += "ContentItemInterface"
    header += "\n"
    header += "{\n"
    header += "    Q_OBJECT\n"
    for property in struct.properties:
        type = getType(property)
        propertyName = formattingtools.camelCase(formattingtools.split(property.name))
        header += "    Q_PROPERTY(" + type + " " + propertyName + " READ " + propertyName
        header += " NOTIFY " + propertyName + "Changed)\n"
        
    # Extra code
    if len(struct.extraPublic) > 0:
        header += "public:\n"
        header += indent(struct.extraPublic, 1)
        header += "\n"
    if len(struct.extraProtected) > 0:
        header += "protected:\n"
        header += indent(struct.extraProtected, 1)
        header += "\n"
    if len(struct.extraPrivate) > 0:
        header += "private:\n"
        header += indent(struct.extraPrivate, 1)
        header += "\n"
        
    header += "public:\n"
    header += "    explicit " + className + "(QObject *parent = 0);\n"
    header += "\n"
    header += "    // Overrides.\n"
    header += "    int type() const;\n"
    
    if struct.identifiable:
        header += "    Q_INVOKABLE bool remove();\n"
        header += "    Q_INVOKABLE bool reload(const QStringList &whichFields = QStringList());\n"
        header += "\n"
        header += "    // Invokable API.\n"
        for method in struct.methods:
            header += "    Q_INVOKABLE bool " + method.name + "("
            
            parameterList = []
            for parameter in method.parameters:
                parameterName = getType(parameter) + parameter.name
                if len(parameter.default) > 0:
                    parameterName += " = " + parameter.default
                parameterList.append(parameterName)
            header += ", ".join(parameterList)
            header += ");\n"
        header += "\n"
    
    header += "    // Accessors\n"
    for property in struct.properties:
        type = getType(property)
        propertyName = formattingtools.camelCase(formattingtools.split(property.name))
        header += "    " + type + " " + propertyName + "() const;\n"
    header += "Q_SIGNALS:\n"
    for property in struct.properties:
        propertyName = formattingtools.camelCase(formattingtools.split(property.name))
        header += "    void " + propertyName + "Changed();\n"
    header += "private:\n"
    header += "    Q_DECLARE_PRIVATE(" + className + ")\n"
    header += "};\n"
    header += "\n"
    header += "#endif // " + className.upper() + "_H\n"

    try:
        f = open(className.lower() + ".h", "w")
        f.write(header)
    except:
        print "Failed to write the header in " + className.lower() + ".h"
    f.close()

    # SOURCE
    # Use minipatcher to read the source
    patcherDataPrivate = minipatcher.extract(className.lower() + "_p.h", "// <<<", "// >>>")
    patcherDataSource = minipatcher.extract(className.lower() + ".cpp", "// <<<", "// >>>")

    haveCustom = False
    for property in struct.properties:
        if property.custom:
            haveCustom = True

    private = license
    if struct.identifiable:
        private += "\n"
        private += "#ifndef " + className.upper() + "_P_H\n"
        private += "#define " + className.upper() + "_P_H\n"

    private += "\n"
    private += "#include \"" + className.lower() + ".h\"\n"
    if not struct.identifiable:
        private += "#include \"facebookinterface.h\"\n"
        private += "#include \"facebookontology_p.h\"\n"
    if struct.identifiable:
        private += "#include \"facebookinterface_p.h\"\n"
        private += "#include \"identifiablecontentiteminterface_p.h\"\n"
    else:
        private += "#include \"contentiteminterface_p.h\"\n"
    if not struct.identifiable:
        private += "// <<< include\n"
        if "include" in patcherDataSource["markers"]:
            private += patcherDataSource["data"]["include"]
        else:
            private += "// Includes goes here\n"
        private += "// >>> include\n"
        
    private += "\n"
    private += "class " + className + "Private: public "
    if struct.identifiable:
        private += "IdentifiableContentItemInterfacePrivate"
    else:
        private += "ContentItemInterfacePrivate"
    private += "\n"
    private += "{\n"
    private += "public:\n"
    private += "    explicit " + className + "Private(" + className + " *q);\n"
    if struct.identifiable:
        private += "    void finishedHandler();\n"
    private += "    void emitPropertyChangeSignals(const QVariantMap &oldData, \
const QVariantMap &newData);\n"

    # Add the custom attributes
    if struct.identifiable:
        private += "    FacebookInterfacePrivate::FacebookAction action;\n"
    for property in struct.properties:
        propertyName = formattingtools.camelCase(formattingtools.split(property.name))
        propertyDefinition = getType(property) + " " + propertyName
        if property.custom:
            private += "    " + propertyDefinition + ";\n"
    private += "private:\n"
    private += "    Q_DECLARE_PUBLIC(" + className + ")\n"
    private += "};\n"

    source = ""
    if struct.identifiable:
        private += "\n"
        private += "#endif // " + className.upper() + "_P_H\n"
        try:
            f = open(className.lower() + "_p.h", "w")
            f.write(private)
        except:
            print "Failed to write the private header in " + className.lower() + "_p.h"
        f.close()

        source += license
        source += "\n"
        source += "#include \"" + className.lower() + "_p.h\"\n"
        source += "#include \"facebookontology_p.h\"\n"
        source += "#include \"facebookinterface.h\"\n"
        source += "// <<< include\n"
        if "include" in patcherDataSource["markers"]:
            source += patcherDataSource["data"]["include"]
        else:
            source += "// Includes goes here\n"
        source += "// >>> include\n"
    else:
        source += private
    source += "\n"
    
    source += className + "Private::" + className + "Private(" + className + " *q)\n"
    source += "    : "
    if struct.identifiable:
        source += "IdentifiableContentItemInterfacePrivate"
    else:
        source += "ContentItemInterfacePrivate"
    source += "(q)\n"
    if struct.identifiable:
        source += "    , action(FacebookInterfacePrivate::NoAction)\n"

    if haveCustom:
        source += "// <<< custom\n"
        if "custom" in patcherDataSource["markers"]:
            source += patcherDataSource["data"]["custom"]
        else:
            source += "    // TODO Initialize custom attributes here\n"
        source += "// >>> custom\n"


    source += "{\n"
    source += "}\n"
    source += "\n"
    
    if struct.identifiable:
        source += "void " + className + "Private::finishedHandler()\n"
        source += "{\n"
        source += "// <<< finishedHandler\n"
        if "finishedHandler" in patcherDataSource["markers"]:
            source += patcherDataSource["data"]["finishedHandler"]
        else:
            source += "    // TODO Implement finishedHandler here\n"
        source += "// >>> finishedHandler\n"
        source += "}"
        source += "\n"
    
    
    source += "void " + className
    source += "Private::emitPropertyChangeSignals(const QVariantMap &oldData,\n"
    source += " " * (40 + len(className)) + "const QVariantMap &newData)\n"
    source += "{\n"
    source += "    Q_Q(" + className + ");\n"
    
    unknownProperties = []
    
    for property in struct.properties:
        type = getType(property)
        splittedPropertyName = formattingtools.split(property.name)
        propertyName = formattingtools.camelCase(splittedPropertyName)
        upperPropertyName = formattingtools.upperCamelCase(splittedPropertyName)

        if len(typehelper.convert(type)) > 0 and not property.custom:
            source += "    " + type + " old" + upperPropertyName + " = "
            source += "oldData.value(" + formattingtools.ontologyKey(splittedPropertyName, prefix)
            source += ")." + typehelper.convert(type) + ";\n"
            source += "    " + type + " new" + upperPropertyName + " = "
            source += "newData.value(" + formattingtools.ontologyKey(splittedPropertyName, prefix)
            source += ")." + typehelper.convert(type) + ";\n"
        else:
            unknownProperties.append(property.name)
    source += "\n"
    for property in struct.properties:
        if not property.name in unknownProperties:
            splittedPropertyName = formattingtools.split(property.name)
            propertyName = formattingtools.camelCase(splittedPropertyName)
            upperPropertyName = formattingtools.upperCamelCase(splittedPropertyName)

            source += "    if (new" + upperPropertyName + " != " + "old" + upperPropertyName + ")\n"
            source += "        emit q->" + propertyName + "Changed();\n"
    
    if haveCustom:
        source += "\n"
        source += "// <<< emitPropertyChangeSignals\n"
        if "emitPropertyChangeSignals" in patcherDataSource["markers"]:
            source += patcherDataSource["data"]["emitPropertyChangeSignals"]
        else:
            source += "    // TODO Implement emitPropertyChangeSignals here if needed\n"
            if len(unknownProperties) > 0:
                source += "    // WARNING: those properties are not handled automatically:\n"
                for unknownProperty in unknownProperties:
                    source += "    // - " + unknownProperty + "\n"
        source += "// >>> emitPropertyChangeSignals\n"
    
    source += "\n"
    source += "    // Call super class implementation\n"
    if struct.identifiable:
        source += "    QVariantMap oldDataWithId = oldData;\n"
        source += "    oldDataWithId.insert(NEMOQMLPLUGINS_SOCIAL_CONTENTITEMID,\n"
        source += "                         oldData.value(FACEBOOK_ONTOLOGY_METADATA_ID));\n"
        source += "    QVariantMap newDataWithId = newData;\n"
        source += "    newDataWithId.insert(NEMOQMLPLUGINS_SOCIAL_CONTENTITEMID,\n"
        source += "                         newData.value(FACEBOOK_ONTOLOGY_METADATA_ID));\n"
        source += "    IdentifiableContentItemInterfacePrivate"
        source += "::emitPropertyChangeSignals(oldDataWithId, newDataWithId);\n"
    else:
        source += "    ContentItemInterfacePrivate"
        source += "::emitPropertyChangeSignals(oldData, newData);\n"
    source += "}\n"
    source += "\n"
    source += "//-------------------------------\n"
    source += "\n"
    source += "/*!\n"
    source += "    \\qmltype " + shortClassName + "\n"
    source += "    \\instantiates " + className + "\n"
    source += indent(struct.doc, 1)
    source += "\n"
    source += "*/\n"
    source += className + "::" + className + "(QObject *parent)\n"
    source += "    : "
    if struct.identifiable:
        source += "IdentifiableContentItemInterface"
    else:
        source += "ContentItemInterface"
    source += "(*(new " + className + "Private(this)), parent)\n"
    source += "{\n"
    if haveCustom:
        source += "// <<< constructor\n"
        if "constructor" in patcherDataSource["markers"]:
            source += patcherDataSource["data"]["constructor"]
        else:
            source += "    // TODO Implement initialization of custom attributes if needed\n"
        source += "// >>> constructor\n"
    
    source += "}\n"
    source += "\n"
    source += "/*! \reimp */\n"
    source += "int " + className + "::type() const\n"
    source += "{\n"
    source += "    return FacebookInterface::" + name + ";\n"
    source += "}\n"
    source += "\n"
    if struct.identifiable:
        source += "/*! \reimp */\n"
        source += "bool " + className +  "::remove()\n"
        source += "{\n"
        source += "// <<< remove\n"
        if "remove" in patcherDataSource["markers"]:
            source += patcherDataSource["data"]["remove"]
        else:
            source += "    // TODO Implement remove if needed\n"
            source += "    return IdentifiableContentItemInterface::remove();\n"
        source += "// >>> remove\n"
        source += "}\n"
        source += "\n"
        source += "/*! \reimp */\n"
        source += "bool " + className +  "::reload(const QStringList &whichFields)\n"
        source += "{\n"
        source += "// <<< reload\n"
        if "reload" in patcherDataSource["markers"]:
            source += patcherDataSource["data"]["reload"]
        else:
            source += "    // TODO Implement reload if needed\n"
            source += "    return IdentifiableContentItemInterface::reload(whichFields);\n"
        source += "// >>> reload\n"
        source += "}\n"
        source += "\n"
    
    
    
    # Methods
    for method in struct.methods:
        
        signature = "bool " + className + "::" + method.name + "("
        parameterList = []
        for parameter in method.parameters:
            parameterName = getType(parameter) + parameter.name
            parameterList.append(parameterName)
        signature += ", ".join(parameterList)
        signature += ")"
        
        source += "/*!\n"
        source += "    \\qmlmethod " + signature + "\n"
        source += indent(method.doc, 1)
        source += "*/\n"
        source += "\n"
        source += signature + "\n"
        source += "{\n"
        source += "// <<< " + method.name + "\n"
        if method.name in patcherDataSource["markers"]:
            source += patcherDataSource["data"][method.name]
        else:
            source += "    // TODO Implement " + method.name + "\n"
        source += "// >>> " + method.name + "\n"
        source += "}\n"
    source += "\n"
        
    
    
    
    for property in struct.properties:
        type = getType(property)
        splittedPropertyName = formattingtools.split(property.name)
        propertyName = formattingtools.camelCase(splittedPropertyName)
        source += "/*!\n"
        source += "    \qmlproperty " + type + " " + className + "::" + propertyName + "\n"
        source += indent(property.doc, 1) + "\n"
        source += "*/\n"
        source += type + " " + className + "::" + propertyName + "() const\n"
        source += "{\n"
        source += "    Q_D(const " + className + ");\n"
        if not property.custom:
            source += "    return d->data().value("
            source += formattingtools.ontologyKey(splittedPropertyName, prefix)
            source += ")." + typehelper.convert(type) + ";\n"
        else:
            source += "    return d->" + propertyName + ";\n"
        source += "}\n"
        source += "\n"
        
    if len(struct.extraSource) > 0:
        source += indent(struct.extraPublic, 0)
        source += "\n"

    try:
        f = open(className.lower() + ".cpp", "w")
        f.write(source)
    except:
        print "Failed to write the source in " + className.lower() + ".cpp"
    f.close()

# Main
if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Interface writer')
    parser.add_argument('structure_file', metavar='structure_file', type=str,
                        help="""Input Facebook object structure file (JSON)""")
    args = parser.parse_args()
    structure_file = args.structure_file
    generate(structure_file)
