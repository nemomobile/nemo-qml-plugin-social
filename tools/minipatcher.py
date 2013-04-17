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

def extract(fileName, startMarker, endMarker):
    currentLine = 0
    markers = []
    data = {}
    try:
        f = open(fileName, "r")
    except:
        return {"markers": [], "data": {}}

    currentEntries = []
    for line in f:
        currentLine += 1
        
        # While there are entries to be written, we should
        # write the entries in the data dictionnary
        # except if the line is an end marker
        if line.strip().startswith(endMarker):
            entry = line.strip()[len(endMarker):].strip()
            
            if not entry in currentEntries:
                print "Line " + str(currentLine) + ": got an end marker without start marker"
            else:
                currentEntries.remove(entry)
        
        if len(currentEntries) != 0:
            for entry in currentEntries:
                data[entry] += line
        
        if line.strip().startswith(startMarker):
            # Extract the entry that is being written
            entry = line.strip()[len(startMarker):].strip()
            
            # We should not have repeated start markers like
            # START_MARKER
            # some code
            # more code
            # START_MARKER
            if entry in currentEntries:
                print "Line " + str(currentLine) + ": the entry " + entry + " is already being written"
            else:
                currentEntries.append(entry)
                data[entry] = ""
                markers.append(entry)
    f.close()
    
    return {"markers": markers, "data": data}
