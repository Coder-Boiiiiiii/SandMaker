#define SDL_MAIN_HANDLED
#define NUM_MATERIALS 6

// C++ Standard Libraries
#include <iostream>
#include <vector>
#include <algorithm>
#include <string>
#include <thread>

// Third Party
#include <SDL.h>
#include <SDL_ttf.h>

// Other scripts
#include "simulation.h"
#include "constants.h"
#include "UiManager.h"

#pragma region Global Variables

//Frame counts and FPS cap
int frameCount = 0;
int fps = 0;
int frameTime;

Uint32 fpsLastTime = SDL_GetTicks();

Uint32 lastTime = SDL_GetTicks();
Uint32 frameStart;

//deltatime stuff
float accumulator = 0.0f;
Uint64 now = SDL_GetPerformanceCounter();
Uint64 last = now;

//UI stuff
UiManager _UiManager;
Theme theme;

bool PickerOpen = false;
bool LmbHeld = false;

int selectionvalue = 1;

#pragma endregion

#pragma region Color Picker Window

void ColorPickerUI(UiManager &_UM, SDL_Color &OG_Col) {
    SDL_Color& colorRef = Get_Curr_Color();
    OG_Col = colorRef;

    _UM.AddColorPicker("Material Color", 25, 25, CELL_SIZE * 50, CELL_SIZE * 50, &colorRef);
    _UM.AddButton("Set Color", 25, (CELL_SIZE * 52) + 25, CELL_SIZE * 50, CELL_SIZE * 7, [&colorRef]() {Set_Curr_Color(colorRef);  PickerOpen = false; });
    _UM.AddButton("Cancel", 25, (CELL_SIZE * 62) + 25, CELL_SIZE * 50, CELL_SIZE * 7, [OG_Col]() {Set_Curr_Color(OG_Col);  PickerOpen = false; });
}

void ColorPickerWindow() {
    UiManager _colorPickerUI;
    SDL_Color OG_Col;

    SDL_Window* colorPickerWindow = SDL_CreateWindow(
        "Set Material Color",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        325, 400,
        SDL_WINDOW_ALWAYS_ON_TOP | SDL_WINDOW_SHOWN
    );

    PickerOpen = true;

    SDL_Renderer* colorPickerRenderer = SDL_CreateRenderer(
        colorPickerWindow, -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
    );

    _colorPickerUI.Init(colorPickerRenderer, theme);
    ColorPickerUI(_colorPickerUI, OG_Col);

    while (PickerOpen) {
        SDL_Event ev;
        LmbHeld = false;

        while (SDL_PollEvent(&ev)) {
            if (ev.type == SDL_QUIT) {
                Set_Curr_Color(OG_Col);
                PickerOpen = false;
            }

            if (ev.type == SDL_WINDOWEVENT && ev.window.event == SDL_WINDOWEVENT_CLOSE) {
                if (ev.window.windowID == SDL_GetWindowID(colorPickerWindow)) {
                    Set_Curr_Color(OG_Col);
                    PickerOpen = false;
                }
            }

            _colorPickerUI.HandleUiEvents(ev);
        }

        _colorPickerUI.SetRenderDrawColor(colorPickerRenderer, theme.Base_Color);
        SDL_RenderClear(colorPickerRenderer);

        _colorPickerUI.Render();

        SDL_RenderPresent(colorPickerRenderer);
    }


    //LmbHeld = false;

    SDL_DestroyWindow(colorPickerWindow);
    SDL_DestroyRenderer(colorPickerRenderer);
}

#pragma endregion

#pragma region Set Up Functions

void InitializeTheme(TTF_Font* DefaultFont, TTF_Font* HeadingFont) {
    theme.Base_Color = { 10, 10, 10, 255 };

    theme.Button_BorderColor = { 255, 255, 255, 255 };
    theme.Button_FillColor = { 25, 25, 25, 255 };

    theme.Dropdown_BorderColor = theme.Button_BorderColor;
    theme.Dropdown_FillColor = theme.Button_FillColor;

    theme.Dropdown_OptionBorderColor = { 200, 200, 200, 255 };
    theme.Dropdown_OptionFillColor = {15, 15, 15, 255};

    theme.SliderBase_FillColor = theme.Dropdown_OptionFillColor;
    theme.SliderBase_BorderColor = theme.Button_BorderColor;

    theme.SliderHandle_FillColor = theme.Button_FillColor;
    theme.SliderHandle_BorderColor = theme.Button_BorderColor;

    theme.Text_Color = { 255, 255, 255, 255 };

    theme.DefaultFont = DefaultFont;
    theme.HeadingFont = HeadingFont;
}

