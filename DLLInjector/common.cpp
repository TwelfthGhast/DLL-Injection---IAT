#include "common.h"

#include <Windows.h>
#include <TlHelp32.h>
#include <iostream>

// Find process by name and return handle
void* FindProcess(wchar_t* ProcessName) {
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    PROCESSENTRY32W pe32;

    if (hSnapshot) {
        pe32.dwSize = sizeof(PROCESSENTRY32W);
        if (Process32First(hSnapshot, &pe32)) {
            do {
                if (!wcscmp(pe32.szExeFile, ProcessName)) {
                    void* cpy = malloc(sizeof(PROCESSENTRY32W));
                    if (cpy == NULL) {
                        std::cerr << "Could not allocate memory" << std::endl;
                        exit(1);
                    }
                    memcpy(cpy, &pe32, sizeof(PROCESSENTRY32W));
                    return cpy;
                }
            } while (Process32Next(hSnapshot, &pe32));
        }
        CloseHandle(hSnapshot);
    }

    return NULL;
}
