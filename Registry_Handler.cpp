typedef struct IUnknown IUknown;

#include <Windows.h>
#include <iostream>
#include <fstream>
#include <string>
#include <initguid.h>
#include <string.h>
#include <comdef.h>
#include <cwctype>


#include "Registry_handler.h"
#include "Function_Helper.h"



void FormatRegistryToUSBDevicesFile(const std::wstring& subkeyName, const std::string& filePath, const std::wstring serial) {
    size_t vidPos = subkeyName.find(L"vid_");
    size_t pidPos = subkeyName.find(L"pid_");

    std::wofstream outFile{};
    outFile.open(filePath, std::ios::app);

    if (vidPos != std::wstring::npos && pidPos != std::wstring::npos) {
        std::wstring vid = subkeyName.substr(vidPos + 4, 4);
        std::wstring pid = subkeyName.substr(pidPos + 4, 4);
        writeToUSBDevicesFile(vid, pid, serial, outFile);
    }
}


int RegistryUSBExtractor(const std::string& filePath) {
    HKEY hKey;
    HKEY hKey2;
    HKEY hKey3;
    LONG result;

    // Specify the registry key path you want to enumerate
    LPCWSTR registryKeyPath = L"SYSTEM\\CurrentControlSet\\Enum\\USB";
    std::wstring slash = L"\\";

    // Open the registry key for reading
    result = RegOpenKeyEx(HKEY_LOCAL_MACHINE, registryKeyPath, 0, KEY_READ, &hKey);
    if (result != ERROR_SUCCESS) {
        std::cerr << "Error opening registry key: " << result << std::endl;
        return 1;
    }

    DWORD index = 0;
    WCHAR keyName[256];
    WCHAR RandkeyName[256];
    DWORD RandkeyNameSize = sizeof(RandkeyName) / sizeof(RandkeyName[0]);
    DWORD keyNameSize = sizeof(keyName) / sizeof(keyName[0]);
    LPCWSTR valueName = L"Service";
    DWORD dataType;
    BYTE buffer[256];
    DWORD bufferSize = sizeof(buffer);
    // Specify the string to search for within key names
    std::wstring searchString = L"pid";
    std::wstring registryKeyPathN;
    std::wstring registryKeyPathM;
    // Enumerate subkeys
    while (true) {
        result = RegEnumKeyEx(hKey, index, keyName, &keyNameSize, NULL, NULL, NULL, NULL);
        if (result != ERROR_SUCCESS) {
            break; // No more subkeys to enumerate
        }

        std::wstring keyNameStr(keyName);

        // Check if the key name contains the desired string
        std::wstring lowerkeyname = L"";
        for (wchar_t c : keyNameStr) {
            lowerkeyname.push_back(std::towlower(c));
        }
        if (lowerkeyname.find(searchString) != std::wstring::npos) {
            registryKeyPathN = registryKeyPath + slash + keyNameStr;
            result = RegOpenKeyEx(HKEY_LOCAL_MACHINE, registryKeyPathN.c_str(), 0, KEY_READ, &hKey2);
            if (result != ERROR_SUCCESS) {
                std::cerr << "Error opening registry key: " << result << std::endl;
                continue;
            }

            result = RegEnumKeyEx(hKey2, 0, RandkeyName, &RandkeyNameSize, NULL, NULL, NULL, NULL);
            registryKeyPathM = registryKeyPathN + slash + RandkeyName;
            result = RegOpenKeyEx(HKEY_LOCAL_MACHINE, registryKeyPathM.c_str(), 0, KEY_READ, &hKey3);
            if (result != ERROR_SUCCESS) {
                std::cerr << "Error opening registry key horam: " << result << std::endl;
                continue;
            }
            // Read the "Service" value data

            result = RegQueryValueEx(hKey3, valueName, NULL, &dataType, buffer, &bufferSize);
            if (result == ERROR_SUCCESS) {
                wchar_t* usbtype = reinterpret_cast<wchar_t*>(buffer);
                _bstr_t conv_usb_type(usbtype);
                std::string str_usb_type = std::string(conv_usb_type);
                if (!strcmp(str_usb_type.c_str(), "USBSTOR"))
                {
                    FormatRegistryToUSBDevicesFile(lowerkeyname, filePath, RandkeyName);
                }
            }
            else if (result == ERROR_MORE_DATA)
            {
                LPBYTE dataBuffer = new BYTE[bufferSize];
                result = RegQueryValueEx(hKey3, valueName, NULL, &dataType, dataBuffer, &bufferSize);
                wchar_t* usbtype = reinterpret_cast<wchar_t*>(dataBuffer);
                _bstr_t conv_usb_type(usbtype);
                std::string str_usb_type = std::string(conv_usb_type);
                if (!strcmp(str_usb_type.c_str(), "USBSTOR"))
                {
                    FormatRegistryToUSBDevicesFile(lowerkeyname, filePath, RandkeyName);
                }
                delete[] dataBuffer;

            }
            // Close the registry key
            RegCloseKey(hKey2);
            RegCloseKey(hKey3);
            RandkeyNameSize = sizeof(RandkeyName) / sizeof(RandkeyName[0]);
        }

        // Reset the keyNameSize for the next iteration
        keyNameSize = sizeof(keyName) / sizeof(keyName[0]);
        index++;
    }

    // Close the registry key
    RegCloseKey(hKey);
    if (!FileExists("c:\\windows\\system32\\usbwhitelist.txt"))
    {
        std::ofstream outFile2(filePath);
        if (outFile2.is_open())
        {
            LogWriter("Whitelist file created but empty.");
            outFile2.close();
        }
        else
        {
            LogWriter("Could not create whitelist file.");
        }
    }
    return 0;
}

