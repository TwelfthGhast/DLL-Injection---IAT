#include "common.h"

#include <Windows.h>
#include <tlhelp32.h>
#include <iostream>

int InjectDLL(char* LocationDLL, wchar_t* VictimProcess) {
    PROCESSENTRY32* pe32 = (PROCESSENTRY32*) FindProcess(VictimProcess);
    if (pe32 == NULL) {
        std::cerr << "Could not find process" << std::endl;
        return 1;
    }
    // Get a handle to process we wish to inject into
    HANDLE inject = OpenProcess(PROCESS_CREATE_THREAD | PROCESS_QUERY_INFORMATION | PROCESS_VM_OPERATION | PROCESS_VM_READ | PROCESS_VM_WRITE, FALSE, pe32->th32ProcessID);
    if (inject) {
        LPVOID baseAddr = VirtualAllocEx(inject, NULL, strlen(LocationDLL), MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
        if (baseAddr) {
            if (!WriteProcessMemory(inject, baseAddr, LocationDLL, strlen(LocationDLL), NULL)) {
                fprintf(stderr, "Could not write path of DLL into process memory.\n");
                return 1;
            }
            else {
                // Returns a pointer to the LoadLibrary address.
                // This will be the same on the remote process as in our current process.
                HMODULE mHandle = GetModuleHandle(L"kernel32.dll");
                if (mHandle == NULL) {
                    std::cerr << "Could not get handle to kernel32.dll" << std::endl;
                    return 1;
                }
                LPVOID loadLibraryAddress = (LPVOID)GetProcAddress(mHandle, "LoadLibraryA");
                if (loadLibraryAddress == NULL) {
                    std::cerr << "Could not load library" << std::endl;
                    return 1;
                }
                HANDLE remoteThread = CreateRemoteThread(inject, NULL, 0, (LPTHREAD_START_ROUTINE)loadLibraryAddress, baseAddr, 0, NULL);
                if (remoteThread == NULL) {
                    std::cerr << "Could not start remote thread" << std::endl;
                    return 1;
                }
                std::cout << "Successfully injected DLL!" << std::endl;
                return 1;
            }
        } else {
            std::cerr << "Could not allocate memory" << std::endl;
            return 1;
        }
    } else {
        std::cerr << "Could not inject into running process" << std::endl;
        return 1;
    }
}
