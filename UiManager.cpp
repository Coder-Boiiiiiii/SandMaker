#include "UiManager.h"

#pragma region Global Variables

float lastHue = -1.0f;       // impossible initial value
float lastBrightness = -1.0f;

Theme MainTheme;

#pragma endregion

#pragma region Helper Functions

void UiManager::SetRenderDrawColor(SDL_Renderer* renderer, SDL_Color color) {
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
}

SDL_Color HSVtoRGB(float h, float s, float v) {
    float c = v * s;
    float x = c * (1 - fabs(fmod(h / 60.0, 2) - 1));
    float m = v - c;

    float r, g, b;
    if (h < 60) { r = c; g = x; b = 0; }
    else if (h < 120) { r = x; g = c; b = 0; }
    else if (h < 180) { r = 0; g = c; b = x; }
    else if (h < 240) { r = 0; g = x; b = c; }
    else if (h < 300) { r = x; g = 0; b = c; }
    else { r = c; g = 0; b = x; }

    SDL_Color color;
    color.r = (Uint8)((r + m) * 255);
    color.g = (Uint8)((g + m) * 255);
    color.b = (Uint8)((b + m) * 255);
    color.a = 255;
    return color;
}

void RGBtoHSV(float r, float g, float b, float& h, float& s, float& v) {
    float max = std::max({ r, g, b });
    float min = std::min({ r, g, b });
    v = max;

    float delta = max - min;
    s = (max == 0) ? 0 : delta / max;

    if (delta == 0) {
        h = 0;
    }
    else if (max == r) {
        h = 60 * fmod(((g - b) / delta), 6);
    }
    else if (max == g) {
        h = 60 * (((b - r) / delta) + 2);
    }
    else {
        h = 60 * (((r - g) / delta) + 4);
    }

    if (h < 0) h += 360;
}

template <typename T>
T clamp(T value, T min, T max) {
    if (value > max) return max;
    else if (value < min) return min;
    return value;
}


void UiManager::RenderText(const std::string& text, int x, int y, SDL_Color color) {
    std::string renderLabel = text;
    TTF_Font* font = DefaultFont;

    SDL_Color textColor = color;

    SDL_Surface* surface = TTF_RenderText_Solid(font, renderLabel.c_str(), color);

    if (!surface) {
        std::cerr << "TTF_RenderText_Solid error: " << TTF_GetError() << "\n";
        return;
    }

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);

    if (!texture) {
        std::cerr << "SDL_CreateTextureFromSurface error: " << SDL_GetError() << "\n";
        SDL_FreeSurface(surface);
        return;
    }

    SDL_Rect textRect = { x, y, surface->w, surface->h };

    SDL_RenderCopy(renderer, texture, NULL, &textRect);
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
}

#pragma endregion

#pragma region Color Picker Methods

void UiManager::InitializeColorPicker(ColorPicker& picker) {
    SDL_Color color = *(picker.targetColor);

    float r = color.r / 255.0f;
    float g = color.g / 255.0f;
    float b = color.b / 255.0f;

    float h, s, v;
    RGBtoHSV(r, g, b, h, s, v);

    picker.hue = h;
    picker.saturation = s;
    picker.brightness = v;

    picker.ColorBox_dirty = true;
    picker.HueSlider_dirty = true;
}

SDL_Texture* CreateHueSatTexture(SDL_Renderer* renderer, float hue, float brightness, int w, int h) {
    SDL_Surface* surface = SDL_CreateRGBSurfaceWithFormat(0, w, h, 32, SDL_PIXELFORMAT_RGBA8888);

    Uint32* pixels = (Uint32*)surface->pixels;
    for (int j = 0; j < h; j++) { //y-axis (saturation)
        for (int i = 0; i < w; i++) { //x-axis (brightness)
            float sat = (float)i / w;
            float bright = 1.0f - (float)j / h;

            SDL_Color c = HSVtoRGB(hue, sat, bright);
            pixels[j * w + i] = SDL_MapRGBA(surface->format, c.r, c.g, c.b, 255);
        }
    }

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);

    return texture;
}

