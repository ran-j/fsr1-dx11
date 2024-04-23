#pragma once
#include <d3d11.h>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "D3DCompiler.lib")

enum FFX_FSR1_QUALITY_MODE
{
    FFX_FSR1_QUALITY_MODE_ULTRA_QUALITY = 0, // Upscaling ratio of 1.3x
    FFX_FSR1_QUALITY_MODE_QUALITY = 1,       // Upscaling ratio of 1.5x
    FFX_FSR1_QUALITY_MODE_BALANCED = 2,      // Upscaling ratio of 1.7x
    FFX_FSR1_QUALITY_MODE_PERFORMANCE = 3    // Upscaling ratio of 2.0x
};

class SimpleFSR1
{
private:
    ID3D11Device *m_device;
    ID3D11DeviceContext *m_context;
    ID3D11ComputeShader *m_csEASU;
    ID3D11ComputeShader *m_csRCAS;
    ID3D11ComputeShader *m_csDL;
    ID3D11Texture2D *m_texColorBuffer;
    ID3D11Texture2D *m_texFSR2x;
    ID3D11Texture2D *m_texFSR1L;
    ID3D11Texture2D *m_texFSRC;
    ID3D11SamplerState *m_samplerPoint;
    ID3D11ShaderResourceView *m_srvColorBuffer;
    ID3D11UnorderedAccessView *m_uavFSR2x;
    ID3D11UnorderedAccessView *m_uavFSR1L;
    ID3D11UnorderedAccessView *m_uavFSRC;
    int m_width;
    int m_height;
    FFX_FSR1_QUALITY_MODE m_qualityMode;

private:
    float GetUpscaleFactor();

    void ReleaseResources();

    void CompileShaders();
    void CompileInternalShader(const char *entryPoint, const char *target, ID3DBlob **blob);

public:
    SimpleFSR1(ID3D11Device *device, ID3D11DeviceContext *context, const int &width, const int &height, const FFX_FSR1_QUALITY_MODE &quality);
    ~SimpleFSR1();

    void SetupResources();

    void OnResize(const int &newWidth, const int &newHeight);
    void SetQualityMode(const FFX_FSR1_QUALITY_MODE &qualityMode);

    // void Upscale(ID3D11ShaderResourceView *inputSRV, ID3D11RenderTargetView *outputRTV);
    void Upscale();
};