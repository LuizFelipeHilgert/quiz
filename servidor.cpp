#define NOMINMAX
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <fstream>
#include <mutex>

#pragma comment(lib, "Ws2_32.lib")

using namespace std;

const int PORTA = 5000;
const int TAMANHO_BUFFER = 1024;

struct Pergunta {
    string enunciado;
    string resposta;
};

vector<Pergunta> perguntas;

struct Placar {
    string ipCliente;
    int pontos;
};

vector<Placar> placares;
mutex mtx;

void carregarPerguntas(const string& nomeArquivo) {
    ifstream arquivo(nomeArquivo);
    if (!arquivo.is_open()) {
        cerr << "erro aquivo de perguntas\n";
        exit(1);
    }

    string linha;
    while (getline(arquivo, linha)) {
        size_t pos = linha.find(';');
        if (pos != string::npos) {
            Pergunta p;
            p.enunciado = linha.substr(0, pos);
            p.resposta = linha.substr(pos + 1);
            perguntas.push_back(p);
        }
    }
}

string normalizar(const string& texto) {
    string s = texto;
    for (char& c : s) c = tolower(c);
    s.erase(s.find_last_not_of("\n\r") + 1);
    return s;
}

void atenderCliente(SOCKET cliente, string ipCliente) {
    char buffer[TAMANHO_BUFFER];
    int pontos = 0;

    for (auto& p : perguntas) {
        send(cliente, p.enunciado.c_str(), p.enunciado.size(), 0);

        int bytesRecebidos = recv(cliente, buffer, sizeof(buffer), 0);
        if (bytesRecebidos <= 0) break;

        string resposta(buffer, bytesRecebidos);

        if (normalizar(resposta) == normalizar(p.resposta)) {
            pontos++;
            string msg = "Correto!\n";
            send(cliente, msg.c_str(), msg.size(), 0);
        } else {
            string msg = "Errado!\n";
            send(cliente, msg.c_str(), msg.size(), 0);
        }
    }

    string fim = "Fim do quiz. Sua pontuação: " + to_string(pontos) + "\n";
    send(cliente, fim.c_str(), fim.size(), 0);


    {
        lock_guard<mutex> lock(mtx);
        placares.push_back({ipCliente, pontos});
    }

    closesocket(cliente);
    cout << "Cliente " << ipCliente << " desconectou com " << pontos << " pontos.\n";
}

int main() {
    carregarPerguntas("perguntas.txt");

    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2,2), &wsaData) != 0) {
        cerr << "Falha ao iniciar Winsock\n";
        return 1;
    }

    SOCKET servidor = socket(AF_INET, SOCK_STREAM, 0);
    if (servidor == INVALID_SOCKET) {
        cerr << "Erro ao criar socket\n";
        WSACleanup();
        return 1;
    }

    sockaddr_in enderecoServidor;
    enderecoServidor.sin_family = AF_INET;
    enderecoServidor.sin_addr.s_addr = INADDR_ANY;
    enderecoServidor.sin_port = htons(PORTA);

    if (bind(servidor, (sockaddr*)&enderecoServidor, sizeof(enderecoServidor)) == SOCKET_ERROR) {
        cerr << "Erro ao associar socket na porta\n";
        closesocket(servidor);
        WSACleanup();
        return 1;
    }

    if (listen(servidor, SOMAXCONN) == SOCKET_ERROR) {
        cerr << "Erro ao escutar na porta\n";
        closesocket(servidor);
        WSACleanup();
        return 1;
    }

    cout << "IP do servidor:\n";
    system("ipconfig | findstr IPv4");

    while (true) {
        sockaddr_in enderecoCliente;
        int tamanhoEndereco = sizeof(enderecoCliente);
        SOCKET cliente = accept(servidor, (sockaddr*)&enderecoCliente, &tamanhoEndereco);
        if (cliente == INVALID_SOCKET) {
            cerr << "Erro ao aceitar cliente\n";
            continue;
        }

        char ipStr[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(enderecoCliente.sin_addr), ipStr, INET_ADDRSTRLEN);
        string ipCliente = ipStr;

        cout << "Novo cliente conectado: " << ipCliente << "\n";

        thread(atenderCliente, cliente, ipCliente).detach();
    }

    closesocket(servidor);
    WSACleanup();
    return 0;
}
