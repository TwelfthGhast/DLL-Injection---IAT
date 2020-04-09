#include "networking.h"
#include "pch.h"
#include "FSTraversal.h"
#include <ws2tcpip.h>
#include <iostream>

#define SEND_PORT "1337"
#define LISTEN_PORT "6969"
#define BUFFER_LEN 1024

wchar_t cur_dir[BUFFER_LEN];

int ListenServer(void) {
	lstrcpynW(cur_dir, L"C:\\*", 4);
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

		iResult = getaddrinfo("127.0.0.1", LISTEN_PORT, &hints, &result);
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
			char recvbuf[BUFFER_LEN];

			int iResult;

			// shutdown the connection for sending since no more data will be sent
			// the client can still use the ConnectSocket for receiving data
			iResult = shutdown(ConnSock, SD_SEND);
			if (iResult == SOCKET_ERROR) {
				printf("[DLL] [LISTENER] Shutdown Failed: %d\n", WSAGetLastError());
				closesocket(ConnSock);
				WSACleanup();
				return 1;
			}

			do {
				iResult = recv(ConnSock, recvbuf, recvbuflen, 0);
				if (iResult > 0) {
					printf("[DLL] [LISTENER] Recieved: %.*s\n", iResult, recvbuf);
					if (!strncmp(recvbuf, "ls", 2)) {
						ReturnFiles(cur_dir);
					}
				}
				else if (iResult == 0) {
					printf("[DLL] [LISTENER] Connection closed\n");
				}
				else {
					printf("[DLL] [LISTENER] recv failed: %d\n", WSAGetLastError());
				}
			} while (iResult > 0);

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

SOCKET EstablishRemoteConn(addrinfo* result) {
	// https://docs.microsoft.com/en-us/windows/win32/winsock/initializing-winsock
	WSADATA wsaData;

	int iResult;

	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (!iResult) {
		// Start making packet
		struct addrinfo* ptr = NULL, hints;
		ZeroMemory(&hints, sizeof(hints));
		hints.ai_family = AF_UNSPEC;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_protocol = IPPROTO_TCP;

		iResult = getaddrinfo("127.0.0.1", SEND_PORT, &hints, &result);
		if (iResult) {
			WSACleanup();
			return NULL;
		}
		// https://docs.microsoft.com/en-us/windows/win32/winsock/creating-a-socket-for-the-client
		SOCKET ConnSock = INVALID_SOCKET;
		ptr = result;
		ConnSock = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
		if (ConnSock == INVALID_SOCKET) {
			freeaddrinfo(result);
			WSACleanup();
			return NULL;
		}
		iResult = connect(ConnSock, ptr->ai_addr, (int)ptr->ai_addrlen);
		if (iResult == SOCKET_ERROR) {
			closesocket(ConnSock);
			ConnSock = INVALID_SOCKET;
		}
		else {
			return ConnSock;
		}
	}
	return NULL;
}

int SendBuffer(SOCKET ConnSock, const char* sendbuf) {
	int iResult;

	// Send a buffer
	iResult = send(ConnSock, sendbuf, (int)strlen(sendbuf), 0);
	if (iResult == SOCKET_ERROR) {
		closesocket(ConnSock);
		WSACleanup();
		return 1;
	}
	return 0;
}

void CloseConn(SOCKET ConnSock, addrinfo* result) {
	int iResult;
	// shutdown the connection for sending since no more data will be sent
	iResult = shutdown(ConnSock, SD_SEND);
	if (iResult == SOCKET_ERROR) {
		printf("shutdown failed: %d\n", WSAGetLastError());
		closesocket(ConnSock);
		WSACleanup();
		return;
	}

	closesocket(ConnSock);
	WSACleanup();

	freeaddrinfo(result);
	if (ConnSock == INVALID_SOCKET) {
		WSACleanup();
	}
	return;
}