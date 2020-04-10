#pragma once
#include <iostream>

void ReturnFiles(const wchar_t* dir);
BOOL ValidDir(const wchar_t* dir);
BOOL ValidFile(const wchar_t* path);
std::string FileSize(const wchar_t* path);