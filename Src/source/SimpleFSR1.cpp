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
	if (m_srvColorBuffer)
	{
		m_srvColorBuffer->Release();
		m_srvColorBuffer = nullptr;
	}
	if (m_texFSR2x)
	{
		m_texFSR2x->Release();
		m_texFSR2x = nullptr;
	}
	if (m_uavFSR2x)
	{
		m_uavFSR2x->Release();
		m_uavFSR2x = nullptr;
	}
	if (m_texFSR1L)
	{
		m_texFSR1L->Release();
		m_texFSR1L = nullptr;
	}
	if (m_uavFSR1L)
	{
		m_uavFSR1L->Release();
		m_uavFSR1L = nullptr;
	}
	if (m_texFSRC)
	{
		m_texFSRC->Release();
		m_texFSRC = nullptr;
	}
	if (m_uavFSRC)
	{
		m_uavFSRC->Release();
		m_uavFSRC = nullptr;
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
	// Ensure we release old resources if we are resetting
	ReleaseResources();

	float upscaleFactor = GetUpscaleFactor();
	int targetWidth = static_cast<int>(m_width * upscaleFactor);
	int targetHeight = static_cast<int>(m_height * upscaleFactor);

	// Setup texture and SRV/UAV
	D3D11_TEXTURE2D_DESC texDesc;
	ZeroMemory(&texDesc, sizeof(texDesc));
	texDesc.Width = m_width;
	texDesc.Height = m_height;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	texDesc.SampleDesc.Count = 1;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	ZeroMemory(&srvDesc, sizeof(srvDesc));
	srvDesc.Format = texDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;

	D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
	ZeroMemory(&uavDesc, sizeof(uavDesc));
	uavDesc.Format = texDesc.Format;
	uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
	uavDesc.Texture2D.MipSlice = 0;

	// Create color buffer and its SRV
	m_device->CreateTexture2D(&texDesc, nullptr, &m_texColorBuffer);
	m_device->CreateShaderResourceView(m_texColorBuffer, &srvDesc, &m_srvColorBuffer);

	// Resize for upscale target dimensions
	texDesc.Width = targetWidth;
	texDesc.Height = targetHeight;

	// Create textures and UAVs for upscale
	m_device->CreateTexture2D(&texDesc, nullptr, &m_texFSR2x);
	m_device->CreateUnorderedAccessView(m_texFSR2x, &uavDesc, &m_uavFSR2x);

	m_device->CreateTexture2D(&texDesc, nullptr, &m_texFSR1L);
	m_device->CreateUnorderedAccessView(m_texFSR1L, &uavDesc, &m_uavFSR1L);

	m_device->CreateTexture2D(&texDesc, nullptr, &m_texFSRC);
	m_device->CreateUnorderedAccessView(m_texFSRC, &uavDesc, &m_uavFSRC);

	// Setup sampler
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
	ID3D11UnorderedAccessView *uavs[] = {m_uavFSR2x, m_uavFSR1L, m_uavFSRC};
	ID3D11ShaderResourceView *srvs[] = {m_srvColorBuffer};

	// Bind samplers, UAVs and SRVs
	m_context->CSSetSamplers(0, 1, &m_samplerPoint);
	m_context->CSSetShaderResources(0, 1, srvs); 

	// EASU Pass
	m_context->CSSetShader(m_csEASU, nullptr, 0);
	m_context->CSSetUnorderedAccessViews(0, 1, &uavs[0], nullptr);
	m_context->Dispatch((m_width + 7) / 8, (m_height + 7) / 8, 1);

	// RCAS Pass
	m_context->CSSetShader(m_csRCAS, nullptr, 0);
	m_context->CSSetUnorderedAccessViews(0, 1, &uavs[1], nullptr);
	m_context->Dispatch((m_width + 15) / 16, (m_height + 15) / 16, 1);

	// DL Pass
	m_context->CSSetShader(m_csDL, nullptr, 0);
	m_context->CSSetUnorderedAccessViews(0, 1, &uavs[2], nullptr);
	m_context->Dispatch((m_width + 7) / 8, (m_height + 7) / 8, 1);
}
