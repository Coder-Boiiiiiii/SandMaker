#define SDL_MAIN_HANDLED
#define NUM_MATERIALS 6

// C++ Standard Libraries
#include <iostream>
#include <vector>
#include <algorithm>
#include <string>

// Third Party
#include <SDL.h>
#include <SDL_ttf.h>

#include "constants.h"

#pragma region Structs & Enums

enum class CellState {
    EMPTY = 0,
    SAND,
    ROCK,
    BEDROCK,
    WATER,
    ACID
};

struct Material {
    SDL_Color color[NUM_MATERIALS]; //Dry color of particle
    SDL_Color WetColor[NUM_MATERIALS]; //Wet color of the particle
    bool SubmersableInLiquid[NUM_MATERIALS]; //Is the particle submersible in lquid or does it just fall in air?
    bool Liquid[NUM_MATERIALS]; //Is particle a liquid? (if not, then solid)
    int Density[NUM_MATERIALS]; //Density of material
    int ReactionDelay[NUM_MATERIALS]; //Combo delays (frame delay)
    int FallSpeed[NUM_MATERIALS]; //Gravity for material
};

struct Cell {
    CellState state; //Cell state, sand, water, etc.
    uint8_t wetness = 0; //Wetness value, 0 = dry, 100 = fully wet.
    uint8_t comboTimer = 0; //Combo timers per cell for delays.
};

#pragma endregion

#pragma region Script Variables
Material materials;
CellState AddableMaterials[] = { CellState::SAND, CellState::WATER, CellState::ROCK, CellState::ACID };

int CurrMaterialIndex = 0; //Initial material is sand

CellState ComboTable[NUM_MATERIALS][NUM_MATERIALS];

int selection_size = SELECTION_SIZE;
#pragma endregion

#pragma region Helper Functions

template <typename T>

T clamp(T val, T min, T max) {
    if (val > max) return max;
    if (val < min) return min;
    return val;
}

#pragma endregion

#pragma region Initializations
CellState ChooseRandomState(CellState State1, CellState State2) {
    if (rand() % 2) return State1;
    else return State2;
}

void UpdateComboTable() {
    ComboTable[(int)CellState::SAND][(int)CellState::ACID] = ChooseRandomState(CellState::ACID, CellState::EMPTY);
    ComboTable[(int)CellState::WATER][(int)CellState::ACID] = ChooseRandomState(CellState::ACID, CellState::EMPTY);
    ComboTable[(int)CellState::ROCK][(int)CellState::ACID] = ChooseRandomState(CellState::ACID, CellState::EMPTY);
}


void InitComboTable() {
    for (int i = 0; i < NUM_MATERIALS; i++) {
        for (int j = 0; j < NUM_MATERIALS; j++) {
            ComboTable[i][j] = (CellState)i;
        }
    }

    UpdateComboTable();
}

void InitializeColors() {
    //Initialize Colors
    materials.color[static_cast<int>(CellState::EMPTY)] = { 13, 13, 13, 255 };
    materials.color[static_cast<int>(CellState::SAND)] = { 226, 202, 118, 128 };
    materials.color[static_cast<int>(CellState::ROCK)] = { 33, 33, 33, 255 };
    materials.color[(int)CellState::BEDROCK] = { 10, 10, 10, 255 };

    materials.color[static_cast<int>(CellState::WATER)] = { 0, 84, 119, 255 };
    materials.color[static_cast<int>(CellState::ACID)] = { 176, 191, 26, 255 };

    //Initialize Wet Colors
    materials.WetColor[static_cast<int>(CellState::EMPTY)] = { 13, 13, 13, 255 };
    materials.WetColor[static_cast<int>(CellState::SAND)] = { 145, 129, 73, 255 };
    materials.WetColor[static_cast<int>(CellState::ROCK)] = { 15, 15, 15, 255 };
    materials.WetColor[(int)CellState::BEDROCK] = { 10, 10, 10, 255 };

    materials.WetColor[static_cast<int>(CellState::WATER)] = { 0, 84, 119, 255 };
    materials.WetColor[static_cast<int>(CellState::ACID)] = { 176, 191, 26, 255 };
}

