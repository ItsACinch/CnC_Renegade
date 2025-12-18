/*
**  D3DX8 Texture Compatibility Header
**
**  D3DX8 texture loading functions. These need D3DX9 library for full functionality.
**  Stub implementations are provided for compilation without D3DX.
*/

#ifndef _D3DX8TEX_COMPAT_H_
#define _D3DX8TEX_COMPAT_H_

#include "d3d8.h"

// Try to include D3DX9 texture if available
#if __has_include(<d3dx9tex.h>)
#include <d3dx9tex.h>
#define HAS_D3DX9_TEX 1
#else
#define HAS_D3DX9_TEX 0

// D3DX Filter flags
#ifndef D3DX_FILTER_NONE
#define D3DX_FILTER_NONE            0x00000001
#define D3DX_FILTER_POINT           0x00000002
#define D3DX_FILTER_LINEAR          0x00000003
#define D3DX_FILTER_TRIANGLE        0x00000004
#define D3DX_FILTER_BOX             0x00000005
#define D3DX_FILTER_MIRROR_U        0x00010000
#define D3DX_FILTER_MIRROR_V        0x00020000
#define D3DX_FILTER_MIRROR_W        0x00040000
#define D3DX_FILTER_MIRROR          0x00070000
#define D3DX_FILTER_DITHER          0x00080000
#define D3DX_FILTER_DITHER_DIFFUSION 0x00100000
#define D3DX_FILTER_SRGB_IN         0x00200000
#define D3DX_FILTER_SRGB_OUT        0x00400000
#define D3DX_FILTER_SRGB            0x00600000
#endif

// D3DX Default values
#ifndef D3DX_DEFAULT
#define D3DX_DEFAULT            ((UINT)-1)
#define D3DX_DEFAULT_NONPOW2    ((UINT)-2)
#define D3DX_DEFAULT_FLOAT      FLT_MAX
#define D3DX_FROM_FILE          ((UINT)-3)
#endif

// Image file formats
typedef enum _D3DXIMAGE_FILEFORMAT {
    D3DXIFF_BMP = 0,
    D3DXIFF_JPG = 1,
    D3DXIFF_TGA = 2,
    D3DXIFF_PNG = 3,
    D3DXIFF_DDS = 4,
    D3DXIFF_PPM = 5,
    D3DXIFF_DIB = 6,
    D3DXIFF_HDR = 7,
    D3DXIFF_PFM = 8,
    D3DXIFF_FORCE_DWORD = 0x7fffffff
} D3DXIMAGE_FILEFORMAT;

// Image info structure
typedef struct _D3DXIMAGE_INFO {
    UINT Width;
    UINT Height;
    UINT Depth;
    UINT MipLevels;
    D3DFORMAT Format;
    D3DRESOURCETYPE ResourceType;
    D3DXIMAGE_FILEFORMAT ImageFileFormat;
} D3DXIMAGE_INFO;

// Stub function declarations
// These return failure - actual implementation needs D3DX9 library

inline HRESULT D3DXLoadSurfaceFromSurface(
    IDirect3DSurface9* pDestSurface,
    const PALETTEENTRY* pDestPalette,
    const RECT* pDestRect,
    IDirect3DSurface9* pSrcSurface,
    const PALETTEENTRY* pSrcPalette,
    const RECT* pSrcRect,
    DWORD Filter,
    D3DCOLOR ColorKey)
{
    // Stub - needs D3DX9 for actual implementation
    return E_NOTIMPL;
}

inline HRESULT D3DXLoadSurfaceFromFile(
    IDirect3DSurface9* pDestSurface,
    const PALETTEENTRY* pDestPalette,
    const RECT* pDestRect,
    LPCSTR pSrcFile,
    const RECT* pSrcRect,
    DWORD Filter,
    D3DCOLOR ColorKey,
    D3DXIMAGE_INFO* pSrcInfo)
{
    return E_NOTIMPL;
}

inline HRESULT D3DXLoadSurfaceFromMemory(
    IDirect3DSurface9* pDestSurface,
    const PALETTEENTRY* pDestPalette,
    const RECT* pDestRect,
    LPCVOID pSrcMemory,
    D3DFORMAT SrcFormat,
    UINT SrcPitch,
    const PALETTEENTRY* pSrcPalette,
    const RECT* pSrcRect,
    DWORD Filter,
    D3DCOLOR ColorKey)
{
    return E_NOTIMPL;
}

inline HRESULT D3DXCreateTexture(
    IDirect3DDevice9* pDevice,
    UINT Width,
    UINT Height,
    UINT MipLevels,
    DWORD Usage,
    D3DFORMAT Format,
    D3DPOOL Pool,
    IDirect3DTexture9** ppTexture)
{
    return E_NOTIMPL;
}

inline HRESULT D3DXCreateTextureFromFile(
    IDirect3DDevice9* pDevice,
    LPCSTR pSrcFile,
    IDirect3DTexture9** ppTexture)
{
    return E_NOTIMPL;
}

inline HRESULT D3DXGetImageInfoFromFile(
    LPCSTR pSrcFile,
    D3DXIMAGE_INFO* pSrcInfo)
{
    return E_NOTIMPL;
}

inline HRESULT D3DXSaveSurfaceToFile(
    LPCSTR pDestFile,
    D3DXIMAGE_FILEFORMAT DestFormat,
    IDirect3DSurface9* pSrcSurface,
    const PALETTEENTRY* pSrcPalette,
    const RECT* pSrcRect)
{
    return E_NOTIMPL;
}

#endif // HAS_D3DX9_TEX

#endif // _D3DX8TEX_COMPAT_H_
