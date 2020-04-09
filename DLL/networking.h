#pragma once
#include <WinSock2.h>

int ListenServer(void);
SOCKET EstablishRemoteConn(addrinfo* result);
int SendBuffer(SOCKET ConnSock, const char* sendbuf);
void CloseConn(SOCKET ConnSock, addrinfo* result);