void UiManager::UpdateColorPickerTexture(SDL_Renderer* renderer, ColorPicker& cPicker) {
    if (cPicker.ColorBox_dirty) {
        // Values changed ¨ rebuild
        if (cPicker.ColorBox_Texture) SDL_DestroyTexture(cPicker.ColorBox_Texture);

        cPicker.ColorBox_Texture = CreateHueSatTexture(
            renderer,
            cPicker.hue,
            cPicker.brightness,
            cPicker.ColorBox_Rect.w,
            cPicker.ColorBox_Rect.h
        );

        lastHue = cPicker.hue;
        lastBrightness = cPicker.brightness;
    }
}

SDL_Texture* UiManager::CreateHueSliderTexture(SDL_Renderer* renderer, ColorPicker cPicker) {

    SDL_Surface* surface = SDL_CreateRGBSurfaceWithFormat(0, cPicker.HueSlider_Rect.w, cPicker.HueSlider_Rect.h, 32, SDL_PIXELFORMAT_RGBA8888);
    Uint32* pixels = (Uint32*)surface->pixels;

    //Drawing Hue Slider
    for (int j = 0; j < cPicker.HueSlider_Rect.h; j++) {
        float hue = (float)j / cPicker.HueSlider_Rect.h * 360.0f;

        SDL_Color c = HSVtoRGB(hue, 1, 1);
        Uint32 color = SDL_MapRGBA(surface->format, c.r, c.g, c.b, 255);

        for (int i = 0; i < cPicker.HueSlider_Rect.w; i++) {
            pixels[j * cPicker.HueSlider_Rect.w + i] = color;
        }
    }

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    return texture;
}

#pragma endregion

void UiManager::Init(SDL_Renderer* renderer_, Theme& theme) {
    renderer = renderer_;
    
    //Fonts
    DefaultFont = theme.DefaultFont;
    HeadingFont = theme.HeadingFont;
    
    //Default Colors
    DefaultColor = theme.Base_Color;
    MainTheme = theme;
}

#pragma region Color Picker Methods

void UiManager::AddColorPicker(const std::string& label, int x, int y, int boxW, int boxH, SDL_Color* targetColor) {
    ColorPicker cp;
    cp.ColorBox_Rect = { x, y, boxW, boxH };
    cp.HueSlider_Rect = { x + boxW + 10, y, 20, boxH };
    cp.targetColor = targetColor;
    colorPickers.push_back(cp);

    InitializeColorPicker(colorPickers.back());
}

#pragma endregion

#pragma region Colored Box

void UiManager::AddColoredBox(int x, int y, int w, int h, const std::function<SDL_Color()>& getColorFunc) {
    ColoredBox box;
    box.Box_rect = { x,y,w,h };
    box.BoxColor = DefaultColor;
    box.getColor = getColorFunc;
    boxes.push_back(box);
}

void UiManager::AddColoredBox(int x, int y, int w, int h, SDL_Color color) {
    ColoredBox box;
    box.Box_rect = { x,y,w,h };
    box.BoxColor = DefaultColor;
    box.getColor = {};
    boxes.push_back(box);
}

#pragma endregion

#pragma region Dropdown Methods

void UiManager::AddDropdown(const std::string& label, int x, int y, int w, int h, const std::vector<std::string>& options, std::function<void(int)> onSelectAction, int DefaultOption) {
    DropDown dropdown;
    dropdown.rect = { x, y, w, h };
    dropdown.label = label;
    dropdown.options = options;
    dropdown.onSelect = onSelectAction;
    dropdown.isOpen = false;
    dropdown.selectedIndex = DefaultOption;

    dropdowns.push_back(dropdown);
}

