typedef struct IUnknown IUknown;

#include <windows.h>
#include <iostream>
#include <Dbt.h>
#include <string>
#include <initguid.h>
#include <string.h>
#include <comdef.h>
#include <vector>
#include <fstream>
#include <cctype>

#include "Global.h"
#include "Monitor_USB_Connection.h"
#include "eject_process.h"
#include "Function_Helper.h"

std::vector<std::string>* globalWhiteList;
std::string whitelistpath;

struct USBInformation
{
    unsigned int pid;
    unsigned int vid;
} typedef USBInformation, * lpUSBInformation;

void blockUSB(void* data) {
    // This function handles calls to block USB connections.
    if (data != NULL) {
        lpUSBInformation usbInfo = (lpUSBInformation)data;
        unsigned int pid = usbInfo->pid;
        unsigned int vid = usbInfo->vid;
        //usbEjectDevice(vid, pid);
        boolean ejected = false;
        int counter = 0;
        while (((!ejected || counter < 240) && (counter < 1200)) && (state == 0)) {
            Sleep(1000);
            ejected = usbEjectDevice(vid, pid);
            counter++;
        }
        
        delete data;
    }
}

void allocateGlobalArray()
// This function will allocate a new vector for the global whitelist
{
    globalWhiteList = new std::vector<std::string>();
}

void copyArray(std::string source[], int size)
// This function copys an array to a vector
{
    for (int i = 0; i < size; i++)
    {
        globalWhiteList->push_back(source[i]);
    }
}

bool in_array(const std::string& value, const std::vector<std::string>& arr)
// This fucntion checks of a string is in vector 
{
    return std::find(arr.begin(), arr.end(), value) != arr.end();
}

DEFINE_GUID(GUID_DEVINTERFACE_USB_DEVICE, 0xA5DCBF10L, 0x6530, 0x11D2, 0x90, 0x1F, 0x00, 0xC0, 0x4F, 0xB9, 0x51, 0xED);
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    if (uMsg == WM_DEVICECHANGE && wParam == DBT_DEVICEARRIVAL) {
        PDEV_BROADCAST_HDR dbhdr = reinterpret_cast<PDEV_BROADCAST_HDR>(lParam);
        std::wstring vid;
        std::wstring pid;
        std::wstring serial;
        if (dbhdr != nullptr) {
            if (dbhdr->dbch_devicetype == DBT_DEVTYP_DEVICEINTERFACE) {
                PDEV_BROADCAST_DEVICEINTERFACE dbdi = reinterpret_cast<PDEV_BROADCAST_DEVICEINTERFACE>(dbhdr);

                if (dbdi != nullptr) {
                    if (dbdi->dbcc_classguid == GUID_DEVINTERFACE_USB_DEVICE) {
                        LogWriter(("USB device connected!"));
                        std::wstring deviceName(dbdi->dbcc_name);
                        std::wstring lowercasedevicename;
                        for (wchar_t c : deviceName)
                        {
                            lowercasedevicename += std::tolower(c);
                        }
                        size_t vidPos = lowercasedevicename.find(L"vid_");
                        size_t pidPos = lowercasedevicename.find(L"pid_");
                        size_t endserialPos = lowercasedevicename.rfind(L"#");
                        
                        if (vidPos != std::wstring::npos && pidPos != std::wstring::npos) {
                            vid = deviceName.substr(vidPos + 4, 4);
                            pid = deviceName.substr(pidPos + 4, 4);
                            serial = deviceName.substr(pidPos + 9, endserialPos-(pidPos + 9)); // Go the 4 spots for pid, the 4 spots of pid and the hash spot.
                            std::wstring lowercaseidentifier;
                            for (wchar_t c : (vid + pid + serial))
                            {
                                lowercaseidentifier += std::tolower(c);
                            }
                            if (state == 1 && !in_array(ws2s(lowercaseidentifier), *globalWhiteList)) // learning mode and not in whitelist
                            {
                                std::string aa = "VID: " + ws2s(vid) + " PID: " + ws2s(pid) + " Serial: " + ws2s(serial);
                                LogWriter("Writing USB to whitelist: " + aa);
                                MessageBox(NULL, L"A known USB device was allowed!", L"Warning", MB_OK | MB_ICONWARNING);
                                std::wofstream outFile{};
                                outFile.open(whitelistpath, std::ios::app);
                                writeToUSBDevicesFile(vid, pid, serial, outFile);
                                globalWhiteList->push_back(ws2s(vid + pid + serial));
                            }
                            else if (state == 0 && !in_array(ws2s(lowercaseidentifier), *globalWhiteList)) // if on blocking and not in whitelist
                            {
                                LogWriter("Software in blocking mode, Thus it will block the USB.");
                                const WCHAR* vidWStr = vid.c_str();
                                _bstr_t vidStr(vidWStr);
                                const char* vidStr_t = vidStr;
                                unsigned int outVid = 0;
                                sscanf(vidStr_t, "%x", &outVid);
                                const WCHAR* pidWStr = pid.c_str();
                                _bstr_t pidStr(pidWStr);
                                const char* pidStr_t = pidStr;
                                unsigned int outPid = 0;
                                sscanf(pidStr_t, "%x", &outPid);
                                std::string aa = "VID: " + ws2s(vid) + " PID: " + ws2s(pid) + " Serial: " + ws2s(serial);
                                LogWriter(("Blocked USB: " + aa));
                                MessageBox(NULL, L"An unknown USB device was blocked!", L"Warning", MB_OK | MB_ICONWARNING);
                                lpUSBInformation lpUsbData = new USBInformation();
                                lpUsbData->pid = outPid;
                                lpUsbData->vid = outVid;
                                HANDLE hThread = CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)blockUSB, (LPVOID)lpUsbData, NULL, NULL);
                            }
                            else if (state == 2) // if on simple allow all devices
                            {
                                const WCHAR* vidWStr = vid.c_str();
                                const WCHAR* pidWStr = pid.c_str();
                                std::string aa = "VID: " + ws2s(vid) + " PID: " + ws2s(pid) + " Serial: " + ws2s(serial);
                                LogWriter("Allowed USB on simple unhardening: " + aa);
                                MessageBox(NULL, L"An unknown USB device was allowed!", L"Warning", MB_OK | MB_ICONWARNING);
                            }
                            else if (state == 0 && in_array(ws2s(lowercaseidentifier), *globalWhiteList))
                            {
                                std::string aa = "VID: " + ws2s(vid) + " PID: " + ws2s(pid) + " Serial: " + ws2s(serial);
                                LogWriter("USB was connected on whitelist on blocking: " + aa);
                            }
                            else if (state == 1 && in_array(ws2s(lowercaseidentifier), *globalWhiteList))
                            {
                                std::string aa = "VID: " + ws2s(vid) + " PID: " + ws2s(pid) + " Serial: " + ws2s(serial);
                                LogWriter("USB was connected on whitelist on learning: " + aa);
                            }
                            else
                            {
                                std::string aa = "VID: " + ws2s(vid) + " PID: " + ws2s(pid) + " Serial: " + ws2s(serial);
                                LogWriter("Something went wrong with USB device: " + aa);
                            }
                        }
                    }
                }
            }
        }
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}



