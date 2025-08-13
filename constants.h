#pragma once
//Constants

const int GRID_LENGTH = 150;
const int GRID_WIDTH = 150;
const int CELL_SIZE = 5;
const int SELECTION_SIZE = 1;

const int HORIZONTAL_PADDING = CELL_SIZE * 70;
const int VERTICAL_PADDING = 0;

const int WINDOW_WIDTH = HORIZONTAL_PADDING + (CELL_SIZE * GRID_WIDTH);
const int WINDOW_HEIGHT = VERTICAL_PADDING + (CELL_SIZE * GRID_LENGTH);

const int FPS_CAP = 60;
const int FRAME_DELAY = 1000 / FPS_CAP;
const float FIXED_TIMESTEP = 1.0f / FPS_CAP;