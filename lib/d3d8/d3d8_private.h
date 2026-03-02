#pragma once

#include <stdlib.h>

#define MASK(mask, val) (((val) << (__builtin_ffs(mask)-1)) & (mask))

#define D3D_FALLTHROUGH() [[fallthrough]]
#define D3D_CALL(...) ({      \
    HRESULT hr = __VA_ARGS__; \
    if(FAILED(hr)) return hr; \
    hr;})                     \

#define memcpyp(dest, src) ({                                                 \
    _Static_assert(sizeof(*(src)) == sizeof(*dest),                           \
                   "memcpyp: mismatch in operand size");                      \
    memcpy((&*(dest)), (&*(src)), sizeof((*(dest)))); })

#define memsetp(p, c)     ({ memset((&*(p)), (c), sizeof((*(p)))); })
#define memzro (p, c, n)  ({ memset((p),     (c), (n)); })
#define memzrop(p) memsetp(p, 0)

int D3D_FormatBPP(D3DFORMAT fmt);
int D3D_FormatBytesPerPixel(D3DFORMAT fmt);

HRESULT Direct3D_ResetDevice(D3DPRESENT_PARAMETERS* pPresentationParameters);