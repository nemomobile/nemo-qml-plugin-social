#!/bin/bash
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

# Regenerate all the Facebook*Interface based on the .json
# files that are provided.

# Move the ontology file
mv ../src/facebook/facebookontology_p.h .
mv facebook/*.json .

for file in `ls -1 *.json`; do
    # Generate ontologies
    ./ontology-writer.py facebookontology_p.h $file Facebook
   
    headerfile=facebook${file/.json/}interface.h
    sourcefile=facebook${file/.json/}interface.cpp
    privatefile=facebook${file/.json/}interface_p.h
    docfile=${file/.json/}.doc

    # Create doc
    if [ -f $docfile ]
    then
        ./doctojson.py $docfile $file
    fi
    
    # Move source and header files
    mv ../src/facebook/$headerfile .
    mv ../src/facebook/$sourcefile .
    if [ -f ../src/facebook/$privatefile ]
    then
        mv ../src/facebook/$privatefile .
    fi
    
    ./interface-writer.py $file Facebook
    
    # Move the files back
    mv $headerfile ../src/facebook/
    mv $sourcefile ../src/facebook/
    
    if [ -f $privatefile ]
    then
        mv $privatefile ../src/facebook/
    fi
done

mv *.json facebook/
mv facebookontology_p.h ../src/facebook/