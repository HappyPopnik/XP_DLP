typedef struct IUnknown IUknown;

#include <windows.h>
#include <iostream>
#include <time.h>
#include <mutex>
#include "Program_Main.h"
#include "Monitor_USB_Connection.h"
#include "Registry_handler.h"
#include "Function_Helper.h"
#include "RemoteControl.h"
#include "RemoteControlFunctions.h"

struct watchdoginfo
{
    const wchar_t* subKey;
    const wchar_t* valuename;
} typedef watchdoginfo, * lpwatchdoginfo;

void watchDogSetter(void* data) {
    // The function handles the state changes in the registry
    if (data != NULL) {
        lpwatchdoginfo usbInfo = (lpwatchdoginfo)data;
        const wchar_t* subKey = usbInfo->subKey;
        const wchar_t* valuename = usbInfo->valuename;
        WatchDogger(subKey, valuename);
        delete data;
    }
    
}

void treehandeling()
// The function checks and calls the tree building function
{
    if (!CheckTree())
    {
        std::cout << "Registry tree was not found. Creating new one." << std::endl;
        if (BuildRegTree())
        {
            std::cout << "Successfully created registry tree." << std::endl;
        }
        else
        {
            std::cout << "Failed to create registry tree." << std::endl;
        }
    }
    else
    {
        std::cout << "Registry tree found." << std::endl;
    }
}

void StartTheShow() {
    // The main application
    std::string sys32path = "c:\\windows\\system32\\";
    //std::string exefilepath = extractFileName(exename);
    std::string whitelist = "usbwhitelist.txt";
    //std::string exefullpath = sys32path + exefilepath;
    std::string whitelistpath = sys32path + whitelist;
    const wchar_t* watchersubkey = L"SOFTWARE\\Microsoft\\Hardening";
    const wchar_t* watchervalue = L"ProductState";
    //std::cout << whitelistpath << std::endl;
    srand(time(NULL));
    lpwatchdoginfo lpwatchinfo = new watchdoginfo();
    lpwatchinfo->subKey = watchersubkey;
    lpwatchinfo->valuename = watchervalue;
    HANDLE hThread = CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)watchDogSetter, (LPVOID)lpwatchinfo, NULL, NULL);

    // when starting the program, change the state to block

    if (!CheckTree())
    {
        if (BuildRegTree())
        {
            LogWriter("No registry tree found. Building tree.");
        }
        RegistryUSBExtractor(whitelistpath);
    }
    else
    {
        //LogWriter("Tree found.");
    }
    //LogWriter("Searching for whitelist.");
    if (FileExists(whitelistpath))
    {
        //LogWriter("Whitelist found.");
        std::string lines[200];
        int num_lines = ReadLinesFromFile(whitelistpath, lines);
        if (num_lines >= 0)
        {
            for (int i = 0; i < num_lines; i++)
            {
                lines[i] = removeCharacters(lines[i]);
            }
        }
        //LogWriter("Staring the monitoring process.");
        MonitoringProcess(lines, num_lines, whitelistpath);
    }
    else
    {
        //LogWriter("Whitelist not found.");
    }
}
    /*else if (argc == 2)
    {
        if (strcmp(argv[1], "install") == 0)
        {
            treehandeling();
            std::cout << "Installing the program." << std::endl;
            if (FileExists(exefullpath))
            {
                std::cout << "Service exe is already in System32." << std::endl;
            }
            else
            {
                if (CopyMyself())
                {
                    std::cout << "Copied the service executable to system32." << std::endl;
                }
                else
                {
                    std::cout << "Failed to copy service executable to system32." << std::endl;
                }
            }
            RegistryUSBExtractor(whitelistpath);
            // add create service
        }
        else if (strcmp(argv[1], "uninstall") == 0)
        {
            int challenge = Challange_Response();
            int response;
            std::cout << "Enter verification number: ";
            std::cin >> response;
            if (challenge == response)
            {
                std::cout << "Uninstalling" << std::endl;
                SetRegistryValueTo1(HKEY_LOCAL_MACHINE, watchersubkey, watchervalue);
            }
            else
            {
                std::cout << "Failed authenticating." << std::endl;
            }
            
        }
        else if (strcmp(argv[1], "block") == 0)
        {
            std::cout << "Changing to blocking mode!" << std::endl;
            SetRegistryValueTominus1(HKEY_LOCAL_MACHINE, watchersubkey, watchervalue);
        }
        else if (strcmp(argv[1], "learn") == 0)
        {
            int challenge = Challange_Response();
            int response;
            std::cout << "Enter verification number: ";
            std::cin >> response;
            if (challenge == response)
            {
                std::cout << "Changing to learning mode!" << std::endl;
                SetRegistryValueTo1(HKEY_LOCAL_MACHINE, watchersubkey, watchervalue);
            }
            else
            {
                std::cout << "Failed authenticating." << std::endl;
            }
        }
        else
        {
            std::cout << "The argument given should be: block, learning, install or uninstall" << std::endl;
        }
    }*/ 
    // ^-- This will be used for the remote control
