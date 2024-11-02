typedef struct IUnknown IUknown;

#ifdef _MSC_VER
#include <crtdbg.h>
#else
#define _ASSERT(expr) ((void)0)

#define _ASSERTE(expr) ((void)0)
#endif

#include <Windows.h>
#include <SetupAPI.h>
#include <cfgmgr32.h>
#include <winioctl.h>
#include <tchar.h>

#include "eject_process.h"
#include <iostream>
#include "Function_Helper.h"
#include <string>
#include <xlocbuf>


// Finds the device interface for the CDROM drive with the given interface number.
DEVINST GetDrivesDevInstByDeviceNumber(long DeviceNumber, const GUID* guid)
{
    // Get device interface info set handle
    // for all devices attached to system
    HDEVINFO hDevInfo = SetupDiGetClassDevs(guid, NULL, NULL, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
    if (hDevInfo == INVALID_HANDLE_VALUE)
        return 0;
    
    // Retrieve a context structure for a device interface of a device information set.
    BYTE                             buf[1024];
    PSP_DEVICE_INTERFACE_DETAIL_DATA pspdidd = (PSP_DEVICE_INTERFACE_DETAIL_DATA)buf;
    SP_DEVICE_INTERFACE_DATA         spdid;
    SP_DEVINFO_DATA                  spdd;
    DWORD                            dwSize;

    spdid.cbSize = sizeof(spdid);

    // Iterate through all the interfaces and try to match one based on
    // the device number.
    for (DWORD i = 0; SetupDiEnumDeviceInterfaces(hDevInfo, NULL, guid, i, &spdid); i++)
    {
        // Get the device path.
        dwSize = 0;
        SetupDiGetDeviceInterfaceDetail(hDevInfo, &spdid, NULL, 0, &dwSize, NULL);
        if (dwSize == 0 || dwSize > sizeof(buf))
            continue;

        pspdidd->cbSize = sizeof(*pspdidd);
        ZeroMemory((PVOID)&spdd, sizeof(spdd));
        spdd.cbSize = sizeof(spdd);
        if (!SetupDiGetDeviceInterfaceDetail(hDevInfo, &spdid, pspdidd,
            dwSize, &dwSize, &spdd))
            continue;

        // Open the device.
        HANDLE hDrive = CreateFile(pspdidd->DevicePath, 0,
            FILE_SHARE_READ | FILE_SHARE_WRITE,
            NULL, OPEN_EXISTING, 0, NULL);
        if (hDrive == INVALID_HANDLE_VALUE)
            continue;

        // Get the device number.
        STORAGE_DEVICE_NUMBER sdn;
        dwSize = 0;
        if (DeviceIoControl(hDrive,
            IOCTL_STORAGE_GET_DEVICE_NUMBER,
            NULL, 0, &sdn, sizeof(sdn),
            &dwSize, NULL))
        {
            // Does it match?
            if (DeviceNumber == (long)sdn.DeviceNumber)
            {
                CloseHandle(hDrive);
                SetupDiDestroyDeviceInfoList(hDevInfo);
                return spdd.DevInst;
            }
        }
        CloseHandle(hDrive);
    }

    SetupDiDestroyDeviceInfoList(hDevInfo);
    return 0;
}


// Returns true if the given device instance belongs to the USB device with the given VID and PID.
bool matchDevInstToUsbDevice(DEVINST device, DWORD vid, DWORD pid)
{
    // This is the string we will be searching for in the device harware IDs.
    TCHAR hwid[128];
    _stprintf(hwid, _T("VID_%04X&PID_%04X"), vid, pid);

    // Get a list of hardware IDs for all USB devices.
    ULONG ulLen = NULL;
    CM_Get_Device_ID_List_Size(&ulLen, NULL, CM_GETIDLIST_FILTER_NONE);
    TCHAR* pszBuffer = new TCHAR[ulLen];
    CM_Get_Device_ID_List(NULL, pszBuffer, ulLen, CM_GETIDLIST_FILTER_NONE);
    // Iterate through the list looking for our ID.
    for (LPTSTR pszDeviceID = pszBuffer; *pszDeviceID; pszDeviceID += _tcslen(pszDeviceID) + 1)
    {
        // Some versions of Windows have the string in upper case and other versions have it
        // in lower case so just make it all upper.
        for (int i = 0; pszDeviceID[i]; i++)
            pszDeviceID[i] = toupper(pszDeviceID[i]);
        if (_tcsstr(pszDeviceID, hwid))
        {
            // Found the device, now we want the grandchild device, which is the "generic volume"
            DEVINST MSDInst = 0;
            if (CR_SUCCESS == CM_Locate_DevNode(&MSDInst, pszDeviceID, CM_LOCATE_DEVNODE_NORMAL))
            {
                DEVINST DiskDriveInst = 0;
                if (CR_SUCCESS == CM_Get_Child(&DiskDriveInst, MSDInst, 0))
                {
                    // Now compare the grandchild node against the given device instance.
                    if (device == DiskDriveInst)
                    {
                        delete[] pszBuffer;
                        return true;
                    }
                }
            }
        }
    }
    delete[] pszBuffer;
    return false;
}

/*void killItWithfire(HANDLE hVolume, TCHAR driveletter)
{
    long DeviceNumber = -1;
    STORAGE_DEVICE_NUMBER sdn;
    DWORD dwBytesReturned = 0;
    long res = DeviceIoControl(hVolume, IOCTL_STORAGE_GET_DEVICE_NUMBER, NULL, 0, &sdn, sizeof(sdn), &dwBytesReturned, NULL);
    if (res)
    {
        DeviceNumber = sdn.DeviceNumber;
    }
    if (DeviceNumber == -1)
    {
        return;
    }
    UINT DriverType = GetDriveType(driveletter);

}*/

// Eject the given drive.
bool ejectDrive(TCHAR driveletter)
// 1 - Broke at INVALID_HANDLE_VALUE
// 2 - Broke at FSCTL_LOCK_VOLUME
// 3 - Broke at FSCTL_DISMOUNT_VOLUME
// 4 - Broke at IOCTL_STORAGE_EJECT_MEDIA
// 5 - Broke at IOCTL_VOLUME_OFFLINE
// 6 - Done
{
    TCHAR devicepath[16];
    _tcscpy(devicepath, _T("\\\\.\\?:"));
    devicepath[4] = driveletter;
    DWORD dwRet = 0;
    HANDLE hVol = CreateFile(devicepath, GENERIC_READ, FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
    Sleep(100);
    if (hVol == INVALID_HANDLE_VALUE)
    {
        CloseHandle(hVol);
        return true;
    }
    if (!DeviceIoControl(hVol, FSCTL_LOCK_VOLUME, 0, 0, 0, 0, &dwRet, 0))
    {
        CloseHandle(hVol);
        return false;
    }
    if (!DeviceIoControl(hVol, FSCTL_DISMOUNT_VOLUME, 0, 0, 0, 0, &dwRet, 0))
    {
        CloseHandle(hVol);
        return false;
    }
    if (!DeviceIoControl(hVol, IOCTL_STORAGE_MEDIA_REMOVAL, 0, 0, 0, 0, &dwRet, 0))
    {
        CloseHandle(hVol);
        return false;
    }
    Sleep(500);
    if (!DeviceIoControl(hVol, IOCTL_STORAGE_EJECT_MEDIA, 0, 0, 0, 0, &dwRet, 0))
    {
        CloseHandle(hVol);
        return false;
    }
    Sleep(500);
    if (!DeviceIoControl(hVol, IOCTL_VOLUME_OFFLINE, 0, 0, 0, 0, &dwRet, 0))
    {
        CloseHandle(hVol);
        return false;
    }
    //killItWithfire(hVol, driveletter);
    CloseHandle(hVol);
        
    
    return true;
}

// Find a USB device by it's Vendor and Product IDs. When found, eject it.
bool usbEjectDevice(unsigned int vid, unsigned  int pid)
{
    TCHAR devicepath[8];
    _tcscpy(devicepath, TEXT("\\\\.\\?:"));
    TCHAR drivepath[4];
    _tcscpy(drivepath, _T("?:\\"));

    // Iterate through every drive letter and check if it is our device.
    for (TCHAR driveletter = _T('A'); driveletter <= _T('Z'); driveletter++)
    {
        drivepath[0] = driveletter;
        // Get the "storage device number" for the current drive.
        long DeviceNumber = -1;
        devicepath[4] = driveletter;
        //std::string charletter(1, static_cast<char>(driveletter)); // DELETE
        HANDLE hVolume = CreateFile(devicepath, 0, FILE_SHARE_READ | FILE_SHARE_WRITE,
            NULL, OPEN_EXISTING, 0, NULL);
        if (INVALID_HANDLE_VALUE == hVolume)
            continue;
        
        STORAGE_DEVICE_NUMBER sdn;
        DWORD dwBytesReturned = 0;
        if (DeviceIoControl(hVolume, IOCTL_STORAGE_GET_DEVICE_NUMBER,
            NULL, 0, &sdn, sizeof(sdn), &dwBytesReturned, NULL))
            DeviceNumber = sdn.DeviceNumber;
        CloseHandle(hVolume);
        if (DeviceNumber < 0)
            continue;
        // Use the data we have collected so far on our drive to find a device instance.
        const GUID* guid_cdrom = &GUID_DEVINTERFACE_CDROM;
        const GUID* guid_disks = &GUID_DEVINTERFACE_DISK;
        
        DEVINST DevInst = GetDrivesDevInstByDeviceNumber(DeviceNumber, guid_cdrom);
        if (!DevInst)
        {
            DevInst = GetDrivesDevInstByDeviceNumber(DeviceNumber, guid_disks);
        }
        // If the device instance corresponds to the USB device we are looking for, eject it.
        if (DevInst)
        {
            std::string chatinst(1, static_cast<char>(DevInst));
            if (matchDevInstToUsbDevice(DevInst, vid, pid)) {
                
                bool a = ejectDrive(driveletter);
                PNP_VETO_TYPE VetoType = PNP_VetoTypeUnknown;
                WCHAR VetoNameW[MAX_PATH];
                VetoNameW[0] = 0;
                bool BSuccess = false;
                
                DEVINST DevInstParent = 0;
                long res = CM_Get_Parent(&DevInstParent, DevInst, 0);
                for (long tries = 1; tries <= 3; tries++)
                {
                    VetoNameW[0] = 0;
                    res = CM_Request_Device_EjectW(DevInstParent, &VetoType, VetoNameW, MAX_PATH, 0);
                    BSuccess = (res == CR_SUCCESS && VetoType == PNP_VetoTypeUnknown);
                    if (BSuccess)
                        break;
                    Sleep(500);
                }
                //In case you want to see the Drive letter:
                //WideCharToMultiByte(CP_ACP, 0, &driveletter, lstrlen(&driveletter), test, 1024, NULL, NULL);
                return a;
            }
        }
    }
}
