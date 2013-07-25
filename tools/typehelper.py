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



def include(property):
    type = property.type
    if type in ["int", "float", "double"]:
        return None
    if type in ["QString", "QVariant", "QUrl", "QVariantMap", "QDateTime"]:
        return "<QtCore/" + type + ">"
    if type in ["QColor"]:
        return "<QtGui/" + type + ">"
    if property.isPointer or property.isList:
        return "\"" + type.lower() + ".h\""

def convert(type, line):
    if type in ["int", "float", "double"]:
        data = "QString numberString = " + line + ".toString();\n"
        data += "bool ok;\n"
        data += type + " number = numberString.to" + type[0].upper() + type[1:]
        data += "(&ok);\n"
        data += "if (ok) {\n"
        data += "    return number;\n"
        data += "}\n"
        if type == "int":
            data += "return -1;"
        else:
            data += "return 0.;"
        return data
    if type == "bool":
        return "return " + line + ".toString() == QLatin1String(\"true\");"
    if type in ["QString"]:
        return "return " + line + ".to" + type[1].upper() + type[2:] + "();"
    if type == "QUrl":
        return "return QUrl::fromEncoded(" + line + ".toString().toLocal8Bit());"
    if type == "QVariantMap":
        return "return " + line + ".toMap();"
    if type == "QColor":
        data = "QString color = " + line + ".toString();\n"
        data += "if (color.startsWith(\"#\")) {\n"
        data += "    return QColor(color);\n"
        data += "} else {\n"
        data += "    color.prepend(\"#\");\n"
        data += "    return QColor(color);\n"
        data += "}"
        return data
    return ""
