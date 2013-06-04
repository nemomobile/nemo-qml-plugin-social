TEMPLATE = subdirs
#SUBDIRS = src
SUBDIRS = src tests
tests.depends = src

OTHER_FILES += \
    rpm/nemo-qml-plugin-social.yaml \
    rpm/nemo-qml-plugin-social.spec \
    rpm/nemo-qml-plugin-social-qt5.yaml \
    rpm/nemo-qml-plugin-social-qt5.spec

