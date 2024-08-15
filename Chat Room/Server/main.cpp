#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <tchar.h>
#include <thread>
#include <vector>

using namespace std;

#pragma comment(lib, "ws2_32.lib")

bool initialize() {
	WSADATA data;
	return WSAStartup(MAKEWORD(2, 2), &data) == 0;
}

void interaction(SOCKET clientSocket, vector<SOCKET> &clients) {
	cout << "Client connected" << endl;

	char buffer[4096];

	while (1) {

		int mybytesrecvd = recv(clientSocket, buffer, sizeof(buffer), 0);

		if (mybytesrecvd <= 0) {
			cout << "Client disconnected" << endl;
			break;
		}

		string message(buffer, mybytesrecvd);
		cout << "Message from " << message << endl;

		for (auto client : clients) {
			if (client != clientSocket) {
				send(client, message.c_str(), message.length(), 0);
			}
		}
	}

	auto deadSocket = find(clients.begin(), clients.end(), clientSocket);
	if (deadSocket != clients.end()) {
		clients.erase(deadSocket);
	}
	closesocket(clientSocket);
}

int main() {
	if (!initialize()) {
		cout << "WinSock initialization failed" << endl;
		return 1;
	}

	cout << "Server Program" << endl;

	SOCKET listenSocket = socket( AF_INET, SOCK_STREAM, 0);

	if (listenSocket == INVALID_SOCKET) {
		cout << "Socket creation failed" << endl;
		return 1;
	}

	sockaddr_in serveraddr;
	int port = 12345;
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(port);

	if (InetPton(AF_INET, _T("0.0.0.0"), &serveraddr.sin_addr) != 1) {
		cout << "setting address structure failed" << endl;
		closesocket(listenSocket);
		WSACleanup;
		return 1;
	}

	if (bind(listenSocket, reinterpret_cast<sockaddr*>(&serveraddr), sizeof(serveraddr)) == SOCKET_ERROR) {
		cout << "Bind failed" << endl;
		closesocket(listenSocket);
		WSACleanup();
		return 1;
	}

	if (listen(listenSocket, SOMAXCONN) == SOCKET_ERROR) {
		cout << "listen failed" << endl;
		closesocket(listenSocket);
		WSACleanup();
		return 1;
	}

	cout << "The Server has started listening to the port : " << port << endl;
	vector<SOCKET> clients;

	while (1) {

		SOCKET clientSocket = accept(listenSocket, nullptr, nullptr);

		if (clientSocket == INVALID_SOCKET) {
			cout << "Invalid client socket" << endl;
		}

		clients.push_back(clientSocket);

		thread t1(interaction, clientSocket, std::ref(clients));
		t1.detach();
	}

	closesocket(listenSocket);
	WSACleanup();

}