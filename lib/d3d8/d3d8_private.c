#include "d3d8types.h"
#include "d3d8_private.h"

#include <assert.h>
#include <stdbool.h>

int D3D_FormatBPP(D3DFORMAT fmt) {
    switch (fmt) {
    case D3DFMT_A8R8G8B8:
    case D3DFMT_X8R8G8B8:
    case D3DFMT_D24S8:
        return 32;
    case D3DFMT_A1R5G5B5:
    case D3DFMT_X1R5G5B5:
        return 15;
    case D3DFMT_R5G6B5:
        return 16;
    default:
        assert(false);
        return 0; 
    }
}

int D3D_FormatBytesPerPixel(D3DFORMAT fmt) {
    return (D3D_FormatBPP(fmt)+7)/8;
}