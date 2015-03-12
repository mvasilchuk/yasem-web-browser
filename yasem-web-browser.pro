#-------------------------------------------------
#
# Project created by QtCreator 2014-02-05T12:01:30
#
#-------------------------------------------------

VERSION = 0.1.0

QT       += core gui widgets webkit webkitwidgets

CONFIG += c++11

TARGET = yasem-web-browser
TEMPLATE = lib

DEFINES += WEBKITBROWSER_LIBRARY

INCLUDEPATH += ../../yasem-core/
DEPENDPATH += ../../yasem-core/

SOURCES += \
        webpage.cpp \
        webview.cpp \
        webpluginfactoryimpl.cpp \
    interceptormanager.cpp \
    networkreply.cpp \
    browserkeyevent.cpp \
    ../../yasem-core/plugin.cpp \
    ../../yasem-core/stbpluginobject.cpp \
    webkitbrowserplugin.cpp \
    webkitpluginobject.cpp


HEADERS +=\
        webkitbrowser_global.h \
        webpage.h \
        webview.h \
        webpluginfactory.h \
        webpluginfactoryimpl.h \
    interceptormanager.h \
    networkinterceptorentry.h \
    networkreply.h \
    browserkeyevent.h \
    cmd_line.h \
    ../../yasem-core/abstractpluginobject.h \
    ../../yasem-core/plugin.h \
    ../../yasem-core/stbpluginobject.h \
    ../../yasem-core/browserpluginobject.h \
    webkitbrowserplugin.h \
    webkitpluginobject.h

unix:!symbian {
    maemo5 {
        target.path = /opt/usr/lib
    } else {
        target.path = /usr/lib
    }
    INSTALLS += target
}

OTHER_FILES += \
    metadata.json \
    js/fix/webobjects.js \
    js/fix/XMLHttpRequest.js \
    js/keyboardevent.js \
    js/mouseevent.js \
    LICENSE \
    README.md

include(../../common.pri)
DESTDIR = $$DEFAULT_PLUGIN_DIR

RESOURCES += \
    resources.qrc

contains(DEFINES, USE_QML_WIDGETS) {
    CONFIG += qml quick
    SOURCES += webviewqml.cpp
    HEADERS += webviewqml.h
}

