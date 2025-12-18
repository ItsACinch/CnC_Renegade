/*
**  Command & Conquer Renegade(tm)
**  Copyright 2025 Electronic Arts Inc.
**
**  This program is free software: you can redistribute it and/or modify
**  it under the terms of the GNU General Public License as published by
**  the Free Software Foundation, either version 3 of the License, or
**  (at your option) any later version.
**
**  This program is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**  GNU General Public License for more details.
**
**  You should have received a copy of the GNU General Public License
**  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "D3D11Device.h"
#include "wwdebug.h"

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")

// Global render device instance
IRenderDevice* g_RenderDevice = nullptr;

// Factory function
IRenderDevice* CreateRenderDevice()
{
    return new D3D11Device();
}

// ============================================================================
// D3D11StateCache Implementation
// ============================================================================

D3D11StateCache::D3D11StateCache()
    : m_Device(nullptr)
{
}

D3D11StateCache::~D3D11StateCache()
{
    Shutdown();
}

void D3D11StateCache::Initialize(ID3D11Device* device)
{
    m_Device = device;
}

void D3D11StateCache::Shutdown()
{
    // Release cached states
    m_Device = nullptr;
}

ID3D11RasterizerState* D3D11StateCache::GetRasterizerState(D3D11_RASTERIZER_DESC& desc)
{
    // TODO: Implement state caching with hash map
    ID3D11RasterizerState* state = nullptr;
    if (m_Device) {
        m_Device->CreateRasterizerState(&desc, &state);
    }
    return state;
}

ID3D11BlendState* D3D11StateCache::GetBlendState(D3D11_BLEND_DESC& desc)
{
    // TODO: Implement state caching with hash map
    ID3D11BlendState* state = nullptr;
    if (m_Device) {
        m_Device->CreateBlendState(&desc, &state);
    }
    return state;
}

ID3D11DepthStencilState* D3D11StateCache::GetDepthStencilState(D3D11_DEPTH_STENCIL_DESC& desc)
{
    // TODO: Implement state caching with hash map
    ID3D11DepthStencilState* state = nullptr;
    if (m_Device) {
        m_Device->CreateDepthStencilState(&desc, &state);
    }
    return state;
}

ID3D11SamplerState* D3D11StateCache::GetSamplerState(D3D11_SAMPLER_DESC& desc)
{
    // TODO: Implement state caching with hash map
    ID3D11SamplerState* state = nullptr;
    if (m_Device) {
        m_Device->CreateSamplerState(&desc, &state);
    }
    return state;
}

// ============================================================================
// D3D11Device Implementation
// ============================================================================

D3D11Device::D3D11Device()
    : m_Initialized(false)
    , m_DeviceLost(false)
    , m_InScene(false)
    , m_Width(0)
    , m_Height(0)
    , m_Windowed(true)
    , m_TransformsDirty(true)
    , m_MaterialDirty(true)
    , m_LightsDirty(true)
    , m_FogDirty(true)
    , m_FrameCount(0)
    , m_DrawCalls(0)
    , m_PrimitivesDrawn(0)
{
    memset(m_DeviceName, 0, sizeof(m_DeviceName));
    memset(m_RenderStates, 0, sizeof(m_RenderStates));
    memset(m_TextureStageStates, 0, sizeof(m_TextureStageStates));

    // Initialize transform constants to identity
    m_TransformConstants.World.Make_Identity();
    m_TransformConstants.View.Make_Identity();
    m_TransformConstants.Projection.Make_Identity();
    m_TransformConstants.WorldViewProjection.Make_Identity();

    // Initialize material to default
    m_MaterialConstants.Diffuse = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
    m_MaterialConstants.Ambient = Vector4(0.2f, 0.2f, 0.2f, 1.0f);
    m_MaterialConstants.Specular = Vector4(0.0f, 0.0f, 0.0f, 1.0f);
    m_MaterialConstants.Emissive = Vector4(0.0f, 0.0f, 0.0f, 1.0f);
    m_MaterialConstants.SpecularPower = 0.0f;

    // Initialize lights
    memset(&m_LightConstants, 0, sizeof(m_LightConstants));

    // Initialize fog
    m_FogConstants.FogColor = Vector4(0.5f, 0.5f, 0.5f, 1.0f);
    m_FogConstants.FogStart = 0.0f;
    m_FogConstants.FogEnd = 1.0f;
    m_FogConstants.FogDensity = 1.0f;
    m_FogConstants.FogEnabled = 0;
}

D3D11Device::~D3D11Device()
{
    Shutdown();
}

bool D3D11Device::Initialize(void* hwnd, int width, int height, bool fullscreen)
{
    if (m_Initialized) {
        return true;
    }

    HWND hWnd = static_cast<HWND>(hwnd);
    if (!hWnd) {
        WWDEBUG_SAY(("D3D11Device::Initialize - Invalid window handle\n"));
        return false;
    }

    m_Width = width;
    m_Height = height;
    m_Windowed = !fullscreen;

    // Create device
    if (!CreateDevice(hWnd)) {
        WWDEBUG_SAY(("D3D11Device::Initialize - Failed to create device\n"));
        return false;
    }

    // Create swap chain
    if (!CreateSwapChain(hWnd, width, height, fullscreen)) {
        WWDEBUG_SAY(("D3D11Device::Initialize - Failed to create swap chain\n"));
        Shutdown();
        return false;
    }

    // Create render target view
    if (!CreateRenderTargetView()) {
        WWDEBUG_SAY(("D3D11Device::Initialize - Failed to create render target view\n"));
        Shutdown();
        return false;
    }

    // Create depth stencil view
    if (!CreateDepthStencilView(width, height)) {
        WWDEBUG_SAY(("D3D11Device::Initialize - Failed to create depth stencil view\n"));
        Shutdown();
        return false;
    }

    // Create constant buffers
    if (!CreateConstantBuffers()) {
        WWDEBUG_SAY(("D3D11Device::Initialize - Failed to create constant buffers\n"));
        Shutdown();
        return false;
    }

    // Load shaders
    if (!LoadShaders()) {
        WWDEBUG_SAY(("D3D11Device::Initialize - Failed to load shaders\n"));
        // Non-fatal for now - shaders can be loaded later
    }

    // Initialize state cache
    m_StateCache.Initialize(m_Device.Get());

    // Set default viewport
    RenderViewport vp;
    vp.X = 0;
    vp.Y = 0;
    vp.Width = width;
    vp.Height = height;
    vp.MinZ = 0.0f;
    vp.MaxZ = 1.0f;
    SetViewport(vp);

    // Bind render targets
    m_Context->OMSetRenderTargets(1, m_RenderTargetView.GetAddressOf(), m_DepthStencilView.Get());

    m_Initialized = true;
    WWDEBUG_SAY(("D3D11Device::Initialize - Success (%dx%d %s)\n",
                 width, height, fullscreen ? "fullscreen" : "windowed"));

    return true;
}

void D3D11Device::Shutdown()
{
    m_StateCache.Shutdown();

    // Release in reverse order of creation
    m_FixedFunctionVS.Reset();
    m_FixedFunctionPS.Reset();
    m_DefaultInputLayout.Reset();

    m_TransformBuffer.Reset();
    m_MaterialBuffer.Reset();
    m_LightBuffer.Reset();
    m_FogBuffer.Reset();

    m_DepthStencilView.Reset();
    m_DepthStencilBuffer.Reset();
    m_RenderTargetView.Reset();
    m_SwapChain.Reset();
    m_Context.Reset();
    m_Device.Reset();

    m_Initialized = false;
    m_DeviceLost = false;

    WWDEBUG_SAY(("D3D11Device::Shutdown\n"));
}

bool D3D11Device::Reset(int width, int height, bool fullscreen)
{
    if (!m_Initialized) {
        return false;
    }

    // Release current render targets
    m_Context->OMSetRenderTargets(0, nullptr, nullptr);
    m_RenderTargetView.Reset();
    m_DepthStencilView.Reset();
    m_DepthStencilBuffer.Reset();

    // Resize swap chain
    HRESULT hr = m_SwapChain->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, 0);
    if (FAILED(hr)) {
        WWDEBUG_SAY(("D3D11Device::Reset - Failed to resize swap chain\n"));
        return false;
    }

    m_Width = width;
    m_Height = height;
    m_Windowed = !fullscreen;

    // Recreate render target view
    if (!CreateRenderTargetView()) {
        return false;
    }

    // Recreate depth stencil view
    if (!CreateDepthStencilView(width, height)) {
        return false;
    }

    // Rebind render targets
    m_Context->OMSetRenderTargets(1, m_RenderTargetView.GetAddressOf(), m_DepthStencilView.Get());

    // Update viewport
    RenderViewport vp;
    vp.X = 0;
    vp.Y = 0;
    vp.Width = width;
    vp.Height = height;
    vp.MinZ = 0.0f;
    vp.MaxZ = 1.0f;
    SetViewport(vp);

    return true;
}

bool D3D11Device::CreateDevice(HWND hwnd)
{
    UINT createDeviceFlags = 0;
#ifdef _DEBUG
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    D3D_FEATURE_LEVEL featureLevels[] = {
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
    };

    D3D_FEATURE_LEVEL featureLevel;

    HRESULT hr = D3D11CreateDevice(
        nullptr,                    // Use default adapter
        D3D_DRIVER_TYPE_HARDWARE,   // Hardware acceleration
        nullptr,                    // No software rasterizer
        createDeviceFlags,
        featureLevels,
        ARRAYSIZE(featureLevels),
        D3D11_SDK_VERSION,
        m_Device.GetAddressOf(),
        &featureLevel,
        m_Context.GetAddressOf()
    );

    if (FAILED(hr)) {
        WWDEBUG_SAY(("D3D11Device::CreateDevice - D3D11CreateDevice failed (0x%08X)\n", hr));
        return false;
    }

    // Get adapter info for device name
    ComPtr<IDXGIDevice> dxgiDevice;
    hr = m_Device.As(&dxgiDevice);
    if (SUCCEEDED(hr)) {
        ComPtr<IDXGIAdapter> adapter;
        hr = dxgiDevice->GetAdapter(adapter.GetAddressOf());
        if (SUCCEEDED(hr)) {
            DXGI_ADAPTER_DESC adapterDesc;
            adapter->GetDesc(&adapterDesc);
            WideCharToMultiByte(CP_ACP, 0, adapterDesc.Description, -1,
                               m_DeviceName, sizeof(m_DeviceName), nullptr, nullptr);
        }
    }

    // Fill in capabilities
    m_Caps.MaxTextureWidth = D3D11_REQ_TEXTURE2D_U_OR_V_DIMENSION;
    m_Caps.MaxTextureHeight = D3D11_REQ_TEXTURE2D_U_OR_V_DIMENSION;
    m_Caps.MaxTextureStages = D3D11_MAX_TEXTURE_STAGES;
    m_Caps.SupportsNonPow2Textures = true;
    m_Caps.SupportsDXT1 = true;
    m_Caps.SupportsDXT3 = true;
    m_Caps.SupportsDXT5 = true;
    m_Caps.SupportsHardwareTnL = true;
    m_Caps.MaxVertexBlendMatrices = 4;
    m_Caps.SupportsZBias = true;
    m_Caps.SupportsFogTable = true;
    m_Caps.SupportsStencil = true;
    m_Caps.MaxAnisotropy = D3D11_MAX_MAXANISOTROPY;

    WWDEBUG_SAY(("D3D11Device::CreateDevice - Using device: %s\n", m_DeviceName));
    return true;
}

bool D3D11Device::CreateSwapChain(HWND hwnd, int width, int height, bool fullscreen)
{
    // Get the DXGI factory
    ComPtr<IDXGIDevice> dxgiDevice;
    HRESULT hr = m_Device.As(&dxgiDevice);
    if (FAILED(hr)) {
        return false;
    }

    ComPtr<IDXGIAdapter> adapter;
    hr = dxgiDevice->GetAdapter(adapter.GetAddressOf());
    if (FAILED(hr)) {
        return false;
    }

    ComPtr<IDXGIFactory> factory;
    hr = adapter->GetParent(IID_PPV_ARGS(factory.GetAddressOf()));
    if (FAILED(hr)) {
        return false;
    }

    // Describe the swap chain
    DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
    swapChainDesc.BufferCount = 2;
    swapChainDesc.BufferDesc.Width = width;
    swapChainDesc.BufferDesc.Height = height;
    swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
    swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.OutputWindow = hwnd;
    swapChainDesc.SampleDesc.Count = 1;
    swapChainDesc.SampleDesc.Quality = 0;
    swapChainDesc.Windowed = !fullscreen;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    hr = factory->CreateSwapChain(m_Device.Get(), &swapChainDesc, m_SwapChain.GetAddressOf());
    if (FAILED(hr)) {
        WWDEBUG_SAY(("D3D11Device::CreateSwapChain - CreateSwapChain failed (0x%08X)\n", hr));
        return false;
    }

    // Disable Alt+Enter fullscreen toggle (game handles this)
    factory->MakeWindowAssociation(hwnd, DXGI_MWA_NO_ALT_ENTER);

    return true;
}

bool D3D11Device::CreateRenderTargetView()
{
    ComPtr<ID3D11Texture2D> backBuffer;
    HRESULT hr = m_SwapChain->GetBuffer(0, IID_PPV_ARGS(backBuffer.GetAddressOf()));
    if (FAILED(hr)) {
        return false;
    }

    hr = m_Device->CreateRenderTargetView(backBuffer.Get(), nullptr, m_RenderTargetView.GetAddressOf());
    return SUCCEEDED(hr);
}

bool D3D11Device::CreateDepthStencilView(int width, int height)
{
    D3D11_TEXTURE2D_DESC depthStencilDesc = {};
    depthStencilDesc.Width = width;
    depthStencilDesc.Height = height;
    depthStencilDesc.MipLevels = 1;
    depthStencilDesc.ArraySize = 1;
    depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthStencilDesc.SampleDesc.Count = 1;
    depthStencilDesc.SampleDesc.Quality = 0;
    depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
    depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

    HRESULT hr = m_Device->CreateTexture2D(&depthStencilDesc, nullptr, m_DepthStencilBuffer.GetAddressOf());
    if (FAILED(hr)) {
        return false;
    }

    hr = m_Device->CreateDepthStencilView(m_DepthStencilBuffer.Get(), nullptr, m_DepthStencilView.GetAddressOf());
    return SUCCEEDED(hr);
}

bool D3D11Device::CreateConstantBuffers()
{
    D3D11_BUFFER_DESC bufferDesc = {};
    bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    // Transform buffer
    bufferDesc.ByteWidth = sizeof(TransformConstants);
    HRESULT hr = m_Device->CreateBuffer(&bufferDesc, nullptr, m_TransformBuffer.GetAddressOf());
    if (FAILED(hr)) return false;

    // Material buffer
    bufferDesc.ByteWidth = sizeof(MaterialConstants);
    hr = m_Device->CreateBuffer(&bufferDesc, nullptr, m_MaterialBuffer.GetAddressOf());
    if (FAILED(hr)) return false;

    // Light buffer
    bufferDesc.ByteWidth = sizeof(LightConstants);
    hr = m_Device->CreateBuffer(&bufferDesc, nullptr, m_LightBuffer.GetAddressOf());
    if (FAILED(hr)) return false;

    // Fog buffer
    bufferDesc.ByteWidth = sizeof(FogConstants);
    hr = m_Device->CreateBuffer(&bufferDesc, nullptr, m_FogBuffer.GetAddressOf());
    if (FAILED(hr)) return false;

    return true;
}

bool D3D11Device::LoadShaders()
{
    // TODO: Load compiled shaders from embedded resources or files
    // For now, return true to allow initialization to continue
    // Shaders will be implemented in a separate step
    return true;
}

void D3D11Device::BeginScene()
{
    m_InScene = true;
}

void D3D11Device::EndScene()
{
    m_InScene = false;
}

void D3D11Device::Present()
{
    HRESULT hr = m_SwapChain->Present(1, 0);  // VSync enabled

    if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET) {
        m_DeviceLost = true;
    }

    m_FrameCount++;
}

void D3D11Device::Clear(unsigned int flags, unsigned int color, float z, unsigned int stencil)
{
    if (flags & CLEAR_TARGET) {
        float clearColor[4];
        clearColor[0] = ((color >> 16) & 0xFF) / 255.0f;  // R
        clearColor[1] = ((color >> 8) & 0xFF) / 255.0f;   // G
        clearColor[2] = (color & 0xFF) / 255.0f;          // B
        clearColor[3] = ((color >> 24) & 0xFF) / 255.0f;  // A
        m_Context->ClearRenderTargetView(m_RenderTargetView.Get(), clearColor);
    }

    if ((flags & CLEAR_ZBUFFER) || (flags & CLEAR_STENCIL)) {
        UINT clearFlags = 0;
        if (flags & CLEAR_ZBUFFER) clearFlags |= D3D11_CLEAR_DEPTH;
        if (flags & CLEAR_STENCIL) clearFlags |= D3D11_CLEAR_STENCIL;
        m_Context->ClearDepthStencilView(m_DepthStencilView.Get(), clearFlags, z, (UINT8)stencil);
    }
}

void D3D11Device::SetViewport(const RenderViewport& viewport)
{
    m_Viewport = viewport;

    D3D11_VIEWPORT d3dViewport;
    d3dViewport.TopLeftX = (FLOAT)viewport.X;
    d3dViewport.TopLeftY = (FLOAT)viewport.Y;
    d3dViewport.Width = (FLOAT)viewport.Width;
    d3dViewport.Height = (FLOAT)viewport.Height;
    d3dViewport.MinDepth = viewport.MinZ;
    d3dViewport.MaxDepth = viewport.MaxZ;

    m_Context->RSSetViewports(1, &d3dViewport);
}

void D3D11Device::GetViewport(RenderViewport& viewport) const
{
    viewport = m_Viewport;
}

void D3D11Device::SetTransform(TransformType type, const Matrix4& matrix)
{
    switch (type) {
        case TRANSFORM_WORLD:
            m_TransformConstants.World = matrix;
            break;
        case TRANSFORM_VIEW:
            m_TransformConstants.View = matrix;
            break;
        case TRANSFORM_PROJECTION:
            m_TransformConstants.Projection = matrix;
            break;
        default:
            // Texture transforms not yet implemented
            break;
    }
    m_TransformsDirty = true;
}

void D3D11Device::GetTransform(TransformType type, Matrix4& matrix) const
{
    switch (type) {
        case TRANSFORM_WORLD:
            matrix = m_TransformConstants.World;
            break;
        case TRANSFORM_VIEW:
            matrix = m_TransformConstants.View;
            break;
        case TRANSFORM_PROJECTION:
            matrix = m_TransformConstants.Projection;
            break;
        default:
            matrix.Make_Identity();
            break;
    }
}

void D3D11Device::SetRenderState(RenderStateType state, unsigned int value)
{
    if (state < RS_COUNT) {
        m_RenderStates[state] = value;
    }
}

unsigned int D3D11Device::GetRenderState(RenderStateType state) const
{
    if (state < RS_COUNT) {
        return m_RenderStates[state];
    }
    return 0;
}

void D3D11Device::SetTextureStageState(unsigned int stage, TextureStageStateType state, unsigned int value)
{
    if (stage < D3D11_MAX_TEXTURE_STAGES && state < TSS_COUNT) {
        m_TextureStageStates[stage][state] = value;
    }
}

unsigned int D3D11Device::GetTextureStageState(unsigned int stage, TextureStageStateType state) const
{
    if (stage < D3D11_MAX_TEXTURE_STAGES && state < TSS_COUNT) {
        return m_TextureStageStates[stage][state];
    }
    return 0;
}

void D3D11Device::SetTexture(unsigned int stage, TextureClass* texture)
{
    // TODO: Convert TextureClass to ID3D11ShaderResourceView and bind
}

void D3D11Device::SetLight(unsigned int index, const RenderLight* light)
{
    if (index >= D3D11_MAX_LIGHTS) return;

    if (light) {
        m_LightConstants.LightType[index] = static_cast<int>(light->Type);
        m_LightConstants.LightPosition[index] = Vector4(light->Position.X, light->Position.Y, light->Position.Z, 1.0f);
        m_LightConstants.LightDirection[index] = Vector4(light->Direction.X, light->Direction.Y, light->Direction.Z, 0.0f);
        m_LightConstants.LightDiffuse[index] = light->Diffuse;
        m_LightConstants.LightAmbient[index] = light->Ambient;
        m_LightConstants.LightSpecular[index] = light->Specular;
        m_LightConstants.LightAttenuation[index] = Vector4(light->Attenuation0, light->Attenuation1, light->Attenuation2, light->Range);
        m_LightConstants.LightEnabled[index] = 1;
    } else {
        m_LightConstants.LightEnabled[index] = 0;
    }
    m_LightsDirty = true;
}

void D3D11Device::EnableLight(unsigned int index, bool enable)
{
    if (index < D3D11_MAX_LIGHTS) {
        m_LightConstants.LightEnabled[index] = enable ? 1 : 0;
        m_LightsDirty = true;
    }
}

void D3D11Device::SetMaterial(const RenderMaterial& material)
{
    m_MaterialConstants.Diffuse = material.Diffuse;
    m_MaterialConstants.Ambient = material.Ambient;
    m_MaterialConstants.Specular = material.Specular;
    m_MaterialConstants.Emissive = material.Emissive;
    m_MaterialConstants.SpecularPower = material.Power;
    m_MaterialDirty = true;
}

void D3D11Device::SetVertexBuffer(VertexBufferClass* vb)
{
    // TODO: Convert VertexBufferClass to ID3D11Buffer and bind
}

void D3D11Device::SetIndexBuffer(IndexBufferClass* ib, unsigned int baseOffset)
{
    // TODO: Convert IndexBufferClass to ID3D11Buffer and bind
}

D3D11_PRIMITIVE_TOPOLOGY D3D11Device::ConvertPrimitiveType(PrimitiveType type) const
{
    switch (type) {
        case PRIMITIVE_POINTLIST:     return D3D11_PRIMITIVE_TOPOLOGY_POINTLIST;
        case PRIMITIVE_LINELIST:      return D3D11_PRIMITIVE_TOPOLOGY_LINELIST;
        case PRIMITIVE_LINESTRIP:     return D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP;
        case PRIMITIVE_TRIANGLELIST:  return D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
        case PRIMITIVE_TRIANGLESTRIP: return D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
        default:                      return D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    }
}

void D3D11Device::DrawPrimitive(PrimitiveType type, unsigned int startVertex, unsigned int primitiveCount)
{
    UpdateConstantBuffers();
    ApplyRenderStates();

    m_Context->IASetPrimitiveTopology(ConvertPrimitiveType(type));

    unsigned int vertexCount = 0;
    switch (type) {
        case PRIMITIVE_POINTLIST:     vertexCount = primitiveCount; break;
        case PRIMITIVE_LINELIST:      vertexCount = primitiveCount * 2; break;
        case PRIMITIVE_LINESTRIP:     vertexCount = primitiveCount + 1; break;
        case PRIMITIVE_TRIANGLELIST:  vertexCount = primitiveCount * 3; break;
        case PRIMITIVE_TRIANGLESTRIP: vertexCount = primitiveCount + 2; break;
        case PRIMITIVE_TRIANGLEFAN:   vertexCount = primitiveCount + 2; break;
    }

    m_Context->Draw(vertexCount, startVertex);
    m_DrawCalls++;
    m_PrimitivesDrawn += primitiveCount;
}

void D3D11Device::DrawIndexedPrimitive(PrimitiveType type, unsigned int startIndex,
                                        unsigned int primitiveCount, unsigned int minVertexIndex,
                                        unsigned int numVertices)
{
    UpdateConstantBuffers();
    ApplyRenderStates();

    m_Context->IASetPrimitiveTopology(ConvertPrimitiveType(type));

    unsigned int indexCount = 0;
    switch (type) {
        case PRIMITIVE_POINTLIST:     indexCount = primitiveCount; break;
        case PRIMITIVE_LINELIST:      indexCount = primitiveCount * 2; break;
        case PRIMITIVE_LINESTRIP:     indexCount = primitiveCount + 1; break;
        case PRIMITIVE_TRIANGLELIST:  indexCount = primitiveCount * 3; break;
        case PRIMITIVE_TRIANGLESTRIP: indexCount = primitiveCount + 2; break;
        case PRIMITIVE_TRIANGLEFAN:   indexCount = primitiveCount + 2; break;
    }

    m_Context->DrawIndexed(indexCount, startIndex, minVertexIndex);
    m_DrawCalls++;
    m_PrimitivesDrawn += primitiveCount;
}

void D3D11Device::SetFog(bool enable, unsigned int color, float start, float end)
{
    m_FogConstants.FogEnabled = enable ? 1 : 0;
    m_FogConstants.FogColor.X = ((color >> 16) & 0xFF) / 255.0f;
    m_FogConstants.FogColor.Y = ((color >> 8) & 0xFF) / 255.0f;
    m_FogConstants.FogColor.Z = (color & 0xFF) / 255.0f;
    m_FogConstants.FogColor.W = 1.0f;
    m_FogConstants.FogStart = start;
    m_FogConstants.FogEnd = end;
    m_FogDirty = true;
}

void D3D11Device::SetGamma(float gamma, float brightness, float contrast)
{
    // TODO: Implement gamma ramp adjustment via DXGI
}

void D3D11Device::ResetStatistics()
{
    m_DrawCalls = 0;
    m_PrimitivesDrawn = 0;
}

void D3D11Device::UpdateConstantBuffers()
{
    D3D11_MAPPED_SUBRESOURCE mappedResource;

    if (m_TransformsDirty) {
        // Calculate combined matrix
        m_TransformConstants.WorldViewProjection =
            m_TransformConstants.World * m_TransformConstants.View * m_TransformConstants.Projection;

        HRESULT hr = m_Context->Map(m_TransformBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
        if (SUCCEEDED(hr)) {
            memcpy(mappedResource.pData, &m_TransformConstants, sizeof(m_TransformConstants));
            m_Context->Unmap(m_TransformBuffer.Get(), 0);
        }
        m_TransformsDirty = false;
    }

    if (m_MaterialDirty) {
        HRESULT hr = m_Context->Map(m_MaterialBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
        if (SUCCEEDED(hr)) {
            memcpy(mappedResource.pData, &m_MaterialConstants, sizeof(m_MaterialConstants));
            m_Context->Unmap(m_MaterialBuffer.Get(), 0);
        }
        m_MaterialDirty = false;
    }

    if (m_LightsDirty) {
        // Count enabled lights
        m_LightConstants.NumLights = 0;
        for (int i = 0; i < D3D11_MAX_LIGHTS; i++) {
            if (m_LightConstants.LightEnabled[i]) {
                m_LightConstants.NumLights++;
            }
        }

        HRESULT hr = m_Context->Map(m_LightBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
        if (SUCCEEDED(hr)) {
            memcpy(mappedResource.pData, &m_LightConstants, sizeof(m_LightConstants));
            m_Context->Unmap(m_LightBuffer.Get(), 0);
        }
        m_LightsDirty = false;
    }

    if (m_FogDirty) {
        HRESULT hr = m_Context->Map(m_FogBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
        if (SUCCEEDED(hr)) {
            memcpy(mappedResource.pData, &m_FogConstants, sizeof(m_FogConstants));
            m_Context->Unmap(m_FogBuffer.Get(), 0);
        }
        m_FogDirty = false;
    }

    // Bind constant buffers to both VS and PS
    ID3D11Buffer* vsBuffers[] = { m_TransformBuffer.Get(), m_MaterialBuffer.Get(), m_LightBuffer.Get(), m_FogBuffer.Get() };
    m_Context->VSSetConstantBuffers(0, 4, vsBuffers);
    m_Context->PSSetConstantBuffers(0, 4, vsBuffers);
}

void D3D11Device::ApplyRenderStates()
{
    // TODO: Convert abstract render states to D3D11 state objects and apply
    // This would use the state cache to avoid redundant state object creation
}