void UiManager::AddDropdown(const std::string& label, int x, int y, int w, int h, const std::vector<std::string>& options, std::function<void(int)> onSelectAction) {
    AddDropdown(label, x, y, w, h, options, onSelectAction, 0);
}

#pragma endregion

#pragma region Text Methods

void UiManager::CreateTextTexture(Text& text) {
    if (text.cachedTexture) {
        SDL_DestroyTexture(text.cachedTexture);
        text.cachedTexture = nullptr;
    }

    TTF_Font* font = (text.Heading) ? HeadingFont : DefaultFont;

    SDL_Surface* surface = TTF_RenderText_Solid(font, text.label.c_str(), text.TextColor);

    if (!surface) {
        std::cerr << "TTF_RenderText_Solid error: " << TTF_GetError() << "\n";
        return;
    }

    text.cachedTexture = SDL_CreateTextureFromSurface(renderer, surface);
    text.width = surface->w;
    text.height = surface->h;
    SDL_FreeSurface(surface);
    text.dirty = false;
}

//Dynamic Texts
void UiManager::AddText(const std::function<std::string()>& getLabelFunc, int x, int y, SDL_Color TextColor, bool Heading) {
    Text text;
    text.label = "";
    text.TextColor = TextColor;
    text.x = x;
    text.y = y;
    text.getLabel = getLabelFunc;
    text.Heading = Heading;
    text.Dynamic = true;
    text.dirty = true;

    texts.push_back(text);
}

void UiManager::AddText(const std::function<std::string()>& getLabelFunc, int x, int y, bool Heading) {
    AddText(getLabelFunc, x, y, MainTheme.Text_Color, Heading);
}

void UiManager::AddText(const std::function<std::string()>& getLabelFunc, int x, int y) {
    AddText(getLabelFunc, x, y, MainTheme.Text_Color, false);
}

//Static Texts
void UiManager::AddText(const std::string& label, int x, int y, SDL_Color TextColor, bool Heading) {
    Text text;
    text.label = label;
    text.TextColor = TextColor;
    text.x = x;
    text.y = y;
    text.getLabel = {};
    text.Heading = Heading;
    text.Dynamic = false;
    text.dirty = true;

    CreateTextTexture(text);

    texts.push_back(text);
}

void UiManager::AddText(const std::string& label, int x, int y, bool Heading) {
    AddText(label, x, y, MainTheme.Text_Color, Heading);
}

void UiManager::AddText(const std::string& label, int x, int y) {
    AddText(label, x, y, MainTheme.Text_Color, false);
}

#pragma endregion

#pragma region Button Methods

//button with static text
void UiManager::AddButton(const std::string& label, int x, int y, int w, int h, std::function<void()> onClickAction) {
    Button btn;
    btn.rect = { x, y, w, h };
    btn.label = label;
    btn.OnClick = onClickAction;

    Text text;

    text.label = label;
    text.TextColor = MainTheme.Text_Color;
    text.getLabel = {};

    text.Heading = false;
    text.Dynamic = false;
    text.dirty = true;

    CreateTextTexture(text);

    text.x = x + (w - text.width) / 2;
    text.y = y + (h - text.height) / 2;

    btn.textObj = text;

    buttons.push_back(btn);
}

#pragma endregion

#pragma region Slider Methods

void UiManager::AddSlider(int x, int y, int w, int h, int minVal, int maxVal, int* targetVal, bool Smooth) {
    Slider s;
    s.BaseRect = {x, y + h / 2 - 2, w, 4 };
    s.HandleRect = { x, y, h, h };
    s.minValue = minVal;
    s.maxValue = maxVal;
    s.valuePtr = targetVal;
    s.SmoothDrag = Smooth;

    float t = float(*targetVal - minVal) / float(maxVal - minVal);

    sliders.push_back(s);
}

void UiManager::AddSlider(int x, int y, int w, int h, int minVal, int maxVal, int* targetVal) {
    AddSlider(x, y, w, h, minVal, maxVal, targetVal, false);
}

