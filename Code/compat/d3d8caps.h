/*
**  DirectX 8 to DirectX 9 Compatibility Header - Caps
**
**  This header redirects d3d8caps.h includes to use d3d9caps.h
**  D3D9 caps are a superset of D3D8 caps.
*/

#ifndef _D3D8CAPS_COMPAT_H_
#define _D3D8CAPS_COMPAT_H_

// Include the main compatibility header to get all the macro definitions
#include "d3d8.h"

// d3d9caps.h is already included by d3d8.h
// This file exists to satisfy #include <d3d8caps.h> directives

#endif // _D3D8CAPS_COMPAT_H_
