#define CURL_STATICLIB
#include <curl/curl.h>
#include "png_convert.h"
#include "error_exit.h"
#include "base64.h"

PBITMAPINFO CreateBitmapInfoStruct(HWND hwnd, HBITMAP hBmp)
{
	BITMAP bmp;
	PBITMAPINFO pbmi;
	WORD    cClrBits;

	// Retrieve the bitmap color format, width, and height.  
	int result = GetObject(hBmp, sizeof(BITMAP), (LPSTR)&bmp);
	if (!result) {
		printf("result: %d\n", result);
		errorExit("Failed to GetObject\n");
	}

	// Convert the color format to a count of bits.  
	cClrBits = (WORD)(bmp.bmPlanes * bmp.bmBitsPixel);
	if (cClrBits == 1)
		cClrBits = 1;
	else if (cClrBits <= 4)
		cClrBits = 4;
	else if (cClrBits <= 8)
		cClrBits = 8;
	else if (cClrBits <= 16)
		cClrBits = 16;
	else if (cClrBits <= 24)
		cClrBits = 24;
	else cClrBits = 32;

	// Allocate memory for the BITMAPINFO structure. (This structure  
	// contains a BITMAPINFOHEADER structure and an array of RGBQUAD  
	// data structures.)  

	if (cClrBits < 24)
		pbmi = (PBITMAPINFO)LocalAlloc(LPTR,
			sizeof(BITMAPINFOHEADER) +
			sizeof(RGBQUAD) * (1 << cClrBits));

	// There is no RGBQUAD array for these formats: 24-bit-per-pixel or 32-bit-per-pixel 

	else
		pbmi = (PBITMAPINFO)LocalAlloc(LPTR,
			sizeof(BITMAPINFOHEADER));

	// Initialize the fields in the BITMAPINFO structure.  

	pbmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	pbmi->bmiHeader.biWidth = bmp.bmWidth;
	pbmi->bmiHeader.biHeight = bmp.bmHeight;
	pbmi->bmiHeader.biPlanes = bmp.bmPlanes;
	pbmi->bmiHeader.biBitCount = bmp.bmBitsPixel;
	if (cClrBits < 24)
		pbmi->bmiHeader.biClrUsed = (1 << cClrBits);

	// If the bitmap is not compressed, set the BI_RGB flag.  
	pbmi->bmiHeader.biCompression = BI_RGB;

	// Compute the number of bytes in the array of color  
	// indices and store the result in biSizeImage.  
	// The width must be DWORD aligned unless the bitmap is RLE 
	// compressed. 
	pbmi->bmiHeader.biSizeImage = ((pbmi->bmiHeader.biWidth * cClrBits + 31) & ~31) / 8
		* pbmi->bmiHeader.biHeight;
	// Set biClrImportant to 0, indicating that all of the  
	// device colors are important.  
	pbmi->bmiHeader.biClrImportant = 0;
	return pbmi;
}

void saveBitmapToFile(char * filePath, char * buffer, int bufferSize) {
	FILE * fileHandle;
	if (fopen_s(&fileHandle, filePath, "wb") != 0) errorExit("saveBitmapToFile fopen_s");
	fwrite(buffer, sizeof(buffer[0]), bufferSize, fileHandle);
	fclose(fileHandle);
}

