TARGET = twittersocialtest
equals(QT_MAJOR_VERSION, 4): PREFIX = /opt/tests/nemo-qml-plugins/social/
equals(QT_MAJOR_VERSION, 5): PREFIX = /opt/tests/nemo-qml-plugins-qt5/social/

equals(QT_MAJOR_VERSION, 4): QT = core gui network declarative
equals(QT_MAJOR_VERSION, 5): QT = core gui network qml quick

target.path = $${PREFIX}

INCLUDEPATH += ../../src/
HEADERS += ../../src/twitter/twitterdatautil_p.h
SOURCES += main.cpp \
            ../../src/twitter/twitterdatautil.cpp

equals(QT_MAJOR_VERSION, 4): OTHER_FILES += *.qml 4/Web.qml
equals(QT_MAJOR_VERSION, 5): OTHER_FILES += $$system(./qt5ify.sh) 5/Web.qml

equals(QT_MAJOR_VERSION, 4): DEFINES *= PLUGIN_PATH=\"\\\"\"$${DEFINES_PREFIX}/$$[QT_INSTALL_IMPORTS]/$$PLUGIN_IMPORT_PATH\"\\\"\"
equals(QT_MAJOR_VERSION, 5): DEFINES *= PLUGIN_PATH=\"\\\"\"$${DEFINES_PREFIX}/$$[QT_INSTALL_QML]/$$PLUGIN_IMPORT_PATH\"\\\"\"
DEFINES *= DEPLOYMENT_PATH=\"\\\"\"$${DEFINES_PREFIX}/$${PREFIX}share/\"\\\"\"

qml.files = $${OTHER_FILES}
qml.path = $${PREFIX}/share

INSTALLS += target qml
