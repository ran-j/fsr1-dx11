#include <SimpleFSR1.h>
#include <FSR1.h>

#include <D3Dcompiler.h>
#include <DirectXMath.h>

SimpleFSR1::SimpleFSR1(ID3D11Device *device, ID3D11DeviceContext *context, const int &width, const int &height, const FFX_FSR1_QUALITY_MODE &quality) : m_device(device), m_context(context), m_width(width), m_height(height), m_qualityMode(quality),
																																						m_csEASU(nullptr), m_csRCAS(nullptr), m_csDL(nullptr),
																																						m_texColorBuffer(nullptr), m_texFSR2x(nullptr), m_texFSR1L(nullptr), m_texFSRC(nullptr), m_samplerPoint(nullptr)
{
	this->m_device->AddRef();
	this->m_context->AddRef();
	CompileShaders();
	SetupResources();
}

SimpleFSR1::~SimpleFSR1()
{
	if (m_csEASU)
		m_csEASU->Release();
	if (m_csRCAS)
		m_csRCAS->Release();
	if (m_csDL)
		m_csDL->Release();

	ReleaseResources();

	if (m_samplerPoint)
		m_samplerPoint->Release();

	// We don't need to release the device and context here, because we don't own them
	// context->Release();
	// device->Release();
}

float SimpleFSR1::GetUpscaleFactor()
{
	switch (m_qualityMode)
	{
	case FFX_FSR1_QUALITY_MODE_ULTRA_QUALITY:
		return 1.3f;
	case FFX_FSR1_QUALITY_MODE_QUALITY:
		return 1.5f;
	case FFX_FSR1_QUALITY_MODE_BALANCED:
		return 1.7f;
	case FFX_FSR1_QUALITY_MODE_PERFORMANCE:
		return 2.0f;
	default:
		return 1.5f; // Default to quality mode
	}
}

void SimpleFSR1::ReleaseResources()
{
	if (m_texColorBuffer)
	{
		m_texColorBuffer->Release();
		m_texColorBuffer = nullptr;
	}
	if (m_texFSR2x)
	{
		m_texFSR2x->Release();
		m_texFSR2x = nullptr;
	}
	if (m_texFSR1L)
	{
		m_texFSR1L->Release();
		m_texFSR1L = nullptr;
	}
	if (m_texFSRC)
	{
		m_texFSRC->Release();
		m_texFSRC = nullptr;
	}
}

void SimpleFSR1::CompileShaders()
{
	ID3DBlob *blob = nullptr;

	CompileInternalShader("mainCS", "cs_5_0", &blob);
	m_device->CreateComputeShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, &m_csEASU);
	assert(m_csEASU != nullptr);
	blob->Release();

	CompileInternalShader("main2CS", "cs_5_0", &blob);
	m_device->CreateComputeShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, &m_csRCAS);
	assert(m_csRCAS != nullptr);
	blob->Release();

	CompileInternalShader("main3CS", "cs_5_0", &blob);
	m_device->CreateComputeShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, &m_csDL);
	assert(m_csDL != nullptr);
	blob->Release();
}

void SimpleFSR1::CompileInternalShader(const char *entryPoint, const char *target, ID3DBlob **blob)
{
	DWORD shaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined(DEBUG) || defined(_DEBUG)
	shaderFlags |= D3DCOMPILE_DEBUG;
#endif

	ID3DBlob *errorBlob = nullptr;
	// HRESULT hr = D3DCompileFromFile(filename, nullptr, nullptr, entryPoint, target, shaderFlags, 0, blob, &errorBlob);
	HRESULT hr = D3DCompile(DefaultShader::fsr1,
							strlen(DefaultShader::fsr1),
							nullptr,
							nullptr,
							nullptr,
							entryPoint,
							target,
							shaderFlags,
							0,
							blob,
							&errorBlob);
	if (FAILED(hr))
	{
		if (errorBlob)
		{
			OutputDebugStringA((char *)errorBlob->GetBufferPointer());
			errorBlob->Release();
		}
	}
	if (errorBlob)
	{
		errorBlob->Release();
	}
}