char* createImageBuffer(PBITMAPINFO pbi, HBITMAP hBMP, HDC hDC, int* bufferSize)
{
	// HANDLE hf;                 // file handle  
	BITMAPFILEHEADER hdr;       // bitmap file-header  
	PBITMAPINFOHEADER pbih;     // bitmap info-header  
	LPBYTE lpBits;              // memory pointer  
	// DWORD dwTotal;              // total count of bytes  
	// DWORD cb;                   // incremental count of bytes  
	// BYTE *hp;                   // byte pointer  
	// DWORD dwTmp;
	// LPTSTR pszFile;

	pbih = &pbi->bmiHeader;
	lpBits = (LPBYTE)GlobalAlloc(GMEM_FIXED, pbih->biSizeImage);

	if (!lpBits) errorExit("GlobalAlloc");

	printf("pbih->biHeight: %d\n", pbih->biHeight);
	printf("pbih->biSizeImage: %d\n", pbih->biSizeImage);

	// Retrieve the color table (RGBQUAD array) and the bits  
	// (array of palette indices) from the DIB.  
	if (!GetDIBits(hDC, hBMP, 0, (WORD)pbih->biHeight, lpBits, pbi, DIB_RGB_COLORS)) {
		errorExit("GetDIBits");
	}

	// Create the .BMP file.  
	/*
	hf = CreateFile(pszFile,
		GENERIC_READ | GENERIC_WRITE,
		(DWORD)0,
		NULL,
		CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		(HANDLE)NULL);
	if (hf == INVALID_HANDLE_VALUE)
		errhandler("CreateFile", hwnd);
	*/

	int totalSize =
		(sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + pbih->biClrUsed * sizeof(RGBQUAD) + pbih->biSizeImage);

	hdr.bfType = 0x4d42;        // 0x42 = "B" 0x4d = "M"  
	// Compute the size of the entire file.  
	hdr.bfSize = totalSize;
	hdr.bfReserved1 = 0;
	hdr.bfReserved2 = 0;
	// Compute the offset to the array of color indices.  
	hdr.bfOffBits = (DWORD) sizeof(BITMAPFILEHEADER) +
		pbih->biSize + pbih->biClrUsed
		* sizeof(RGBQUAD);

	unsigned char * imageBuffer = malloc(totalSize);
	int i = 0;
	printf("test pointer: \n");
	printf("imageBuffer: %s\n", imageBuffer);
	printf("imageBuffer + i: %s\n", imageBuffer + i);
	printf("&imageBuffer[i]: %s\n", &imageBuffer[i]);
	printf("imageBuffer + totalSize: %s\n", imageBuffer + totalSize);
	if (memcpy_s((void *)(&imageBuffer[i]), totalSize, (void *)&hdr, sizeof(BITMAPFILEHEADER)) != 0) errorExit("memcpy hdr");
	i += sizeof(BITMAPFILEHEADER);
	memcpy_s((void *)(&imageBuffer[i]), totalSize, (void *)pbih, sizeof(BITMAPINFOHEADER) + pbih->biClrUsed * sizeof(RGBQUAD));
	i += sizeof(BITMAPINFOHEADER) + pbih->biClrUsed * sizeof(RGBQUAD);
	if (memcpy_s((void *)(&imageBuffer[i]), totalSize, (void *)lpBits, pbih->biSizeImage) != 0) errorExit("memcpy lpBits");
	if (bufferSize) *bufferSize = totalSize;

	return imageBuffer;

	/*
	// Copy the BITMAPFILEHEADER into the .BMP file.  
	if (!WriteFile(hf, (LPVOID)&hdr, sizeof(BITMAPFILEHEADER),
		(LPDWORD)&dwTmp, NULL))
	{
		errhandler("WriteFile", hwnd);
	}

	// Copy the BITMAPINFOHEADER and RGBQUAD array into the file.  
	if (!WriteFile(hf, (LPVOID)pbih, sizeof(BITMAPINFOHEADER)
		+ pbih->biClrUsed * sizeof(RGBQUAD),
		(LPDWORD)&dwTmp, (NULL)))
		errhandler("WriteFile", hwnd);

	// Copy the array of color indices into the .BMP file.  
	dwTotal = cb = pbih->biSizeImage;
	hp = lpBits;
	if (!WriteFile(hf, (LPSTR)hp, (int)cb, (LPDWORD)&dwTmp, NULL))
		errhandler("WriteFile", hwnd);

	// Close the .BMP file.  
	if (!CloseHandle(hf))
		errhandler("CloseHandle", hwnd);

	// Free memory.  
	GlobalFree((HGLOBAL)lpBits);
	*/
}

