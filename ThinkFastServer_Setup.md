# Requirements

* Windows 10 / Windows 11
* MinGW GCC Compiler
* C++17 support

Recommended installation method:

Install MSYS2 (includes MinGW GCC).

---

# Installing the Compiler

## Install MSYS2 using Winget

Open PowerShell or Command Prompt:

```
winget install -e --id MSYS2.MSYS2
```

After installation open **MSYS2 UCRT64** and run:

```
pacman -S mingw-w64-ucrt-x86_64-gcc
```

---

# Add GCC to PATH

Add the following directory to your Windows PATH:

```
C:\msys64\ucrt64\bin
```

Verify installation:

```
g++ --version
```

---

# Project Structure

```
ThinkFast
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
```

---

# Compiling the Server

Navigate to the project root and run:

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

Output:

```
[ThinkFast] Loading dictionary...
[ThinkFast] Server starting on port 8080
```

Server URL:

```
http://localhost:8080
```

---

# API Endpoints

| Endpoint     | Method | Description               |
| ------------ | ------ | ------------------------- |
| /auth        | POST   | User login authentication |
| /validate    | POST   | Validate submitted words  |
| /leaderboard | POST   | Retrieve leaderboard      |
| /room        | POST   | Manage multiplayer rooms  |

---

# Data Storage

User accounts are stored in:

```
data/users.csv
```

If the file does not exist, the server creates demo accounts:

```
heart / 123
angel / 123
```

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

Educational project for ThinkFast game backend.
