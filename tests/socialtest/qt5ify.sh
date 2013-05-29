#!/bin/sh

mkdir qt5

for qml in `ls *.qml`; do
    sed 's|import QtQuick 1.*|import QtQuick 2.0|g' < $qml > qt5/$qml
done

ls qt5/*.qml