void InitializeMaterials() {
    //Separate function in case we want to reset random colors.
    InitializeColors();

    //Initialize Submersable
    materials.SubmersableInLiquid[static_cast<int>(CellState::EMPTY)] = false;
    materials.SubmersableInLiquid[static_cast<int>(CellState::SAND)] = true;
    materials.SubmersableInLiquid[static_cast<int>(CellState::ROCK)] = false;
    materials.SubmersableInLiquid[(int)CellState::BEDROCK] = false;

    materials.SubmersableInLiquid[static_cast<int>(CellState::WATER)] = true;
    materials.SubmersableInLiquid[static_cast<int>(CellState::ACID)] = true;

    //Initialize State (Default is Solid)
    materials.Liquid[static_cast<int>(CellState::EMPTY)] = false;
    materials.Liquid[static_cast<int>(CellState::SAND)] = false;
    materials.Liquid[static_cast<int>(CellState::ROCK)] = false;
    materials.Liquid[(int)CellState::BEDROCK] = false;

    materials.Liquid[static_cast<int>(CellState::WATER)] = true;
    materials.Liquid[static_cast<int>(CellState::ACID)] = true;

    //Initialize Densities
    materials.Density[static_cast<int>(CellState::EMPTY)] = 0;
    materials.Density[static_cast<int>(CellState::SAND)] = 5;
    materials.Density[static_cast<int>(CellState::ROCK)] = 10;
    materials.Density[(int)CellState::BEDROCK] = 500;

    materials.Density[static_cast<int>(CellState::WATER)] = 3;
    materials.Density[static_cast<int>(CellState::ACID)] = 4;

    //Initialize Combo Delays
    materials.ReactionDelay[static_cast<int>(CellState::EMPTY)] = 0;
    materials.ReactionDelay[static_cast<int>(CellState::SAND)] = 5;
    materials.ReactionDelay[static_cast<int>(CellState::ROCK)] = 150;
    materials.ReactionDelay[(int)CellState::BEDROCK] = 500;

    materials.ReactionDelay[static_cast<int>(CellState::WATER)] = 5;
    materials.ReactionDelay[static_cast<int>(CellState::ACID)] = 5;

    //Initialize Gravities
    materials.FallSpeed[static_cast<int>(CellState::EMPTY)] = 0;
    materials.FallSpeed[static_cast<int>(CellState::SAND)] = 1;
    materials.FallSpeed[static_cast<int>(CellState::ROCK)] = 0;
    materials.FallSpeed[(int)CellState::BEDROCK] = 0;

    materials.FallSpeed[static_cast<int>(CellState::WATER)] = 1;
    materials.FallSpeed[static_cast<int>(CellState::ACID)] = 1;
}

void InitializeGrid(Cell Grid[GRID_LENGTH][GRID_WIDTH]) {
    for (int y = 0; y < GRID_LENGTH; y++)
    {
        for (int x = 0; x < GRID_WIDTH; x++)
        {
            Cell CurrCell = Grid[y][x];

            if (x == (GRID_WIDTH - 1) || x == 0 || y == (GRID_LENGTH - 1) || y == 0) {
                CurrCell.state = CellState::BEDROCK;
                CurrCell.comboTimer = materials.ReactionDelay[static_cast<int>(CellState::BEDROCK)];
            }

            else {
                CurrCell.state = CellState::EMPTY;
                CurrCell.comboTimer = materials.ReactionDelay[static_cast<int>(CellState::EMPTY)];
            }

            CurrCell.wetness = 0;

            Grid[y][x] = CurrCell;
        }
    }
}

#pragma endregion

#pragma region Debug Methods

const std::string CellStateToString(CellState state) {
    switch (state) {
    case CellState::EMPTY: return "EMPTY";
    case CellState::SAND: return "SAND";
    case CellState::ROCK: return "ROCK";
    case CellState::WATER: return "WATER";
    case CellState::ACID: return "ACID";
    default: return "UNKNOWN";
    }
}

