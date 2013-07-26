#!/bin/sh

if [ ! -d qt5 ]; then
    mkdir qt5
fi

for qml in `ls *.qml`; do
    sed 's|import QtQuick 1.*|import QtQuick 2.0|g' < $qml | sed 's|import QtWebKit 1.*|import QtWebKit 3.0|g' > qt5/$qml
done

ls qt5/*.qml
