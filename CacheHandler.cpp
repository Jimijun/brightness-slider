#include "CacheHandler.h"

#include <QDir>

const QString gCachePath = ".cache/brightness-slider/";
const QString gFileName = "brightness.cache";

CacheHandler *CacheHandler::instance()
{
    static CacheHandler handler;
    return &handler;
}

CacheHandler::CacheHandler()
{
    m_home_path = qgetenv("HOME");
    if (!m_home_path.isEmpty())
        m_home_path += "/";
}

std::list<DisplayInfo::InfoStruct> CacheHandler::getCacheInfo()
{
    const QString file_path = m_home_path + gCachePath + gFileName;
    std::list<DisplayInfo::InfoStruct> cache_info;
    m_file.setFileName(file_path);
    if (!m_file.open(QIODevice::ReadOnly | QIODevice::Text))
        return cache_info;

    while (!m_file.atEnd()) {
        const QString line = m_file.readLine();
        const QStringList &list = line.split(",");
        if (list.size() != 5) {
            cache_info.clear();
            break;
        }

        const QString &name = list[0];
        const QString &mfg = list[1];
        const QString &sn = list[2];
        const uint16_t max_brightness = list[3].toUShort();
        const uint16_t current_brightness = list[4].toUShort();
        cache_info.push_back(DisplayInfo::InfoStruct{name, mfg, sn, max_brightness, current_brightness});
    }

    m_file.close();
    return cache_info;
}

void CacheHandler::updateCacheInfo()
{
    const QString file_dir = m_home_path + gCachePath;
    QDir dir (file_dir);
    if (!dir.exists()) {
#if QT_VERSION < QT_VERSION_CHECK(6, 10, 0)
        dir.mkdir(file_dir);
#else
        dir.mkpath(file_dir);
#endif
    }

    const QString file_path = file_dir + gFileName;
    m_file.setFileName(file_path);
    if (!m_file.open(QIODevice::WriteOnly | QIODevice::Text))
        return;

    const std::list<DisplayInfo> &list = DisplayInfo::displayInfoList();
    for (const auto &display : list) {
        const DisplayInfo::InfoStruct &info = display.info();
        const QString content = QString("%1,%2,%3,%4,%5\n")
                .arg(info.name).arg(info.mfg).arg(info.sn).arg(info.max).arg(info.current);
        m_file.write(content.toUtf8());
    }

    m_file.close();
}
