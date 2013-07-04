TEMPLATE = app
TARGET = snitest

equals(QT_MAJOR_VERSION, 4): {
    QT += declarative network
    CONFIG  += qtestlib
}
equals(QT_MAJOR_VERSION, 5): {
    QT += qml network testlib
}


lessThan(QT_MAJOR_VERSION, 5) {
    CONFIG += link_pkgconfig
    PKGCONFIG += QJson
}

equals(QT_MAJOR_VERSION, 5): DEFINES += QT_VERSION_5

INCLUDEPATH += ../../src/

HEADERS +=  ../../src/contentiteminterface.h \
            ../../src/contentiteminterface_p.h \
            ../../src/identifiablecontentiteminterface.h \
            ../../src/identifiablecontentiteminterface_p.h \
            ../../src/socialnetworkinterface.h \
            ../../src/socialnetworkinterface_p.h \
            ../../src/filterinterface.h \
            ../../src/contentitemtypefilterinterface.h \

SOURCES +=  ../../src/contentiteminterface.cpp \
            ../../src/identifiablecontentiteminterface.cpp \
            ../../src/socialnetworkinterface.cpp \
            ../../src/filterinterface.cpp \
            ../../src/contentitemtypefilterinterface.cpp \
            main.cpp
