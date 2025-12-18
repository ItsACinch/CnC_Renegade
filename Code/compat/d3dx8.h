/*
**  D3DX8 to D3DX9 Compatibility Header
**
**  This header provides D3DX8 function compatibility using D3DX9 where possible,
**  or provides inline stub implementations where functions don't exist in D3DX9.
*/

#ifndef _D3DX8_COMPAT_H_
#define _D3DX8_COMPAT_H_

#include "d3d8.h"
#include <cstdio>   // For snprintf
#include <cstring>  // For strncpy

// Try to include D3DX9 if available
// Note: D3DX9 is deprecated but still available in June 2010 DirectX SDK
#if __has_include(<d3dx9.h>)
#include <d3dx9.h>
#define HAS_D3DX9 1
#else
#define HAS_D3DX9 0
#endif

// ============================================================================
// D3DX FVF utilities
// ============================================================================

#if HAS_D3DX9
// D3DX9 has D3DXGetFVFVertexSize - use it directly
// No mapping needed
#else
// Provide our own implementation
inline UINT D3DXGetFVFVertexSize(DWORD FVF)
{
    UINT size = 0;

    // Position (required for most FVFs)
    if (FVF & D3DFVF_XYZ) size += 12;       // 3 floats
    else if (FVF & D3DFVF_XYZRHW) size += 16; // 4 floats
    else if (FVF & D3DFVF_XYZW) size += 16;   // 4 floats

    // Blending weights
    switch (FVF & D3DFVF_POSITION_MASK) {
    case D3DFVF_XYZB1: size += 4; break;
    case D3DFVF_XYZB2: size += 8; break;
    case D3DFVF_XYZB3: size += 12; break;
    case D3DFVF_XYZB4: size += 16; break;
    case D3DFVF_XYZB5: size += 20; break;
    }

    // Normal
    if (FVF & D3DFVF_NORMAL) size += 12;    // 3 floats

    // Point size
    if (FVF & D3DFVF_PSIZE) size += 4;      // 1 float

    // Diffuse color
    if (FVF & D3DFVF_DIFFUSE) size += 4;    // 1 DWORD

    // Specular color
    if (FVF & D3DFVF_SPECULAR) size += 4;   // 1 DWORD

    // Texture coordinates
    UINT numTexCoords = (FVF & D3DFVF_TEXCOUNT_MASK) >> D3DFVF_TEXCOUNT_SHIFT;
    for (UINT i = 0; i < numTexCoords; i++) {
        UINT texCoordSize = (FVF >> (16 + i * 2)) & 0x3;
        switch (texCoordSize) {
        case D3DFVF_TEXTUREFORMAT1: size += 4; break;   // 1 float (1D)
        case D3DFVF_TEXTUREFORMAT2: size += 8; break;   // 2 floats (2D) - default
        case D3DFVF_TEXTUREFORMAT3: size += 12; break;  // 3 floats (3D)
        case D3DFVF_TEXTUREFORMAT4: size += 16; break;  // 4 floats (4D)
        default: size += 8; break;  // Default to 2D
        }
    }

    return size;
}
#endif

// ============================================================================
// D3DX Error strings
// ============================================================================

// D3DXGetErrorString was in D3DX8 but removed in D3DX9
// Original signature: HRESULT D3DXGetErrorStringA(HRESULT hr, LPSTR pBuffer, UINT BufferLen)
inline HRESULT D3DXGetErrorStringA(HRESULT hr, char* buffer, UINT bufferSize)
{
    if (!buffer || bufferSize == 0) {
        return E_INVALIDARG;
    }

    // Format error info
    const char* errStr = NULL;
    switch (hr) {
    case D3D_OK: errStr = "D3D_OK"; break;
    case D3DERR_WRONGTEXTUREFORMAT: errStr = "D3DERR_WRONGTEXTUREFORMAT"; break;
    case D3DERR_UNSUPPORTEDCOLOROPERATION: errStr = "D3DERR_UNSUPPORTEDCOLOROPERATION"; break;
    case D3DERR_UNSUPPORTEDCOLORARG: errStr = "D3DERR_UNSUPPORTEDCOLORARG"; break;
    case D3DERR_UNSUPPORTEDALPHAOPERATION: errStr = "D3DERR_UNSUPPORTEDALPHAOPERATION"; break;
    case D3DERR_UNSUPPORTEDALPHAARG: errStr = "D3DERR_UNSUPPORTEDALPHAARG"; break;
    case D3DERR_TOOMANYOPERATIONS: errStr = "D3DERR_TOOMANYOPERATIONS"; break;
    case D3DERR_CONFLICTINGTEXTUREFILTER: errStr = "D3DERR_CONFLICTINGTEXTUREFILTER"; break;
    case D3DERR_UNSUPPORTEDFACTORVALUE: errStr = "D3DERR_UNSUPPORTEDFACTORVALUE"; break;
    case D3DERR_CONFLICTINGRENDERSTATE: errStr = "D3DERR_CONFLICTINGRENDERSTATE"; break;
    case D3DERR_UNSUPPORTEDTEXTUREFILTER: errStr = "D3DERR_UNSUPPORTEDTEXTUREFILTER"; break;
    case D3DERR_CONFLICTINGTEXTUREPALETTE: errStr = "D3DERR_CONFLICTINGTEXTUREPALETTE"; break;
    case D3DERR_DRIVERINTERNALERROR: errStr = "D3DERR_DRIVERINTERNALERROR"; break;
    case D3DERR_NOTFOUND: errStr = "D3DERR_NOTFOUND"; break;
    case D3DERR_MOREDATA: errStr = "D3DERR_MOREDATA"; break;
    case D3DERR_DEVICELOST: errStr = "D3DERR_DEVICELOST"; break;
    case D3DERR_DEVICENOTRESET: errStr = "D3DERR_DEVICENOTRESET"; break;
    case D3DERR_NOTAVAILABLE: errStr = "D3DERR_NOTAVAILABLE"; break;
    case D3DERR_OUTOFVIDEOMEMORY: errStr = "D3DERR_OUTOFVIDEOMEMORY"; break;
    case D3DERR_INVALIDDEVICE: errStr = "D3DERR_INVALIDDEVICE"; break;
    case D3DERR_INVALIDCALL: errStr = "D3DERR_INVALIDCALL"; break;
    case D3DERR_DRIVERINVALIDCALL: errStr = "D3DERR_DRIVERINVALIDCALL"; break;
    case D3DERR_WASSTILLDRAWING: errStr = "D3DERR_WASSTILLDRAWING"; break;
    case E_OUTOFMEMORY: errStr = "E_OUTOFMEMORY"; break;
    default: break;
    }

    if (errStr) {
        strncpy(buffer, errStr, bufferSize);
    } else {
        snprintf(buffer, bufferSize, "D3D Error 0x%08X", (unsigned int)hr);
    }
    buffer[bufferSize - 1] = '\0';
    return D3D_OK;
}

// ============================================================================
// D3DX Texture creation helpers
// ============================================================================

// D3DXCreateTexture and related functions need the D3DX9 library
// If not available, code will need to use alternative texture loading methods

#endif // _D3DX8_COMPAT_H_
