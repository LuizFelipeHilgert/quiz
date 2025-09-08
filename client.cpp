#define NOMINMAX
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <string>
#include <limits>  

using namespace std;

#pragma comment(lib, "Ws2_32.lib") 

const int PORT = 5000;

int main() {
    string SERVER_IP; 
    cout << "Digite o IP do servidor: ";
    cin >> SERVER_IP;
    cin.ignore(numeric_limits<streamsize>::max(), '\n'); 


    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2,2), &wsaData) != 0) {
        cerr << "Falha ao iniciar Winsock\n";
        return 1;
    }

    SOCKET client = socket(AF_INET, SOCK_STREAM, 0);
    if (client == INVALID_SOCKET) {
        cerr << "Erro ao criar socket\n";
        WSACleanup();
        return 1;
    }

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    if (inet_pton(AF_INET, SERVER_IP.c_str(), &serverAddr.sin_addr) <= 0) {
        cerr << "IP inválido\n";
        closesocket(client);
        WSACleanup();
        return 1;
    }

    if (connect(client, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        cerr << "Erro ao conectar ao servidor\n";
        closesocket(client);
        WSACleanup();
        return 1;
    }

    cout << "[+] Conectado ao servidor " << SERVER_IP << ":" << PORT << "\n";

    char buffer[1024];

    while (true) {
        int bytes = recv(client, buffer, sizeof(buffer) - 1, 0);
        if (bytes <= 0) {
            cout << "[-] Conexão encerrada pelo servidor.\n";
            break;
        }

        buffer[bytes] = '\0';
        string msgServidor(buffer);

        cout << "[Servidor diz] " << msgServidor << endl;

        if (msgServidor.find("Fim do quiz") != string::npos) {
            break;
        }

        cout << "Digite sua resposta: ";
        string resposta;
        getline(cin, resposta);

        send(client, resposta.c_str(), resposta.size(), 0);
        cout << "[Você respondeu] " << resposta << "\n";
    }

    closesocket(client);
    WSACleanup();
    return 0;
}
