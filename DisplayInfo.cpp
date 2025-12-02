#include "DisplayInfo.h"
#include "CacheHandler.h"

#include <QThread>

extern "C" {
#include <ddcutil_c_api.h>
}

const uint8_t VCP_BRIGHTNESS = 0x10;

class VCPHandler: public QThread
{
public:
    VCPHandler(QObject *parent): QThread(parent) {}

protected:
    void run()
    {
        DisplayInfo *m_display = reinterpret_cast<DisplayInfo *>(parent());
        if (!m_display->m_ref) {
            DDCA_Display_Identifier id;
            if (ddca_create_mfg_model_sn_display_identifier(m_display->m_info.mfg.toUtf8(), m_display->m_info.name.toUtf8(),
                    m_display->m_info.sn.toUtf8(), &id) != 0)
                return;
            DDCA_Status status = ddca_get_display_ref(id, &m_display->m_ref);
            ddca_free_display_identifier(id);
            if (status != 0) {
                m_display->m_ref = nullptr;
                return;
            }
        }

        DDCA_Display_Handle handle;
        if (ddca_open_display2(m_display->m_ref, false, &handle) != 0)
            return;
        m_display->m_brightness_mutex.lock();
        uint16_t brightness = m_display->m_info.current;
        m_display->m_brightness_mutex.unlock();
        ddca_set_non_table_vcp_value(handle, VCP_BRIGHTNESS, brightness >> 8, brightness & 0xFF);
        ddca_close_display(handle);
    }
};

std::list<DisplayInfo> DisplayInfo::s_display_list;

DisplayInfo::DisplayInfo(const InfoStruct &info, DDCA_Display_Ref ref)
    : m_info(info)
    , m_ref(ref)
    , m_delay_timer(new QTimer(this))
    , m_vcp_handler(nullptr)
{
    m_delay_timer->setInterval(300);
    m_delay_timer->setSingleShot(true);
    connect(m_delay_timer, &QTimer::timeout, this, &DisplayInfo::delayTimeout);
}

const std::list<DisplayInfo> &DisplayInfo::displayInfoList()
{
    if (!s_display_list.empty())
        return s_display_list;

    std::list<InfoStruct> cache_info = CacheHandler::instance()->getCacheInfo();
    if (!cache_info.empty()) {
        for (auto &info : cache_info)
            s_display_list.emplace_back(info, nullptr);
    } else {
        syncDisplayInfoList();
    }
    return s_display_list;
}

void DisplayInfo::syncDisplayInfoList()
{
    s_display_list.clear();
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
        s_display_list.emplace_back(
                InfoStruct{list->info[i].model_name, list->info[i].mfg_id, list->info[i].sn, max_val, cur_val, cur_val},
                list->info[i].dref);

        ddca_close_display(handle);
    }
    ddca_free_display_info_list(list);
}

void DisplayInfo::updateCache()
{
    for (auto &display : s_display_list) {
        if (display.m_delay_timer->isActive())
            display.m_delay_timer->stop();
    }

    for (auto &display : s_display_list) {
        if (display.m_vcp_handler && display.m_vcp_handler->isRunning())
            display.m_vcp_handler->wait();
    }

    CacheHandler::instance()->updateCacheInfo();
}

void DisplayInfo::updateBrightness(uint16_t value) const
{
    m_info.pending = value;
    if (m_delay_timer->isActive())
        m_delay_timer->stop();
    m_delay_timer->start();
}

void DisplayInfo::delayTimeout()
{
    if (!m_vcp_handler)
        m_vcp_handler = new VCPHandler(this);
    if (m_vcp_handler->isRunning()) {
        updateBrightness(m_info.pending);
        return;
    }
    m_brightness_mutex.lock();
    m_info.current = m_info.pending;
    m_brightness_mutex.unlock();
    m_vcp_handler->start();
}
