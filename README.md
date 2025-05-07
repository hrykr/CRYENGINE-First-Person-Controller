# CRYENGINE First Person Controller (C++)
### Keyboard + Mouse / Gamepad

A simple first-person controller built using C++ in CRYENGINE, based on the official tutorial series by Crytek.

## 📹 Tutorial Series

This project follows the official CRYENGINE C++ tutorials:

1. [Part 1 – First Person C++ Character Controller](https://youtu.be/63PuJoVoQMg?si=H2Jtb9pEk49eeK4y)
2. [Part 2 – Walk and Sprinting](https://youtu.be/xWExyPNIW30?si=uR4ELFGu2UcV9TUz)
3. [Part 3 – Jumping](https://youtu.be/85hiBjsGClI?si=xVMV4vruO1vMsk8w)
4. [Part 4 – Crouching](https://youtu.be/H8U8UU0Y1m8?si=_Ule5kMgZ6vlZuCz)

---

## 📁 Setup Instructions

1. Clone or download this repository.
2. Place `Player.cpp` and `Player.h` into your project's `Code/Component/` folder.
3. Place `GamePlugin.cpp` and `GamePlugin.h` into your project's `Code/` folder.
4. Right click on `Game.cryproject` -> `Generate solution` to create solution.
5. Build solution via Visual Studio.
6. In CRYENGINE Sandbox:
   - Create an entity.
   - Add the **CPlayerComponent** to it.
   - Setup values.

---

## 🧠 Features

- Basical Movement (`WASD` / `Left Stick`)
- Mouse look (`Mouse` / `Right Stick`)
- Sprinting (`Shift` / `Left Stick Hold`)
- Jumping (`Space` / `🅐`)
- Crouching (`C` / `🅑`)

---

## 🔧 Requirements

- CRYENGINE 5.7 LTS (or compatible version)
- Visual Studio 2017 or higher
- Windows 10 or later
- Basic C++ knowledge
