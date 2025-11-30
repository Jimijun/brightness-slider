#pragma once

#include "DisplayInfo.h"

#include <QFile>
#include <QString>

#include <list>

class CacheHandler
{
public:
    static CacheHandler *instance();

    std::list<DisplayInfo::InfoStruct> getCacheInfo();
    void updateCacheInfo();

private:
    CacheHandler();
    ~CacheHandler() {}

    QString m_home_path;
    QFile m_file;
};