void SimpleFSR1::SetupResources()
{
	// Release existing resources if any
	ReleaseResources();

	float upscaleFactor = GetUpscaleFactor();
	int targetWidth = static_cast<int>(m_width * upscaleFactor);
	int targetHeight = static_cast<int>(m_height * upscaleFactor);

	D3D11_TEXTURE2D_DESC desc;
	ZeroMemory(&desc, sizeof(desc));
	desc.Width = m_width;
	desc.Height = m_height;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	desc.SampleDesc.Count = 1;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;

	m_device->CreateTexture2D(&desc, nullptr, &m_texColorBuffer);
	desc.Width = targetWidth;
	desc.Height = targetHeight;
	m_device->CreateTexture2D(&desc, nullptr, &m_texFSR2x);
	m_device->CreateTexture2D(&desc, nullptr, &m_texFSR1L);
	m_device->CreateTexture2D(&desc, nullptr, &m_texFSRC);

	D3D11_SAMPLER_DESC sampDesc;
	ZeroMemory(&sampDesc, sizeof(sampDesc));
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
	m_device->CreateSamplerState(&sampDesc, &m_samplerPoint);
}

void SimpleFSR1::OnResize(const int &newWidth, const int &newHeight)
{
	if (newWidth == m_width && newHeight == m_height)
		return; // No change

	m_width = newWidth;
	m_height = newHeight;

	// Re-create resources with new dimensions
	SetupResources();
}

void SimpleFSR1::SetQualityMode(const FFX_FSR1_QUALITY_MODE &qualityMode)
{
	if (m_qualityMode == qualityMode)
		return; // No change

	m_qualityMode = qualityMode;

	// Re-create resources with new quality mode
	SetupResources();
}

void SimpleFSR1::Upscale()
{
	// Set the compute shaders
	m_context->CSSetSamplers(0, 1, &m_samplerPoint);
	m_context->CSSetShader(m_csEASU, nullptr, 0);
	m_context->Dispatch((m_width + 7) / 8, (m_height + 7) / 8, 1);

	m_context->CSSetShader(m_csRCAS, nullptr, 0);
	m_context->Dispatch((m_width + 15) / 16, (m_height + 15) / 16, 1);

	m_context->CSSetShader(m_csDL, nullptr, 0);
	m_context->Dispatch((m_width + 7) / 8, (m_height + 7) / 8, 1);

	// Set the resources
	// ID3D11ShaderResourceView *srvs[1] = {nullptr};
	// ID3D11UnorderedAccessView *uavs[1] = {nullptr};
	// ID3D11SamplerState *samplers[1] = {m_samplerPoint};

	// m_context->CSSetShaderResources(0, 1, srvs);
	// m_context->CSSetShaderResources(1, 1, srvs);
	// m_context->CSSetShaderResources(2, 1, srvs);
	// m_context->CSSetShaderResources(3, 1, srvs);
	// m_context->CSSetShaderResources(4, 1, srvs);
	// m_context->CSSetShaderResources(5, 1, srvs);
	// m_context->CSSetShaderResources(6, 1, srvs);
	// m_context->CSSetShaderResources(7, 1, srvs);
	// m_context->CSSetShaderResources(8, 1, srvs);
	// m_context->CSSetShaderResources(9, 1, srvs);
	// m_context->CSSetShaderResources(10, 1, srvs);
	// m_context->CSSetShaderResources(11, 1, srvs);
	// m_context->CSSetShaderResources(12, 1, srvs);
	// m_context->CSSetShaderResources(13, 1, srvs);
	// m_context->CSSetShaderResources(14, 1, srvs);
	// m_context->CSSetShaderResources(15, 1, srvs);
	// m_context->CSSetShaderResources(16, 1, srvs);
	// m_context->CSSetShaderResources(17, 1, srvs);
	// m_context->CSSetShaderResources(18, 1, srvs);
	// m_context->CSSetShaderResources(19, 1, srvs);
	// m_context->CSSetShaderResources(20, 1, srvs);
	// m_context->CSSetShaderResources(21, 1, srvs);
	// m_context->CSSetShaderResources(22, 1, srvs);
	// m_context->CSSetShaderResources(23, 1, srvs);
}
