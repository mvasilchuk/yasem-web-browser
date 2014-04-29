#ifndef BROWSERKEYEVENT_H
#define BROWSERKEYEVENT_H

#include "customkeyevent.h"

#include <QObject>

namespace yasem
{

class BrowserKeyEvent : public QObject, public CustomKeyEvent
{
    Q_OBJECT

public:
    explicit BrowserKeyEvent(int keyCode, int which, bool alt = false, bool ctrl = false, bool shift = false);
    explicit BrowserKeyEvent(int keyCode, int which, int keyCode2, int which2, bool alt = false, bool ctrl = false, bool shift = false);

    void init(int keyCode, int which, int keyCode2, int which2, bool alt = false, bool ctrl = false, bool shift = false);
    QString toString();



protected:
    void initJsCode();
    static QString keyboardEventJs;
    static QString mouseEventJs;
};



}

#endif // BROWSERKEYEVENT_H
