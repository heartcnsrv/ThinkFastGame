#pragma once
// 
//  ThinkFast  |  src/server/HttpServer.h
//  Windows (Winsock2) + Linux (POSIX) compatible HTTP server
//

#include "../core/AuthManager.h"
#include "../core/WordValidator.h"
#include "../core/RoomManager.h"
#include <string>
#include <thread>
#include <atomic>
#include <mutex>

#ifdef _WIN32
  #include <winsock2.h>
  #include <ws2tcpip.h>
  typedef SOCKET sock_t;
  #define SOCK_INVALID INVALID_SOCKET
  #define SOCK_ERR     SOCKET_ERROR
  #define sock_close   closesocket
#else
  #include <sys/socket.h>
  #include <netinet/in.h>
  #include <unistd.h>
  typedef int sock_t;
  #define SOCK_INVALID (-1)
  #define SOCK_ERR     (-1)
  #define sock_close   close
#endif

namespace ThinkFast {

class HttpServer {
public:
    HttpServer(int port,
               AuthManager&   auth,
               WordValidator& validator,
               RoomManager&   rooms);
    ~HttpServer();

    void run();
    void stop();

private:
    int            port_;
    AuthManager&   auth_;
    WordValidator& validator_;
    RoomManager&   rooms_;
    std::atomic<bool> running_{true};

    void handleClient(sock_t fd);
    std::string dispatch(const std::string& path, const std::string& body);

    std::string routeAuth        (const std::string& body);
    std::string routeValidate    (const std::string& body);
    std::string routeLeaderboard (const std::string& body);
    std::string routeRoom        (const std::string& body);

    static std::string jsonGet   (const std::string& json, const std::string& key);
    static int         jsonGetInt(const std::string& json, const std::string& key, int def = 0);
    static std::string jsonEscape(const std::string& s);
    static std::string ok (const std::string& body = "");
    static std::string err(const std::string& msg);
};

}