void CurrentMaterialCheck() {
    std::cout << "Current Material Index: " << CurrMaterialIndex << "\n";
    std::cout << "Current Mateirla: " << CellStateToString(AddableMaterials[CurrMaterialIndex]) << "\n";
}

void ComboLookupTableString(CellState ComboTable[NUM_MATERIALS][NUM_MATERIALS]) {
    for (int y = 0; y < NUM_MATERIALS; y++) {
        for (int x = 0; x < NUM_MATERIALS; x++) {
            std::cout << "(" << CellStateToString(static_cast<CellState>(x)) << ", " << CellStateToString(static_cast<CellState>(y)) << ") : "
                << CellStateToString(ComboTable[y][x]) << "\n";
        }
    }
}

void OutputContacts(Cell& CurrCell, Cell& OtherCell) {
    if (OtherCell.state != CurrCell.state) {
        std::cout << CellStateToString(CurrCell.state) << " Contacted " << CellStateToString(OtherCell.state) << "\n";
        std::cout << "Current Cell's Combo Timer: " << (int)CurrCell.comboTimer << "\n";
        std::cout << "Current State's Delay: " << materials.ReactionDelay[static_cast<int>(CurrCell.state)] << "\n\n";
    }
}

#pragma endregion

#pragma region Grid Operations

SDL_Color LerpColor(const SDL_Color& a, const SDL_Color& b, float t) { //where t is the wetness/100
    SDL_Color result;

    result.r = a.r + (b.r - a.r) * t;
    result.g = a.g + (b.g - a.g) * t;
    result.b = a.b + (b.b - a.b) * t;
    result.a = a.a + (b.a - a.a) * t;;

    return result;
}

SDL_Color Darken_Color(SDL_Color Color, Uint8 DarknessFac) {
    return {
        static_cast<Uint8>(std::max(0, Color.r - DarknessFac)),
        static_cast<Uint8>(std::max(0, Color.g - DarknessFac)),
        static_cast<Uint8>(std::max(0, Color.b - DarknessFac)),
        Color.a
    };
}

//Check if particaly actually has a combo
bool CanChangeState(Cell& CurrCell, Cell& OtherCell) {

    //Wetness from direct contact with liquid
    if (materials.Liquid[(int)OtherCell.state]) {
        CurrCell.wetness = std::min(CurrCell.wetness + 5, 100);
    }

    //Wetness spread from a wet neighbor
    if (OtherCell.wetness > 80) {
        CurrCell.wetness = std::min(CurrCell.wetness + 1, 100);

        //Rare drying of neighbor
        if (rand() % 10 == 0) {
            OtherCell.wetness = std::max(OtherCell.wetness - 2, 0);
        }
    }

    //Gradual drying if not near liquid
    if (!materials.Liquid[(int)OtherCell.state] && !materials.Liquid[(int)CurrCell.state]) {
        if (rand() % 300 == 0) { // slow drying
            CurrCell.wetness = std::max(CurrCell.wetness - 1, 0);
        }
    }

    if (CurrCell.comboTimer > 0 || OtherCell.comboTimer > 0) return false;

    if (CurrCell.comboTimer <= 0 && CurrCell.state != CellState::EMPTY && OtherCell.state != CellState::EMPTY && CurrCell.state != OtherCell.state) {
        CellState oldState = CurrCell.state;

        CurrCell.state = ComboTable[(int)CurrCell.state][(int)OtherCell.state];

        CurrCell.comboTimer = materials.ReactionDelay[(int)oldState];
        OtherCell.comboTimer = materials.ReactionDelay[(int)CurrCell.state];

        return true;
    }


    return false;
}

