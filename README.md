# ThinkFastGame

## HOW TO RUN (WINDOWS POWERSHELL)

### STEP 1 - GO TO PROJECT FOLDER

```powershell
cd "D:\Programming Projects\ThinkFastGame"
```

---

### STEP 2 - BUILD THE TERMINAL VERSION

```powershell
g++ -std=c++17 -O2 -o ThinkFast.exe main.cpp src/core/GameTypes.cpp src/core/WordValidator.cpp src/core/GameEngine.cpp src/core/AuthManager.cpp src/ui/ConsoleUI.cpp src/ui/Screens.cpp src/utils/CSVManager.cpp src/utils/BotNames.cpp
```

---

### STEP 3 - RUN THE TERMINAL VERSION

```powershell
.\ThinkFast.exe
```

---

### STEP 4 - BUILD THE BACKEND SERVER

```powershell
g++ -std=c++17 -O2 -o ThinkFastServer.exe server_main.cpp src/core/RoomManager.cpp src/server/HttpServer.cpp src/core/GameTypes.cpp src/core/WordValidator.cpp src/core/AuthManager.cpp src/utils/CSVManager.cpp src/utils/BotNames.cpp -lws2_32
```

---

### STEP 5 - RUN THE BACKEND SERVER

```powershell
.\ThinkFastServer.exe
```

---

## HOW THE PROJECT LOGIC WORKS

### 1. Data is prepared first

The project creates or reads `data/users.csv`.
This file acts like a simple database.
It stores usernames, passwords, and player statistics.

This step is important because the game needs saved accounts and saved progress.
Without this file, the system would forget users after the program closes.

### 2. Core services are created

After storage is ready, the program creates the main C++ objects:

- `AuthManager`
- `WordValidator`
- `GameEngine`
- `RoomManager`
- `HttpServer`

These classes were chosen because each one solves one clear problem.
This makes the project easier to understand for other programmers.

### 3. The player enters the system

In the terminal version, the user logs in, registers, or enters as a guest.
In the web version, the browser sends requests to the backend server.

This step is important because every game session starts with a player identity.
The program needs to know if the player is a saved user, a guest, a bot, or a remote player.

### 4. The game rules are applied

When a match starts, the game checks turns, words, hearts, eliminations, and winners.
The `GameEngine` handles terminal matches.
The `RoomManager` handles multiplayer room logic for the web version.

This is important because the rules are the center of the project.
Without clear rule handling, the game would not be fair or consistent.

### 5. Words are checked

The program uses `WordValidator` to decide if a word is acceptable.
It first checks a local built-in dictionary.
If the word is not found there, it can ask an online dictionary API.
The result is cached so the same check does not repeat too many times.

This step was chosen to balance speed and flexibility.
Local checking is fast.
API checking helps when a valid word is not in the local list.

### 6. Results are saved

After the game, player statistics can be written back to `users.csv`.
This is handled by `AuthManager`.

This is important because players expect their wins, losses, and games played to remain saved for the next session.

## WHY THE C++ STRUCTURE IS IMPORTANT

The project uses several common C++ structures and ideas.
These are important because they help organize the system clearly.

### Classes

Classes such as `AuthManager`, `GameEngine`, `WordValidator`, `RoomManager`, and `HttpServer` group related functions and data together.

This is important because each class becomes responsible for one main role.
It also keeps the project more professional and easier to expand.

### Structs

Structs such as `Player`, `GameSession`, `Room`, and `RoomPlayer` store related data in one place.

This is important because the program needs a clean way to represent a player, a match, or a room.
Without structs, the code would need too many separate variables, and that would be confusing.

### Enums

Enums such as `GameMode`, `PlayerType`, and `RoomStatus` give names to fixed choices.

This is important because named values are easier to understand than plain numbers.
They also make the code safer and more readable.

### Vectors

The project uses `std::vector` to store lists such as players, words, and room logs.

This is important because the number of players or words can change while the program is running.
Vectors make that easier to manage.

### Unordered sets and unordered maps

The project uses `std::unordered_set` and `std::unordered_map` in places such as word checking, used words, API cache, and room storage.

These are important because they support fast searching.
That matters in a game where quick checking improves performance.

### Header and source files

The project separates `.h` and `.cpp` files.
The header files declare what a module offers.
The source files contain the real implementation.

This is important because it improves organization and supports better code reuse.

## WHY THE MAIN FILES MATTER

### `main.cpp`

This file starts the terminal version.
It prepares storage, creates the main objects, and sends the player into the terminal screens.

### `server_main.cpp`

This file starts the web backend server.
It prepares the shared services and opens the HTTP routes used by the browser frontend.

### `GameTypes.h`

This file contains shared enums and structs used across the project.
It is important because many modules depend on the same player and session data shapes.

### `AuthManager`

This part controls login, registration, guest access, saved stats, and leaderboard data.
It is important because account logic should stay in one place.

### `WordValidator`

This part checks words and supports bot choices.
It is important because the game depends on valid words.

### `GameEngine`

This part controls the rules of the terminal version.
It is important because it manages turns, hearts, eliminations, and winners.

### `RoomManager`

