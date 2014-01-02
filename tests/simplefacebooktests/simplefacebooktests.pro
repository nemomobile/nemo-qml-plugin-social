TEMPLATE = app
TARGET = simplefacebooktests

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

HEADERS +=  \
            ../../src/contentiteminterface.h \
            ../../src/contentiteminterface_p.h \
            ../../src/identifiablecontentiteminterface.h \
            ../../src/identifiablecontentiteminterface_p.h \
            ../../src/socialnetworkinterface.h \
            ../../src/socialnetworkinterface_p.h \
            ../../src/socialnetworkmodelinterface.h \
            ../../src/socialnetworkmodelinterface_p.h \
            ../../src/filterinterface.h \
            ../../src/facebook/facebookontology_p.h \
            ../../src/facebook/facebookinterface.h \
            ../../src/facebook//facebookinterface_p.h \
            ../../src/facebook/facebookobjectreferenceinterface.h \
            ../../src/facebook/facebookobjectreferenceinterface_p.h \
            ../../src/facebook/facebookalbuminterface.h \
            ../../src/facebook/facebookalbuminterface_p.h \
            ../../src/facebook/facebookcommentinterface.h \
            ../../src/facebook/facebookcommentinterface_p.h \
            ../../src/facebook/facebooknotificationinterface.h \
            ../../src/facebook/facebooknotificationinterface_p.h \
            ../../src/facebook/facebookphotointerface.h \
            ../../src/facebook/facebookphotointerface_p.h \
            ../../src/facebook/facebookpostinterface.h \
            ../../src/facebook/facebookpostinterface_p.h \
            ../../src/facebook/facebookuserinterface.h \
            ../../src/facebook/facebookuserinterface_p.h \
            ../../src/facebook/facebooklikeinterface.h \
            ../../src/facebook/facebooknametaginterface.h \
            ../../src/facebook/facebookphotoimageinterface.h \
            ../../src/facebook/facebookphototaginterface.h \
            ../../src/facebook/facebookpostactioninterface.h \
            ../../src/facebook/facebookpostpropertyinterface.h \
            ../../src/facebook/facebookusercoverinterface.h \
            ../../src/facebook/facebookuserpictureinterface.h \
            ../../src/facebook/facebookitemfilterinterface.h \
            ../../src/facebook/facebookrelateddatafilterinterface.h
SOURCES +=  \
            ../../src/contentiteminterface.cpp \
            ../../src/identifiablecontentiteminterface.cpp \
            ../../src/socialnetworkinterface.cpp \
            ../../src/socialnetworkmodelinterface.cpp \
            ../../src/filterinterface.cpp \
            ../../src/facebook/facebookinterface.cpp \
            ../../src/facebook/facebookobjectreferenceinterface.cpp \
            ../../src/facebook/facebookalbuminterface.cpp \
            ../../src/facebook/facebookcommentinterface.cpp \
            ../../src/facebook/facebooknotificationinterface.cpp \
            ../../src/facebook/facebookphotointerface.cpp \
            ../../src/facebook/facebookpostinterface.cpp \
            ../../src/facebook/facebookuserinterface.cpp \
            ../../src/facebook/facebooklikeinterface.cpp \
            ../../src/facebook/facebooknametaginterface.cpp \
            ../../src/facebook/facebookphotoimageinterface.cpp \
            ../../src/facebook/facebookphototaginterface.cpp \
            ../../src/facebook/facebookpostactioninterface.cpp \
            ../../src/facebook/facebookpostpropertyinterface.cpp \
            ../../src/facebook/facebookusercoverinterface.cpp \
            ../../src/facebook/facebookuserpictureinterface.cpp \
            ../../src/facebook/facebookitemfilterinterface.cpp \
            ../../src/facebook/facebookrelateddatafilterinterface.cpp \
            main.cpp

RESOURCES += \
    res.qrc

