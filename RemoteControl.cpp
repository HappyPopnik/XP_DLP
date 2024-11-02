typedef struct IUnknown IUknown;

#include <Windows.h>
#include <iostream>

#include "RemoteControl.h"
#include "Global.h"
#include "RemoteControlFunctions.h"
#include "Function_Helper.h"
#include "Registry_handler.h"

DWORD WINAPI LikeClockwork(LPVOID a)
// Timer for learning mode. when timer finished it will restate the state
{
    Sleep(300000);
    // fun time over
    LogWriter("learning mode timer is over");
    state = 0;
    return 0;
}

void WaitForRegistryChange(HKEY hKey, const wchar_t* subKey, const wchar_t* valuename) {
    Sleep(10000);
    LogWriter("Started Registry watch");
    HANDLE hThread;
    DWORD dwThreadID;
    HKEY hKey2;
    DWORD dwDataSize = MAX_PATH;
    char dwData[MAX_PATH];
    DWORD dwType;
    while (true)
    {
        HANDLE event = CreateEvent(NULL, TRUE, FALSE, NULL);
        if (event == NULL) {
            std::cerr << "Error creating event object: " << GetLastError() << std::endl;
            return;
        }
        LONG result = RegNotifyChangeKeyValue(hKey, TRUE, REG_NOTIFY_CHANGE_LAST_SET, event, TRUE);
        if (result != ERROR_SUCCESS) {
            std::cerr << "Error registering for registry change notifications: " << result << std::endl;
            CloseHandle(event);
            return;
        }
        DWORD waitResult = WaitForSingleObject(event, INFINITE);

        if (waitResult == WAIT_OBJECT_0) {
            DWORD value;
            DWORD valueSize = sizeof(DWORD);

            result = RegQueryValueEx(hKey, valuename, NULL, NULL, (LPBYTE)&value, &valueSize);

            if (result == ERROR_SUCCESS) {
                if (value == 1) {
                    LogWriter("Registry key has changed to learning mode");
                    if (state == 1)
                    {
                        // read from registry
                        if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Hardening", 0, KEY_READ, &hKey2) == ERROR_SUCCESS)
                        {
                            if (RegQueryValueEx(hKey2, L"ReleasedUntil", NULL, &dwType, (LPBYTE)dwData, &dwDataSize) == ERROR_SUCCESS)
                            {
                                std::string timetolearn = "State is already on learning. Will be on learning until: " + (std::string)dwData;
                                LogWriter(timetolearn);
                            }
                            RegCloseKey(hKey2);
                        }
                        else
                        {
                            LogWriter("State is already on learning.");
                        }
                    }
                    else if (state == 2)
                    {
                        // read from registry
                        if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Hardening", 0, KEY_READ, &hKey2) == ERROR_SUCCESS)
                        {
                            if (RegQueryValueEx(hKey2, L"ReleasedUntil", NULL, &dwType, (LPBYTE)dwData, &dwDataSize) == ERROR_SUCCESS)
                            {
                                std::string timetolearn = "State is already on unblocking. Will be on learning until: " + (std::string)dwData;
                                LogWriter(timetolearn);
                            }
                            RegCloseKey(hKey2);
                        }
                        else
                        {
                            LogWriter("State is already on learning.");
                        }
                    }
                    else
                    {
                        LogWriter("Changed state of software to learning.");
                        SetRegistryValueTo0(HKEY_LOCAL_MACHINE, subKey, valuename);
                        state = 1;
                        Learningmodupdate("SOFTWARE\\Microsoft\\Hardening", "ReleasedUntil", 1);
                        hThread = CreateThread(NULL, 0, LikeClockwork, NULL, 0, &dwThreadID);
                    }
                }
                else if (value == 2)
                {
                    LogWriter("Registry key has changed to unblocking mode");
                    if (state == 1)
                    {
                        // read from registry
                        if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Hardening", 0, KEY_READ, &hKey2) == ERROR_SUCCESS)
                        {
                            if (RegQueryValueEx(hKey2, L"ReleasedUntil", NULL, &dwType, (LPBYTE)dwData, &dwDataSize) == ERROR_SUCCESS)
                            {
                                std::string timetolearn = "State is already on learning. Will be on learning until: " + (std::string)dwData;
                                LogWriter(timetolearn);
                            }
                            RegCloseKey(hKey2);
                        }
                        else
                        {
                            LogWriter("State is already on learning.");
                        }
                    }
                    else if (state == 2)
                    {
                        // read from registry
                        if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Hardening", 0, KEY_READ, &hKey2) == ERROR_SUCCESS)
                        {
                            if (RegQueryValueEx(hKey2, L"ReleasedUntil", NULL, &dwType, (LPBYTE)dwData, &dwDataSize) == ERROR_SUCCESS)
                            {
                                std::string timetolearn = "State is already on blocking. Will be on learning until: " + (std::string)dwData;
                                LogWriter(timetolearn);
                            }
                            RegCloseKey(hKey2);
                        }
                        else
                        {
                            LogWriter("State is already on learning.");
                        }
                    }
                    else
                    {
                        LogWriter("Changed state of software to unblocking.");
                        SetRegistryValueTo0(HKEY_LOCAL_MACHINE, subKey, valuename);
                        state = 2;
                        Learningmodupdate("SOFTWARE\\Microsoft\\Hardening", "ReleasedUntil", 1);
                        hThread = CreateThread(NULL, 0, LikeClockwork, NULL, 0, &dwThreadID);
                    }
                }
                else if (value == -1)
                {
                    state = 1;
                    Learningmodupdate("SOFTWARE\\Microsoft\\Hardening", "ReleasedUntil", 0);
                }
                else if (value != 0)
                {
                    LogWriter("registry value changed to illegal number.");
                    SetRegistryValueTo0(HKEY_LOCAL_MACHINE, subKey, valuename);
                }
            }
            else {
                std::cerr << "Error querying registry value: " << result << std::endl;
            }
        }
        else {
            std::cerr << "Error waiting for registry change: " << GetLastError() << std::endl;
        }

        CloseHandle(event);
    }
}

void WatchDogger(const wchar_t* subKey, const wchar_t* valuename) {
    HKEY hKey;
    LONG result = RegOpenKeyEx(HKEY_LOCAL_MACHINE, subKey, 0, KEY_NOTIFY | KEY_READ, &hKey);
    while (true)
    {
        if (result == ERROR_SUCCESS) {
            std::cout << "Starting watchdog" << std::endl;
            WaitForRegistryChange(hKey, subKey, valuename);
        }
        else {
            std::cerr << "Error opening registry key: " << result << std::endl;
            break;
        }
    }
    RegCloseKey(hKey);
}