#pragma endregion

#pragma region Ui Handling Methods

void UiManager::Handle_DropdownUI(SDL_Event& event, SDL_Point& mousePoint) {
    if (event.type == SDL_MOUSEBUTTONDOWN) {
        //Run dropdown logic and stuff
        for (auto& drp : dropdowns) {

            if (SDL_PointInRect(&mousePoint, &drp.rect)) {
                drp.isOpen = !drp.isOpen;
                return;
            }

            if (drp.isOpen) {
                for (size_t i = 0; i < drp.options.size(); i++) {
                    SDL_Rect optionRect = {
                        drp.rect.x,
                        drp.rect.y + drp.rect.h * (int)(i + 1),
                        drp.rect.w,
                        drp.rect.h
                    };

                    if (SDL_PointInRect(&mousePoint, &optionRect)) {
                        drp.selectedIndex = (int)i;
                        drp.isOpen = false;

                        if (drp.onSelect) drp.onSelect((int)i);
                        return;
                    }
                }
            }
        }
    }
}

void UiManager::Handle_ButtonUI(SDL_Event& event, SDL_Point& mousePoint) {
    if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
        //Run button action if the mouse clicks in the button's area
        for (auto& btn : buttons) {
            if (SDL_PointInRect(&mousePoint, &btn.rect)) {
                if (btn.OnClick) btn.OnClick();
                return;
            }
        }
    }
}

void UiManager::Handle_ColorPickerUI(SDL_Event& event, SDL_Point& mousePoint) {
    //Run Color Picker logic and all
    for (auto& cPicker : colorPickers) {
        if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
            if (SDL_PointInRect(&mousePoint, &cPicker.ColorBox_Rect)) {
                cPicker.pickingBox = true;
            }

            if (SDL_PointInRect(&mousePoint, &cPicker.HueSlider_Rect)) {
                cPicker.pickingSlider = true;
            }
        }

        if (event.type == SDL_MOUSEBUTTONUP) {
            cPicker.pickingBox = false;
            cPicker.pickingSlider = false;
        }

        if (event.type == SDL_MOUSEMOTION) {
            int mx = event.motion.x;
            int my = event.motion.y;

            if (cPicker.pickingBox) {
                cPicker.saturation = clamp((float)(mx - cPicker.ColorBox_Rect.x) / cPicker.ColorBox_Rect.w, 0.0f, 1.0f);
                cPicker.brightness = clamp(1.0f - (float)(my - cPicker.ColorBox_Rect.y) / cPicker.ColorBox_Rect.h, 0.0f, 1.0f);
            }

            if (cPicker.pickingSlider) {
                cPicker.hue = clamp(((float)(my - cPicker.HueSlider_Rect.y) / cPicker.HueSlider_Rect.h) * 360.0f, 0.0f, 360.0f);
            }
        }

        // Update target color
        *(cPicker.targetColor) = HSVtoRGB(cPicker.hue, cPicker.saturation, cPicker.brightness);
    }
}

void UiManager::Handle_SliderUI(SDL_Event& event, SDL_Point& mousePoint) {
    if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
        for (auto& slider : sliders) {
            if (SDL_PointInRect(&mousePoint, &slider.HandleRect)) {
                slider.isDragging = true;
            }
        }
    }

    if (event.type == SDL_MOUSEBUTTONUP && event.button.button == SDL_BUTTON_LEFT) {
        for (auto& slider : sliders) {
            slider.isDragging = false;
        }
    }

    if (event.type == SDL_MOUSEMOTION) {
        int mx = event.motion.x;

        for (auto& slider : sliders) {
            if (slider.isDragging) {
                int minX = slider.BaseRect.x;
                int maxX = slider.BaseRect.x + slider.BaseRect.w - slider.HandleRect.w;

                if (slider.SmoothDrag) {
                    slider.HandleRect.x = std::max(minX, std::min(mx - slider.HandleRect.w / 2, maxX));

                    float t = float(slider.HandleRect.x - minX) / float(maxX - minX);
                    *slider.valuePtr = slider.minValue + int(t * (slider.maxValue - slider.minValue));
                }

                else {
                    int steps = slider.maxValue - slider.minValue;
                    float t = float(mx - minX) / float(maxX - minX);
                    int stepIndex = std::round(t * steps);

                    stepIndex = std::max(0, std::min(stepIndex, steps));
                    *slider.valuePtr = slider.minValue + stepIndex;

                    slider.HandleRect.x = minX + (stepIndex * (maxX - minX) / steps);
                }

            }
        }
    }
}

