#pragma once

#include <string>
#include "singleton.hpp"
#include "imgui/imgui.h"
#include <d3dx9tex.h>
#pragma comment(lib,"d3dx9.lib")


struct IDirect3DDevice9;



class Menu
    : public Singleton<Menu>
{
public:
    void Initialize();
    void Shutdown();
    IDirect3DTexture9* smoke = nullptr;
    void OnDeviceLost();
    void OnDeviceReset();

    void Render();

    void Toggle();

    bool IsVisible() const { return _visible; }

private:
    void CreateStyle();

    ImGuiStyle        _style;
    bool              _visible;
};