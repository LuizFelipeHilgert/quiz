#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <string>

#pragma comment(lib, "Ws2_32.lib")

const int PORT = 5000;
const std::string SERVER_IP = "127.0.0.1";

int main() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2,2), &wsaData) != 0) {
        std::cerr << "Falha ao iniciar Winsock\n";
        return 1;
    }

    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) {
        std::cerr << "Erro ao criar socket\n";
        WSACleanup();
        return 1;
    }

    sockaddr_in servidor;
    servidor.sin_family = AF_INET;
    servidor.sin_port = htons(PORT);
    inet_pton(AF_INET, SERVER_IP.c_str(), &servidor.sin_addr);

    if (connect(sock, (sockaddr*)&servidor, sizeof(servidor)) == SOCKET_ERROR) {
        std::cerr << "Falha ao conectar\n";
        closesocket(sock);
        WSACleanup();
        return 1;
    }

    char buffer[1024];
    while (true) {
        memset(buffer, 0, sizeof(buffer));
        int bytes = recv(sock, buffer, sizeof(buffer), 0);
        if (bytes <= 0) break;

        std::string pergunta(buffer);
        std::cout << pergunta << std::endl;

        if (pergunta.find("Fim do quiz") != std::string::npos)
            break;

        std::string resposta;
        std::getline(std::cin, resposta);
        send(sock, resposta.c_str(), resposta.size(), 0);
    }

    closesocket(sock);
    WSACleanup();
    return 0;
}
