#include "browserkeyevent.h"
#include <QStringList>
#include <QDebug>
#include <QFile>

using namespace yasem;


QString BrowserKeyEvent::keyboardEventJs = "";
QString BrowserKeyEvent::mouseEventJs = "";

BrowserKeyEvent::BrowserKeyEvent(int keyCode, int which, bool alt, bool ctrl, bool shift)
{
    init(keyCode, which, keyCode, which, alt, ctrl, shift);
}

BrowserKeyEvent::BrowserKeyEvent(int keyCode, int which, int keyCode2, int which2, bool alt, bool ctrl, bool shift)
{
    init(keyCode, which, keyCode2, which2, alt, ctrl, shift);
}

void BrowserKeyEvent::init(int keyCode, int which, int keyCode2, int which2, bool alt, bool ctrl, bool shift)
{
    initJsCode();
    this->keyCode = keyCode;
    this->which = which > 0 ? which : keyCode;
    this->keyCode2 = keyCode2;
    this->which2 = which2 > 0 ? which2 : keyCode2;
    this->alt = alt;
    this->ctrl = ctrl;
    this->shift = shift;

}

QString BrowserKeyEvent::toString()
{
    const int MAX_EVENTS = 6;
    QString eventName = "";
    QStringList result;
    int type = CustomKeyEvent::TypeUnknown;

    QString preData = "\
var activeFrame =  window, elem = null; \
for(var index = 0; index < frames.length; index++)\
{\
   var frame = frames[index];\
   activeFrame = frame;\
}\
if(activeFrame.document.activeElement) elem = activeFrame.document.activeElement; \
else elem = activeFrame.document.body;\
elem.focus();";

    for(int index = 0; index < MAX_EVENTS; index++)
    {
        int keyCode = this->keyCode;
        int which = this->which;
        if(index == CustomKeyEvent::KeyPress)
        {
            eventName = "keypress";
            keyCode = this->keyCode2;
            which = this->which2;
            type = TypeKeyboard;
        }
        else if(index == CustomKeyEvent::KeyDown)
        {
            eventName = "keydown";
            type = TypeKeyboard;
        }
        else if(index == CustomKeyEvent::KeyRelease)
        {
            continue;
            eventName = "keyup";
            type = TypeKeyboard;
        }
        else if(index == CustomKeyEvent::MouseDown)
        {
            if(this->keyCode != 13) continue;
            eventName = "mousedown";
            type = TypeMouse;
        }
        else if(index == CustomKeyEvent::MouseClick)
        {
            if(this->keyCode != 13) continue;
            eventName = "click";
            type = TypeMouse;
        }
        else if(index == CustomKeyEvent::MouseUp)
        {
            continue;
            eventName = "mouseup";
            type = TypeMouse;
        }


        if(type == TypeKeyboard)
        {
            result.append(BrowserKeyEvent::keyboardEventJs
                          .arg(keyCode)
                          .arg(which)
                          .arg(QVariant::fromValue(alt).toString())
                          .arg(QVariant::fromValue(ctrl).toString())
                          .arg(QVariant::fromValue(shift).toString())
                          .arg(eventName));
        }
        else if(type == TypeMouse)
        {
            result.append(BrowserKeyEvent::mouseEventJs.arg(eventName));
        }

    }

    return preData + result.join(" && ") + ";";
}

void BrowserKeyEvent::initJsCode()
{
    if(BrowserKeyEvent::keyboardEventJs.isEmpty())
    {
        QFile res(QString(":/webkitbrowser/js/keyboardevent.js"));
        res.open(QIODevice::ReadOnly|QIODevice::Text);
        BrowserKeyEvent::keyboardEventJs = res.readAll();
    }

    if(BrowserKeyEvent::mouseEventJs.isEmpty())
    {
        QFile res(QString(":/webkitbrowser/js/mouseevent.js"));
        res.open(QIODevice::ReadOnly|QIODevice::Text);
        BrowserKeyEvent::mouseEventJs = res.readAll();
    }
}
