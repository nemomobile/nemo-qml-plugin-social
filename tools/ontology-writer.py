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

def generate(ontology_file, structure_file):

    # Extract ontologies and structure
    data = minipatcher.extract(ontology_file, "// <<<", "// >>>")
    struct = structure.extract(structure_file)

    if data == None:
        print "Failed to open the ontology file"
        return

    # Generate the defines lines
    name = struct.name.lower()
    keys = []
    prefix = struct.name.upper()
    defaultKey = "#define " + formattingtools.ontologyKey("", prefix)
    defaultKey = formattingtools.addSpaces(defaultKey, 58) + " QLatin1String(\"" + name + "\")"
    keys.append(defaultKey)
    for property in struct.ontologyProperties:
        splitted = formattingtools.split(property.name)
        key = "#define " + formattingtools.ontologyKey(splitted, prefix)
        key = formattingtools.addSpaces(key, 58) + " QLatin1String(\"" + property.key + "\")"
        keys.append(key)
    entry = "\n".join(keys)

    # Write the new file
    ontology = license
    ontology += "#ifndef FACEBOOKONTOLOGY_P_H\n"
    ontology += "#define FACEBOOKONTOLOGY_P_H\n"
    ontology += "\n"
    for marker in data["markers"]:
        ontology += "// <<< " + marker + "\n"

        if marker != name:
            ontology += data["data"][marker]
        else:
            ontology += entry + "\n"

        ontology += "// >>> " + marker + "\n"
        ontology += "\n"

    if not name in data["markers"]:
        ontology += "// <<< " + name + "\n"
        ontology += entry + "\n"
        ontology += "// >>> " + name + "\n"
        ontology += "\n"
    ontology += "#endif // FACEBOOKONTOLOGY_P_H\n"
    try:
        f = open(ontology_file, 'w')
    except:
        print "Failed write to " + ontology_file
        return
    f.write(ontology)
    f.close()

# Main
if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Ontology writer')
    parser.add_argument('ontology_file', metavar='ontology_file', type=str,
                        help="""Input ontology file""")
    parser.add_argument('structure_file', metavar='structure_file', type=str,
                        help="""Input Facebook object structure file (JSON)""")
    args = parser.parse_args()
    ontology_file = args.ontology_file
    structure_file = args.structure_file
    generate(ontology_file, structure_file)


