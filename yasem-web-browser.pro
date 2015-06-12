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
    webpage.cpp \
    webview.cpp \
    webpluginfactoryimpl.cpp \
    interceptormanager.cpp \
    networkreply.cpp \
    browserkeyevent.cpp \
    $${CORE_ROOT_DIR}/stbpluginobject.cpp \
    $${CORE_ROOT_DIR}/mediaplayerpluginobject.cpp \
    webkitbrowserplugin.cpp \
    webkitpluginobject.cpp \
    $${CORE_ROOT_DIR}/abstracthttpproxy.cpp

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
    $${CORE_ROOT_DIR}/stbpluginobject.h \
    $${CORE_ROOT_DIR}/browserpluginobject.h \
    $${CORE_ROOT_DIR}/mediaplayerpluginobject.h \
    webkitbrowserplugin.h \
    webkitpluginobject.h \
    $${CORE_ROOT_DIR}/abstracthttpproxy.h \
    $${CORE_ROOT_DIR}/guipluginobject.h

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

RESOURCES += \
    resources.qrc

contains(DEFINES, USE_QML_WIDGETS) {
    CONFIG += qml quick
    SOURCES += webviewqml.cpp
    HEADERS += webviewqml.h
}
