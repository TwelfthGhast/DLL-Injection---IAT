#include <iostream>
#include "classic.h"

int main(void) {
	char buffer[1024];
	strncpy_s(buffer, "DLL.DLL", 1024);
	wchar_t wbuffer[1024];
	wcsncpy_s(wbuffer, L"VictimProcess.exe", 1024);
	InjectDLL(buffer, wbuffer);
}