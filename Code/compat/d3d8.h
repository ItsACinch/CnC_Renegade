/*
**  DirectX 8 to DirectX 9 Compatibility Header
**
**  This header provides D3D8 type definitions using the D3D9 SDK.
**  D3D9 maintains backward compatibility with most D3D8 interfaces and types.
**
**  Note: The June 2010 DirectX SDK includes D3D9 but not D3D8 headers.
**  This compatibility layer maps D3D8 names to their D3D9 equivalents.
**
**  IMPORTANT: This header MUST be included before any D3D8 forward declarations.
**  The compat directory is added to the include path globally in CMake.
*/

#ifndef _D3D8_COMPAT_H_
#define _D3D8_COMPAT_H_

// ============================================================================
// Interface name mappings (D3D8 -> D3D9) using macros
// These MUST be defined before including d3d9.h so that forward declarations
// like "struct IDirect3DTexture8;" become "struct IDirect3DTexture9;"
// ============================================================================

#ifndef IDirect3D8
#define IDirect3D8                  IDirect3D9
#endif
#ifndef IDirect3DDevice8
#define IDirect3DDevice8            IDirect3DDevice9
#endif
#ifndef IDirect3DResource8
#define IDirect3DResource8          IDirect3DResource9
#endif
#ifndef IDirect3DBaseTexture8
#define IDirect3DBaseTexture8       IDirect3DBaseTexture9
#endif
#ifndef IDirect3DTexture8
#define IDirect3DTexture8           IDirect3DTexture9
#endif
#ifndef IDirect3DCubeTexture8
#define IDirect3DCubeTexture8       IDirect3DCubeTexture9
#endif
#ifndef IDirect3DVolumeTexture8
#define IDirect3DVolumeTexture8     IDirect3DVolumeTexture9
#endif
#ifndef IDirect3DVertexBuffer8
#define IDirect3DVertexBuffer8      IDirect3DVertexBuffer9
#endif
#ifndef IDirect3DIndexBuffer8
#define IDirect3DIndexBuffer8       IDirect3DIndexBuffer9
#endif
#ifndef IDirect3DSurface8
#define IDirect3DSurface8           IDirect3DSurface9
#endif
#ifndef IDirect3DVolume8
#define IDirect3DVolume8            IDirect3DVolume9
#endif
#ifndef IDirect3DSwapChain8
#define IDirect3DSwapChain8         IDirect3DSwapChain9
#endif
#ifndef IDirect3DVertexDeclaration8
#define IDirect3DVertexDeclaration8 IDirect3DVertexDeclaration9
#endif
#ifndef IDirect3DVertexShader8
#define IDirect3DVertexShader8      IDirect3DVertexShader9
#endif
#ifndef IDirect3DPixelShader8
#define IDirect3DPixelShader8       IDirect3DPixelShader9
#endif

// ============================================================================
// Structure type mappings (using macros for forward declaration compatibility)
// ============================================================================

#ifndef D3DCAPS8
#define D3DCAPS8                    D3DCAPS9
#endif
#ifndef _D3DCAPS8
#define _D3DCAPS8                   _D3DCAPS9
#endif
#ifndef D3DPRESENT_PARAMETERS8
#define D3DPRESENT_PARAMETERS8      D3DPRESENT_PARAMETERS
#endif
#ifndef D3DADAPTER_IDENTIFIER8
#define D3DADAPTER_IDENTIFIER8      D3DADAPTER_IDENTIFIER9
#endif
#ifndef D3DDISPLAYMODE8
#define D3DDISPLAYMODE8             D3DDISPLAYMODE
#endif
#ifndef D3DLOCKED_RECT8
#define D3DLOCKED_RECT8             D3DLOCKED_RECT
#endif
#ifndef D3DLOCKED_BOX8
#define D3DLOCKED_BOX8              D3DLOCKED_BOX
#endif
#ifndef D3DMATERIAL8
#define D3DMATERIAL8                D3DMATERIAL9
#endif
#ifndef _D3DMATERIAL8
#define _D3DMATERIAL8               _D3DMATERIAL9
#endif
#ifndef D3DLIGHT8
#define D3DLIGHT8                   D3DLIGHT9
#endif
#ifndef _D3DLIGHT8
#define _D3DLIGHT8                  _D3DLIGHT9
#endif
#ifndef D3DVIEWPORT8
#define D3DVIEWPORT8                D3DVIEWPORT9
#endif
#ifndef D3DCLIPSTATUS8
#define D3DCLIPSTATUS8              D3DCLIPSTATUS9
#endif
#ifndef D3DGAMMARAMP8
#define D3DGAMMARAMP8               D3DGAMMARAMP
#endif
#ifndef D3DVERTEXELEMENT8
#define D3DVERTEXELEMENT8           D3DVERTEXELEMENT9
#endif
#ifndef D3DSURFACE_DESC8
#define D3DSURFACE_DESC8            D3DSURFACE_DESC
#endif
#ifndef D3DVOLUME_DESC8
#define D3DVOLUME_DESC8             D3DVOLUME_DESC
#endif
#ifndef D3DRASTER_STATUS8
#define D3DRASTER_STATUS8           D3DRASTER_STATUS
#endif

