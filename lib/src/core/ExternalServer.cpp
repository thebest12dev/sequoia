#include "ExternalServer.h"
#include <iostream>
#include "LogHelpers.h"
#include <thread>
namespace sequoia {
  namespace net {
    BinaryPacket BinaryPacket::fromRaw(Buffer data) {
      BinaryPacket pc{};
      
      pc.type = static_cast<PacketType>(data[0]);
      pc.payload = std::vector<uint8_t>{ data.begin() + 1, data.end() - 1};
    }
    ExternalServer::ExternalServer(uint16_t port) : port(port) {
    }
    void ExternalServer::handleClient(SOCKET clientSocket)
    {
      char buffer[1024];
      int bytesReceived;

      while (true)
      {
        bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
        if (bytesReceived <= 0)
          break;
        // convert to packet
        Buffer buffer(buffer, buffer + bytesReceived);
    
        BinaryPacket packet = BinaryPacket::fromRaw(buffer);
        if (SEQUOIA_DEBUG) {
          std::cout << "received: type=" << static_cast<uint8_t>(packet.type) << std::endl;
        }
        switch (packet.type) {
        case PacketType::PACKET_CLIENT_HANDSHAKE:
          // give client server features
          BinaryPacket packet{};
          packet.type = PacketType::PACKET_SERVER_FEATURES;
          //packet.payload =
        }

          
          
      }

      closesocket(clientSocket);
      SEQUOIA_LOG_DEBUG("info: client disconnected!");
    }

    void ExternalServer::start() {
      serverThread = std::thread{ serverHandler };
      serverThread->detach();
    }
    void ExternalServer::serverHandler()
    {
      WSADATA wsaData;
      SOCKET serverSocket, clientSocket;
      sockaddr_in serverAddr{}, clientAddr{};
      int clientSize = sizeof(clientAddr);

      if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
      {
        return;
      }


      serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
      if (serverSocket == INVALID_SOCKET)
      {
        std::cerr << "socket creation failed! is it an invalid port?\n";
        WSACleanup();
        return;
      }


      serverAddr.sin_family = AF_INET;
      serverAddr.sin_port = htons(port);
      serverAddr.sin_addr.s_addr = INADDR_ANY;

      if (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
      {
        std::cerr << "bind failed! is the port being used?\n";
        closesocket(serverSocket);
        WSACleanup();
        return;
      }


      if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR)
      {
        std::cerr << "listen failed! sequoia will not listen to client connections\n";
        closesocket(serverSocket);
        WSACleanup();
        return;
      }

      std::cout << "server listening on port " << port << "...\n";


      while (true)
      {
        clientSocket = accept(serverSocket, (sockaddr*)&clientAddr, &clientSize);
        if (clientSocket == INVALID_SOCKET)
          continue;

        SEQUOIA_LOG_DEBUG("info: client connected!");


        std::thread t(handleClient, clientSocket);
        t.detach(); // new client
      }

      closesocket(serverSocket);
      WSACleanup();
    }
  }
  
}