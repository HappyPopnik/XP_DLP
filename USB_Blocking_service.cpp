typedef struct IUnknown IUknown;

#include "USB_Blocking_service.h"

#include <WtsApi32.h>

#pragma comment(lib, "Wtsapi32.lib")
#include "Program_Main.h"
#include "Function_Helper.h"

void USBBlockService::OnStart(DWORD /*argc*/, TCHAR** /*argv[]*/) {
    HANDLE hThread = CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)StartTheShow, NULL, NULL, NULL);
    
}

void USBBlockService::OnStop() {
    // Doesn't matter if it's open.
}
