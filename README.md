# SDL Raycasting Demo

A lightweight 2D raycasting engine written in C++ using SDL3.  
This project demonstrates how raycasting can be used to simulate a first-person perspective from a top-down map by casting rays in real-time to detect walls and render a pseudo-3D scene.

---

## Screenshot

![image](https://github.com/user-attachments/assets/bf2c8366-6bdf-4eb3-b3df-fce77afc6816)

---

## Overview

This project implements a basic, real-time raycasting renderer similar to early games like **Wolfenstein 3D**.  
Rays are cast from the player's position across a configurable **field of view (FOV)**. Intersections between rays and map-defined walls are computed, and a simple projection transforms these distances into vertical wall slices on the screen. Wall shading is applied based on distance to simulate lighting and depth.

The environment is defined by a simple text-based map file, with characters representing walls and the player’s start position.

---

## Features

- Real-time player movement and view rotation
- Adjustable **FOV**, **ray count (density)**, and **wall height** at runtime via hotkeys
- Text-based map file loading (`map.txt`)
- Distance-based shading for depth simulation
- 2D top-down view rendering for debugging and map layout
- Clean and minimal C++20 / SDL3 implementation

---

## Map File Format

The map is stored in a plain text file (`map.txt`) where:

- `#` — represents a square wall cell  
- `p` — defines the player's start position  
- Spaces (` `) are empty areas

Each wall cell is converted to four line segments forming a square.

### Example `map.txt`:

![image](https://github.com/user-attachments/assets/93f26cab-b175-4964-b262-c2452a085c41)


---

## Controls

| Key               | Action                                    |
|:------------------|:-------------------------------------------|
| **W / A / S / D** | Move player (forward, left, back, right)   |
| **Q / E**         | Rotate view left / right                   |
| **F**             | Increase FOV (`Shift` + `F` decreases)     |
| **V**             | Increase ray count (`Shift` + `V` decreases)|
| **H**             | Increase wall height (`Shift` + `H` decreases) |
| **Left Shift**    | Hold to move faster or reverse hotkey actions |
| **ESC / Close window** | Quit |

---

Have fun playing this exciting game

<sub><sup><sup>Note: Parts of this README were assisted by AI-generated content for formatting and description clarity</sup></sub></sup>


