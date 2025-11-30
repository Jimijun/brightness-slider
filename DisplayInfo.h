#pragma once

#include <QString>
#include <QTimer>

#include <cstdint>
#include <list>

extern "C" {
#include <ddcutil_types.h>
}

class DisplayInfo: public QObject
{
    Q_OBJECT
public:
    struct InfoStruct
    {
        const QString name, mfg, sn;
        const uint16_t max;
        mutable uint16_t current;
    };

    DisplayInfo(const InfoStruct &info, DDCA_Display_Ref ref);

    const InfoStruct &info() const { return m_info; }
    void updateBrightness(uint16_t value) const;

    static const std::list<DisplayInfo> &displayInfoList();
    static void updateCache();

private slots:
    void delayTimeout();

private:
    static void syncDisplayInfoList();

    InfoStruct m_info;
    DDCA_Display_Ref m_ref;
    QTimer *m_delay_timer;

    static std::list<DisplayInfo> s_info_list;
};
