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
 ***                           R E N D E R   D E V I C E                                     ***
 ***********************************************************************************************
 *                                                                                             *
 *  Phase 2 Modernization: Renderer Abstraction Layer                                         *
 *                                                                                             *
 *  This interface abstracts the rendering backend, allowing both legacy D3D8 and modern      *
 *  D3D11 implementations. The goal is to minimize changes to existing game code while        *
 *  enabling a modern rendering backend.                                                      *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*/

#ifndef RENDER_DEVICE_H
#define RENDER_DEVICE_H

#include "always.h"
#include "matrix4.h"
#include "matrix3d.h"
#include "vector3.h"
#include "vector4.h"
#include <cstring>  // For memset

// Forward declarations
class TextureClass;
class VertexBufferClass;
class IndexBufferClass;
class VertexMaterialClass;
class ShaderClass;
class LightClass;
class SurfaceClass;

/*
** Render device capabilities
*/
struct RenderDeviceCaps
{
    // Texture capabilities
    unsigned int MaxTextureWidth;
    unsigned int MaxTextureHeight;
    unsigned int MaxTextureStages;
    bool SupportsNonPow2Textures;
    bool SupportsDXT1;
    bool SupportsDXT3;
    bool SupportsDXT5;

    // Vertex processing
    bool SupportsHardwareTnL;
    unsigned int MaxVertexBlendMatrices;

    // Other capabilities
    bool SupportsZBias;
    bool SupportsFogTable;
    bool SupportsStencil;
    unsigned int MaxAnisotropy;

    RenderDeviceCaps() { memset(this, 0, sizeof(*this)); }
};

/*
** Primitive types for drawing
*/
enum PrimitiveType
{
    PRIMITIVE_POINTLIST = 1,
    PRIMITIVE_LINELIST = 2,
    PRIMITIVE_LINESTRIP = 3,
    PRIMITIVE_TRIANGLELIST = 4,
    PRIMITIVE_TRIANGLESTRIP = 5,
    PRIMITIVE_TRIANGLEFAN = 6
};

/*
** Transform types
*/
enum TransformType
{
    TRANSFORM_WORLD = 0,
    TRANSFORM_VIEW = 1,
    TRANSFORM_PROJECTION = 2,
    TRANSFORM_TEXTURE0 = 16,
    TRANSFORM_TEXTURE1 = 17,
    TRANSFORM_TEXTURE2 = 18,
    TRANSFORM_TEXTURE3 = 19
};

/*
** Render states (abstracted from D3D render states)
*/
enum RenderStateType
{
    RS_ZENABLE,
    RS_ZWRITEENABLE,
    RS_ZFUNC,
    RS_ALPHATESTENABLE,
    RS_ALPHAREF,
    RS_ALPHAFUNC,
    RS_ALPHABLENDENABLE,
    RS_SRCBLEND,
    RS_DESTBLEND,
    RS_CULLMODE,
    RS_FILLMODE,
    RS_SHADEMODE,
    RS_LIGHTING,
    RS_SPECULARENABLE,
    RS_FOGENABLE,
    RS_FOGCOLOR,
    RS_FOGSTART,
    RS_FOGEND,
    RS_FOGDENSITY,
    RS_FOGTABLEMODE,
    RS_FOGVERTEXMODE,
    RS_STENCILENABLE,
    RS_STENCILFUNC,
    RS_STENCILREF,
    RS_STENCILMASK,
    RS_STENCILWRITEMASK,
    RS_STENCILFAIL,
    RS_STENCILZFAIL,
    RS_STENCILPASS,
    RS_DITHERENABLE,
    RS_COLORWRITEENABLE,
    RS_ZBIAS,
    RS_COUNT
};

/*
** Texture stage states
*/
enum TextureStageStateType
{
    TSS_COLOROP,
    TSS_COLORARG1,
    TSS_COLORARG2,
    TSS_ALPHAOP,
    TSS_ALPHAARG1,
    TSS_ALPHAARG2,
    TSS_TEXCOORDINDEX,
    TSS_ADDRESSU,
    TSS_ADDRESSV,
    TSS_MAGFILTER,
    TSS_MINFILTER,
    TSS_MIPFILTER,
    TSS_MIPMAPLODBIAS,
    TSS_MAXMIPLEVEL,
    TSS_MAXANISOTROPY,
    TSS_TEXTURETRANSFORMFLAGS,
    TSS_COUNT
};

/*
** Light structure (abstracted from D3DLIGHT8)
*/
struct RenderLight
{
    enum LightType { LIGHT_POINT, LIGHT_SPOT, LIGHT_DIRECTIONAL };

    LightType Type;
    Vector4 Diffuse;
    Vector4 Specular;
    Vector4 Ambient;
    Vector3 Position;
    Vector3 Direction;
    float Range;
    float Falloff;
    float Attenuation0;
    float Attenuation1;
    float Attenuation2;
    float Theta;
    float Phi;

    RenderLight() { memset(this, 0, sizeof(*this)); Type = LIGHT_DIRECTIONAL; }
};

/*
** Material structure (abstracted from D3DMATERIAL8)
*/
struct RenderMaterial
{
    Vector4 Diffuse;
    Vector4 Ambient;
    Vector4 Specular;
    Vector4 Emissive;
    float Power;