void SetUpUI() {
    //Canvas texts
    _UiManager.AddText([&]() {
        return "FPS: " + std::to_string(fps);
        }, CELL_SIZE * 3, CELL_SIZE * 3);

    //Sidebar
    _UiManager.AddText("Particle Settings", CELL_SIZE * 156.5, CELL_SIZE * 3, true);
    _UiManager.AddText("Material Settings", CELL_SIZE * 155, CELL_SIZE * 14);

    _UiManager.AddDropdown("Current Material", CELL_SIZE * 155, CELL_SIZE * 21, CELL_SIZE * 50, CELL_SIZE * 7, GetAddableMaterials(), ([=](int selectedIndex) {
        std::cout << "Selected: " << selectedIndex << "\n";
        Switch_Material(selectedIndex);
    }));

    _UiManager.AddText("Material Color: ", CELL_SIZE * 155, CELL_SIZE * 32);

    _UiManager.AddButton("Change", CELL_SIZE * 196.5, CELL_SIZE * 32, CELL_SIZE * 18, CELL_SIZE * 7, [] {
            std::thread pickerThread([]() {
                ColorPickerWindow();
                });
            pickerThread.detach();
        });

    _UiManager.AddColoredBox(CELL_SIZE * 186, CELL_SIZE * 32, CELL_SIZE * 8, CELL_SIZE * 7, [&]() {
        return Get_Curr_Color();
        });

    _UiManager.AddText([&]() {
        return "Brush Size: " + std::to_string(selectionvalue);
        }, CELL_SIZE * 155, CELL_SIZE * 42);

    _UiManager.AddSlider(CELL_SIZE * 155, CELL_SIZE * 50, CELL_SIZE * 50, CELL_SIZE * 5, 1, 8, &selectionvalue);
}

#pragma endregion

int main(int argc, char* argv[]) {
    SDL_SetMainReady();

    //Initialize Grid
    Cell Grid[GRID_LENGTH][GRID_WIDTH] = {CellState::EMPTY};

    InitializeSim(Grid);

#pragma region Initialize Window

    SDL_Window* window = nullptr;

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cout << "SDL could not be initialized: " <<
            SDL_GetError();
    }
    else {
        std::cout << "SDL video system is ready to go\n";
    }

    window = SDL_CreateWindow("Sand Maker V1",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH, WINDOW_HEIGHT,
        SDL_WINDOW_SHOWN);

    SDL_Renderer* renderer = nullptr;
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    if (!renderer) {
        SDL_DestroyWindow(window);
        std::cout << "SDL Renderer Error: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    if (TTF_Init() == -1) {
        std::cout << "TTF_Init Error: " << TTF_GetError() << std::endl;
    }

    TTF_Font* DefaultFont = TTF_OpenFont("Assets/Ithaca-LVB75.ttf", 30);
    TTF_Font* HeadingFont = TTF_OpenFont("Assets/Ithaca-LVB75.ttf", 50);

    if (!DefaultFont || !HeadingFont) {
        std::cout << "Font Load Error: " << TTF_GetError() << std::endl;
    }

    InitializeTheme(DefaultFont, HeadingFont);

    _UiManager.Init(renderer, theme);

#pragma endregion

    SetUpUI();

    // Infinite loop for application
    bool gameIsRunning = true;

    while (gameIsRunning) {
        now = SDL_GetPerformanceCounter();
        float deltaTime = (float)(now - last) / SDL_GetPerformanceFrequency();
        last = now;

        accumulator += deltaTime;

        frameStart = SDL_GetTicks();
        frameCount++;

        Uint32 currentTime = SDL_GetTicks();
        if (currentTime - fpsLastTime >= 1000) {
            fps = frameCount; // frames per last second
            frameCount = 0;
            fpsLastTime = currentTime;
        }

        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            // Handle each specific event
            if (event.type == SDL_QUIT) {
                gameIsRunning = false; //Stop game
            }

            //Logic to spawn particles on button hold
            if (event.button.button == SDL_BUTTON_LEFT) {
                if (event.type == SDL_MOUSEBUTTONDOWN) {
                    LmbHeld = true;
                }

                if (event.type == SDL_MOUSEBUTTONUP) {
                    LmbHeld = false;
                }
            }

            HandleSimulationEvents(event, Grid);
            _UiManager.HandleUiEvents(event);
        }

        while (accumulator >= FIXED_TIMESTEP) {
            UpdateGrid(Grid, LmbHeld);
            accumulator -= FIXED_TIMESTEP;
        }

        SetBrushSize(selectionvalue);

        RenderGrid(renderer, Grid);

        _UiManager.Render();

        SDL_RenderPresent(renderer);

        frameTime = SDL_GetTicks() - frameStart;
        if (frameTime < FRAME_DELAY) {
            SDL_Delay(FRAME_DELAY - frameTime);
        }

    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    TTF_CloseFont(DefaultFont);
    TTF_CloseFont(HeadingFont);
    TTF_Quit();

    SDL_Quit();
    return 0;
}