#pragma endregion

#pragma region Element Rendering Methods

void UiManager::Render_BoxUIs() {
    for (auto& box : boxes) {

        if (box.getColor) {
            SetRenderDrawColor(renderer, box.getColor());
            SDL_RenderFillRect(renderer, &box.Box_rect);
        }

        else {
            SetRenderDrawColor(renderer, box.BoxColor);
            SDL_RenderFillRect(renderer, &box.Box_rect);
        }

        SetRenderDrawColor(renderer, MainTheme.Button_BorderColor);
        SDL_RenderDrawRect(renderer, &box.Box_rect);
    }
}

void UiManager::Render_SliderUIs() {
    for (auto& slider : sliders) {
        SetRenderDrawColor(renderer, MainTheme.SliderBase_FillColor);
        SDL_RenderFillRect(renderer, &slider.BaseRect);

        SetRenderDrawColor(renderer, MainTheme.SliderBase_BorderColor);
        SDL_RenderDrawRect(renderer, &slider.BaseRect);

        SetRenderDrawColor(renderer, MainTheme.SliderHandle_FillColor);
        SDL_RenderFillRect(renderer, &slider.HandleRect);

        SetRenderDrawColor(renderer, MainTheme.SliderBase_BorderColor);
        SDL_RenderDrawRect(renderer, &slider.HandleRect);
    }
}

void UiManager::Render_ColorPickerUIs() {
    for (auto& cPicker : colorPickers) {

        //Draw Color Box

        UpdateColorPickerTexture(renderer, cPicker);
        SDL_RenderCopy(renderer, cPicker.ColorBox_Texture, NULL, &cPicker.ColorBox_Rect);

        if (cPicker.hue != lastHue || cPicker.brightness != lastBrightness) cPicker.ColorBox_dirty = true;

        //Draw Hue Slider
        if (cPicker.HueSlider_dirty) {
            if (cPicker.HueSlider_Texture) {
                SDL_DestroyTexture(cPicker.HueSlider_Texture);
            }

            cPicker.HueSlider_Texture = CreateHueSliderTexture(renderer, cPicker);
            cPicker.HueSlider_dirty = false;
        }

        SDL_RenderCopy(renderer, cPicker.HueSlider_Texture, NULL, &cPicker.HueSlider_Rect);

        // Draw selection marker on color box
        int markerX = cPicker.ColorBox_Rect.x + (int)(cPicker.saturation * cPicker.ColorBox_Rect.w);
        int markerY = cPicker.ColorBox_Rect.y + (int)((1 - cPicker.brightness) * cPicker.ColorBox_Rect.h);

        SetRenderDrawColor(renderer, { 0, 0, 0, 255 });
        SDL_Rect marker = { markerX - 2, markerY - 2, 5, 5 };
        SDL_RenderDrawRect(renderer, &marker);

        // Draw selection marker on slider
        int sliderY = cPicker.HueSlider_Rect.y + (int)((cPicker.hue / 360.0f) * cPicker.HueSlider_Rect.h);

        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderDrawLine(renderer, cPicker.HueSlider_Rect.x, sliderY, cPicker.HueSlider_Rect.x + cPicker.HueSlider_Rect.w, sliderY);
    }
}

