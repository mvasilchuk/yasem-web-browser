#ifndef BROWSERKEYEVENT_H
#define BROWSERKEYEVENT_H

#include "customkeyevent.h"

#include <QObject>

namespace yasem
{

class BrowserKeyEvent : public QObject, public SDK::CustomKeyEvent
{
    Q_OBJECT

public:
    explicit BrowserKeyEvent(int m_key_code, int m_which, bool m_alt = false, bool m_ctrl = false, bool m_shift = false);
    explicit BrowserKeyEvent(int m_key_code, int m_which, int m_key_code2, int m_which_2, bool m_alt = false, bool m_ctrl = false, bool m_shift = false);

    void init(int m_key_code, int m_which, int m_key_code2, int m_which_2, bool m_alt = false, bool m_ctrl = false, bool m_shift = false);
    QString toString();



protected:
    void initJsCode();
    static QString keyboardEventJs;
    static QString mouseEventJs;
    static QString m_pre_data;
};



}

#endif // BROWSERKEYEVENT_H
