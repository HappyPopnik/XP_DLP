#pragma once
#ifndef USER_TRACKER_SERVICE_H_
#define USER_TRACKER_SERVICE_H_

#include <fstream>

#include "service_base.h"

class USBBlockService : public ServiceBase {
public:
    USBBlockService(const USBBlockService& other) = delete;
    USBBlockService& operator=(const USBBlockService& other) = delete;

    USBBlockService(USBBlockService&& other) = delete;
    USBBlockService& operator=(USBBlockService&& other) = delete;

    USBBlockService()
        : ServiceBase(_T("RemoteRouting"),
            _T("Remote Routing"),
            SERVICE_DEMAND_START,
            SERVICE_ERROR_NORMAL,
            SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SESSIONCHANGE) {}
private:
    void OnStart(DWORD argc, TCHAR* argv[]) override;
    void OnStop() override;

#ifdef UNICODE
    using tofstream = std::wofstream;
#else
    using tofstream = std::ofstream;
#endif

};

#endif // USER_TRACKER_SERVICE_H_
