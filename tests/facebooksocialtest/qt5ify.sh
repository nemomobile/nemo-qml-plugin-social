#!/bin/sh

if [ ! -d qt5 ]; then
    mkdir qt5
fi

for qml in `ls *.qml`; do
    sed 's|import QtQuick 1.*|import QtQuick 2.0|g' < $qml > qt5/$qml
done

ls qt5/*.qml