//Check particle neighbors for combinations
void CheckForCombos(Cell(&Grid)[GRID_LENGTH][GRID_WIDTH], int Curr_y, int Curr_x) {
    Cell& CurrCell = Grid[Curr_y][Curr_x];

    if (CurrCell.comboTimer > 0) {
        return;
    }

    Cell& UpperCell = Grid[Curr_y - 1][Curr_x];
    Cell& LowerCell = Grid[Curr_y + 1][Curr_x];
    Cell& RightCell = Grid[Curr_y][Curr_x + 1];
    Cell& LeftCell = Grid[Curr_y][Curr_x - 1];

    if (CanChangeState(CurrCell, UpperCell)) {
        return;
    }

    else if (CanChangeState(CurrCell, LowerCell)) {
        return;
    }

    else if (CanChangeState(CurrCell, RightCell)) {
        return;
    }

    else if (CanChangeState(CurrCell, LeftCell)) {
        return;
    }
}

//Try moving the particle
bool TryMove(Cell& CurrCell, Cell& OtherCell) {
    bool& IsCurrSubmersibleInLiquids = materials.SubmersableInLiquid[static_cast<int>(CurrCell.state)];
    bool& isOtherLiquid = materials.Liquid[static_cast<int>(OtherCell.state)];

    int& currDensity = materials.Density[(int)CurrCell.state];
    int& otherDensity = materials.Density[(int)OtherCell.state];

    if (OtherCell.state == CellState::EMPTY) {
        OtherCell.state = CurrCell.state;
        CurrCell.state = CellState::EMPTY;

        return true;
    }

    else if ((IsCurrSubmersibleInLiquids && isOtherLiquid) && (currDensity > otherDensity)) {

        if (rand() % 200 == 0) {
            OtherCell.state = CurrCell.state;
            CurrCell.state = CellState::EMPTY;
        }

        else {
            std::swap(CurrCell, OtherCell);
        }

        return true;
    }

    return false;
}

//Update Sand Particle Function
void UpdateSandParticle(Cell(&Grid)[GRID_LENGTH][GRID_WIDTH], int& Curr_y, int& Curr_x) {
    Cell& CurrCell = Grid[Curr_y][Curr_x];

    Cell& RightLowerCell = Grid[Curr_y + 1][Curr_x + 1];
    Cell& LeftLowerCell = Grid[Curr_y + 1][Curr_x - 1];
    Cell& LowerCell = Grid[Curr_y + 1][Curr_x];

    bool CanGoLowRight = true;
    bool CanGoLowLeft = true;

    if (Curr_x == 1) CanGoLowLeft = false;
    if (Curr_x == GRID_WIDTH - 1) CanGoLowRight = false;

    if (LowerCell.state == CellState::EMPTY) {
        if (TryMove(CurrCell, LowerCell)) return;
    }

    else {
        if (rand() % 2) {
            if (CanGoLowRight && TryMove(CurrCell, RightLowerCell)) return;
            if (CanGoLowLeft && TryMove(CurrCell, LeftLowerCell)) return;
        }
        else {
            if (CanGoLowLeft && TryMove(CurrCell, LeftLowerCell)) return;
            if (CanGoLowRight && TryMove(CurrCell, RightLowerCell)) return;
        }

        if (TryMove(CurrCell, LowerCell)) return;
    }
}

//Update Water Particle Function
void UpdateWaterParticle(Cell(&Grid)[GRID_LENGTH][GRID_WIDTH], int& Curr_y, int& Curr_x) {
    Cell& CurrCell = Grid[Curr_y][Curr_x];

    Cell& RightLowerCell = Grid[Curr_y + 1][Curr_x + 1];
    Cell& LeftLowerCell = Grid[Curr_y + 1][Curr_x - 1];
    Cell& LowerCell = Grid[Curr_y + 1][Curr_x];

    Cell& RightCell = Grid[Curr_y][Curr_x + 1];
    Cell& LeftCell = Grid[Curr_y][Curr_x - 1];

    bool CanGoLowRight = true;
    bool CanGoLowLeft = true;

    if (Curr_x == 1) CanGoLowLeft = false;
    if (Curr_x == GRID_WIDTH - 1) CanGoLowRight = false;

    if (LowerCell.state == CellState::EMPTY) {
        if (TryMove(CurrCell, LowerCell)) return;
    }

    else {
        if (rand() % 2) {
            if (CanGoLowRight && TryMove(CurrCell, RightLowerCell)) return;
            if (CanGoLowLeft && TryMove(CurrCell, LeftLowerCell)) return;

            if (TryMove(CurrCell, RightCell)) return;
            if (TryMove(CurrCell, LeftCell)) return;
        }

        else {
            if (CanGoLowLeft && TryMove(CurrCell, LeftLowerCell)) return;
            if (CanGoLowRight && TryMove(CurrCell, RightLowerCell)) return;

            if (TryMove(CurrCell, LeftCell)) return;
            if (TryMove(CurrCell, RightCell)) return;
        }

        if (TryMove(CurrCell, LowerCell)) return;
    }
}

