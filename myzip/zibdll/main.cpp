#include <stdio.h>
#include <Windows.h>

//BOOL WINAPI DllMain(
//  __in  HINSTANCE hinstDLL,
//  __in  DWORD fdwReason,
//  __in  LPVOID lpvReserved
//)
//{
//    switch( fdwReason )
//    {
//        case DLL_PROCESS_ATTACH:
//         // Initialize once for each new process.
//         // Return FALSE to fail DLL load.
//            break;

//        case DLL_THREAD_ATTACH:
//         // Do thread-specific initialization.
//            break;

//        case DLL_THREAD_DETACH:
//         // Do thread-specific cleanup.
//            break;

//        case DLL_PROCESS_DETACH:
//         // Perform any necessary cleanup.
//            break;
//    }
//    return TRUE;  // Successful DLL_PROCESS_ATTACH.
//}

extern "C" _declspec(dllexport) void ShowMessage( LPCTSTR pstrMsg, LPCTSTR pstrTitle )
{

}
