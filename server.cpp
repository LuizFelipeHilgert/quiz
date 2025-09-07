#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <map>

#pragma comment(lib, "Ws2_32.lib") // Linka a biblioteca Winsock

const int PORT = 5000;

std::vector<std::pair<std::string,std::string>> perguntas = {
    {"Quanto é 2+2?", "4"},
    {"Qual a capital da França?", "paris"},
    {"Quem descobriu o Brasil?", "cabral"}
};

std::map<SOCKET, int> pontuacoes;

// Função para identificar cliente pelo IP e porta
std::string identificar_cliente(sockaddr_in addr) {
    char ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(addr.sin_addr), ip, INET_ADDRSTRLEN);
    int porta = ntohs(addr.sin_port);
    return std::string(ip) + ":" + std::to_string(porta);
}

void atender_cliente(SOCKET cliente, sockaddr_in clienteAddr) {
    std::string id = identificar_cliente(clienteAddr);
    std::cout << "[+] Novo cliente conectado: " << id << "\n";

    pontuacoes[cliente] = 0;

    for (auto& p : perguntas) {
        // Envia pergunta
        send(cliente, p.first.c_str(), p.first.size(), 0);
        std::cout << "[Servidor -> " << id << "] Pergunta enviada: " << p.first << "\n";

        char buffer[1024] = {0};
        int bytes = recv(cliente, buffer, sizeof(buffer), 0);
        if (bytes <= 0) break;

        std::string resposta(buffer, bytes);
        resposta.erase(resposta.find_last_not_of("\n\r") + 1);
        for(auto & c: resposta) c = tolower(c);

        std::cout << "[Servidor <- " << id << "] Resposta recebida: " << resposta << "\n";

        if (resposta == p.second) {
            pontuacoes[cliente]++;
            std::string msg = "✔ Resposta correta!\n";
            send(cliente, msg.c_str(), msg.size(), 0);
            std::cout << "[Servidor -> " << id << "] " << msg;
        } else {
            std::string msg = "✘ Resposta errada!\n";
            send(cliente, msg.c_str(), msg.size(), 0);
            std::cout << "[Servidor -> " << id << "] " << msg;
        }
    }

    std::string fim = "Fim do quiz! Sua pontuação: " + std::to_string(pontuacoes[cliente]) + "\n";
    send(cliente, fim.c_str(), fim.size(), 0);
    std::cout << "[Servidor -> " << id << "] " << fim;

    closesocket(cliente);
    std::cout << "[-] Cliente desconectado: " << id << "\n";
}

int main() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2,2), &wsaData) != 0) {
        std::cerr << "Falha ao iniciar Winsock\n";
        return 1;
    }

    SOCKET server = socket(AF_INET, SOCK_STREAM, 0);
    if (server == INVALID_SOCKET) {
        std::cerr << "Erro ao criar socket\n";
        WSACleanup();
        return 1;
    }

    sockaddr_in endereco;
    endereco.sin_family = AF_INET;
    endereco.sin_addr.s_addr = INADDR_ANY;
    endereco.sin_port = htons(PORT);

    if (bind(server, (sockaddr*)&endereco, sizeof(endereco)) == SOCKET_ERROR) {
        std::cerr << "Bind falhou\n";
        closesocket(server);
        WSACleanup();
        return 1;
    }

    if (listen(server, 5) == SOCKET_ERROR) {
        std::cerr << "Listen falhou\n";
        closesocket(server);
        WSACleanup();
        return 1;
    }

    std::cout << "Servidor rodando na porta " << PORT << "...\n";

    while (true) {
        sockaddr_in clienteAddr;
        int clienteLen = sizeof(clienteAddr);

        SOCKET cliente = accept(server, (sockaddr*)&clienteAddr, &clienteLen);
        if (cliente == INVALID_SOCKET) {
            std::cerr << "Accept falhou\n";
            continue;
        }

        std::thread(atender_cliente, cliente, clienteAddr).detach();
    }

    closesocket(server);
    WSACleanup();
    return 0;
}
