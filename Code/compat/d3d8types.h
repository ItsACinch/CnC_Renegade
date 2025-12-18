/*
**  DirectX 8 to DirectX 9 Compatibility Header - Types
**
**  This header redirects d3d8types.h includes to use d3d9types.h
**  D3D9 types are nearly identical to D3D8 types.
*/

#ifndef _D3D8TYPES_COMPAT_H_
#define _D3D8TYPES_COMPAT_H_

// Include the main compatibility header to get all the macro definitions
#include "d3d8.h"

// d3d9types.h is already included by d3d8.h
// This file exists to satisfy #include <d3d8types.h> directives

#endif // _D3D8TYPES_COMPAT_H_
