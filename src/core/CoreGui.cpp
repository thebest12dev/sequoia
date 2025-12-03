#include "CoreGui.h"
#include <windows.h>
#include <d3d11.h>
#include <tchar.h>

#include "imgui.h"
#include <dxgi1_2.h>
#include <iostream>
#include "backends/imgui_impl_win32.h"
#include "backends/imgui_impl_dx11.h"
#pragma comment(lib, "d3d11.lib")
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(
  HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam
);
namespace sequoia {
  // rt create/destroy
  void CoreGui::createRenderTarget() {
    ID3D11Texture2D* backBuffer = nullptr;
    dxgiSwapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer));
    d3dDevice->CreateRenderTargetView(backBuffer, nullptr, &renderTargetView);
    backBuffer->Release();
  }

  void CoreGui::cleanupRenderTarget() {
    if (renderTargetView) {
      renderTargetView->Release();
      renderTargetView = nullptr;
    }
  }

  
  // dev creation
  bool CoreGui::d3dCreateDevice(HWND hwnd) {
   
    UINT createDeviceFlags = 0;
    D3D_FEATURE_LEVEL featureLevel;
    D3D_FEATURE_LEVEL featureLevelArray[1] = { D3D_FEATURE_LEVEL_11_0 };

    HRESULT hr = D3D11CreateDevice(
      nullptr,                        
      D3D_DRIVER_TYPE_HARDWARE,
      nullptr,
      createDeviceFlags,
      featureLevelArray,
      1,
      D3D11_SDK_VERSION,
      &d3dDevice,
      &featureLevel,
      &d3dDeviceContext
    );

    if (FAILED(hr))
      return false;

    
    IDXGIDevice* dxgiDevice = nullptr;
    hr = d3dDevice->QueryInterface(__uuidof(IDXGIDevice), (void**)&dxgiDevice);
    if (FAILED(hr))
      return false;

    IDXGIAdapter* adapter = nullptr;
    dxgiDevice->GetAdapter(&adapter);

    IDXGIFactory2* factory2 = nullptr;
    hr = adapter->GetParent(__uuidof(IDXGIFactory2), (void**)&factory2);
    if (FAILED(hr))
      return false;

    
    DXGI_SWAP_CHAIN_DESC1 sd{};
    sd.Width = 0;                    
    sd.Height = 0;
    sd.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.SampleDesc.Count = 1;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.BufferCount = 2;
    sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;
    sd.Scaling = DXGI_SCALING_NONE;
    sd.AlphaMode = DXGI_ALPHA_MODE_IGNORE;

    hr = factory2->CreateSwapChainForHwnd(
      d3dDevice,
      hwnd,
      &sd,
      nullptr,    
      nullptr,    
      &dxgiSwapChain
    );
    if (FAILED(hr))
      return false;




    factory2->Release();
    adapter->Release();
    dxgiDevice->Release();

    createRenderTarget();
    return true;
  }

  namespace {
    int screen = 1;
    bool initialized = false;
    long ctr = 0;
    bool darkMode()
    {
      HKEY hKey;
      DWORD value = 1; 
      DWORD dataSize = sizeof(value);

      if (RegOpenKeyExW(HKEY_CURRENT_USER,
        L"Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize",
        0, KEY_READ, &hKey) == ERROR_SUCCESS)
      {
        RegQueryValueExW(hKey, L"AppsUseLightTheme", nullptr, nullptr,
          reinterpret_cast<BYTE*>(&value), &dataSize);
        RegCloseKey(hKey);
      }

      return (value == 0); 
    }
  }
  void CoreGui::renderFrame() {
    if (initialized) {
      ImGui_ImplDX11_NewFrame();
      ImGui_ImplWin32_NewFrame();

      ImGui::NewFrame();


      ImGui::SetNextWindowSize(ImVec2(600, 500), ImGuiCond_Always);

      static float f = 0.0f;
      static int counter = 0;
      ImGuiViewport* viewport = ImGui::GetMainViewport();
      ImGuiStyle& style = ImGui::GetStyle();
      style.WindowRounding = 10.0f;
      style.FrameRounding = 5.0f;
      ImGui::SetNextWindowPos(ImVec2(viewport->Pos.x - 2, viewport->Pos.y - 2));
      ImGui::SetNextWindowSize(ImVec2(viewport->Size.x + 4, viewport->Size.y + 4));
      ImGui::SetNextWindowViewport(viewport->ID);
      ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar
        | ImGuiWindowFlags_NoResize
        | ImGuiWindowFlags_NoMove
        | ImGuiWindowFlags_NoCollapse
        | ImGuiWindowFlags_NoBringToFrontOnFocus
        | ImGuiWindowFlags_NoNavFocus;
      ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
      ImGui::Begin("Sequoia", nullptr, flags);
      if (screen == 3) {

        ImGui::Text("Settings");
        ImVec2 winSize = ImGui::GetWindowSize();

        float px = 1.f;
        float py = 0.f;

        float ox = -107.f;
        float oy = 10.f;
        ImVec2 pos;
        pos.x = winSize.x * px + ox;
        pos.y = winSize.y * py + oy;

        ImGui::SetCursorPos(pos);
        if (ImGui::Button("Back")) {
          screen = 3;
        };
        ImGui::SameLine();
        if (ImGui::Button("Save")) {

        };
        ImGui::End();
        ImGui::PopStyleVar();
      }
      else if (screen == 2) {
        ImGui::Text("World: New World (10)");
        SetWindowText(hwnd, "Sequoia: New World (10)");
        

        if (ImGui::TreeNodeEx("Actions", ImGuiTreeNodeFlags_DefaultOpen))
        {
          if (ImGui::TreeNodeEx("Backups")) {
            if (ImGui::Selectable("View...")) {
            }
            if (ImGui::Selectable("Create...")) {

            }
            if (ImGui::Selectable("Config...")) {

            }
            if (ImGui::Selectable("Manage...")) {

            }
            ImGui::TreePop();
          }



          ImGui::TreePop();
        }
        ImVec2 winSize = ImGui::GetWindowSize();

        float px = 1.f;
        float py = 0.f;

        float ox = -130.f;
        float oy = 10.f;
        ImVec2 pos;
        pos.x = winSize.x * px + ox;
        pos.y = winSize.y * py + oy;

        ImGui::SetCursorPos(pos);
        if (ImGui::Button("Settings")) {
          screen = 3;
        };
        ImGui::SameLine();
        if (ImGui::Button("Close")) {

        };
        ImGui::End();
        ImGui::PopStyleVar();
      }
      else if (screen == 1) {
        ImGui::Text("Welcome to Sequoia!");
        ImGui::Text("Choose a world in the dropdown to view backups, manage data and more!");

        static int current = 0;

        if (ImGui::TreeNodeEx("Minecraft: Java Edition saves", ImGuiTreeNodeFlags_DefaultOpen))
        {
          if (ImGui::Selectable("New World (10)")) {
            screen = 2;
          }
          if (ImGui::Selectable("New World (3)")) {

          }


          ImGui::TreePop();
        }
        if (ImGui::Selectable("Select world...")) {

        }
        ImGui::End();
        ImGui::PopStyleVar();
      }
    }

    ImGui::Render();
    d3dDeviceContext->OMSetRenderTargets(1, &renderTargetView, nullptr);

    const float clear_color[4] = { 1.f,1.f,1.f,1.f };
    d3dDeviceContext->ClearRenderTargetView(renderTargetView, clear_color);


    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

    dxgiSwapChain->Present(1, 0);
  }
  void CoreGui::d3dCleanupDevice() {
    cleanupRenderTarget();
    if (dxgiSwapChain) { dxgiSwapChain->Release(); dxgiSwapChain = nullptr; }
    if (d3dDeviceContext) { d3dDeviceContext->Release(); d3dDeviceContext = nullptr; }
    if (d3dDevice) { d3dDevice->Release(); d3dDevice = nullptr; }
  }

 

  LRESULT CALLBACK CoreGui::windowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wParam, lParam))
      return true;
    ctr++;
    switch (msg) {
    case WM_SIZE:
      if (d3dDevice && wParam != SIZE_MINIMIZED) {
        cleanupRenderTarget();
        dxgiSwapChain->ResizeBuffers(0,
          LOWORD(lParam), HIWORD(lParam),
          DXGI_FORMAT_UNKNOWN, DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING);
        createRenderTarget();
        if (ctr > 32) {
          renderFrame();
        }
        
      }
      break;


    case WM_DESTROY:
      PostQuitMessage(0);
      return 0;
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
  }
  void CoreGui::launchGui() {

    WNDCLASSEX wc = {
        sizeof(WNDCLASSEX),
        CS_CLASSDC,
        windowProc,
        0L,
        0L,
        GetModuleHandle(NULL),
        nullptr, nullptr, nullptr, nullptr,
        _T("Sequoia"),
        nullptr
    };
    RegisterClassEx(&wc);

    // the window
    hwnd = CreateWindow( wc.lpszClassName, _T("Sequoia"),
      WS_OVERLAPPEDWINDOW,
      100, 100, 800, 500,
      nullptr, nullptr, wc.hInstance, nullptr);


    // direct3d 11
    if (!d3dCreateDevice(hwnd)) {
      d3dCleanupDevice();
      UnregisterClass(wc.lpszClassName, wc.hInstance);
      return;
    }

    ShowWindow(hwnd, SW_SHOWDEFAULT);
    UpdateWindow(hwnd);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(d3dDevice, d3dDeviceContext);
    
    ImGuiIO& io = ImGui::GetIO();

    // whatever font
    io.Fonts->AddFontFromFileTTF("fonts/Roboto-Regular.ttf", 18.0f);
    // loop
    BOOL useDarkMode = darkMode();
    HRESULT I = DwmSetWindowAttribute(hwnd, 20, &useDarkMode, sizeof(useDarkMode));
    DWM_SYSTEMBACKDROP_TYPE backdrop = DWMSBT_TABBEDWINDOW; // better effects
    DwmSetWindowAttribute(hwnd, 38, &backdrop, sizeof(backdrop));
    MSG msg = {};
    while (msg.message != WM_QUIT) {
      if (PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
        continue;
      }
      initialized = true;
      renderFrame();
      

      

    }

    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();


    d3dCleanupDevice();
    DestroyWindow(hwnd);
    UnregisterClass(wc.lpszClassName, wc.hInstance);
  }
}