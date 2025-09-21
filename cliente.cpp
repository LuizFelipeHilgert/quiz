#define NOMINMAX
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <string>
#include <limits>

#pragma comment(lib, "Ws2_32.lib")

using namespace std;

const int PORTA = 5000;
const int TAMANHO_BUFFER = 1024;

int main() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2,2), &wsaData) != 0) {
        cerr << "Falha ao iniciar Winsock\n";
        return 1;
    }

    SOCKET cliente = socket(AF_INET, SOCK_STREAM, 0);
    if (cliente == INVALID_SOCKET) {
        cerr << "Erro ao criar socket\n";
        WSACleanup();
        return 1;
    }

    sockaddr_in servidor;
    servidor.sin_family = AF_INET;
    servidor.sin_port = htons(PORTA);

    string ipServidor;
    cout << "Digite o IP do servidor: ";
    cin >> ipServidor;
    cin.ignore(numeric_limits<streamsize>::max(), '\n'); 
    inet_pton(AF_INET, ipServidor.c_str(), &servidor.sin_addr);

    if (connect(cliente, (sockaddr*)&servidor, sizeof(servidor)) == SOCKET_ERROR) {
        cerr << "Não foi possível conectar ao servidor\n";
        closesocket(cliente);
        WSACleanup();
        return 1;
    }

    cout << "Conectado ao servidor!\n\n";

    char buffer[TAMANHO_BUFFER];

    while (true) {
        int bytesRecebidos = recv(cliente, buffer, sizeof(buffer), 0);
        if (bytesRecebidos <= 0) break;

        string msg(buffer, bytesRecebidos);
        cout << msg << endl;

        if (msg.find("Fim do quiz") != string::npos) break;

      
        string resposta;
        getline(cin, resposta);

        send(cliente, resposta.c_str(), resposta.size(), 0);
    }

    closesocket(cliente);
    WSACleanup();

    cout << "\nPressione ENTER para sair...";
    cin.get(); 
    return 0;
}
