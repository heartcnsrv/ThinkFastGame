# Requirements

* Windows 10 / Windows 11
* MinGW-w64 GCC Compiler
* C++17 support
* WAMP Server
* Web Browser (Chrome, Edge, Firefox)

Recommended installation method:

Install **MinGW-w64 using WinLibs**.

---

# Installing the Compiler

## Download MinGW (WinLibs)

Go to:

```
https://winlibs.com
```

Download the **latest GCC release for Windows**:

* Win64
* Without LLVM
* `.zip` version

Extract the archive to:

```
C:\mingw64
```

---

# Add GCC to PATH

Add the following directory to your Windows PATH:

```
C:\mingw64\bin
```

Steps:

1. Press **Win + R**
2. Type:

```
sysdm.cpl
```

3. Go to **Advanced → Environment Variables**
4. Under **System Variables**, select **Path**
5. Click **Edit → New**
6. Add:

```
C:\mingw64\bin
```

Verify installation:

```
g++ --version
```

You should see a **GCC version number**.

---

# Project Structure

```
ThinkFast3
│
├─ server_main.cpp
├─ src
│  ├─ core
│  │  ├─ RoomManager.cpp
│  │  ├─ GameTypes.cpp
│  │  ├─ WordValidator.cpp
│  │  └─ AuthManager.cpp
│  │
│  ├─ server
│  │  └─ HttpServer.cpp
│  │
│  └─ utils
│     ├─ CSVManager.cpp
│     └─ BotNames.cpp
│
├─ api
├─ src/gui
└─ data
```

---

# Compiling the Server

Open **Command Prompt** and navigate to the project folder:

```
cd C:\wamp64\www\ThinkFast3
```

Compile the server:

```
g++ -std=c++17 -O2 -o ThinkFastServer.exe ^
server_main.cpp ^
src/core/RoomManager.cpp ^
src/server/HttpServer.cpp ^
src/core/GameTypes.cpp ^
src/core/WordValidator.cpp ^
src/core/AuthManager.cpp ^
src/utils/CSVManager.cpp ^
src/utils/BotNames.cpp ^
-lws2_32
```

This generates:

```
ThinkFastServer.exe
```

---

# Running the Server

Start the server:

```
ThinkFastServer.exe
```

Expected output:

```
[ThinkFast] Loading dictionary...
[ThinkFast] Dictionary ready (XXXX words).
[ThinkFast] Server starting on port 8080
[ThinkFast] Endpoints:
   POST http://localhost:8080/auth
   POST http://localhost:8080/validate
   POST http://localhost:8080/leaderboard
   POST http://localhost:8080/room
```

Server URL:

```
http://localhost:8080
```

Leave this window open while the game is running.

---

# Starting the Web Interface

1. Start **WAMP Server**
2. Wait until the **tray icon turns green**
3. Open your browser:

```
http://thinkfast/src/gui/
```

---

# Testing the Connection

Open the API endpoint:

```
http://thinkfast/api/leaderboard
```

Expected response:

```
{"ok":true,"players":[...]}
```

This confirms:

* Apache is running
* PHP routing works
* The C++ server is connected

---

# API Endpoints

| Endpoint     | Method | Description               |
| ------------ | ------ | ------------------------- |
| /auth        | POST   | User login authentication |
| /validate    | POST   | Validate submitted words  |
| /leaderboard | POST   | Retrieve leaderboard      |
| /room        | POST   | Manage multiplayer rooms  |

---

# Quick Start (Running the Game)

| Step | Action                                                          |
| ---- | --------------------------------------------------------------- |
| 1    | Run **ThinkFastServer.exe**                                     |
| 2    | Start **WAMP → Start All Services**                             |
| 3    | Open **[http://thinkfast/src/gui/](http://thinkfast/src/gui/)** |

---

# Stopping the Server

Press:

```
Ctrl + C
```

The server shuts down safely.

---

# Network Access

Other devices in the same network can access the server using:

```
http://SERVER_IP:8080
```

Example:

```
http://192.168.1.10:8080
```

---

# License

Educational project for the **ThinkFast multiplayer word game backend**.
