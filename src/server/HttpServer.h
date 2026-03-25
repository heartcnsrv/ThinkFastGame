#pragma once

#include "../core/AuthManager.h"
#include "../core/RoomManager.h"
#include "../core/WordValidator.h"
#include <atomic>
#include <mutex>
#include <string>
#include <thread>

#ifdef _WIN32
  #include <winsock2.h>
  #include <ws2tcpip.h>
  typedef SOCKET sock_t;
  #define SOCK_INVALID INVALID_SOCKET
  #define SOCK_ERR     SOCKET_ERROR
  #define sock_close   closesocket
#else
  #include <netinet/in.h>
  #include <sys/socket.h>
  #include <unistd.h>
  typedef int sock_t;
  #define SOCK_INVALID (-1)
  #define SOCK_ERR     (-1)
  #define sock_close   close
#endif

namespace ThinkFast {

/*
 * Lightweight HTTP server for the web version of ThinkFast.
 * This class is the transport layer only.
 * It receives raw HTTP requests, extracts JSON payloads,
 * and forwards the work to AuthManager, WordValidator, and RoomManager.
 */
class HttpServer {
public:
    /*
     * Prepares the server with the core services it needs.
     * - port: TCP port to listen on
     * - auth: authentication/account service
     * - validator: word-checking service
     * - rooms: multiplayer room service
     */
    HttpServer(int port,
               AuthManager&   auth,
               WordValidator& validator,
               RoomManager&   rooms);
    ~HttpServer();

    /* Starts accepting HTTP client connections. */
    void run();
    /* Requests server shutdown. */
    void stop();

private:
    int               port_;          /* Listening TCP port. */
    AuthManager&      auth_;          /* CSV-backed account service. */
    WordValidator&    validator_;     /* Shared word validation service. */
    RoomManager&      rooms_;         /* Live multiplayer room service. */
    std::atomic<bool> running_{true}; /* Main server loop flag. */

    /* Handles one connected client socket. */
    void handleClient(sock_t fd);
    /* Routes one parsed request path to the matching handler. */
    std::string dispatch(const std::string& path, const std::string& body);

    /* Handles /auth requests. */
    std::string routeAuth(const std::string& body);
    /* Handles /validate requests. */
    std::string routeValidate(const std::string& body);
    /* Handles /leaderboard requests. */
    std::string routeLeaderboard(const std::string& body);
    /* Handles /room requests. */
    std::string routeRoom(const std::string& body);

    /* Reads a string field from a simple JSON body. */
    static std::string jsonGet(const std::string& json, const std::string& key);
    /* Reads an integer field from a simple JSON body. */
    static int jsonGetInt(const std::string& json, const std::string& key, int def = 0);
    /* Escapes text before placing it into JSON. */
    static std::string jsonEscape(const std::string& s);
    /* Wraps a successful JSON response body. */
    static std::string ok(const std::string& body = "");
    /* Wraps an error JSON response body. */
    static std::string err(const std::string& msg);
};

}