    RenderMaterial() { memset(this, 0, sizeof(*this)); }
};

/*
** Viewport structure
*/
struct RenderViewport
{
    unsigned int X;
    unsigned int Y;
    unsigned int Width;
    unsigned int Height;
    float MinZ;
    float MaxZ;

    RenderViewport() : X(0), Y(0), Width(0), Height(0), MinZ(0.0f), MaxZ(1.0f) {}
};

/*
** Texture format enumeration
*/
enum RenderTextureFormat
{
    TEXFMT_UNKNOWN = 0,
    TEXFMT_A8R8G8B8,
    TEXFMT_X8R8G8B8,
    TEXFMT_R5G6B5,
    TEXFMT_A1R5G5B5,
    TEXFMT_A4R4G4B4,
    TEXFMT_A8,
    TEXFMT_L8,
    TEXFMT_A8L8,
    TEXFMT_DXT1,
    TEXFMT_DXT3,
    TEXFMT_DXT5,
    TEXFMT_D16,
    TEXFMT_D24S8,
    TEXFMT_D32
};

/*
** Clear flags
*/
enum ClearFlags
{
    CLEAR_TARGET = 1,
    CLEAR_ZBUFFER = 2,
    CLEAR_STENCIL = 4
};

/*
** IRenderDevice - Abstract rendering device interface
**
** This interface provides a graphics API-agnostic way to perform rendering.
** Implementations exist for D3D11 (and potentially legacy D3D8 for testing).
*/
class IRenderDevice
{
public:
    virtual ~IRenderDevice() {}

    // =========================================================================
    // Initialization
    // =========================================================================

    virtual bool Initialize(void* hwnd, int width, int height, bool fullscreen) = 0;
    virtual void Shutdown() = 0;
    virtual bool Reset(int width, int height, bool fullscreen) = 0;
    virtual bool IsInitialized() const = 0;
    virtual bool IsDeviceLost() const = 0;

    // =========================================================================
    // Device information
    // =========================================================================

    virtual const RenderDeviceCaps& GetCaps() const = 0;
    virtual const char* GetDeviceName() const = 0;
    virtual int GetWidth() const = 0;
    virtual int GetHeight() const = 0;
    virtual bool IsWindowed() const = 0;

    // =========================================================================
    // Frame management
    // =========================================================================

    virtual void BeginScene() = 0;
    virtual void EndScene() = 0;
    virtual void Present() = 0;
    virtual void Clear(unsigned int flags, unsigned int color, float z, unsigned int stencil) = 0;

    // =========================================================================
    // Viewport
    // =========================================================================

    virtual void SetViewport(const RenderViewport& viewport) = 0;
    virtual void GetViewport(RenderViewport& viewport) const = 0;

    // =========================================================================
    // Transforms
    // =========================================================================

    virtual void SetTransform(TransformType type, const Matrix4& matrix) = 0;
    virtual void GetTransform(TransformType type, Matrix4& matrix) const = 0;

    // =========================================================================
    // Render states
    // =========================================================================

    virtual void SetRenderState(RenderStateType state, unsigned int value) = 0;
    virtual unsigned int GetRenderState(RenderStateType state) const = 0;

    // =========================================================================
    // Texture stage states
    // =========================================================================

    virtual void SetTextureStageState(unsigned int stage, TextureStageStateType state, unsigned int value) = 0;
    virtual unsigned int GetTextureStageState(unsigned int stage, TextureStageStateType state) const = 0;

    // =========================================================================
    // Textures
    // =========================================================================

    virtual void SetTexture(unsigned int stage, TextureClass* texture) = 0;

    // =========================================================================
    // Lighting
    // =========================================================================

    virtual void SetLight(unsigned int index, const RenderLight* light) = 0;
    virtual void EnableLight(unsigned int index, bool enable) = 0;
    virtual void SetMaterial(const RenderMaterial& material) = 0;

    // =========================================================================
    // Vertex/Index buffers
    // =========================================================================

    virtual void SetVertexBuffer(VertexBufferClass* vb) = 0;
    virtual void SetIndexBuffer(IndexBufferClass* ib, unsigned int baseOffset) = 0;

    // =========================================================================
    // Drawing
    // =========================================================================

    virtual void DrawPrimitive(PrimitiveType type, unsigned int startVertex, unsigned int primitiveCount) = 0;
    virtual void DrawIndexedPrimitive(PrimitiveType type, unsigned int startIndex,
                                       unsigned int primitiveCount, unsigned int minVertexIndex,
                                       unsigned int numVertices) = 0;

    // =========================================================================
    // Fog
    // =========================================================================

    virtual void SetFog(bool enable, unsigned int color, float start, float end) = 0;

    // =========================================================================
    // Gamma
    // =========================================================================

    virtual void SetGamma(float gamma, float brightness, float contrast) = 0;

    // =========================================================================
    // Statistics
    // =========================================================================

    virtual unsigned int GetFrameCount() const = 0;
    virtual void ResetStatistics() = 0;
};

/*
** Factory function to create the appropriate render device
*/
IRenderDevice* CreateRenderDevice();

/*
** Global render device instance
*/
extern IRenderDevice* g_RenderDevice;

#endif // RENDER_DEVICE_H