void CreateBMPFile(LPTSTR pszFile, PBITMAPINFO pbi,
	HBITMAP hBMP, HDC hDC, int * fileSize)
{
	HANDLE hf;                 // file handle  
	BITMAPFILEHEADER hdr;       // bitmap file-header  
	PBITMAPINFOHEADER pbih;     // bitmap info-header  
	LPBYTE lpBits;              // memory pointer  
	DWORD dwTotal;              // total count of bytes  
	DWORD cb;                   // incremental count of bytes  
	BYTE *hp;                   // byte pointer  
	DWORD dwTmp;

	pbih = (PBITMAPINFOHEADER)pbi;
	lpBits = (LPBYTE)GlobalAlloc(GMEM_FIXED, pbih->biSizeImage);

	if (!lpBits)
		errorExit("GlobalAlloc");

	// Retrieve the color table (RGBQUAD array) and the bits  
	// (array of palette indices) from the DIB.  
	if (!GetDIBits(hDC, hBMP, 0, (WORD)pbih->biHeight, lpBits, pbi,
		DIB_RGB_COLORS))
	{
		errorExit("GetDIBits");
	}

	// Create the .BMP file.  
	hf = CreateFile(pszFile,
		GENERIC_READ | GENERIC_WRITE,
		(DWORD)0,
		NULL,
		CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		(HANDLE)NULL);
	if (hf == INVALID_HANDLE_VALUE)
		errorExit("CreateFile");
	hdr.bfType = 0x4d42;        // 0x42 = "B" 0x4d = "M"  
	// Compute the size of the entire file.  
	*fileSize = hdr.bfSize = (DWORD)(sizeof(BITMAPFILEHEADER) +
		pbih->biSize + pbih->biClrUsed
		* sizeof(RGBQUAD) + pbih->biSizeImage);
	hdr.bfReserved1 = 0;
	hdr.bfReserved2 = 0;

	// Compute the offset to the array of color indices.  
	hdr.bfOffBits = (DWORD) sizeof(BITMAPFILEHEADER) +
		pbih->biSize + pbih->biClrUsed
		* sizeof(RGBQUAD);

	// Copy the BITMAPFILEHEADER into the .BMP file.  
	if (!WriteFile(hf, (LPVOID)&hdr, sizeof(BITMAPFILEHEADER),
		(LPDWORD)&dwTmp, NULL))
	{
		errorExit("WriteFile");
	}

	// Copy the BITMAPINFOHEADER and RGBQUAD array into the file.  
	if (!WriteFile(hf, (LPVOID)pbih, sizeof(BITMAPINFOHEADER)
		+ pbih->biClrUsed * sizeof(RGBQUAD),
		(LPDWORD)&dwTmp, (NULL)))
		errorExit("WriteFile");

	// Copy the array of color indices into the .BMP file.  
	dwTotal = cb = pbih->biSizeImage;
	hp = lpBits;
	if (!WriteFile(hf, (LPSTR)hp, (int)cb, (LPDWORD)&dwTmp, NULL))
		errorExit("WriteFile");

	// Close the .BMP file.  
	if (!CloseHandle(hf))
		errorExit("CloseHandle");

	// Free memory.  
	GlobalFree((HGLOBAL)lpBits);
}

size_t write_data(void *buffer, size_t size, size_t nmemb, void *userp) {
	printf("%s", buffer);
	return nmemb;
}

