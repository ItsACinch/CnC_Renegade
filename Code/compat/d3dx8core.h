/*
**  DirectX 8 to DirectX 9 Compatibility Header - D3DX Core
**
**  D3DX8 was replaced by D3DX9. Most interfaces are similar.
**  This header provides D3DX8 -> D3DX9 mappings.
*/

#ifndef _D3DX8CORE_COMPAT_H_
#define _D3DX8CORE_COMPAT_H_

// Include D3D8 compat first to get the type mappings
#include "d3d8.h"
#include "d3dx8.h"  // For D3DXGetErrorStringA and other utilities

// Note: D3DX9 is not included by default in modern Windows SDK
// The legacy DirectX SDK (June 2010) contains d3dx9.h
// For now, we provide stubs for the commonly used D3DX8 interfaces

// D3DX8 sprite interface - stub declaration
// The full D3DX9 library would need to be linked for actual usage
#ifndef LPD3DXSPRITE
typedef interface ID3DXSprite *LPD3DXSPRITE;
#endif

// D3DX8 font interface
#ifndef LPD3DXFONT
typedef interface ID3DXFont *LPD3DXFONT;
#endif

// D3DX error codes
#ifndef D3DXERR_INVALIDDATA
#define D3DXERR_INVALIDDATA MAKE_HRESULT(1, 0x876, 2)
#endif

// D3DX math is handled separately in d3dx8math.h

#endif // _D3DX8CORE_COMPAT_H_
