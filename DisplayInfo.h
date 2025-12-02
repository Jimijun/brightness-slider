#pragma once

#include <QMutex>
#include <QString>
#include <QTimer>

#include <cstdint>
#include <list>

extern "C" {
#include <ddcutil_types.h>
}

class VCPHandler;

class DisplayInfo: public QObject
{
    Q_OBJECT
    friend class VCPHandler;
public:
    struct InfoStruct
    {
        const QString name, mfg, sn;
        const uint16_t max;
        mutable uint16_t current, pending;
    };

    DisplayInfo(const InfoStruct &info, DDCA_Display_Ref ref);
    DisplayInfo(const DisplayInfo &) = delete;

    const InfoStruct &info() const { return m_info; }
    void updateBrightness(uint16_t value) const;

    static const std::list<DisplayInfo> &displayInfoList(bool force_update = false);
    static void updateCache();

private slots:
    void delayTimeout();

private:
    static void syncDisplayInfoList();

    InfoStruct m_info;
    DDCA_Display_Ref m_ref;
    QTimer *m_delay_timer;

    mutable QMutex m_brightness_mutex;
    VCPHandler *m_vcp_handler;

    static std::list<DisplayInfo> s_display_list;
};
