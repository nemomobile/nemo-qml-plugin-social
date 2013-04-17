TARGET = nemosocial
PLUGIN_IMPORT_PATH = org/nemomobile/social

TEMPLATE = lib
CONFIG += qt plugin hide_symbols
QT += declarative

target.path = $$[QT_INSTALL_IMPORTS]/$$PLUGIN_IMPORT_PATH
INSTALLS += target

qmldir.files += $$_PRO_FILE_PWD_/qmldir
qmldir.path +=  $$[QT_INSTALL_IMPORTS]/$$$$PLUGIN_IMPORT_PATH
INSTALLS += qmldir

QT += declarative network

lessThan(QT_MAJOR_VERSION, 5) {
    CONFIG += link_pkgconfig
    PKGCONFIG += QJson
}

include(facebook/facebook.pri)

SOURCES += \
    $$PWD/plugin.cpp \
    $$PWD/contentiteminterface.cpp \
    $$PWD/filterinterface.cpp \
    $$PWD/contentitemtypefilterinterface.cpp \
    $$PWD/identifiablecontentiteminterface.cpp \
    $$PWD/socialnetworkinterface.cpp \
    $$PWD/sorterinterface.cpp

HEADERS += \
    $$PWD/contentiteminterface.h \
    $$PWD/contentiteminterface_p.h \
    $$PWD/contentitemtypefilterinterface.h \
    $$PWD/filterinterface.h \
    $$PWD/filterinterface_p.h \
    $$PWD/identifiablecontentiteminterface.h \
    $$PWD/identifiablecontentiteminterface_p.h \
    $$PWD/socialnetworkinterface.h \
    $$PWD/socialnetworkinterface_p.h \
    $$PWD/sorterinterface.h \
    $$PWD/sorterinterface_p.h \
    $$PWD/util_p.h

OTHER_FILES += qmldir social.qdoc social.qdocconf

MOC_DIR = $$PWD/../.moc
OBJECTS_DIR = $$PWD/../.obj
