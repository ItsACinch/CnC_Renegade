/*
**  D3DX8 Math Compatibility Header
**
**  D3DX8 math functions and types. Most are provided by D3DX9.
**  For functions not available, inline implementations are provided.
*/

#ifndef _D3DX8MATH_COMPAT_H_
#define _D3DX8MATH_COMPAT_H_

#include "d3d8.h"

// Try to include D3DX9 math if available
#if __has_include(<d3dx9math.h>)
#include <d3dx9math.h>
#define HAS_D3DX9_MATH 1
#else
#define HAS_D3DX9_MATH 0

// Provide basic math types and functions if D3DX9 not available
// These are simplified versions - full implementations would be in a math library

typedef struct D3DXVECTOR2 {
    float x, y;
    D3DXVECTOR2() : x(0), y(0) {}
    D3DXVECTOR2(float _x, float _y) : x(_x), y(_y) {}
} D3DXVECTOR2, *LPD3DXVECTOR2;

typedef struct D3DXVECTOR3 {
    float x, y, z;
    D3DXVECTOR3() : x(0), y(0), z(0) {}
    D3DXVECTOR3(float _x, float _y, float _z) : x(_x), y(_y), z(_z) {}
} D3DXVECTOR3, *LPD3DXVECTOR3;

typedef struct D3DXVECTOR4 {
    float x, y, z, w;
    D3DXVECTOR4() : x(0), y(0), z(0), w(0) {}
    D3DXVECTOR4(float _x, float _y, float _z, float _w) : x(_x), y(_y), z(_z), w(_w) {}
} D3DXVECTOR4, *LPD3DXVECTOR4;

typedef struct D3DXMATRIX : public D3DMATRIX {
    D3DXMATRIX() {}
} D3DXMATRIX, *LPD3DXMATRIX;

typedef struct D3DXQUATERNION {
    float x, y, z, w;
    D3DXQUATERNION() : x(0), y(0), z(0), w(1) {}
    D3DXQUATERNION(float _x, float _y, float _z, float _w) : x(_x), y(_y), z(_z), w(_w) {}
} D3DXQUATERNION, *LPD3DXQUATERNION;

typedef struct D3DXPLANE {
    float a, b, c, d;
    D3DXPLANE() : a(0), b(0), c(0), d(0) {}
    D3DXPLANE(float _a, float _b, float _c, float _d) : a(_a), b(_b), c(_c), d(_d) {}
} D3DXPLANE, *LPD3DXPLANE;

typedef struct D3DXCOLOR {
    float r, g, b, a;
    D3DXCOLOR() : r(0), g(0), b(0), a(1) {}
    D3DXCOLOR(float _r, float _g, float _b, float _a) : r(_r), g(_g), b(_b), a(_a) {}
} D3DXCOLOR, *LPD3DXCOLOR;

// Matrix functions - stubs
inline D3DXMATRIX* D3DXMatrixIdentity(D3DXMATRIX* pOut) {
    if (pOut) {
        pOut->_11 = 1; pOut->_12 = 0; pOut->_13 = 0; pOut->_14 = 0;
        pOut->_21 = 0; pOut->_22 = 1; pOut->_23 = 0; pOut->_24 = 0;
        pOut->_31 = 0; pOut->_32 = 0; pOut->_33 = 1; pOut->_34 = 0;
        pOut->_41 = 0; pOut->_42 = 0; pOut->_43 = 0; pOut->_44 = 1;
    }
    return pOut;
}

#endif // HAS_D3DX9_MATH

#endif // _D3DX8MATH_COMPAT_H_
