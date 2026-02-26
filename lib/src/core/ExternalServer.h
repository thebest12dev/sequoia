#pragma once
#include <stdint.h>
#include <thread>
#include <optional>
#include <vector>
#include <WinSock2.h>
namespace sequoia {
  namespace net {
    using Buffer = std::vector<uint8_t>;
    struct BinaryPacket {
      PacketType type;
      Buffer payload;
      static BinaryPacket fromRaw(Buffer data);
    };
    enum class PacketType : uint8_t {
      PACKET_CLIENT_HANDSHAKE = 0x01,
      PACKET_SERVER_FEATURES = 0x02,
      PACKET_CLIENT_IDENTITY = 0x03
    };
    struct Payload {
      static void toBuffer();
    };
    struct PayloadServerFeatures : public Payload {
      bool serverRequiresAuthentication;
      bool serverIsLocal;
      uint32_t serverVersion;
      uint32_t serverProtocolVersion;
      uint8_t serverVersionMajor;
      uint8_t serverVersionMinor;
      uint8_t serverVersionPatch;

    };
    struct PayloadClientIdentity : public Payload {
      uint32_t clientVersion;
      uint32_t clientProtocolVersion;
      uint8_t clientVersionMajor;
      uint8_t clientVersionMinor;
      uint8_t clientVersionPatch;

    };
    class ExternalServer {
    private:

      std::optional<std::thread> serverThread;
      uint16_t port;
      void handleClient(SOCKET clientSocket);
      void serverHandler();
    public:
      ExternalServer(uint16_t port = 32067);
      void start();
      void stop();
      ~ExternalServer();

    };
  }
  
}