// ============================================================================
// Include the actual DirectX 9 headers from the SDK
// ============================================================================

#include <d3d9.h>
#include <d3d9types.h>
#include <d3d9caps.h>

// ============================================================================
// Function mappings
// ============================================================================

// Direct3DCreate8 -> Direct3DCreate9
#ifndef Direct3DCreate8
#define Direct3DCreate8(SDKVersion) Direct3DCreate9(D3D_SDK_VERSION)
#endif

// D3D8 SDK version constant
#ifndef D3D_SDK_VERSION_DX8
#define D3D_SDK_VERSION_DX8 220
#endif

// ============================================================================
// D3D8-specific render states that were removed or renamed in D3D9
// ============================================================================

// D3DRS_ZBIAS was removed in D3D9 (use D3DRS_DEPTHBIAS instead, but needs different handling)
// For compatibility, we define it as an enum cast to avoid type mismatch errors
#ifndef D3DRS_ZBIAS
#define D3DRS_ZBIAS ((D3DRENDERSTATETYPE)999)  // D3D9 ignores; value 7 = ZENABLE which breaks z-buffer!
#endif

// D3DRS_SOFTWAREVERTEXPROCESSING existed in D3D8 but was removed in D3D9
// In D3D9, software vertex processing is set at device creation time only
#ifndef D3DRS_SOFTWAREVERTEXPROCESSING
#define D3DRS_SOFTWAREVERTEXPROCESSING ((D3DRENDERSTATETYPE)153)  // Original D3D8 value
#endif

// D3DRS_PATCHSEGMENTS existed in D3D8 but was removed in D3D9
// In D3D9, this was replaced by SetNPatchMode()
#ifndef D3DRS_PATCHSEGMENTS
#define D3DRS_PATCHSEGMENTS ((D3DRENDERSTATETYPE)164)  // Original D3D8 value
#endif

// ============================================================================
// D3D8-specific capability flags removed in D3D9
// ============================================================================

// D3DPRASTERCAPS_ZBIAS was removed in D3D9
#ifndef D3DPRASTERCAPS_ZBIAS
#define D3DPRASTERCAPS_ZBIAS 0x00004000L
#endif

// D3DENUM_NO_WHQL_LEVEL was a D3D8 flag for GetAdapterIdentifier
// In D3D9, just pass 0 for flags
#ifndef D3DENUM_NO_WHQL_LEVEL
#define D3DENUM_NO_WHQL_LEVEL 0x00000002L
#endif

// ============================================================================
// D3D8-specific texture formats removed in D3D9
// ============================================================================

// D3DFMT_W11V11U10 was a D3D8 format, not available in D3D9
// Map to a similar 32-bit format for compatibility
#ifndef D3DFMT_W11V11U10
#define D3DFMT_W11V11U10 D3DFMT_A2W10V10U10  // Closest D3D9 equivalent
#endif

// ============================================================================
// D3DPRESENT_PARAMETERS field name changes
// ============================================================================