void UiManager::Render_ButtonUIs() {
    for (auto& btn : buttons) {
        SetRenderDrawColor(renderer, MainTheme.Button_FillColor);
        SDL_RenderFillRect(renderer, &btn.rect);

        SetRenderDrawColor(renderer, MainTheme.Button_BorderColor);
        SDL_RenderDrawRect(renderer, &btn.rect);

        if (DefaultFont) {
            if (btn.textObj.Dynamic) {
                btn.textObj.label = (btn.textObj.getLabel) ? btn.textObj.getLabel() : btn.textObj.label;
                CreateTextTexture(btn.textObj);
            }

            else if (btn.textObj.dirty) {
                CreateTextTexture(btn.textObj);
            }

            if (btn.textObj.cachedTexture) {
                SDL_Rect dst = { btn.textObj.x, btn.textObj.y, btn.textObj.width, btn.textObj.height };
                SDL_RenderCopy(renderer, btn.textObj.cachedTexture, nullptr, &dst);
            }
        }
    }
}

void UiManager::Render_TextUIs() {
    for (auto& txt : texts) {
        if (txt.Dynamic) {
            txt.label = (txt.getLabel) ? txt.getLabel() : txt.label;
            CreateTextTexture(txt);
        }

        else if (txt.dirty) {
            CreateTextTexture(txt);
        }

        if (txt.cachedTexture) {
            SDL_Rect dst = { txt.x, txt.y, txt.width, txt.height };
            SDL_RenderCopy(renderer, txt.cachedTexture, nullptr, &dst);
        }
    }
}

void UiManager::Render_DropdownUIs() {
    for (auto& drp : dropdowns) {
        SetRenderDrawColor(renderer, MainTheme.Dropdown_FillColor);
        SDL_RenderFillRect(renderer, &drp.rect);

        SetRenderDrawColor(renderer, MainTheme.Dropdown_BorderColor);
        SDL_RenderDrawRect(renderer, &drp.rect);

        // Draw selected label
        std::string textToRender = drp.label + ": ";

        if (drp.selectedIndex < drp.options.size()) {
            textToRender += drp.options[drp.selectedIndex];
        }

        RenderText(textToRender, drp.rect.x + 5, drp.rect.y + 5, MainTheme.Text_Color);

        if (drp.isOpen) {
            for (size_t i = 0; i < drp.options.size(); i++) {
                SDL_Rect optionRect = {
                    drp.rect.x,
                    drp.rect.y + drp.rect.h * (int)(i + 1),
                    drp.rect.w,
                    drp.rect.h
                };

                SetRenderDrawColor(renderer, MainTheme.Dropdown_OptionFillColor);
                SDL_RenderFillRect(renderer, &optionRect);

                SetRenderDrawColor(renderer, MainTheme.Dropdown_OptionBorderColor);
                SDL_RenderDrawRect(renderer, &optionRect);

                RenderText(drp.options[i], optionRect.x + 5, optionRect.y + 5, MainTheme.Text_Color);
            }
        }
    }
}

#pragma endregion

#pragma region Main Methods

void UiManager::HandleUiEvents(SDL_Event& event) {
    int mouseX = event.button.x;
    int mouseY = event.button.y;

    SDL_Point mousePoint = { mouseX, mouseY };

    Handle_DropdownUI(event, mousePoint);
    Handle_ButtonUI(event, mousePoint);
    Handle_ColorPickerUI(event, mousePoint);
    Handle_SliderUI(event, mousePoint);
}

void UiManager::Render() {
    SetRenderDrawColor(renderer, DefaultColor);

    Render_BoxUIs();
    Render_SliderUIs();
    Render_ColorPickerUIs();
    Render_ButtonUIs();    
    Render_TextUIs();    
    Render_DropdownUIs();

    SetRenderDrawColor(renderer, DefaultColor);
}

#pragma endregion