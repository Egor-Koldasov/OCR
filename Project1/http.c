#include "stdio.h"
#include <winsock2.h>
#include <mstcpip.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib") //Winsock Library
#pragma comment(lib, "fwpuclnt.lib")

void get_website();

boolean initWSA(WSADATA* wsa_data) {
	printf("\nInitialising Winsock...");
	if (WSAStartup(MAKEWORD(2, 2), &wsa_data) == SOCKET_ERROR) {
		printf("Failed. Error Code : %d", WSAGetLastError());
		return FALSE;
	}
	printf(" Initialised.\n");
	return TRUE;
}

void get_website() {
	char* host = "vision.googleapis.com";
	char http [500];
	char* request_json = (
		"{'requests':[{'image':{'source':{'gcsImageUri': 'gs://cloud-samples-data/vision/using_curl/shanghai.jpeg'}}}]}"
		);
	char request_json_len [3];

	sprintf_s(request_json_len, sizeof(request_json_len), "%d", strlen(request_json_len));

	sprintf_s(
		http,
		sizeof(http),
		(
			"POST /v1/images:annotate HTTP/1.1\n"
			"Host: %s\n"
			"Authorization: Bearer AIzaSyDV7KXUzz-kXJkgMgpgobyHBQRwo7lJe1s\n"
			"Content-Type: application/json; charset=utf-8\n"
			"Content-Length: %s\n"
			"Connection: close\n\n"
			"%s\n"
		),
		host,
		request_json_len,
		request_json
	);
	printf("%s", http);

	WSADATA wsa_data;
	if (!initWSA(&wsa_data)) {
		return;
	}
	const SOCKET socket_descriptor = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (socket_descriptor == INVALID_SOCKET) {
		printf("Socket function failed with error = %d\n", WSAGetLastError());
	}
	int status;

	struct addrinfo hints;
	memset(&hints, 0, sizeof(hints));
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	struct addrinfo* addr = NULL;

	status = getaddrinfo(host, "443", &hints, &addr);
	if (status == SOCKET_ERROR) {
		printf("Failed to retrieve address info: %s\n", gai_strerror(status));
		closesocket(socket_descriptor);
		return;
	}

	status = connect(socket_descriptor, addr->ai_addr, addr->ai_addrlen);
	if (status == SOCKET_ERROR) {
		printf("Socket connect failed with error = %d\n", WSAGetLastError());
		closesocket(socket_descriptor);
		return;
	}

	status = send(socket_descriptor, http, strlen(http), 0);
	if (status == SOCKET_ERROR) {
		printf("Socket connect failed with error = %d\n", WSAGetLastError());
		closesocket(socket_descriptor);
		return;
	}
	printf("Bytes sent - %d\n", status);

	char receiving_buffer[1000000];
	int receiving_buffer_len = 1000000;
	int actual_responce_len = 0;
	do {
		status = recv(socket_descriptor, receiving_buffer, receiving_buffer_len, 0);
		if (status > 0) {
			printf("Bytes received: %d\n", status);
			actual_responce_len = actual_responce_len + status;
		}
		else if (status == 0)
			printf("Connection closed\n");
		else
			printf("recv failed: %d\n", WSAGetLastError());

	} while (status > 0);
	printf("\nHTTP receved [%d bytes]:\n\n\n %.*s\n\n\n\n", actual_responce_len, actual_responce_len, receiving_buffer);
	getchar();
	closesocket(socket_descriptor);
}