void UpdateParticle(Cell(&Grid)[GRID_LENGTH][GRID_WIDTH], int& Curr_y, int& Curr_x) {
    switch (Grid[Curr_y][Curr_x].state) {
    case CellState::SAND:
        UpdateSandParticle(Grid, Curr_y, Curr_x);
        break;

    case CellState::WATER:
        UpdateWaterParticle(Grid, Curr_y, Curr_x);
        break;

    case CellState::ACID:
        UpdateWaterParticle(Grid, Curr_y, Curr_x);
        break;
    }

    CheckForCombos(Grid, Curr_y, Curr_x);
}

void UpdateComboTimer(Cell& CurrCell) {
    if (CurrCell.comboTimer > 0) {
        CurrCell.comboTimer--;
    }
}

//Update Grid Values
void PaintGrid(Cell(&Grid)[GRID_LENGTH][GRID_WIDTH]) {
    for (int y = GRID_LENGTH - 2; y > 0; y--) {
        if (rand() % 2) {
            for (int x = 1; x < GRID_WIDTH - 1; x++) {
                UpdateComboTimer(Grid[y][x]);
                UpdateParticle(Grid, y, x);
            }
        }

        else {
            for (int x = GRID_WIDTH - 1; x > 1; x--) {
                UpdateComboTimer(Grid[y][x]);
                UpdateParticle(Grid, y, x);
            }
        }
    }
}


#pragma endregion

#pragma region Grid Drawers

//Create cell at location
void CreateCell(SDL_Renderer* renderer, Cell& cell, int row, int col) {
    if (cell.state != CellState::EMPTY) {
        float wetFactor = std::max(0.0f, std::min(cell.wetness / 100.0f, 1.0f));
        SDL_Color color = LerpColor(materials.color[static_cast<int>(cell.state)], materials.WetColor[static_cast<int>(cell.state)], wetFactor);

        SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    }

    else {
        cell.wetness = 0;
        SDL_SetRenderDrawColor(renderer, materials.color[static_cast<int>(cell.state)].r, materials.color[static_cast<int>(cell.state)].g, materials.color[static_cast<int>(cell.state)].b, materials.color[static_cast<int>(cell.state)].a);
    }

    SDL_Rect cellRect{ col * CELL_SIZE, row * CELL_SIZE, CELL_SIZE, CELL_SIZE };
    SDL_RenderFillRect(renderer, &cellRect);
}

void SpawnCell(Cell(&Grid)[GRID_LENGTH][GRID_WIDTH], CellState state) {
    int MouseX, MouseY;
    Uint32 mouseState = SDL_GetMouseState(&MouseX, &MouseY);

    int CellX = MouseX / CELL_SIZE;
    int CellY = MouseY / CELL_SIZE;

    for (int dy = -selection_size; dy <= selection_size; ++dy) {
        for (int dx = -selection_size; dx <= selection_size; ++dx) {
            int x = CellX + dx;
            int y = CellY + dy;

            if (x >= 0 && x < GRID_WIDTH && y >= 0 && y < GRID_LENGTH) {
                if (Grid[y][x].state == CellState::EMPTY) {
                    Grid[y][x].state = state;
                }
            }
        }
    }
}

#pragma region Wrapper Functions (For Bulk Running)