// In D3D8, FullScreen_PresentationInterval was a field
// In D3D9, it's just PresentationInterval
// This can't be easily fixed with macros since it's a struct member
// Code must be updated to use PresentationInterval instead

// ============================================================================
// D3D8-specific methods compatibility notes
// ============================================================================

// D3D8 IDirect3DDevice8::CopyRects() was replaced by several methods in D3D9:
// - UpdateSurface() for SYSTEMMEM -> DEFAULT transfers
// - GetRenderTargetData() for DEFAULT -> SYSTEMMEM transfers
// - StretchRect() for DEFAULT -> DEFAULT copies
//
// Code using CopyRects needs to be updated to call the appropriate D3D9 method.
// A helper function can be provided if needed.

// D3D8 IDirect3DDevice8::GetInfo() doesn't exist in D3D9
// Query objects should be used instead

// ============================================================================
// Device creation flags (identical between D3D8 and D3D9)
// ============================================================================

// D3DCREATE_HARDWARE_VERTEXPROCESSING
// D3DCREATE_SOFTWARE_VERTEXPROCESSING
// D3DCREATE_MIXED_VERTEXPROCESSING
// D3DCREATE_FPU_PRESERVE
// etc. - all compatible

// ============================================================================
// Color manipulation macros (same in D3D8 and D3D9, but ensure defined)
// ============================================================================

#ifndef D3DCOLOR_ARGB
#define D3DCOLOR_ARGB(a,r,g,b) \
    ((D3DCOLOR)((((a)&0xff)<<24)|(((r)&0xff)<<16)|(((g)&0xff)<<8)|((b)&0xff)))
#endif

#ifndef D3DCOLOR_RGBA
#define D3DCOLOR_RGBA(r,g,b,a) D3DCOLOR_ARGB(a,r,g,b)
#endif

#ifndef D3DCOLOR_XRGB
#define D3DCOLOR_XRGB(r,g,b) D3DCOLOR_ARGB(0xff,r,g,b)
#endif

#ifndef D3DCOLOR_COLORVALUE
#define D3DCOLOR_COLORVALUE(r,g,b,a) \
    D3DCOLOR_RGBA((DWORD)((r)*255.f),(DWORD)((g)*255.f),(DWORD)((b)*255.f),(DWORD)((a)*255.f))
#endif

// ============================================================================
// D3D8 pointer typedefs (LP* versions)
// ============================================================================

#ifndef LPDIRECT3DSURFACE8
#define LPDIRECT3DSURFACE8 LPDIRECT3DSURFACE9
#endif
#ifndef LPDIRECT3DTEXTURE8
#define LPDIRECT3DTEXTURE8 LPDIRECT3DTEXTURE9
#endif
#ifndef LPDIRECT3DDEVICE8
#define LPDIRECT3DDEVICE8 LPDIRECT3DDEVICE9
#endif
#ifndef LPDIRECT3D8
#define LPDIRECT3D8 LPDIRECT3D9
#endif
#ifndef LPDIRECT3DVERTEXBUFFER8
#define LPDIRECT3DVERTEXBUFFER8 LPDIRECT3DVERTEXBUFFER9
#endif
#ifndef LPDIRECT3DINDEXBUFFER8
#define LPDIRECT3DINDEXBUFFER8 LPDIRECT3DINDEXBUFFER9
#endif

// ============================================================================
// D3D8-specific render states that were removed in D3D9
// ============================================================================

// D3DRS_LINEPATTERN was removed in D3D9 (line patterns not supported)
#ifndef D3DRS_LINEPATTERN
#define D3DRS_LINEPATTERN ((D3DRENDERSTATETYPE)10)
#endif

// D3DRS_ZVISIBLE was removed in D3D9 (use queries instead)
#ifndef D3DRS_ZVISIBLE
#define D3DRS_ZVISIBLE ((D3DRENDERSTATETYPE)30)
#endif

// D3DRS_EDGEANTIALIAS was removed in D3D9
#ifndef D3DRS_EDGEANTIALIAS
#define D3DRS_EDGEANTIALIAS ((D3DRENDERSTATETYPE)40)
#endif

