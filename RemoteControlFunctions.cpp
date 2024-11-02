typedef struct IUnknown IUknown;

#include <Windows.h>
#include <iostream>

#include "RemoteControlFunctions.h"

void SetRegistryValueTo0(HKEY hKey, const wchar_t* subKey, const wchar_t* valueName) {
    // Open the registry key
    HKEY hSubKey;
    if (RegOpenKeyEx(hKey, subKey, 0, KEY_SET_VALUE, &hSubKey) == ERROR_SUCCESS) {
        DWORD value = 0;
        if (RegSetValueEx(hSubKey, valueName, 0, REG_DWORD, (BYTE*)&value, sizeof(DWORD)) == ERROR_SUCCESS) {
            RegCloseKey(hSubKey);
        }
        RegCloseKey(hSubKey);
    }
    // Close the registry key
    

}

void SetRegistryValueTo1(HKEY hKey, const wchar_t* subKey, const wchar_t* valueName) {
    // Open the registry key
    HKEY hSubKey;
    if (RegOpenKeyEx(hKey, subKey, 0, KEY_SET_VALUE, &hSubKey) == ERROR_SUCCESS) {
        DWORD value = 1;
        if (RegSetValueEx(hSubKey, valueName, 0, REG_DWORD, (BYTE*)&value, sizeof(DWORD)) == ERROR_SUCCESS) {
            RegCloseKey(hSubKey);
        }
        RegCloseKey(hSubKey);
    }
    DWORD a = GetLastError();
}

void SetRegistryValueTominus1(HKEY hKey, const wchar_t* subKey, const wchar_t* valueName) {
    // Open the registry key
    HKEY hSubKey;
    if (RegOpenKeyEx(hKey, subKey, 0, KEY_SET_VALUE, &hSubKey) == ERROR_SUCCESS) {
        DWORD value = -1;
        if (RegSetValueEx(hSubKey, valueName, 0, REG_DWORD, (BYTE*)&value, sizeof(DWORD)) == ERROR_SUCCESS) {
            RegCloseKey(hSubKey);
        }
    }
    RegCloseKey(hSubKey);
}