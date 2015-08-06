#include "browserkeyevent.h"
#include <QStringList>
#include <QDebug>
#include <QFile>

using namespace yasem;


QString BrowserKeyEvent::keyboardEventJs = "";
QString BrowserKeyEvent::mouseEventJs = "";
QString BrowserKeyEvent::m_pre_data = "";

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
    this->m_key_code = keyCode;
    this->m_which = which > 0 ? which : keyCode;
    this->m_key_code2 = keyCode2;
    this->m_which_2 = which2 > 0 ? which2 : keyCode2;
    this->m_alt = alt;
    this->m_ctrl = ctrl;
    this->m_shift = shift;

}

QString BrowserKeyEvent::toString()
{
    const int MAX_EVENTS = 6;
    QString eventName = "";
    QStringList result;
    int type = CustomKeyEvent::TypeUnknown;



    for(int index = 0; index < MAX_EVENTS; index++)
    {
        int keyCode = this->m_key_code;
        int which = this->m_which;
        if(index == CustomKeyEvent::KeyPress)
        {
            eventName = "keypress";
            keyCode = this->m_key_code2;
            which = this->m_which_2;
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
            //eventName = "keyup";
            //type = TypeKeyboard;
        }
        else if(index == CustomKeyEvent::MouseDown)
        {
            if(this->m_key_code != 13) continue;
            eventName = "mousedown";
            type = TypeMouse;
        }
        else if(index == CustomKeyEvent::MouseClick)
        {
            if(this->m_key_code != 13) continue;
            eventName = "click";
            type = TypeMouse;
        }
        else if(index == CustomKeyEvent::MouseUp)
        {
            continue;
            //eventName = "mouseup";
            //type = TypeMouse;
        }


        if(type == TypeKeyboard)
        {
            result.append(BrowserKeyEvent::keyboardEventJs
                          .arg(keyCode)
                          .arg(which)
                          .arg(QVariant::fromValue(m_alt).toString())
                          .arg(QVariant::fromValue(m_ctrl).toString())
                          .arg(QVariant::fromValue(m_shift).toString())
                          .arg(eventName));
        }
        else if(type == TypeMouse)
        {
            result.append(BrowserKeyEvent::mouseEventJs.arg(eventName));
        }

    }

    return QString("(function(){").append(m_pre_data).append(result.join(" && ")).append("})();");
}

void BrowserKeyEvent::initJsCode()
{
    if(BrowserKeyEvent::keyboardEventJs.isEmpty())
    {
        QFile res(QString(":/webkitbrowser/js/keyboardevent.js"));
        res.open(QIODevice::ReadOnly|QIODevice::Text);
        BrowserKeyEvent::keyboardEventJs = res.readAll();

        QFile pre_data(QString(":/webkitbrowser/js/js_pre_data.js"));
        pre_data.open(QIODevice::ReadOnly|QIODevice::Text);
        BrowserKeyEvent::m_pre_data = pre_data.readAll();
    }

    if(BrowserKeyEvent::mouseEventJs.isEmpty())
    {
        QFile res(QString(":/webkitbrowser/js/mouseevent.js"));
        res.open(QIODevice::ReadOnly|QIODevice::Text);
        BrowserKeyEvent::mouseEventJs = res.readAll();
    }
}