// ============================================================================
// D3D8 texture stage states that became sampler states in D3D9
// In D3D8, these were D3DTSS_* and passed to SetTextureStageState
// In D3D9, these are D3DSAMP_* and passed to SetSamplerState
//
// We define the D3D8 constants with their original D3D8 numeric values.
// Wrapper code must detect these and call SetSamplerState instead.
// ============================================================================

// D3D8 D3DTSS_* values for states that moved to sampler states in D3D9
// These use the original D3D8 enum values (different from D3D9 D3DTSS_*)
#ifndef D3DTSS_ADDRESSU
#define D3DTSS_ADDRESSU      ((D3DTEXTURESTAGESTATETYPE)13)  // D3D8 value
#endif
#ifndef D3DTSS_ADDRESSV
#define D3DTSS_ADDRESSV      ((D3DTEXTURESTAGESTATETYPE)14)  // D3D8 value
#endif
#ifndef D3DTSS_ADDRESSW
#define D3DTSS_ADDRESSW      ((D3DTEXTURESTAGESTATETYPE)25)  // D3D8 value
#endif
#ifndef D3DTSS_BORDERCOLOR
#define D3DTSS_BORDERCOLOR   ((D3DTEXTURESTAGESTATETYPE)15)  // D3D8 value
#endif
#ifndef D3DTSS_MAGFILTER
#define D3DTSS_MAGFILTER     ((D3DTEXTURESTAGESTATETYPE)16)  // D3D8 value
#endif
#ifndef D3DTSS_MINFILTER
#define D3DTSS_MINFILTER     ((D3DTEXTURESTAGESTATETYPE)17)  // D3D8 value
#endif
#ifndef D3DTSS_MIPFILTER
#define D3DTSS_MIPFILTER     ((D3DTEXTURESTAGESTATETYPE)18)  // D3D8 value
#endif
#ifndef D3DTSS_MIPMAPLODBIAS
#define D3DTSS_MIPMAPLODBIAS ((D3DTEXTURESTAGESTATETYPE)19)  // D3D8 value
#endif
#ifndef D3DTSS_MAXMIPLEVEL
#define D3DTSS_MAXMIPLEVEL   ((D3DTEXTURESTAGESTATETYPE)20)  // D3D8 value
#endif
#ifndef D3DTSS_MAXANISOTROPY
#define D3DTSS_MAXANISOTROPY ((D3DTEXTURESTAGESTATETYPE)21)  // D3D8 value
#endif

// Helper to check if a D3D8 texture stage state should be a D3D9 sampler state
inline bool D3D8_Is_Sampler_State(D3DTEXTURESTAGESTATETYPE state)
{
    DWORD val = (DWORD)state;
    return (val == 13 || val == 14 || val == 15 || val == 16 ||
            val == 17 || val == 18 || val == 19 || val == 20 ||
            val == 21 || val == 25);
}

// Convert D3D8 texture stage state to D3D9 sampler state
inline D3DSAMPLERSTATETYPE D3D8_TSS_To_D3D9_Sampler(D3DTEXTURESTAGESTATETYPE state)
{
    DWORD val = (DWORD)state;
    switch (val) {
        case 13: return D3DSAMP_ADDRESSU;
        case 14: return D3DSAMP_ADDRESSV;
        case 15: return D3DSAMP_BORDERCOLOR;
        case 16: return D3DSAMP_MAGFILTER;
        case 17: return D3DSAMP_MINFILTER;
        case 18: return D3DSAMP_MIPFILTER;
        case 19: return D3DSAMP_MIPMAPLODBIAS;
        case 20: return D3DSAMP_MAXMIPLEVEL;
        case 21: return D3DSAMP_MAXANISOTROPY;
        case 25: return D3DSAMP_ADDRESSW;
        default: return (D3DSAMPLERSTATETYPE)0;
    }
}

// ============================================================================
// D3D8 texture filter types that were removed in D3D9
// ============================================================================

