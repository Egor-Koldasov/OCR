#include <iostream>
#include <windows.h>
#include "http.h"

using namespace std;

HBITMAP screenToBitmap(POINT a, POINT b) {
	// copy screen to bitmap
	HDC     hScreen = GetDC(NULL);
	HDC     hDC = CreateCompatibleDC(hScreen);
	HBITMAP hBitmap = CreateCompatibleBitmap(hScreen, abs(b.x - a.x), abs(b.y - a.y));
	HGDIOBJ old_obj = SelectObject(hDC, hBitmap);
	BOOL    bRet = BitBlt(hDC, 0, 0, abs(b.x - a.x), abs(b.y - a.y), hScreen, a.x, a.y, SRCCOPY);
	// clean up
	SelectObject(hDC, old_obj);
	DeleteDC(hDC);
	ReleaseDC(NULL, hScreen);
	return hBitmap;
}

void setClipboardBitmap(HBITMAP bitmap) {
	// save bitmap to clipboard
	OpenClipboard(NULL);
	EmptyClipboard();
	SetClipboardData(CF_BITMAP, bitmap);
	CloseClipboard();
}

void takeScreenshotToClipboard(POINT a, POINT b) {
	HBITMAP screenBitmap = screenToBitmap(a, b);
	setClipboardBitmap(screenBitmap);
	DeleteObject(screenBitmap);
}

//Returns the last Win32 error, in string format. Returns an empty string if there is no error.
std::string getLastErrorAsString()
{
	//Get the error message, if any.
	DWORD errorMessageID = ::GetLastError();
	if (errorMessageID == 0)
		return std::string(); //No error message has been recorded

	LPSTR messageBuffer = nullptr;
	size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);

	std::string message(messageBuffer, size);

	//Free the buffer.
	LocalFree(messageBuffer);

	return message;
}

int main() {
	HINSTANCE hinst = LoadLibrary("C:\\Users\\eooaa\\source\\repos\\Project1\\Release\\keylistenerdll.dll");
	if (hinst == NULL)
	{
		printf("null hinst\n");
		wchar_t buf[256];
		FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			buf, (sizeof(buf) / sizeof(wchar_t)), NULL);
		wprintf(buf);
	}
	typedef void(*Install)();
	typedef void(*Uninstall)();

	Install install = (Install)GetProcAddress(hinst, "install");
	Uninstall uninstall = (Uninstall)GetProcAddress(hinst, "uninstall");


	/*
	printf("install\n");
	install();
	printf("install done\n");

	// exit
	MSG msg;
	// create thread message queue
	PeekMessage(&msg, 0, WM_USER, WM_USER, PM_NOREMOVE);
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	printf("uninstall\n");
	uninstall();
	printf("uninstall done\n");
	*/
	int mainMonitorX = GetSystemMetrics(SM_CXSCREEN);
	int mainMonitorY = GetSystemMetrics(SM_CYSCREEN);

	POINT a = { 0, 0 };
	POINT b = { mainMonitorX, mainMonitorY };
	HBITMAP screenBitmap = screenToBitmap(a, b);

	// int trying = letsTry();
	// printf("trying - %d\n", trying);

	
	getchar();
	return 0;
}