This part controls multiplayer room state for the web version.
It is important because online rooms need shared rules and shared state.

### `CSVManager`

This part reads and writes CSV files.
It is important because it keeps file-format logic separate from game logic.

### `HttpServer`

This part receives web requests and sends them to the correct service.
It is important because it connects the browser version to the C++ backend.

## WHY THESE DESIGN CHOICES WERE CHOSEN

### Why use CSV?

CSV is simple and easy to inspect.
It is a good choice for a small project where a full database is not necessary.

### Why separate terminal and web logic?

The project supports two ways to use the same game idea.
Separating them helps keep each version cleaner.

### Why keep gameplay logic away from storage logic?

This prevents one part of the code from doing too many jobs.
It also makes debugging easier.

### Why use comments?

Comments help other people understand what each module does and why it exists.
That is especially useful in student projects and team projects.

## QUESTIONS ABOUT THE C++ STRUCTURE

### 1. Why does the project use classes?

Classes organize the code by responsibility.

### 2. Why are classes important here?

They help separate login, word checking, gameplay, and server work.

### 3. Why does the project use structs?

Structs group related data such as player information and match state.

### 4. Why are structs important?

They keep the data clean and easier to manage.

### 5. Why does the project use enums?

Enums represent fixed choices like game mode and player type.

### 6. Why are enums important?

They make the code easier to read than using raw numbers.

### 7. Why does the project use `std::vector`?

Because the number of players and words can change.

### 8. Why is `std::vector` important?

It gives flexible storage for lists in the program.

### 9. Why does the project use `std::unordered_set`?

It supports fast checking for used words and known dictionary words.

### 10. Why is `std::unordered_set` important?

It improves speed when the game needs quick lookup.

### 11. Why does the project use `std::unordered_map`?

It stores key-value data such as API cache and room storage.

### 12. Why is `std::unordered_map` important?

It makes searching by key faster and clearer.

### 13. Why are there header files?

Header files declare the structure of the code.

### 14. Why are header files important?

They show what each module provides to the rest of the project.

### 15. Why are there source files?

Source files contain the real code implementation.

### 16. Why are source files important?

They keep the implementation separate from declarations.

### 17. Why does the project use namespaces?

Namespaces help group the project code under `ThinkFast`.

### 18. Why are namespaces important?

They reduce name conflicts and improve organization.

### 19. Why does the project pass objects like `WordValidator&` by reference?

It allows modules to share one service without copying it.

### 20. Why is passing by reference important?

It is more efficient and keeps shared logic consistent.

### 21. Why does the project keep `Player` and `GameSession` separate?

Because a player and a match are not the same kind of data.

### 22. Why is that separation important?

It helps the code clearly separate long-term account data from temporary game state.

### 23. Why does the project have an `AuthManager` class?

It centralizes account-related logic.

### 24. Why is `AuthManager` important?

It prevents account code from being repeated in many files.

### 25. Why does the project have a `WordValidator` class?

It centralizes all word-checking logic.

### 26. Why is `WordValidator` important?

It keeps game validation consistent in every mode.

### 27. Why does the project have a `GameEngine` class?

It handles the rules of the terminal game.

### 28. Why is `GameEngine` important?

It keeps gameplay logic separate from UI and storage.

### 29. Why does the project have a `RoomManager` class?

It controls multiplayer room state for the web version.

### 30. Why is `RoomManager` important?

It keeps shared room logic in one place.

### 31. Why does the project have a `CSVManager` class?

It handles reading and writing CSV files.

### 32. Why is `CSVManager` important?

It keeps storage details away from the main game logic.

### 33. Why does the project have an `HttpServer` class?

It handles web communication with the browser frontend.

### 34. Why is `HttpServer` important?

It connects the web interface to the C++ backend logic.

### 35. Why are `main.cpp` and `server_main.cpp` separate?

They start two different versions of the project.

### 36. Why is that separation important?

It keeps terminal startup logic and web startup logic clear.

### 37. Why is `users.csv` used by both terminal and web modes?

Both versions need the same saved account data.

### 38. Why is shared storage important?

It keeps user progress consistent across versions.

### 39. Why does the project use a built-in dictionary?

It gives fast local word checking.

### 40. Why is the built-in dictionary important?

It reduces delay during gameplay.

### 41. Why does the project still use an online API?

Some valid words may not exist in the local list.

### 42. Why is API fallback important?

It makes the validator more flexible.

### 43. Why are game stats saved after matches?

Because users expect progress tracking.

### 44. Why is saving stats important?

It gives meaning to wins, losses, and leaderboards.

### 45. Why are comments written in a simple style?

Because the project should be understandable to other readers.

### 46. Why is simple explanation important in a project?

It helps classmates, teachers, and future developers understand it faster.

### 47. Why is modular code important in C++?

It makes large programs easier to maintain.

### 48. Why is readability important in this project?

Because the project is not only for running, but also for learning.

### 49. Why is this project a good example of structured C++ code?

Because it uses clear modules, shared types, and separate responsibilities.

### 50. Why is the overall code structure important?

It helps the whole project stay organized, understandable, and easier to improve.