// D3DTEXF_FLATCUBIC and D3DTEXF_GAUSSIANCUBIC were D3D8 filter types
// They don't exist in D3D9 - map to nearest equivalent
#ifndef D3DTEXF_FLATCUBIC
#define D3DTEXF_FLATCUBIC D3DTEXF_LINEAR
#endif
#ifndef D3DTEXF_GAUSSIANCUBIC
#define D3DTEXF_GAUSSIANCUBIC D3DTEXF_LINEAR
#endif

// ============================================================================
// D3D8-specific swap effects removed in D3D9
// ============================================================================

// D3DSWAPEFFECT_COPY_VSYNC was removed in D3D9
// Use D3DSWAPEFFECT_COPY with D3DPRESENT_INTERVAL_ONE for similar behavior
#ifndef D3DSWAPEFFECT_COPY_VSYNC
#define D3DSWAPEFFECT_COPY_VSYNC D3DSWAPEFFECT_COPY
#endif

// ============================================================================
// D3D8 D3DSURFACE_DESC had a Size field that was removed in D3D9
// This helper calculates the size from Width, Height, and Format
// ============================================================================

inline UINT D3D8_Calculate_Surface_Size(UINT Width, UINT Height, D3DFORMAT Format)
{
    // Calculate bytes per pixel or block size based on format
    switch (Format) {
        // DXT compressed formats (4x4 blocks)
        case D3DFMT_DXT1:
            // DXT1: 8 bytes per 4x4 block
            return ((Width + 3) / 4) * ((Height + 3) / 4) * 8;
        case D3DFMT_DXT2:
        case D3DFMT_DXT3:
        case D3DFMT_DXT4:
        case D3DFMT_DXT5:
            // DXT2-5: 16 bytes per 4x4 block
            return ((Width + 3) / 4) * ((Height + 3) / 4) * 16;

        // 32-bit formats
        case D3DFMT_A8R8G8B8:
        case D3DFMT_X8R8G8B8:
        case D3DFMT_A2B10G10R10:
        case D3DFMT_A8B8G8R8:
        case D3DFMT_X8B8G8R8:
        case D3DFMT_G16R16:
        case D3DFMT_A2R10G10B10:
        case D3DFMT_Q8W8V8U8:
        case D3DFMT_V16U16:
        case D3DFMT_A2W10V10U10:
        case D3DFMT_D32:
        case D3DFMT_D24S8:
        case D3DFMT_D24X8:
        case D3DFMT_D24X4S4:
            return Width * Height * 4;

        // 24-bit formats
        case D3DFMT_R8G8B8:
            return Width * Height * 3;

        // 16-bit formats
        case D3DFMT_R5G6B5:
        case D3DFMT_X1R5G5B5:
        case D3DFMT_A1R5G5B5:
        case D3DFMT_A4R4G4B4:
        case D3DFMT_A8R3G3B2:
        case D3DFMT_X4R4G4B4:
        case D3DFMT_A8P8:
        case D3DFMT_A8L8:
        case D3DFMT_V8U8:
        case D3DFMT_L6V5U5:
        case D3DFMT_D16_LOCKABLE:
        case D3DFMT_D15S1:
        case D3DFMT_D16:
        case D3DFMT_L16:
        case D3DFMT_R16F:
            return Width * Height * 2;

        // 8-bit formats
        case D3DFMT_R3G3B2:
        case D3DFMT_A8:
        case D3DFMT_P8:
        case D3DFMT_L8:
        case D3DFMT_A4L4:
            return Width * Height;

        // 64-bit formats
        case D3DFMT_A16B16G16R16:
        case D3DFMT_A16B16G16R16F:
        case D3DFMT_G32R32F:
            return Width * Height * 8;

        // 128-bit formats
        case D3DFMT_A32B32G32R32F:
            return Width * Height * 16;

        // Default to 32-bit if unknown
        default:
            return Width * Height * 4;
    }
}

// Helper macro to get size from D3DSURFACE_DESC (emulates D3D8 behavior)
#define D3D8_GET_SURFACE_SIZE(desc) D3D8_Calculate_Surface_Size((desc).Width, (desc).Height, (desc).Format)

#endif // _D3D8_COMPAT_H_
