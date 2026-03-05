CXX      = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2

# Sources shared by BOTH binaries (no ConsoleUI, no GameEngine)
SHARED_CORE = \
    src/core/GameTypes.cpp      \
    src/core/WordValidator.cpp  \
    src/core/AuthManager.cpp    \
    src/utils/CSVManager.cpp    \
    src/utils/BotNames.cpp

# Console game: needs GameEngine + ConsoleUI + Screens
CONSOLE_SRCS = \
    main.cpp                    \
    src/core/GameEngine.cpp     \
    src/ui/ConsoleUI.cpp        \
    src/ui/Screens.cpp          \
    $(SHARED_CORE)

# HTTP server: needs RoomManager + HttpServer (no ConsoleUI, no GameEngine)
SERVER_SRCS = \
    server_main.cpp             \
    src/core/RoomManager.cpp    \
    src/server/HttpServer.cpp   \
    $(SHARED_CORE)

all: ThinkFast ThinkFastServer

ThinkFast: $(CONSOLE_SRCS:.cpp=.o)
	$(CXX) $(CXXFLAGS) -o $@ $^

ThinkFastServer: $(SERVER_SRCS:.cpp=.o)
	$(CXX) $(CXXFLAGS) -o $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

server: ThinkFastServer
run-server: ThinkFastServer
	./ThinkFastServer
clean:
	find . -name "*.o" -delete
	rm -f ThinkFast ThinkFastServer
.PHONY: all server run-server clean
