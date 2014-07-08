#-------------------------------------------------
#
# Project created by QtCreator 2014-02-05T12:01:30
#
#-------------------------------------------------

VERSION = 1.0.0

QT       += core gui widgets webkit webkitwidgets

CONFIG += c++11

TARGET = yasem-web-browser
TEMPLATE = lib

DEFINES += WEBKITBROWSER_LIBRARY

INCLUDEPATH += ../../yasem-core

SOURCES += webkitbrowser.cpp \
        webpage.cpp \
        webview.cpp \
        webpluginfactoryimpl.cpp \
    interceptormanager.cpp \
    networkreply.cpp \
    browserkeyevent.cpp

HEADERS += webkitbrowser.h\
        webkitbrowser_global.h \
        webpage.h \
        webview.h \
        webpluginfactory.h \
        webpluginfactoryimpl.h \
    interceptormanager.h \
    networkinterceptorentry.h \
    networkreply.h \
    browserkeyevent.h \
    cmd_line.h

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

