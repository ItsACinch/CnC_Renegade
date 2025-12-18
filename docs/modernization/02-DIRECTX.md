# Phase 2: DirectX Modernization

## Objective

Migrate from DirectX 8 fixed-function pipeline to DirectX 11 (or optionally SDL2+OpenGL for broader compatibility).

## Current DirectX 8 Usage

### Components Used
| Component | Version | Purpose |
|-----------|---------|---------|
| Direct3D | 8.0 | 3D rendering |
| DirectInput | 8.0 | Keyboard, mouse, joystick |
| DirectSound | 8.0 | Audio (via Miles) |
| DirectDraw | 7.0 | Legacy 2D surfaces |

### Key Wrapper Files
```
Code/ww3d2/
├── dx8wrapper.h/cpp      # Main D3D8 device wrapper (~1220 lines)
├── dx8caps.h/cpp         # Hardware capability detection
├── dx8fvf.h/cpp          # Flexible Vertex Format handling
├── dx8renderer.h/cpp     # Batch rendering system
├── dx8vertexbuffer.h/cpp # Vertex buffer management
├── dx8indexbuffer.h/cpp  # Index buffer management
├── dx8texman.h/cpp       # Texture management
└── (15+ more dx8*.cpp files)

Code/Combat/
└── directinput.h/cpp     # Input device handling
```

### D3D8-Specific APIs Requiring Replacement

| D3D8 API | D3D11 Equivalent | Notes |
|----------|------------------|-------|
| `IDirect3D8` | `IDXGIFactory` | Device enumeration |
| `IDirect3DDevice8` | `ID3D11Device` + `ID3D11DeviceContext` | Split architecture |
| `CopyRects()` | `CopyResource()` / `CopySubresourceRegion()` | Surface copying |
| `CreateAdditionalSwapChain()` | `IDXGIFactory::CreateSwapChain()` | Multi-window |
| `SetRenderState()` | Rasterizer/Blend/DepthStencil State objects | State management |
| `SetTextureStageState()` | Shaders + Sampler State | Texture operations |
| `SetTransform()` | Constant buffers | Matrix transforms |
| `D3DFVF_*` | Input Layouts + Vertex Shaders | Vertex formats |
| `D3DLIGHT8` | Shaders | Lighting |
| `D3DMATERIAL8` | Shaders | Materials |

## Migration Strategy

### Option A: Direct3D 11 (Recommended)
- Native Windows support
- Well-documented migration path
- Good tooling (PIX, RenderDoc)
- Requires shader implementation

### Option B: SDL2 + OpenGL
- Cross-platform potential
- OpenGL 2.1 can emulate fixed-function
- More work for Windows-specific features

### Recommendation: Direct3D 11

## Implementation Plan

### 2.1 Create Abstraction Layer

Create a renderer abstraction that both D3D8 (legacy) and D3D11 can implement:

```cpp
// Code/ww3d2/RenderDevice.h
class IRenderDevice {
public:
    virtual ~IRenderDevice() = default;

    // Initialization
    virtual bool Initialize(HWND hwnd, int width, int height, bool fullscreen) = 0;
    virtual void Shutdown() = 0;

    // Frame management
    virtual void BeginFrame() = 0;
    virtual void EndFrame() = 0;
    virtual void Present() = 0;

    // State management
    virtual void SetRenderState(RenderState state, uint32_t value) = 0;
    virtual void SetTransform(TransformType type, const Matrix4& matrix) = 0;

    // Resource creation
    virtual IVertexBuffer* CreateVertexBuffer(const VertexBufferDesc& desc) = 0;
    virtual IIndexBuffer* CreateIndexBuffer(const IndexBufferDesc& desc) = 0;
    virtual ITexture* CreateTexture(const TextureDesc& desc) = 0;

    // Drawing
    virtual void DrawPrimitive(PrimitiveType type, uint32_t startVertex, uint32_t count) = 0;
    virtual void DrawIndexedPrimitive(PrimitiveType type, uint32_t startIndex, uint32_t count) = 0;
};
```

### 2.2 Implement Fixed-Function Shader Emulation

The D3D8 fixed-function pipeline must be emulated with shaders:

```hlsl
// shaders/FixedFunction.hlsl

cbuffer TransformBuffer : register(b0) {
    matrix World;
    matrix View;
    matrix Projection;
};

cbuffer MaterialBuffer : register(b1) {
    float4 Diffuse;
    float4 Ambient;
    float4 Specular;
    float4 Emissive;
    float SpecularPower;
};

cbuffer LightBuffer : register(b2) {
    float4 LightDirection[8];
    float4 LightDiffuse[8];
    float4 LightAmbient[8];
    int NumLights;
};

struct VS_INPUT {
    float3 Position : POSITION;
    float3 Normal : NORMAL;
    float2 TexCoord : TEXCOORD0;
    float4 Color : COLOR0;
};

struct PS_INPUT {
    float4 Position : SV_POSITION;
    float4 Color : COLOR0;
    float2 TexCoord : TEXCOORD0;
};
```

### 2.3 Create FVF to Input Layout Converter

Map D3D8 FVF codes to D3D11 input layouts:

