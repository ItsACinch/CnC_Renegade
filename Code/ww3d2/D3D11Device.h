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

/***********************************************************************************************
 ***                      D I R E C T 3 D   1 1   D E V I C E                                ***
 ***********************************************************************************************
 *                                                                                             *
 *  Phase 2 Modernization: Direct3D 11 Rendering Backend                                       *
 *                                                                                             *
 *  This class implements the IRenderDevice interface using Direct3D 11.                       *
 *  It emulates the D3D8 fixed-function pipeline using shaders.                                *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*/

#ifndef D3D11_DEVICE_H
#define D3D11_DEVICE_H

#include "RenderDevice.h"
#include <d3d11.h>
#include <dxgi.h>
#include <wrl/client.h>  // For ComPtr

using Microsoft::WRL::ComPtr;

// Maximum constants
const unsigned int D3D11_MAX_TEXTURE_STAGES = 8;
const unsigned int D3D11_MAX_LIGHTS = 8;

/*
** Constant buffer structures for shader emulation
*/
struct TransformConstants
{
    Matrix4 World;
    Matrix4 View;
    Matrix4 Projection;
    Matrix4 WorldViewProjection;
};

struct MaterialConstants
{
    Vector4 Diffuse;
    Vector4 Ambient;
    Vector4 Specular;
    Vector4 Emissive;
    float SpecularPower;
    float Padding[3];
};

struct LightConstants
{
    Vector4 LightDirection[D3D11_MAX_LIGHTS];
    Vector4 LightPosition[D3D11_MAX_LIGHTS];
    Vector4 LightDiffuse[D3D11_MAX_LIGHTS];
    Vector4 LightAmbient[D3D11_MAX_LIGHTS];
    Vector4 LightSpecular[D3D11_MAX_LIGHTS];
    Vector4 LightAttenuation[D3D11_MAX_LIGHTS];  // (const, linear, quadratic, range)
    int LightType[D3D11_MAX_LIGHTS];             // 0=directional, 1=point, 2=spot
    int LightEnabled[D3D11_MAX_LIGHTS];
    int NumLights;
    int Padding[3];
};

struct FogConstants
{
    Vector4 FogColor;
    float FogStart;
    float FogEnd;
    float FogDensity;
    int FogEnabled;
};

/*
** D3D11 render state cache
*/
class D3D11StateCache
{
public:
    D3D11StateCache();
    ~D3D11StateCache();

    void Initialize(ID3D11Device* device);
    void Shutdown();

    ID3D11RasterizerState* GetRasterizerState(D3D11_RASTERIZER_DESC& desc);
    ID3D11BlendState* GetBlendState(D3D11_BLEND_DESC& desc);
    ID3D11DepthStencilState* GetDepthStencilState(D3D11_DEPTH_STENCIL_DESC& desc);
    ID3D11SamplerState* GetSamplerState(D3D11_SAMPLER_DESC& desc);

private:
    ID3D11Device* m_Device;
    // State caches would use hash maps in full implementation
};

/*
** D3D11Device - Direct3D 11 implementation of IRenderDevice
*/
class D3D11Device : public IRenderDevice
{
public:
    D3D11Device();
    virtual ~D3D11Device();

    // =========================================================================
    // IRenderDevice Implementation
    // =========================================================================

    // Initialization
    virtual bool Initialize(void* hwnd, int width, int height, bool fullscreen) override;
    virtual void Shutdown() override;
    virtual bool Reset(int width, int height, bool fullscreen) override;
    virtual bool IsInitialized() const override { return m_Initialized; }
    virtual bool IsDeviceLost() const override { return m_DeviceLost; }

    // Device information
    virtual const RenderDeviceCaps& GetCaps() const override { return m_Caps; }
    virtual const char* GetDeviceName() const override { return m_DeviceName; }
    virtual int GetWidth() const override { return m_Width; }
    virtual int GetHeight() const override { return m_Height; }
    virtual bool IsWindowed() const override { return m_Windowed; }

    // Frame management
    virtual void BeginScene() override;
    virtual void EndScene() override;
    virtual void Present() override;
    virtual void Clear(unsigned int flags, unsigned int color, float z, unsigned int stencil) override;

    // Viewport
    virtual void SetViewport(const RenderViewport& viewport) override;
    virtual void GetViewport(RenderViewport& viewport) const override;

    // Transforms
    virtual void SetTransform(TransformType type, const Matrix4& matrix) override;
    virtual void GetTransform(TransformType type, Matrix4& matrix) const override;

