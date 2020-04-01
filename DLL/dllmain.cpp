// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"

#include <iostream>

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        MessageBoxA(0, "DLL has been attached to process.", "DLL Message", MB_OK);
        std::cout << "[DLL] Attached to process!" << std::endl;
        break;
    case DLL_THREAD_ATTACH:
        MessageBoxA(0, "DLL has been attached to thread.", "DLL Message", MB_OK);
        std::cout << "[DLL] Attached to thread!" << std::endl;
        break;
    case DLL_THREAD_DETACH:
        MessageBoxA(0, "DLL has been detached from thread.", "DLL Message", MB_OK);
        std::cout << "[DLL] Detached from thread!" << std::endl;
        break;
    case DLL_PROCESS_DETACH:
        MessageBoxA(0, "DLL has been detached from process.", "DLL Message", MB_OK);
        std::cout << "[DLL] Detached from process!" << std::endl;
        break;
    }
    return TRUE;
}