bool BuildRegTree()
/*
* This function will build the registry tree for the hardening program.
* Registry path: HKLM\SYSTEM\CurrentControlSet\Policies\Hardening
*/
{
    char* pTimestamp = nullptr;

    const char* keypath = "SOFTWARE\\Microsoft\\Hardening";
    const char* keypath2 = "SOFTWARE\\Microsoft\\Hardening\\Log";
    const char* Date = GetCurrentDate("%Y-%m-%d %H:%M:%S", pTimestamp);
    const char* dateval = "InstallDate";
    const char* ReleasedUntil = "ReleasedUntil";
    const char* lstchngedateval = "LastChange";
    const char* Active = "ProductState";
    const char* Firstlog = "First_log";
    const char* LogDetails = "First Log!";
    DWORD activated = 0;
    HKEY hKey_new;

    free(pTimestamp);

    // Create the key 'Handening'
    LONG createresult = RegCreateKeyExA(HKEY_LOCAL_MACHINE, keypath, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey_new, NULL);
    if (createresult == ERROR_SUCCESS)
    {

        // Create the Value 'InstallDate'
        LONG set_installdate_result = RegSetValueExA(hKey_new, dateval, 0, REG_SZ, reinterpret_cast<const BYTE*>(Date), strlen(Date) + 1);
        if (set_installdate_result == ERROR_SUCCESS)
        {
            std::cout << "Created registry value: \"InstallDate\"" << std::endl;
        }
        else
        {
            std::cout << "Failed to create registry value: \"InstallDate\"" << std::endl;
            return false;
        }

        // Create the Value 'Released Until'
        LONG set_ReleasedUntil_result = RegSetValueExA(hKey_new, ReleasedUntil, 0, REG_SZ, reinterpret_cast<const BYTE*>(Date), strlen(Date) + 1);
        if (set_ReleasedUntil_result == ERROR_SUCCESS)
        {
            std::cout << "Created registry value: \"ReleasedUntil\"" << std::endl;
        }
        else
        {
            std::cout << "Failed to create registry value: \"ReleasedUntil\"" << std::endl;
            return false;
        }

        // Create the Value 'LastChange'
        LONG set_lastchnge_result = RegSetValueExA(hKey_new, lstchngedateval, 0, REG_SZ, reinterpret_cast<const BYTE*>(Date), strlen(Date) + 1);
        if (set_lastchnge_result == ERROR_SUCCESS)
        {
            std::cout << "Created registry value: \"LastChange\"" << std::endl;
        }
        else
        {
            std::cout << "Failed to create registry value: \"LastChange\"" << std::endl;
            return false;
        }
        // Create and set the "HardeningActivated" value to 0.
        LONG set_activated_result = RegSetValueExA(hKey_new, Active, 0, REG_DWORD, reinterpret_cast<const BYTE*>(&activated), sizeof(activated));
        if (set_activated_result == ERROR_SUCCESS)
        {
            std::cout << "Created registry value: \"ProductState\"" << std::endl;
        }
        else
        {
            std::cout << "Failed to create registry value: \"ProductState\"" << std::endl;
            return false;
        }
        LONG createresult = RegCreateKeyExA(HKEY_LOCAL_MACHINE, keypath2, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey_new, NULL);
        if (createresult == ERROR_SUCCESS)
        {
            // Create the first value for log
            LONG log_value_result = RegSetValueExA(hKey_new, Firstlog, 0, REG_SZ, LPBYTE(LogDetails), strlen(LogDetails) + 1);
            if (log_value_result == ERROR_SUCCESS)
            {
                std::cout << "Created registry value: \"InstallDate\"" << std::endl;
            }
            else
            {
                std::cout << "Failed to create registry value: \"InstallDate\"" << std::endl;
                return false;
            }
        }
        RegCloseKey(hKey_new);
    }
    else
    {
        std::cout << "Failed to create registry key: \"Hardening\"" << std::endl;
        return false;
    }
    return true;
}

bool CheckTree()
// This function checks if the registry tree of the software exists.
{
    const char* keypath = "SOFTWARE\\Microsoft\\Hardening";
    HKEY hKey;
    LONG openResult = RegOpenKeyExA(HKEY_LOCAL_MACHINE, keypath, 0, KEY_SET_VALUE, &hKey);
    if (openResult == ERROR_SUCCESS)
        return true;
    return false;
}

void Learningmodupdate(const char* subKey, const char* value, int future)
// This function updates the LastChange value in the registry
{
    // Get the Furute date.
    const char* Date;
    char* pTimestamp = nullptr;
    if (future == 1)
    {
        Date = GetFutureDate("%Y-%m-%d %H:%M:%S", pTimestamp);
    }
    else 
    {
        Date = GetCurrentDate("%Y-%m-%d %H:%M:%S", pTimestamp);
    }
    free(pTimestamp);
    HKEY hKey;

    // Open the handle to the last change key.
    LONG openResult = RegOpenKeyExA(HKEY_LOCAL_MACHINE, subKey, 0, KEY_SET_VALUE, &hKey);
    if (openResult != ERROR_SUCCESS)
    {
        RegCloseKey(hKey);
        std::cout << "Failed to open registry key" << std::endl;
        return;
    }

    // Set the data of the value to the new date.
    LONG set_lastchange_result = RegSetValueExA(hKey, value, 0, REG_SZ, reinterpret_cast<const BYTE*>(Date), strlen(Date) + 1);
    if (set_lastchange_result == ERROR_SUCCESS)
    {
        RegCloseKey(hKey);
        std::cout << "Updated registry value: \"ReleasedUntil\"" << std::endl;
        return;
    }
    else
    {
        std::cout << "Failed to update registry value: \"ReleasedUntil\"" << std::endl;
        RegCloseKey(hKey);
        return;
    }
}
