#-------------------------------------------------
#
# Project created by QtCreator 2014-02-05T12:01:30
#
#-------------------------------------------------

VERSION = 0.1.0
TARGET = yasem-web-browser
TEMPLATE = lib

include($${top_srcdir}/common.pri)

QT       += core gui widgets webkit webkitwidgets

DEFINES += WEBKITBROWSER_LIBRARY

SOURCES += \
    webview.cpp \
    webpluginfactoryimpl.cpp \
    interceptormanager.cpp \
    networkreply.cpp \
    browserkeyevent.cpp \
    webkitbrowserplugin.cpp \
    webkitpluginobject.cpp \
    qtwebpage.cpp

HEADERS +=\
    webkitbrowser_global.h \
    webview.h \
    webpluginfactory.h \
    webpluginfactoryimpl.h \
    interceptormanager.h \
    networkinterceptorentry.h \
    networkreply.h \
    browserkeyevent.h \
    cmd_line.h \
    webkitbrowserplugin.h \
    webkitpluginobject.h \
    qtwebpage.h

win32: {
HEADERS +=
    $${SDK_DIR}/browser.h
}

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

contains(DEFINES, USE_QML_WIDGETS) {
    CONFIG += qml quick
    SOURCES += webviewqml.cpp
    HEADERS += webviewqml.h
}

RESOURCES +=  resources.qrc
