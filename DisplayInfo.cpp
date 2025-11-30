#include "DisplayInfo.h"
#include "CacheHandler.h"

extern "C" {
#include <ddcutil_c_api.h>
}

const uint8_t VCP_BRIGHTNESS = 0x10;

std::list<DisplayInfo> DisplayInfo::s_info_list;

DisplayInfo::DisplayInfo(const InfoStruct &info, DDCA_Display_Ref ref)
    : m_info(info)
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

    std::list<InfoStruct> cache_info = CacheHandler::instance()->getCacheInfo();
    if (!cache_info.empty()) {
        for (auto &info : cache_info)
            s_info_list.emplace_back(info, nullptr);
    } else {
        syncDisplayInfoList();
    }
    return s_info_list;
}

void DisplayInfo::syncDisplayInfoList()
{
    s_info_list.clear();
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
        s_info_list.emplace_back(
                InfoStruct{list->info[i].model_name, list->info[i].mfg_id, list->info[i].sn, max_val, cur_val},
                list->info[i].dref);

        ddca_close_display(handle);
    }
    ddca_free_display_info_list(list);
}

void DisplayInfo::updateCache()
{
    CacheHandler::instance()->updateCacheInfo();
}

void DisplayInfo::updateBrightness(uint16_t value) const
{
    m_info.current = value;
    if (m_delay_timer->isActive())
        m_delay_timer->stop();
    m_delay_timer->start();
}

void DisplayInfo::delayTimeout()
{
    if (!m_ref) {
        DDCA_Display_Identifier id;
        if (ddca_create_mfg_model_sn_display_identifier(m_info.mfg.toUtf8(), m_info.name.toUtf8(),
                m_info.sn.toUtf8(), &id) != 0)
            return;
        DDCA_Status status = ddca_get_display_ref(id, &m_ref);
        ddca_free_display_identifier(id);
        if (status != 0) {
            m_ref = nullptr;
            return;
        }
    }

    DDCA_Display_Handle handle;
    if (ddca_open_display2(m_ref, false, &handle) != 0)
        return;
    ddca_set_non_table_vcp_value(handle, VCP_BRIGHTNESS, m_info.current >> 8, m_info.current & 0xFF);
    ddca_close_display(handle);
}
