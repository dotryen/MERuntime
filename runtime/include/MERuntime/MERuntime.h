//
// Created by ryen on 2/20/25.
//

#pragma once

#include <MECore/MECore.h>

namespace ME {
    struct AppInfo {
        std::string name;
    };

    inline AppInfo appInfo;

    bool Runtime_Initialize();
    void Runtime_Shutdown();
}
