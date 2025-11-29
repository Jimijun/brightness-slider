#include "DisplayInfo.h"

extern "C" {
#include <ddcutil_c_api.h>
}

const uint8_t VCP_BRIGHTNESS = 0x10;

std::list<DisplayInfo> DisplayInfo::s_info_list;

DisplayInfo::DisplayInfo(const QString &name, uint16_t current, uint16_t max, DDCA_Display_Ref ref)
    : m_name(name)
    , m_max(max)
    , m_current(current)
    , m_ref(ref)
    , m_delay_timer(new QTimer(this))
{
    m_delay_timer->setInterval(300);
    m_delay_timer->setSingleShot(true);
    connect(m_delay_timer, &QTimer::timeout, this, &DisplayInfo::delayTimeout);
}

const std::list<DisplayInfo> &DisplayInfo::displayInfoList()
{
    if (!s_info_list.empty())
        return s_info_list;

    DDCA_Display_Info_List *list = nullptr;
    ddca_get_display_info_list2(false, &list);
    DDCA_Status status;
    for (size_t i = 0; i < list->ct; ++i) {
        DDCA_Display_Handle handle;
        status = ddca_open_display2(list->info[i].dref, false, &handle);
        if (status != 0)
            continue;

        DDCA_Non_Table_Vcp_Value vcp_value;
        ddca_get_non_table_vcp_value(handle, VCP_BRIGHTNESS, &vcp_value);
        uint16_t max_val = vcp_value.mh << 8 | vcp_value.ml;
        uint16_t cur_val = vcp_value.sh << 8 | vcp_value.sl;
        s_info_list.emplace_back(list->info[i].model_name, cur_val, max_val, list->info[i].dref);

        ddca_close_display(handle);
    }
    ddca_free_display_info_list(list);

    return s_info_list;
}

void DisplayInfo::updateBrightness(uint16_t value) const
{
    m_current = value;
    if (m_delay_timer->isActive())
        m_delay_timer->stop();
    m_delay_timer->start();
}

void DisplayInfo::delayTimeout()
{
    DDCA_Display_Handle handle;
    if (ddca_open_display2(m_ref, false, &handle) != 0)
        return;
    ddca_set_non_table_vcp_value(handle, VCP_BRIGHTNESS, m_current >> 8, m_current & 0xff);
    ddca_close_display(handle);
}
