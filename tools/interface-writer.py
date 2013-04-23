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

def generate(structure_file):
    struct = structure.extract(structure_file)

    name = formattingtools.upperCamelCase(formattingtools.split(struct.name))
    className = "Facebook" + name + "Interface"
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

    # Get a list the includes that will be used
    includeList = []
    for property in struct.properties:
        include = typehelper.include(struct.types[property])
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
        type = struct.types[property]
        propertyName = formattingtools.camelCase(formattingtools.split(property))
        header += "    Q_PROPERTY(" + type + " " + propertyName + " READ " + propertyName
        header += " NOTIFY " + propertyName + "Changed)\n"
    header += "public:\n"
    header += "    explicit " + className + "(QObject *parent = 0);\n"
    header += "\n"
    header += "    // Overrides.\n"
    header += "    int type() const;\n"
    header += "\n"
    header += "    // Accessors\n"
    for property in struct.properties:
        type = struct.types[property]
        propertyName = formattingtools.camelCase(formattingtools.split(property))
        header += "    " + type + " " + propertyName + "() const;\n"
    header += "Q_SIGNALS:\n"
    for property in struct.properties:
        propertyName = formattingtools.camelCase(formattingtools.split(property))
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

    if struct.identifiable:
        print "Generating source file for identifiable objects is not yet supported"
        return

    # SOURCE
    source = license
    source += "\n"
    source += "#include \"" + className.lower() + ".h\"\n"
    source += "#include \"facebookontology_p.h\"\n"
    source += "#include \"facebookinterface.h\"\n"
    if struct.identifiable:
        source += "#include \"identifiablecontentiteminterface_p.h\"\n"
    else:
        source += "#include \"contentiteminterface_p.h\"\n"
    source += "\n"
    source += "class " + className + "Private: public "
    if struct.identifiable:
        source += "IdentifiableContentItemInterfacePrivate"
    else:
        source += "ContentItemInterfacePrivate"
    source += "\n"
    source += "{\n"
    source += "public:\n"
    source += "    explicit " + className + "Private(" + className + " *q);\n"
    source += "    void emitPropertyChangeSignals(const QVariantMap &oldData, \
const QVariantMap &newData);\n"

    # Add identifiable specific stuff here
    # end
    source += "private:\n"
    source += "     Q_DECLARE_PUBLIC(" + className + ")\n"
    source += "};\n"
    source += "\n"
    source += className + "Private::" + className + "Private(" + className + " *q)\n"
    source += "    : "
    if struct.identifiable:
        source += "IdentifiableContentItemInterfacePrivate"
    else:
        source += "ContentItemInterfacePrivate"
    source += "(q)\n"
    source += "{\n"
    source += "}\n"
    source += "\n"
    source += "void " + className
    source += "Private::emitPropertyChangeSignals(const QVariantMap &oldData,\n"
    source += " " * (40 + len(className)) + "const QVariantMap &newData)\n"
    source += "{\n"
    source += "    Q_Q(" + className + ");\n"
    for property in struct.properties:
        type = struct.types[property]
        splittedPropertyName = formattingtools.split(property)
        propertyName = formattingtools.camelCase(splittedPropertyName)
        upperPropertyName = formattingtools.upperCamelCase(splittedPropertyName)

        source += "    " + type + " old" + upperPropertyName + " = "
        source += "oldData.value(" + formattingtools.ontologyKey(splittedPropertyName, prefix)
        source += ")." + typehelper.convert(type) + ";\n"
        source += "    " + type + " new" + upperPropertyName + " = "
        source += "newData.value(" + formattingtools.ontologyKey(splittedPropertyName, prefix)
        source += ")." + typehelper.convert(type) + ";\n"
    source += "\n"
    for property in struct.properties:
        splittedPropertyName = formattingtools.split(property)
        propertyName = formattingtools.camelCase(splittedPropertyName)
        upperPropertyName = formattingtools.upperCamelCase(splittedPropertyName)

        source += "    if (new" + upperPropertyName + " != " + "old" + upperPropertyName + ")\n"
        source += "        emit q->" + propertyName + "Changed();\n"
    source += "\n"
    source += "    // Call super class implementation\n"
    source += "    "
    if struct.identifiable:
        source += "IdentifiableContentItemInterfacePrivate"
    else:
        source += "ContentItemInterfacePrivate"
    source += "::emitPropertyChangeSignals(oldData, newData);\n"
    source += "}\n"
    source += "\n"
    source += "//-------------------------------\n"
    source += "\n"
    source += className + "::" + className + "(QObject *parent)\n"
    source += "    : "
    if struct.identifiable:
        source += "IdentifiableContentItemInterface"
    else:
        source += "ContentItemInterface"
    source += "(*(new " + className + "Private(this)), parent)\n"
    source += "{\n"
    source += "}\n"
    source += "\n"
    source += "int " + className + "::type() const\n"
    source += "{\n"
    source += "    return FacebookInterface::" + name + ";\n"
    source += "}\n"
    source += "\n"
    for property in struct.properties:
        type = struct.types[property]
        doc = struct.docs[property]
        splittedPropertyName = formattingtools.split(property)
        propertyName = formattingtools.camelCase(splittedPropertyName)
        source += "/*!\n"
        source += "    \qmlproperty " + type + " " + className + "::" + propertyName + "\n"
        # TODO: put doc here !
        source += "    "  + doc + "\n"
        source += "*/\n"
        source += type + " " + className + "::" + propertyName + "() const\n"
        source += "{\n"
        source += "    Q_D(const " + className + ");\n"
        source += "    return d->data().value("
        source += formattingtools.ontologyKey(splittedPropertyName, prefix)
        source += ")." + typehelper.convert(type) + ";\n"
        source += "}\n"
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
