#pragma once
#include <SDL.h>
#include <SDL_ttf.h>
#include <string>
#include <vector>
#include <functional>
#include <iostream>
#include <algorithm>
#include "constants.h"

struct Theme {
	SDL_Color Base_Color;
	SDL_Color Text_Color;

	SDL_Color Button_FillColor;
	SDL_Color Button_BorderColor;

	SDL_Color Dropdown_FillColor;
	SDL_Color Dropdown_BorderColor;
	SDL_Color Dropdown_OptionFillColor;
	SDL_Color Dropdown_OptionBorderColor;

	SDL_Color SliderBase_FillColor;
	SDL_Color SliderBase_BorderColor;
	SDL_Color SliderHandle_FillColor;
	SDL_Color SliderHandle_BorderColor;

	TTF_Font* HeadingFont;
	TTF_Font* DefaultFont;
};

class UiManager {
	public:
		void Init(SDL_Renderer* renderer_, Theme& theme);

		void HandleUiEvents(SDL_Event& event);
		void Update();
		void Render();

		void SetRenderDrawColor(SDL_Renderer* renderer, SDL_Color color);

		void AddColorPicker(const std::string& label, int x, int y, int boxW, int boxH, SDL_Color* targetColor);

		void AddDropdown(const std::string& label, int x, int y, int w, int h, const std::vector<std::string>& options, std::function<void(int)> onSelectAction, int DefaultOption);
		void AddDropdown(const std::string& label, int x, int y, int w, int h, const std::vector<std::string>& options, std::function<void(int)> onSelectAction);

		void AddButton(const std::string& label, int x, int y, int w, int h, std::function<void()> onClickAction);

		void AddText(const std::string& label, int x, int y);
		void AddText(const std::string& label, int x, int y, bool Heading);
		void AddText(const std::string& label, int x, int y, SDL_Color TextColor, bool Heading);

		void AddText(const std::function<std::string()>& getLabelFunc, int x, int y);
		void AddText(const std::function<std::string()>& getLabelFunc, int x, int y, bool Heading);
		void AddText(const std::function<std::string()>& getLabelFunc, int x, int y, SDL_Color TextColor, bool Heading);

		void AddColoredBox(int x, int y, int w, int h, SDL_Color color);
		void AddColoredBox(int x, int y, int w, int h, const std::function<SDL_Color()>& getColorFunc);

		void AddSlider(int x, int y, int w, int h, int minVal, int maxVal, int* targetVal, bool Smooth);
		void AddSlider(int x, int y, int w, int h, int minVal, int maxVal, int* targetVal);

	private:
		struct Text {
			int x, y;
			std::string label;
			SDL_Color TextColor;
			std::function<std::string()> getLabel = {};

			bool Heading = false;
			bool Dynamic = false;

			SDL_Texture* cachedTexture = nullptr;
			int width, height = 0;
			bool dirty = true; //true if a re render is needed.
		};

		struct Button {
			SDL_Rect rect;
			std::string label;
			std::function<void()> OnClick;

			Text textObj;
		};

		struct DropDown {
			SDL_Rect rect;
			std::string label;
			std::vector <std::string> options;
			bool isOpen;
			std::function<void(int)> onSelect; 
			int selectedIndex;
		};

		struct ColorPicker {
			SDL_Rect ColorBox_Rect;   // Sat/Brightness box
			SDL_Rect HueSlider_Rect; // Hue slider
			SDL_Color* targetColor; //Chosen color

			float hue = 0.0f;   // 0 - 360
			float saturation = 1.0f; // 0 - 1
			float brightness = 1.0f; // 0 - 1

			bool pickingBox = false;
			bool pickingSlider = false;

			SDL_Texture* ColorBox_Texture = nullptr;
			SDL_Texture* HueSlider_Texture = nullptr;

			bool HueSlider_dirty = true;
			bool ColorBox_dirty = true;
		};

		struct ColoredBox {
			SDL_Rect Box_rect;
			SDL_Color BoxColor;

			std::function<SDL_Color()> getColor = {};
		};

		struct Slider {
			SDL_Rect BaseRect;
			SDL_Rect HandleRect;

			int minValue;
			int maxValue;
			int* valuePtr;
			bool isDragging = false;
			bool SmoothDrag = false;
		};

		std::vector<Slider> sliders;
		std::vector<ColorPicker> colorPickers;
		std::vector<ColoredBox> boxes;
		std::vector<DropDown> dropdowns;
		std::vector<Button> buttons;
		std::vector<Text> texts;

		TTF_Font* DefaultFont = nullptr;
		TTF_Font* HeadingFont = nullptr;

		SDL_Renderer* renderer = nullptr;
		
		SDL_Color DefaultColor;

		void InitializeColorPicker(ColorPicker& picker);
		void UpdateColorPickerTexture(SDL_Renderer* renderer, ColorPicker& cPicker);
		void RenderText(const std::string& text, int x, int y, SDL_Color color);
		void CreateTextTexture(Text& text);

		SDL_Texture* CreateHueSliderTexture(SDL_Renderer* renderer, ColorPicker cPicker);

		//Ui Handlers
		void Handle_DropdownUI(SDL_Event& event, SDL_Point& mousePoint);
		void Handle_ButtonUI(SDL_Event& event, SDL_Point& mousePoint);
		void Handle_ColorPickerUI(SDL_Event& event, SDL_Point& mousePoint);
		void Handle_SliderUI(SDL_Event& event, SDL_Point& mousePoint);

		//Render Methods

		void Render_BoxUIs();
		void Render_SliderUIs();
		void Render_ButtonUIs();
		void Render_TextUIs();
		void Render_ColorPickerUIs();
		void Render_DropdownUIs();
};