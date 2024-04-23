#include <windows.h>
#include <d3d11.h>
#include <dxgi.h>
#include <SimpleFSR1.h>

bool bRunning = true;

SimpleFSR1 *fsr1 = nullptr;

IDXGISwapChain *swapChain = nullptr;
ID3D11Device *device = nullptr;
ID3D11DeviceContext *context = nullptr;
ID3D11RenderTargetView *renderTargetView = nullptr;

const UINT width = 800;
const UINT height = 600;

LRESULT CALLBACK WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    const char CLASS_NAME[] = "Sample Window Class";

    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(
        0,
        CLASS_NAME,
        "DirectX 11 and FSR 1.0 Example",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, width, height,
        nullptr, nullptr, hInstance, nullptr);

    if (hwnd == nullptr)
    {
        return 0;
    }

    ShowWindow(hwnd, nCmdShow);

    if (!InitD3D(hwnd))
    {
        Cleanup();
        return 0;
    }

    while (bRunning)
    {
        MSG msg;
        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
            {
                bRunning = false;
            }
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        float bgColor[4] = {0.0f, 0.2f, 0.4f, 1.0f};
        context->ClearRenderTargetView(renderTargetView, bgColor);

        fsr1->Upscale();
        swapChain->Present(0, 0);
    }

    Cleanup();
    return 0;
}

bool InitD3D(HWND hwnd)
{
    DXGI_SWAP_CHAIN_DESC scd = {};
    scd.BufferCount = 1;
    scd.BufferDesc.Width = width;
    scd.BufferDesc.Height = height;
    scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    scd.BufferDesc.RefreshRate.Numerator = 60;
    scd.BufferDesc.RefreshRate.Denominator = 1;
    scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    scd.OutputWindow = hwnd;
    scd.SampleDesc.Count = 1;
    scd.SampleDesc.Quality = 0;
    scd.Windowed = TRUE;

    D3D_FEATURE_LEVEL featureLevels[] = {D3D_FEATURE_LEVEL_11_0};
    D3D_FEATURE_LEVEL featureLevel;

    HRESULT hr = D3D11CreateDeviceAndSwapChain(
        nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr,
        0, featureLevels, 1,
        D3D11_SDK_VERSION, &scd,
        &swapChain, &device, &featureLevel, &context);

    if (FAILED(hr))
    {
        return false;
    }

    ID3D11Texture2D *pBackBuffer = nullptr;
    swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void **)&pBackBuffer);

    device->CreateRenderTargetView(pBackBuffer, nullptr, &renderTargetView);
    pBackBuffer->Release();

    context->OMSetRenderTargets(1, &renderTargetView, nullptr);

    D3D11_VIEWPORT viewport;
    ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));
    viewport.TopLeftX = 0;
    viewport.TopLeftY = 0;
    viewport.Width = static_cast<float>(width);
    viewport.Height = static_cast<float>(height);
    context->RSSetViewports(1, &viewport);

    fsr1 = new SimpleFSR1(device, context, width, height, FFX_FSR1_QUALITY_MODE::FFX_FSR1_QUALITY_MODE_ULTRA_QUALITY);

    return true;
}

void Cleanup()
{
    if (renderTargetView)
        renderTargetView->Release();
    if (swapChain)
        swapChain->Release();
    if (device)
        device->Release();
    if (context)
        context->Release();
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProc(hwnd, message, wParam, lParam);
}
