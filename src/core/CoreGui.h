#pragma once
#include <windows.h>
#include <d3d11.h>
#include <tchar.h>
#include <dxgi1_2.h>
#include "imgui.h"
#include "backends/imgui_impl_win32.h"
#include "backends/imgui_impl_dx11.h"
#include <dwmapi.h>
#pragma comment(lib, "dwmapi.lib")
#pragma comment(lib, "d3d11.lib")
namespace sequoia {
  // Single class that practically manages the entire Sequoia GUI
  // Don't link against sequoia core libraries, this is part of the CLI.
  class CoreGui {
  private:
    static inline ID3D11Device* d3dDevice = nullptr;
    static inline ID3D11DeviceContext* d3dDeviceContext = nullptr;
    static inline IDXGISwapChain1* dxgiSwapChain = nullptr;
    static inline ID3D11RenderTargetView* renderTargetView = nullptr;
    static inline HWND hwnd;

    static void createRenderTarget();
    static void cleanupRenderTarget();
    static bool d3dCreateDevice(HWND hwnd);
    static void d3dCleanupDevice();
    static void renderFrame();
    static LRESULT CALLBACK windowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
  public:
    static void launchGui();
  };
}