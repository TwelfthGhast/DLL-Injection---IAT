#include "networking.h"
#include "pch.h"
#include "FSTraversal.h"
#include <ws2tcpip.h>
#include <iostream>
#include <vector>

#define SEND_PORT "1337"
#define LISTEN_PORT "6969"
#define BUFFER_LEN 1024

wchar_t cur_dir[BUFFER_LEN];

int ListenServer(void) {
	lstrcpynW(cur_dir, L"C:\\", 4);
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
			memset(recvbuf, 0, BUFFER_LEN);

			int iResult;

			do {
				iResult = recv(ConnSock, recvbuf, recvbuflen, 0);
				if (iResult > 0) {
					printf("[DLL] [LISTENER] Recieved: %.*s\n", iResult, recvbuf);
					if (!strncmp(recvbuf, "ls", 2)) {
						ReturnFiles(cur_dir);
					}
					else if (!strncmp(recvbuf, "ping", 4)) {
						// Send a buffer
						iResult = send(ConnSock, "pong", 4, 0);
						if (iResult == SOCKET_ERROR) {
							closesocket(ConnSock);
							WSACleanup();
							return 1;
						}
					}
					else if (!strncmp(recvbuf, "cd ", 3)) {
						std::string recv = recvbuf;
						std::wstring wrecv = std::wstring(recv.begin(), recv.end());
						memset(recvbuf, 0, BUFFER_LEN);
						std::wstring wrecv_arg = wrecv.substr(3, wrecv.size() - 3);
						if (wrecv_arg.back() != '\\') {
							wrecv_arg += '\\';
						}
						if (ValidDir(wrecv_arg.c_str())) {
							wcsncpy_s(cur_dir, wrecv_arg.c_str(), wrecv_arg.length());
							// Send a buffer
							char buffer[BUFFER_LEN];
							memset(buffer, 0, BUFFER_LEN);
							size_t   i;
							wcstombs_s(&i, buffer, BUFFER_LEN, wrecv_arg.c_str(), BUFFER_LEN);
							iResult = send(ConnSock, buffer, strlen(buffer), 0);
							if (iResult == SOCKET_ERROR) {
								closesocket(ConnSock);
								WSACleanup();
								return 1;
							}
						}
						else if (wrecv_arg.compare(L"~")) {
							wcsncpy_s(cur_dir, L"C:\\", 4);
							// Send a buffer
							iResult = send(ConnSock, "C:\\", 4, 0);
							if (iResult == SOCKET_ERROR) {
								closesocket(ConnSock);
								WSACleanup();
								return 1;
							}
						}
						else {
							// Send a buffer
							iResult = send(ConnSock, "C:\\", 4, 0);
							if (iResult == SOCKET_ERROR) {
								closesocket(ConnSock);
								WSACleanup();
								return 1;
							}
						}
					}
					else if (!strncmp(recvbuf, "size ", 5)) {
						std::string recv = recvbuf;
						std::wstring wrecv = std::wstring(recv.begin(), recv.end());
						memset(recvbuf, 0, BUFFER_LEN);
						std::wstring wrecv_arg = wrecv.substr(5, wrecv.size() - 5);
						
						std::string filelen = FileSize(wrecv_arg.c_str());
						iResult = send(ConnSock, filelen.c_str(), filelen.length(), 0);
						if (iResult == SOCKET_ERROR) {
							closesocket(ConnSock);
							WSACleanup();
							return 1;
						}
					}
					else if (!strncmp(recvbuf, "dump ", 5)) {
						/*
						std::string recv = recvbuf;
						std::wstring wrecv = std::wstring(recv.begin(), recv.end());
						memset(recvbuf, 0, BUFFER_LEN);
						std::wstring wrecv_arg = wrecv.substr(5, wrecv.size() - 5);

						int fsize = FileSize(wrecv_arg.c_str());
						std::vector<std::byte>;
						iResult = send(ConnSock, "test", fsize, 0);
						if (iResult == SOCKET_ERROR) {
							closesocket(ConnSock);
							WSACleanup();
							return 1;
						}
						*/
					}
				}
				else if (iResult == 0) {
					printf("[DLL] [LISTENER] Connection closed\n");
				}
				// Timeout
				else if (iResult == 10060){
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