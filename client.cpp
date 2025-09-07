#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <string>

#pragma comment(lib, "Ws2_32.lib") // Winsock

const int PORT = 5000;
const char* SERVER_IP = "192.168.1.11"; // Localhost

int main() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2,2), &wsaData) != 0) {
        std::cerr << "Falha ao iniciar Winsock\n";
        return 1;
    }

    SOCKET client = socket(AF_INET, SOCK_STREAM, 0);
    if (client == INVALID_SOCKET) {
        std::cerr << "Erro ao criar socket\n";
        WSACleanup();
        return 1;
    }

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    inet_pton(AF_INET, SERVER_IP, &serverAddr.sin_addr);

    if (connect(client, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Erro ao conectar ao servidor\n";
        closesocket(client);
        WSACleanup();
        return 1;
    }

    std::cout << "[+] Conectado ao servidor " << SERVER_IP << ":" << PORT << "\n";

    char buffer[1024];

    while (true) {
        int bytes = recv(client, buffer, sizeof(buffer) - 1, 0);
        if (bytes <= 0) {
            std::cout << "[-] Conexão encerrada pelo servidor.\n";
            break;
        }

        buffer[bytes] = '\0';
        std::string msgServidor(buffer);

        // Mostra mensagem do servidor
        std::cout << "[Servidor diz] " << msgServidor << std::endl;

        // Se for a mensagem final, não precisa responder
        if (msgServidor.find("Fim do quiz") != std::string::npos) {
            break;
        }

        // Cliente digita resposta
        std::cout << "Digite sua resposta: ";
        std::string resposta;
        std::getline(std::cin, resposta);

        send(client, resposta.c_str(), resposta.size(), 0);
        std::cout << "[Você respondeu] " << resposta << "\n";
    }

    closesocket(client);
    WSACleanup();
    return 0;
}
