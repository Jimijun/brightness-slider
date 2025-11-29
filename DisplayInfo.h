#pragma once

#include <QString>
#include <QTimer>

#include <cstdint>
#include <list>

extern "C" {
#include <ddcutil_types.h>
}

class DDCUtil;

class DisplayInfo: public QObject
{
    Q_OBJECT
public:
    DisplayInfo(const QString &name, uint16_t current, uint16_t max, DDCA_Display_Ref ref);

    const QString &name() const { return m_name; }
    const uint16_t maxBrightness() const { return m_max; }
    uint16_t currentBrightness() const { return m_current; }
    void updateBrightness(uint16_t value) const;

    static const std::list<DisplayInfo> &displayInfoList();

private slots:
    void delayTimeout();

private:
    const QString m_name;
    const uint16_t m_max;
    mutable uint16_t m_current;
    const DDCA_Display_Ref m_ref;
    QTimer *m_delay_timer;

    static std::list<DisplayInfo> s_info_list;
};