```cpp
// Code/ww3d2/dx11inputlayout.cpp

D3D11_INPUT_ELEMENT_DESC* ConvertFVFToInputLayout(DWORD fvf, UINT& elementCount) {
    std::vector<D3D11_INPUT_ELEMENT_DESC> elements;
    UINT offset = 0;

    if (fvf & D3DFVF_XYZ) {
        elements.push_back({"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offset, ...});
        offset += 12;
    }
    if (fvf & D3DFVF_NORMAL) {
        elements.push_back({"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offset, ...});
        offset += 12;
    }
    if (fvf & D3DFVF_DIFFUSE) {
        elements.push_back({"COLOR", 0, DXGI_FORMAT_B8G8R8A8_UNORM, 0, offset, ...});
        offset += 4;
    }
    // ... continue for all FVF components

    elementCount = elements.size();
    return CopyToArray(elements);
}
```

### 2.4 Texture Format Mapping

```cpp
DXGI_FORMAT ConvertD3D8Format(D3DFORMAT d3d8Format) {
    switch (d3d8Format) {
        case D3DFMT_A8R8G8B8: return DXGI_FORMAT_B8G8R8A8_UNORM;
        case D3DFMT_X8R8G8B8: return DXGI_FORMAT_B8G8R8X8_UNORM;
        case D3DFMT_R5G6B5:   return DXGI_FORMAT_B5G6R5_UNORM;
        case D3DFMT_A1R5G5B5: return DXGI_FORMAT_B5G5R5A1_UNORM;
        case D3DFMT_DXT1:     return DXGI_FORMAT_BC1_UNORM;
        case D3DFMT_DXT3:     return DXGI_FORMAT_BC2_UNORM;
        case D3DFMT_DXT5:     return DXGI_FORMAT_BC3_UNORM;
        case D3DFMT_D16:      return DXGI_FORMAT_D16_UNORM;
        case D3DFMT_D24S8:    return DXGI_FORMAT_D24_UNORM_S8_UINT;
        default:              return DXGI_FORMAT_UNKNOWN;
    }
}
```

### 2.5 DirectInput Replacement

Replace DirectInput with Raw Input or XInput:

```cpp
// Code/Combat/rawinput.cpp

class RawInputManager {
public:
    bool Initialize(HWND hwnd);
    void ProcessMessage(UINT msg, WPARAM wParam, LPARAM lParam);

    // Keyboard
    bool IsKeyDown(int vkey) const;
    bool WasKeyPressed(int vkey) const;

    // Mouse
    void GetMouseDelta(int& dx, int& dy) const;
    void GetMousePosition(int& x, int& y) const;
    bool IsMouseButtonDown(int button) const;

    // Joystick (via XInput for Xbox controllers)
    bool IsControllerConnected(int index) const;
    void GetControllerState(int index, ControllerState& state) const;
};
```

### 2.6 State Block Conversion

D3D8 uses per-call state changes; D3D11 uses immutable state objects:

```cpp
// Create state object cache
class D3D11StateCache {
    std::unordered_map<RasterizerStateDesc, ID3D11RasterizerState*> rasterizerStates;
    std::unordered_map<BlendStateDesc, ID3D11BlendState*> blendStates;
    std::unordered_map<DepthStencilStateDesc, ID3D11DepthStencilState*> depthStates;
    std::unordered_map<SamplerStateDesc, ID3D11SamplerState*> samplerStates;

public:
    ID3D11RasterizerState* GetRasterizerState(const RasterizerStateDesc& desc);
    ID3D11BlendState* GetBlendState(const BlendStateDesc& desc);
    // ...
};
```

## File Changes Required

| Original File | Action | New File |
|--------------|--------|----------|
| dx8wrapper.h/cpp | Rewrite | dx11wrapper.h/cpp |
| dx8caps.h/cpp | Rewrite | dx11caps.h/cpp |
| dx8fvf.h/cpp | Replace | dx11inputlayout.h/cpp |
| dx8renderer.h/cpp | Adapt | dx11renderer.h/cpp |
| dx8vertexbuffer.h/cpp | Rewrite | dx11vertexbuffer.h/cpp |
| dx8indexbuffer.h/cpp | Rewrite | dx11indexbuffer.h/cpp |
| directinput.h/cpp | Replace | rawinput.h/cpp |
| dsurface.h/cpp | Remove | (use textures) |
| ddraw.cpp | Remove | - |

## Shader Requirements

| Shader | Purpose |
|--------|---------|
| FixedFunction_VS.hlsl | Emulate D3D8 T&L pipeline |
| FixedFunction_PS.hlsl | Emulate texture stages |
| SkinMesh_VS.hlsl | Skeletal animation |
| Terrain_VS/PS.hlsl | Terrain rendering |
| Water_VS/PS.hlsl | Water effects |
| Particle_VS/PS.hlsl | Particle systems |

## Verification

1. Render test scenes matching D3D8 output
2. Compare screenshots pixel-by-pixel
3. Verify all blend modes work correctly
4. Test all texture formats load properly
5. Confirm skeletal animation works
6. Validate particle effects

## Estimated Effort

- Abstraction layer: 3-4 days
- D3D11 implementation: 10-15 days
- Shader writing: 5-7 days
- Input system: 2-3 days
- Testing/debugging: 5-7 days
- **Total: 25-36 days**

## Risks

1. **Visual differences**: Shader emulation may not be pixel-perfect
2. **Performance**: State object creation overhead
3. **Missing features**: Some D3D8 features have no D3D11 equivalent
4. **Driver bugs**: Different GPU vendors may behave differently
