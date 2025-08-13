#pragma once
// C++ Standard Libraries
#include <iostream>
#include <vector>
#include <algorithm>
#include <string>

// Third Party
#include <SDL.h>
#include <SDL_ttf.h>

#include "constants.h"

enum class CellState {
    EMPTY = 0,
    SAND,
    ROCK,
    BEDROCK,
    WATER,
    ACID
};

struct Cell {
    CellState state; //Cell state, sand, water, etc.
    uint8_t wetness = 0; //Wetness value, 0 = dry, 100 = fully wet.
    uint8_t comboTimer = 0; //Combo timers per cell for delays.
};

void InitializeSim(Cell Grid[GRID_LENGTH][GRID_WIDTH]);
void UpdateGrid(Cell(&Grid)[GRID_LENGTH][GRID_WIDTH], bool& LmbHeld);
void RenderGrid(SDL_Renderer* renderer, Cell(&Grid)[GRID_LENGTH][GRID_WIDTH]);
void SetBrushSize(int&);

//UI Function
void Switch_Material();
void Switch_Material(int);
void Set_Curr_Color(SDL_Color Color);
SDL_Color& Get_Curr_Color();

void HandleSimulationEvents(SDL_Event& event, Cell (&Grid)[GRID_LENGTH][GRID_WIDTH]);

std::string GetCurrentMaterial();
std::vector<std::string> GetAddableMaterials();