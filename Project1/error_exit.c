#include <curl/curl.h>

void errorExit(char* msg) {
	if (msg) {
		printf("%s:\n", msg);
	}
	wchar_t buf[256];
	FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		buf, (sizeof(buf) / sizeof(wchar_t)), NULL);
	wprintf(buf);
	getchar();
	exit(1);
}