//Update Grid
void UpdateGrid(Cell(&Grid)[GRID_LENGTH][GRID_WIDTH], bool& LmbHeld) {
    if (LmbHeld) {
        SpawnCell(Grid, AddableMaterials[CurrMaterialIndex]);
    }

    PaintGrid(Grid);

    //Randomly shuffle the combos (since a few are randomly decided
    if (rand() % 10 == 0) {
        UpdateComboTable();
    }
}

//Render Grid
void RenderGrid(SDL_Renderer* renderer, Cell(&Grid)[GRID_LENGTH][GRID_WIDTH]) {
    SDL_RenderClear(renderer);

    for (int row = 0; row < GRID_LENGTH; ++row) {
        for (int col = 0; col < GRID_WIDTH; ++col) {
            CreateCell(renderer, Grid[row][col], row, col);
        }
    }
}

//Initialization
void InitializeSim(Cell Grid[GRID_LENGTH][GRID_WIDTH]) {
    InitializeGrid(Grid);
    InitializeMaterials();
    InitComboTable();
}

std::string GetCurrentMaterial() {
    return CellStateToString(AddableMaterials[CurrMaterialIndex]);
}

void Set_Curr_Color(SDL_Color Color) {
    materials.color[(int)AddableMaterials[CurrMaterialIndex]] = Color;
    materials.WetColor[(int)AddableMaterials[CurrMaterialIndex]] = Darken_Color(Color, 30);
}

SDL_Color& Get_Curr_Color() {
    return materials.color[(int)AddableMaterials[CurrMaterialIndex]];
}

std::vector<std::string> GetAddableMaterials() {
    std::vector<std::string> addables;

    for (CellState state : AddableMaterials) {
        addables.push_back(CellStateToString(state));
    }

    return addables;
}

#pragma endregion


#pragma region Debug Methods

//Debug Grid
void ShowGrid(Cell Grid[GRID_LENGTH][GRID_WIDTH]) {

    std::cout << "  ";
    for (int col = 0; col < GRID_WIDTH; ++col) {
        std::cout << col % 10 << " ";
    }
    std::cout << "\n";

    for (int row = 0; row < GRID_LENGTH; ++row) {
        std::cout << row << " ";

        for (int col = 0; col < GRID_WIDTH; ++col) {
            switch (Grid[row][col].state) {
            case CellState::EMPTY: std::cout << "  "; break;
            case CellState::SAND:  std::cout << "S "; break;
            case CellState::ROCK:  std::cout << "R "; break;
            case CellState::WATER: std::cout << "W "; break;
            case CellState::ACID: std::cout << "A "; break;
            case CellState::BEDROCK: std::cout << "B "; break;
            }
        }

        std::cout << "\n";
    }
}

#pragma endregion


#pragma endregion

#pragma region Key Actions

// Switching Materials

void Switch_Material() {
    if (CurrMaterialIndex == sizeof(AddableMaterials) / sizeof(*AddableMaterials) - 1) {
        CurrMaterialIndex = 0;
    }

    else {
        CurrMaterialIndex++;
    }
}

void Switch_Material(int index) {
    CurrMaterialIndex = index;
}

// Colored sand

void Randomize_Color() {
    materials.color[(int)AddableMaterials[CurrMaterialIndex]] = { static_cast<Uint8>(rand() % 256), static_cast<Uint8>(rand() % 256), static_cast<Uint8>(rand() % 256), 255 };
    materials.WetColor[(int)AddableMaterials[CurrMaterialIndex]] = Darken_Color(materials.color[(int)AddableMaterials[CurrMaterialIndex]], 25);
}

void HandleSimulationEvents(SDL_Event& event, Cell(&Grid)[GRID_LENGTH][GRID_WIDTH]) {
    if (event.type == SDL_KEYDOWN) {
        switch (event.key.keysym.sym) {
        case SDLK_BACKSPACE:
            InitializeGrid(Grid); //Reset grid
            break;

        case SDLK_s:
            Switch_Material();
            break;

        case SDLK_c:
            Randomize_Color(); //Randomize current color
            break;
        }
    }
}

void SetBrushSize(int& size) {
    selection_size = size;
}

#pragma endregion