    // Render states
    virtual void SetRenderState(RenderStateType state, unsigned int value) override;
    virtual unsigned int GetRenderState(RenderStateType state) const override;

    // Texture stage states
    virtual void SetTextureStageState(unsigned int stage, TextureStageStateType state, unsigned int value) override;
    virtual unsigned int GetTextureStageState(unsigned int stage, TextureStageStateType state) const override;

    // Textures
    virtual void SetTexture(unsigned int stage, TextureClass* texture) override;

    // Lighting
    virtual void SetLight(unsigned int index, const RenderLight* light) override;
    virtual void EnableLight(unsigned int index, bool enable) override;
    virtual void SetMaterial(const RenderMaterial& material) override;

    // Vertex/Index buffers
    virtual void SetVertexBuffer(VertexBufferClass* vb) override;
    virtual void SetIndexBuffer(IndexBufferClass* ib, unsigned int baseOffset) override;

    // Drawing
    virtual void DrawPrimitive(PrimitiveType type, unsigned int startVertex, unsigned int primitiveCount) override;
    virtual void DrawIndexedPrimitive(PrimitiveType type, unsigned int startIndex,
                                       unsigned int primitiveCount, unsigned int minVertexIndex,
                                       unsigned int numVertices) override;

    // Fog
    virtual void SetFog(bool enable, unsigned int color, float start, float end) override;

    // Gamma
    virtual void SetGamma(float gamma, float brightness, float contrast) override;

    // Statistics
    virtual unsigned int GetFrameCount() const override { return m_FrameCount; }
    virtual void ResetStatistics() override;

    // =========================================================================
    // D3D11-specific accessors (for internal use)
    // =========================================================================

    ID3D11Device* GetDevice() const { return m_Device.Get(); }
    ID3D11DeviceContext* GetContext() const { return m_Context.Get(); }

private:
    // Internal helpers
    bool CreateDevice(HWND hwnd);
    bool CreateSwapChain(HWND hwnd, int width, int height, bool fullscreen);
    bool CreateRenderTargetView();
    bool CreateDepthStencilView(int width, int height);
    bool CreateConstantBuffers();
    bool LoadShaders();
    void UpdateConstantBuffers();
    void ApplyRenderStates();

    D3D11_PRIMITIVE_TOPOLOGY ConvertPrimitiveType(PrimitiveType type) const;

    // Device state
    bool m_Initialized;
    bool m_DeviceLost;
    bool m_InScene;
    int m_Width;
    int m_Height;
    bool m_Windowed;
    char m_DeviceName[256];
    RenderDeviceCaps m_Caps;

    // D3D11 objects
    ComPtr<ID3D11Device> m_Device;
    ComPtr<ID3D11DeviceContext> m_Context;
    ComPtr<IDXGISwapChain> m_SwapChain;
    ComPtr<ID3D11RenderTargetView> m_RenderTargetView;
    ComPtr<ID3D11DepthStencilView> m_DepthStencilView;
    ComPtr<ID3D11Texture2D> m_DepthStencilBuffer;

    // Constant buffers
    ComPtr<ID3D11Buffer> m_TransformBuffer;
    ComPtr<ID3D11Buffer> m_MaterialBuffer;
    ComPtr<ID3D11Buffer> m_LightBuffer;
    ComPtr<ID3D11Buffer> m_FogBuffer;

    // Shaders (fixed-function emulation)
    ComPtr<ID3D11VertexShader> m_FixedFunctionVS;
    ComPtr<ID3D11PixelShader> m_FixedFunctionPS;
    ComPtr<ID3D11InputLayout> m_DefaultInputLayout;

    // Current state
    TransformConstants m_TransformConstants;
    MaterialConstants m_MaterialConstants;
    LightConstants m_LightConstants;
    FogConstants m_FogConstants;
    bool m_TransformsDirty;
    bool m_MaterialDirty;
    bool m_LightsDirty;
    bool m_FogDirty;

    // Render state tracking
    unsigned int m_RenderStates[RS_COUNT];
    unsigned int m_TextureStageStates[D3D11_MAX_TEXTURE_STAGES][TSS_COUNT];
    RenderViewport m_Viewport;

    // State cache
    D3D11StateCache m_StateCache;

    // Statistics
    unsigned int m_FrameCount;
    unsigned int m_DrawCalls;
    unsigned int m_PrimitivesDrawn;
};

#endif // D3D11_DEVICE_H
