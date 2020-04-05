#include <iostream>
#include <Windows.h>
#include <TlHelp32.h>

// Find process by name and return handle
void PrintProcesses(void){
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    PROCESSENTRY32W pe32;

    if (hSnapshot) {
        pe32.dwSize = sizeof(PROCESSENTRY32W);
        if (Process32First(hSnapshot, &pe32)) {
            do {
                std::wcout << pe32.szExeFile << std::endl;
            } while (Process32Next(hSnapshot, &pe32));
        }
        CloseHandle(hSnapshot);
    } else {
        std::cout << "Failed to get running processes" << std::endl;
    }
}

int main(void) {
	while (1) {
		char buffer[1024];
		std::cin >> buffer;
		std::cout << buffer << std::endl;
        PrintProcesses();
	}
}