int MonitoringProcess(std::string* whitelistarray, int sizearray, std::string whitelistpathf)
{
    if (state == 1)
        LogWriter(("State of product is learning mode"));
    else
        LogWriter(("State of product is blocking mode"));
    allocateGlobalArray();
    copyArray(whitelistarray, sizearray);
    whitelistpath = whitelistpathf;
    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = GetModuleHandle(nullptr);
    wc.lpszClassName = L"USBNotificationWindowClass";

    if (RegisterClass(&wc)) {
        HWND hwnd = CreateWindow(wc.lpszClassName, L"USB Notification Window",
            0, 0, 0, 0, 0, HWND_MESSAGE, nullptr, wc.hInstance, nullptr);

        if (hwnd) {
            DEV_BROADCAST_DEVICEINTERFACE notificationFilter;
            ZeroMemory(&notificationFilter, sizeof(notificationFilter));
            notificationFilter.dbcc_size = sizeof(notificationFilter);
            notificationFilter.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
            notificationFilter.dbcc_classguid = GUID_DEVINTERFACE_USB_DEVICE;

            HDEVNOTIFY hDevNotify = RegisterDeviceNotification(hwnd, &notificationFilter, DEVICE_NOTIFY_WINDOW_HANDLE);

            if (hDevNotify != nullptr) {
                MSG msg;
                while (GetMessage(&msg, nullptr, 0, 0)) {
                    TranslateMessage(&msg);
                    DispatchMessage(&msg);
                }

                UnregisterDeviceNotification(hDevNotify);
            }
            else {
                LogWriter(("Failed to register for device notifications"));
            }
        }
        else {
            LogWriter(("Failed to create window"));
        }
    }
    else {
        LogWriter(("Failed to register window class"));
    }
    return 0;
}