int main() {
	/*
	png_convert_test();
	return 0;
	*/
	char* url = "https://vision.googleapis.com/v1/images:annotate?key=""AIzaSyDV7KXUzz-kXJkgMgpgobyHBQRwo7lJe1s";
	char* request_json = (
		"{'requests':[{'image':{'source':{'imageUri': 'https://cloud.google.com/vision/docs/images/shanghai_small.jpeg'}}, 'features': [{'type': 'TEXT_DETECTION', 'maxResults': 1, 'model': 'builtin / latest'}]}]}"
		);
	char request_json_len [3];

	curl_global_init(CURL_GLOBAL_ALL);

	CURL* curl_easy = curl_easy_init();

	struct curl_slist *headers = NULL;
	//headers = curl_slist_append(headers, "Authorization: Bearer AIzaSyDV7KXUzz-kXJkgMgpgobyHBQRwo7lJe1s");
	headers = curl_slist_append(headers, "Content-Type: application/json; charset=utf-8");
	curl_easy_setopt(curl_easy, CURLOPT_HTTPHEADER, headers);

	curl_easy_setopt(curl_easy, CURLOPT_POSTFIELDS, request_json);
	//curl_easy_setopt(curl_easy, CURLOPT_POSTFIELDSIZE, request_json_len);

	curl_easy_setopt(curl_easy, CURLOPT_URL, url);
	curl_easy_setopt(curl_easy, CURLOPT_WRITEFUNCTION, write_data);
	//curl_easy_setopt(curl_easy, CURLOPT_HEADER, 1);
	//curl_easy_setopt(curl_easy, CURLOPT_VERBOSE, 1);

	//int success = curl_easy_perform(curl_easy);

	if (!OpenClipboard(NULL)) {
		errorExit("Failed to open the clipboard\n");
	}
	
	HBITMAP clipboardBitmapHandle = NULL;
	UINT clipboardFormat = 0;
	do {
		clipboardFormat = EnumClipboardFormats(clipboardFormat);
		printf("EnumClipboardFormats: %d\n", clipboardFormat);
		if (clipboardFormat == CF_BITMAP) clipboardBitmapHandle = GetClipboardData(clipboardFormat);
	} while (clipboardFormat);
	// printf("clipboardBitmapHandle: %s, size: %d\n", clipboardBitmapHandle, sizeof(clipboardBitmapHandle));

	HDC deviceContext = GetDC(NULL);

	// SelectObject(deviceContext, clipboardBitmapHandle);

	PBITMAPINFO bitmapInfo = CreateBitmapInfoStruct(NULL, clipboardBitmapHandle);
	int bitmapBufferSize;
	char* bitmapBuffer = createImageBuffer(bitmapInfo, clipboardBitmapHandle, deviceContext, &bitmapBufferSize);
	printf("clipboard bitmap buffer total size: %d\n", bitmapBufferSize);
	// Test file creation
	char * testFileName = "D:\\test.BMP";
	int bmpFileSize;
	CreateBMPFile(testFileName, bitmapInfo, clipboardBitmapHandle, deviceContext, &bmpFileSize);
	// read tested file
	FILE * fileHandle;
	fopen_s(&fileHandle, testFileName, "rb");
	char * readFileBuffer = malloc(bmpFileSize);
	fread_s(readFileBuffer, bmpFileSize, sizeof(char), bmpFileSize, fileHandle);
	/*
	// print the whole buffer
	fwrite(readFileBuffer, sizeof(readFileBuffer[0]), bmpFileSize, stdout);
	*/

	int base64BitmapSize;
	// char* base64Bitmap = base64_encode(&clipboardBitmapHandle, (bitmapBufferSize / sizeof(char)) - 1, &base64BitmapSize);
	// MEMORY_WRITER_STATE png = bitmapToPng(bitmapInfo->bmiHeader.biWidth, bitmapInfo->bmiHeader.biHeight, 8, bitmapBuffer);
	// printf("png.buffer: %s\n", png.buffer);
	// char* base64Bitmap = base64_encode(png.buffer, png.bufsize, &base64BitmapSize);
	// printf("base64 bitmapBuffer: %s\n", base64Bitmap);

	saveBitmapToFile("D:\\test_1.BMP", bitmapBuffer, bitmapBufferSize);

	// paste url to clipboard
	EmptyClipboard();
	char * base64ImagePrefix = "data:image/png;base64,";
	const base64ImagePrefixLen = strlen(base64ImagePrefix);
	const size_t len = bmpFileSize + base64ImagePrefixLen + 1;
	HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, len);
	memcpy(GlobalLock(hMem), base64ImagePrefix, base64ImagePrefixLen);
	memcpy((size_t) GlobalLock(hMem) + base64ImagePrefixLen, readFileBuffer, bmpFileSize);
	GlobalUnlock(hMem);
	if (!SetClipboardData(CF_TEXT, hMem)) errorExit(NULL);


	CloseClipboard();
	getchar();
}