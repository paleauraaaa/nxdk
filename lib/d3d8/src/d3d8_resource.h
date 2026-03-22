// This file contains the internal definitions of D3DResource and the objects 
// that inherit from it (D3DSurface, D3DTexture, etc.), and the definitions for
// their interfaces. 

// Anything contained herein is intended to be private to the outside world,
// but public within the D3D implementation. Some of these functions will be
// "exported" via the vtables, but their declarations or definitions need not
// be exposed to facilitate that.

#pragma once

#include "d3d8.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus   

#ifndef __cplusplus
extern IDirect3DResourceVtbl8*      g_pResourceVtbl;
extern IDirect3DBaseTextureVtbl8*   g_pBaseTextureVtbl;
extern IDirect3DTextureVtbl8*       g_pTextureVtbl;
extern IDirect3DCubeTextureVtbl8*   g_pCubeTextureVtbl;
extern IDirect3DSurfaceVtbl8*       g_pSurfaceVtbl;
extern IDirect3DVertexBufferVtbl8*  g_pVertexBufferVtbl;
extern IDirect3DPushBufferVtbl8*    g_pPushBufferVtbl;
#endif // __cplusplus

typedef struct D3DRectLock {
    RECT  Rect;
    DWORD Flags;
} D3DRectLock;

// ============================================================================
typedef struct D3DRefcount {
    ULONG c;
} D3DRefcount;

ULONG D3DRefcount_AddRef(D3DRefcount* pThis);
ULONG D3DRefcount_Release(D3DRefcount *pThis);
// ============================================================================

// ============================================================================
typedef struct D3DResourceInner {
    D3DRefcount        refcount;
    D3DRESOURCETYPE    type;
    BOOL               bManuallyRegistered;
    DWORD              Data;
    PVOID              pContiguousMemory;
    DWORD              Fence;
} D3DResourceInner;

typedef struct D3DResource IMPLEMENTS(IDirect3DResource8) {
#ifndef __cplusplus
    IDirect3DResource8 iface;
#else
    virtual D3DAPI ULONG AddRef() override;
    virtual D3DAPI ULONG Release() override;
    virtual D3DAPI D3DRESOURCETYPE GetType() override;
    virtual D3DAPI VOID  Register(PVOID pBase) override;
    virtual D3DAPI VOID  BlockUntilNotBusy() override;
    virtual D3DAPI BOOL  IsBusy() override;
#endif // __cplusplus
    D3DResourceInner   inner;
} D3DResource;

D3DEXTERN D3DAPI ULONG D3DResource_AddRef(LPDIRECT3DRESOURCE8 pThis);
D3DEXTERN D3DAPI ULONG D3DResource_Release(LPDIRECT3DRESOURCE8 pThis);
D3DEXTERN D3DAPI D3DRESOURCETYPE D3DResource_GetType(
    LPDIRECT3DRESOURCE8 pThis);
D3DEXTERN D3DAPI VOID D3DResource_Register(LPDIRECT3DRESOURCE8 pThis, 
                                           PVOID pBase);
D3DEXTERN D3DAPI VOID D3DResource_BlockUntilNotBusy(LPDIRECT3DRESOURCE8 pThis);
D3DEXTERN D3DAPI BOOL D3DResource_IsBusy(LPDIRECT3DRESOURCE8 pThis);
// ============================================================================

// ============================================================================

typedef struct D3DPushBuffer IMPLEMENTS(IDirect3DPushBuffer8) {
#ifndef __cplusplus
    IDirect3DPushBuffer8 iface;
#else
    virtual D3DAPI ULONG   AddRef() override;
    virtual D3DAPI ULONG   Release() override;
    virtual D3DAPI D3DRESOURCETYPE GetType() override;
    virtual D3DAPI VOID    Register(PVOID pBase) override;
    virtual D3DAPI VOID    BlockUntilNotBusy() override;
    virtual D3DAPI BOOL    IsBusy() override;
    virtual D3DAPI HRESULT GetSize(UINT* pSize) override;
    virtual D3DAPI HRESULT GetData(CONST DWORD** ppData) override;
#endif // __cplusplus
    D3DResourceInner     resource;
    DWORD                Size;       // in DWORDs
    DWORD                SizeNeeded; // in DWORDs
    BOOL                 bCpu;
    PDWORD               p;
    PDWORD               pLastPush;
} D3DPushBuffer;

D3DEXTERN D3DAPI ULONG D3DPushBuffer_AddRef(LPDIRECT3DPUSHBUFFER8 pThis);
D3DEXTERN D3DAPI ULONG D3DPushBuffer_Release(LPDIRECT3DPUSHBUFFER8 pThis);
D3DEXTERN D3DAPI D3DRESOURCETYPE D3DPushBuffer_GetType(
    LPDIRECT3DPUSHBUFFER8 pThis);
D3DEXTERN D3DAPI VOID D3DPushBuffer_Register(LPDIRECT3DPUSHBUFFER8 pThis, 
                                             PVOID pBase);
D3DEXTERN D3DAPI VOID D3DPushBuffer_BlockUntilNotBusy(
    LPDIRECT3DPUSHBUFFER8 pThis);
D3DEXTERN D3DAPI BOOL D3DPushBuffer_IsBusy(LPDIRECT3DPUSHBUFFER8 pThis);
D3DEXTERN D3DAPI HRESULT D3DPushBuffer_GetSize(LPDIRECT3DPUSHBUFFER8 pThis, 
                                               UINT* pSize);
D3DEXTERN D3DAPI HRESULT D3DPushBuffer_GetData(LPDIRECT3DPUSHBUFFER8 pThis, 
                                               CONST DWORD** ppData);
HRESULT D3DPushBuffer_Verify(LPDIRECT3DPUSHBUFFER8 pThis, PDWORD pdwPos);

#define D3D_NV2A_PFIFO_METHOD_CMD_MASK      0x00001FFC
#define D3D_NV2A_PFIFO_METHOD_SUBCH_MASK    0x0000E000
#define D3D_NV2A_PFIFO_METHOD_NPARAM_MASK   0x1FFC0000

#define D3D_NV2A_PFIFO_METHOD_CMD(method)     (method &        0x00001FFC)
#define D3D_NV2A_PFIFO_METHOD_SUBCH(method)  ((method >> 13) & 0x00000007)
#define D3D_NV2A_PFIFO_METHOD_NPARAM(method) ((method >> 18) & 0x000007FF)

#define D3D_NV2A_PFIFO_SUBCHANNEL_3D       0
#define D3D_NV2A_PFIFO_SUBCHANNEL_2        2
#define D3D_NV2A_PFIFO_SUBCHANNEL_3        3
#define D3D_NV2A_PFIFO_SUBCHANNEL_4        4
#define D3D_NV2A_PFIFO_NEXT_SUBCHANNEL     5
#define D3D_NV2A_PFIFO_ENCODE_METHOD(subchannel, command, nparam) ({          \
    DWORD method = ((nparam << 18) + (subchannel << 13) + command);           \
    if (method & 0xA0030003)                                                  \
        D3D_DebugPrintf("Invalid method 0x%08X\n", method);                   \
    D3D_ASSERT_IF(D3DERR_DRIVERINTERNALERROR, (method & 0xA0030003) != 0);    \
    method;                                                                   \
})

#define D3D_NV2A_PFIFO_ENCODE_3D_METHOD(command, nparam) \
    D3D_NV2A_PFIFO_ENCODE_METHOD(D3D_NV2A_PFIFO_SUBCHANNEL_3D, command, nparam)

#define D3D_NV2A_PFIFO_ENCODE_JUMP(vaddr) ({                                  \
    D3D_ASSERT_IF(D3DERR_INVALIDCALL, ((DWORD)vaddr & 0x00000003) != 0);      \
    ((DWORD)vaddr | 0x00000001);                                              \
})

BOOL D3DPushBuffer_IsFull(LPDIRECT3DPUSHBUFFER8 pThis);
HRESULT D3DPushBuffer_Push1(
    D3DPushBuffer* pThis, DWORD dwData, BOOL bLoop);
HRESULT D3DPushBuffer_PushCmd(
    D3DPushBuffer* pThis, DWORD cmd, DWORD dwData, BOOL bLoop);
HRESULT D3DPushBuffer_PushCmdA(
    D3DPushBuffer* pThis, DWORD cmd, 
    CONST DWORD* pdwData, SIZE_T n, 
    BOOL bIncrement, BOOL bLoop);
HRESULT D3DPushBuffer_PushCmd2(
    D3DPushBuffer* pThis, DWORD cmd, 
    DWORD dwData1, DWORD dwData2, 
    BOOL bIncrement, BOOL bLoop);
HRESULT D3DPushBuffer_PushCmd3(
    D3DPushBuffer* pThis, DWORD cmd, 
    DWORD dwData1, DWORD dwData2, DWORD dwData3, 
    BOOL bIncrement, BOOL bLoop);
HRESULT D3DPushBuffer_PushCmd4(
    D3DPushBuffer* pThis, DWORD cmd, 
    DWORD dwData1, DWORD dwData2, 
    DWORD dwData3, DWORD dwData4, 
    BOOL bIncrement, BOOL bLoop);
HRESULT D3DPushBuffer_PushCmdf(D3DPushBuffer* pThis, DWORD cmd, 
                               float fData, BOOL bLoop);
HRESULT D3DPushBuffer_PushCmd2f(D3DPushBuffer* pThis, DWORD cmd, 
                                float fData1, float fData2, 
                                BOOL bIncrement, BOOL bLoop);
HRESULT D3DPushBuffer_PushCmd3f(D3DPushBuffer* pThis, DWORD cmd, 
                                float fData1, float fData2, 
                                float fData3, BOOL bIncrement, BOOL bLoop);
HRESULT D3DPushBuffer_PushCmd4f(D3DPushBuffer* pThis, DWORD cmd, 
                                float fData1, float fData2, 
                                float fData3, float fData4,
                                BOOL bIncrement, BOOL bLoop);
HRESULT D3DPushBuffer_PushN(
    D3DPushBuffer* pThis, DWORD dwData, SIZE_T n, BOOL bLoop);
HRESULT D3DPushBuffer_PushA(
    D3DPushBuffer* pThis, CONST DWORD* pdwData, SIZE_T n, BOOL bLoop);
HRESULT D3DPushBuffer_PushJump(D3DPushBuffer* pThis, PVOID vaddr, BOOL bLoop);
DWORD D3DPushBuffer_BytesRemaining(D3DPushBuffer* pThis);
// ============================================================================

struct D3DBaseTexture;
typedef struct D3DBaseTexture D3DBaseTexture;

// ============================================================================
typedef struct D3DSurface IMPLEMENTS(IDirect3DSurface8) {
#ifndef __cplusplus
    IDirect3DSurface8 iface;
#else
    virtual D3DAPI ULONG   AddRef() override;
    virtual D3DAPI ULONG   Release() override;
    virtual D3DAPI D3DRESOURCETYPE GetType() override;
    virtual D3DAPI VOID    Register(PVOID pBase) override;
    virtual D3DAPI VOID    BlockUntilNotBusy() override;
    virtual D3DAPI BOOL    IsBusy() override;
    virtual D3DAPI HRESULT GetContainer(
        LPDIRECT3DBASETEXTURE8* ppContainer, REFIID riid) override;
    virtual D3DAPI HRESULT GetDesc(D3DSURFACE_DESC* pDesc) override;
    virtual D3DAPI HRESULT LockRect(
        D3DLOCKED_RECT* pLockedRect, 
        CONST RECT* pRect, DWORD Flags) override;
    virtual D3DAPI HRESULT UnlockRect() override;
#endif // __cplusplus
    D3DResourceInner  resource;
    D3DSURFACE_DESC   desc;
    D3DRectLock       lock;
    D3DBaseTexture*   pContainer; 
} D3DSurface;

D3DEXTERN D3DAPI ULONG D3DSurface_AddRef(LPDIRECT3DSURFACE8 pThis);
D3DEXTERN D3DAPI ULONG D3DSurface_Release(LPDIRECT3DSURFACE8 pThis);
D3DEXTERN D3DAPI D3DRESOURCETYPE D3DSurface_GetType(LPDIRECT3DSURFACE8 pThis);
D3DEXTERN D3DAPI VOID D3DSurface_Register(LPDIRECT3DSURFACE8 pThis, 
                                          PVOID pBase);
D3DEXTERN D3DAPI BOOL D3DSurface_IsBusy(LPDIRECT3DSURFACE8 pThis);
D3DEXTERN D3DAPI VOID D3DSurface_BlockUntilNotBusy(LPDIRECT3DSURFACE8 pThis);
D3DEXTERN D3DAPI HRESULT D3DSurface_GetContainer(
    LPDIRECT3DSURFACE8 pThis, 
    LPDIRECT3DBASETEXTURE8* ppContainer,
    REFIID riid);
D3DEXTERN D3DAPI HRESULT D3DSurface_GetDesc(LPDIRECT3DSURFACE8 pThis, 
                                            D3DSURFACE_DESC* pDesc);
D3DEXTERN D3DAPI HRESULT D3DSurface_LockRect(LPDIRECT3DSURFACE8 pThis, 
                                             D3DLOCKED_RECT* pLockedRect, 
                                             CONST RECT* pRect, DWORD Flags);
D3DEXTERN D3DAPI HRESULT D3DSurface_UnlockRect(LPDIRECT3DSURFACE8 pThis);
// ============================================================================

// ============================================================================
typedef struct D3DVertexBuffer IMPLEMENTS(IDirect3DVertexBuffer8) {
#ifndef __cplusplus
    IDirect3DVertexBuffer8 iface;
#else 
    virtual D3DAPI ULONG   AddRef() override;
    virtual D3DAPI ULONG   Release() override;
    virtual D3DAPI D3DRESOURCETYPE GetType() override;
    virtual D3DAPI VOID    Register(PVOID pBase) override;
    virtual D3DAPI VOID    BlockUntilNotBusy() override;
    virtual D3DAPI BOOL    IsBusy() override;
    virtual D3DAPI HRESULT GetDesc(
        D3DVERTEXBUFFER_DESC* pDesc) override;
    virtual D3DAPI HRESULT Lock(
        UINT OffsetToLock, UINT SizeToLock, 
        BYTE** ppbData, DWORD Flags) override;
    virtual D3DAPI HRESULT Unlock() override;
#endif // __cplusplus
    D3DResourceInner       resource;
    D3DVERTEXBUFFER_DESC   desc;
} D3DVertexBuffer;

D3DEXTERN D3DAPI ULONG D3DVertexBuffer_AddRef(LPDIRECT3DVERTEXBUFFER8 pThis);
D3DEXTERN D3DAPI ULONG D3DVertexBuffer_Release(LPDIRECT3DVERTEXBUFFER8 pThis);
D3DEXTERN D3DAPI D3DRESOURCETYPE D3DVertexBuffer_GetType(
    LPDIRECT3DVERTEXBUFFER8 pThis);
D3DEXTERN D3DAPI VOID D3DVertexBuffer_Register(
    LPDIRECT3DVERTEXBUFFER8 pThis, PVOID pBase);
D3DEXTERN D3DAPI BOOL D3DVertexBuffer_IsBusy(LPDIRECT3DVERTEXBUFFER8 pThis);
D3DEXTERN D3DAPI VOID D3DVertexBuffer_BlockUntilNotBusy(
    LPDIRECT3DVERTEXBUFFER8 pThis);
D3DEXTERN D3DAPI HRESULT D3DVertexBuffer_GetDesc(LPDIRECT3DVERTEXBUFFER8 pThis, 
                                D3DVERTEXBUFFER_DESC* pDesc);
D3DEXTERN D3DAPI HRESULT D3DVertexBuffer_Lock(LPDIRECT3DVERTEXBUFFER8 pThis, 
                                              UINT OffsetToLock, 
                                              UINT SizeToLock, BYTE** ppbData, 
                                              DWORD Flags);
D3DEXTERN D3DAPI HRESULT D3DVertexBuffer_Unlock(LPDIRECT3DVERTEXBUFFER8 pThis);
// ============================================================================

// ============================================================================
typedef struct D3DBaseTextureInner {
    D3DResourceInner resource;
    DWORD            dwLevelCount;
} D3DBaseTextureInner;

struct D3DBaseTexture IMPLEMENTS(IDirect3DBaseTexture8) {
#ifndef __cplusplus
    IDirect3DBaseTexture8 iface;
#else
    virtual D3DAPI ULONG AddRef() override;
    virtual D3DAPI ULONG Release() override;
    virtual D3DAPI D3DRESOURCETYPE GetType() override;
    virtual D3DAPI VOID Register(PVOID pBase) override;
    virtual D3DAPI VOID BlockUntilNotBusy() override;
    virtual D3DAPI BOOL IsBusy() override;
    virtual D3DAPI DWORD GetLevelCount() override;
#endif // __cplusplus
    D3DBaseTextureInner   inner;
};

D3DEXTERN D3DAPI ULONG D3DBaseTexture_AddRef(LPDIRECT3DBASETEXTURE8 pThis);
D3DEXTERN D3DAPI ULONG D3DBaseTexture_Release(LPDIRECT3DBASETEXTURE8 pThis);
D3DEXTERN D3DAPI D3DRESOURCETYPE D3DBaseTexture_GetType(
    LPDIRECT3DBASETEXTURE8 pThis);
D3DEXTERN D3DAPI VOID D3DBaseTexture_Register(LPDIRECT3DBASETEXTURE8 pThis, 
                                              PVOID pBase);
D3DEXTERN D3DAPI BOOL D3DBaseTexture_IsBusy(LPDIRECT3DBASETEXTURE8 pThis);
D3DEXTERN D3DAPI VOID D3DBaseTexture_BlockUntilNotBusy(
    LPDIRECT3DBASETEXTURE8 pThis);
D3DEXTERN D3DAPI DWORD D3DBaseTexture_GetLevelCount(
    LPDIRECT3DBASETEXTURE8 pThis);
// ============================================================================

// ============================================================================
typedef struct D3DTexture IMPLEMENTS(IDirect3DTexture8) {
#ifndef __cplusplus
    IDirect3DTexture8   iface;
#else
    virtual D3DAPI ULONG   AddRef() override;
    virtual D3DAPI ULONG   Release() override;
    virtual D3DAPI D3DRESOURCETYPE GetType() override;
    virtual D3DAPI VOID    Register(PVOID pBase) override;
    virtual D3DAPI VOID    BlockUntilNotBusy() override;
    virtual D3DAPI BOOL    IsBusy() override;
    virtual D3DAPI DWORD   GetLevelCount() override;
    virtual D3DAPI HRESULT GetLevelDesc(
        UINT Level, D3DSURFACE_DESC* pDesc) override;
    virtual D3DAPI HRESULT GetSurfaceLevel(
        UINT Level, LPDIRECT3DSURFACE8* ppSurfaceLevel) override;
    virtual D3DAPI HRESULT LockRect(
        UINT Level, D3DLOCKED_RECT* pLockedRect, 
        CONST RECT* pRect, DWORD Flags) override;
    virtual D3DAPI HRESULT UnlockRect(UINT Level) override;
#endif // __cplusplus
    D3DBaseTextureInner base;
    D3DSURFACE_DESC     desc;
    D3DSurface*         pLevels;
} D3DTexture;

D3DEXTERN D3DAPI ULONG D3DTexture_AddRef(LPDIRECT3DTEXTURE8 pThis);
D3DEXTERN D3DAPI ULONG D3DTexture_Release(LPDIRECT3DTEXTURE8 pThis);
D3DEXTERN D3DAPI D3DRESOURCETYPE D3DTexture_GetType(LPDIRECT3DTEXTURE8 pThis);
D3DEXTERN D3DAPI VOID D3DTexture_Register(LPDIRECT3DTEXTURE8 pThis, 
                                          PVOID pBase);
D3DEXTERN D3DAPI BOOL D3DTexture_IsBusy(LPDIRECT3DTEXTURE8 pThis);
D3DEXTERN D3DAPI VOID D3DTexture_BlockUntilNotBusy(LPDIRECT3DTEXTURE8 pThis);
D3DEXTERN D3DAPI DWORD D3DTexture_GetLevelCount(LPDIRECT3DTEXTURE8 pThis);
D3DEXTERN D3DAPI HRESULT D3DTexture_GetLevelDesc(LPDIRECT3DTEXTURE8 pThis, 
                                                 UINT Level, 
                                                 D3DSURFACE_DESC* pDesc);
D3DEXTERN D3DAPI HRESULT D3DTexture_GetSurfaceLevel(
    LPDIRECT3DTEXTURE8 pThis, UINT Level, LPDIRECT3DSURFACE8* ppSurfaceLevel);
D3DEXTERN D3DAPI HRESULT D3DTexture_LockRect(LPDIRECT3DTEXTURE8 pThis, 
                                             UINT Level,
                                             D3DLOCKED_RECT* pLockedRect, 
                                             CONST RECT* pRect,
                                             DWORD Flags);
D3DEXTERN D3DAPI HRESULT D3DTexture_UnlockRect(LPDIRECT3DTEXTURE8 pThis, 
                                               UINT Level);
// ============================================================================

// ============================================================================
typedef struct D3DCubeTexture IMPLEMENTS(IDirect3DCubeTexture8) {
#ifndef __cplusplus
    IDirect3DCubeTexture8   iface;
#else
    virtual D3DAPI ULONG   AddRef() override;
    virtual D3DAPI ULONG   Release() override;
    virtual D3DAPI D3DRESOURCETYPE GetType() override;
    virtual D3DAPI VOID    Register(PVOID pBase) override;
    virtual D3DAPI VOID    BlockUntilNotBusy() override;
    virtual D3DAPI BOOL    IsBusy() override;
    virtual D3DAPI DWORD   GetLevelCount() override;
    virtual D3DAPI HRESULT GetLevelDesc(
        UINT Level, D3DSURFACE_DESC* pDesc) override;
    virtual D3DAPI HRESULT LockRect(
        D3DCUBEMAP_FACES FaceType, UINT Level, D3DLOCKED_RECT* pLockedRect, 
        CONST RECT* pRect, DWORD Flags) override;
    virtual D3DAPI HRESULT UnlockRect(
        D3DCUBEMAP_FACES FaceType, UINT Level) override;
    virtual D3DAPI HRESULT GetCubeMapSurface(
        D3DCUBEMAP_FACES FaceType, UINT Level, 
        LPDIRECT3DSURFACE8* ppCubeMapSurface) override;
#endif // __cplusplus
    D3DBaseTextureInner     base;
    D3DTexture*             pCubeSurfaces;
} D3DCubeTexture;

D3DEXTERN D3DAPI ULONG D3DCubeTexture_AddRef(LPDIRECT3DCUBETEXTURE8 pThis);
D3DEXTERN D3DAPI ULONG D3DCubeTexture_Release(LPDIRECT3DCUBETEXTURE8 pThis);
D3DEXTERN D3DAPI D3DRESOURCETYPE D3DCubeTexture_GetType(
    LPDIRECT3DCUBETEXTURE8 pThis);
D3DEXTERN D3DAPI VOID D3DCubeTexture_Register(LPDIRECT3DCUBETEXTURE8 pThis, 
                                              PVOID pBase);
D3DEXTERN D3DAPI BOOL D3DCubeTexture_IsBusy(LPDIRECT3DCUBETEXTURE8 pThis);
D3DEXTERN D3DAPI VOID D3DCubeTexture_BlockUntilNotBusy(
    LPDIRECT3DCUBETEXTURE8 pThis);
D3DEXTERN D3DAPI DWORD D3DCubeTexture_GetLevelCount(
    LPDIRECT3DCUBETEXTURE8 pThis);
D3DEXTERN D3DAPI HRESULT D3DCubeTexture_GetLevelDesc(
    LPDIRECT3DCUBETEXTURE8 pThis, UINT Level, D3DSURFACE_DESC* pDesc);
D3DEXTERN D3DAPI HRESULT D3DCubeTexture_LockRect(LPDIRECT3DCUBETEXTURE8 pThis,
                                                 D3DCUBEMAP_FACES FaceType, 
                                                 UINT Level, 
                                                 D3DLOCKED_RECT* pLockedRect,
                                                 CONST RECT* pRect, 
                                                 DWORD Flags);
D3DEXTERN D3DAPI HRESULT D3DCubeTexture_UnlockRect(LPDIRECT3DCUBETEXTURE8 pThis, 
                                                   D3DCUBEMAP_FACES FaceType, 
                                                   UINT Level);
D3DEXTERN D3DAPI HRESULT D3DCubeTexture_GetCubeMapSurface(
    LPDIRECT3DCUBETEXTURE8 pThis, D3DCUBEMAP_FACES FaceType, 
    UINT Level, LPDIRECT3DSURFACE8* ppCubeMapSurface);
// ============================================================================

VOID D3D_CreateResource(D3DRESOURCETYPE type, DWORD Data, 
                        PVOID pContiguousMemory, 
                        D3DResourceInner* pResource);
HRESULT D3D_CreateSurface(UINT Width, UINT Height, D3DFORMAT Format,
                          DWORD Usage, D3DMULTISAMPLE_TYPE MultiSampleType, 
                          PVOID pContiguousMemory,
                          D3DBaseTexture* pContainer,
                          D3DSurface* pSurf);
HRESULT D3D_CreatePushBuffer(
    DWORD Size, BOOL bCpu, PVOID pContiguousMemory, D3DPushBuffer* pPB);
HRESULT D3D_CreateTexture(UINT Width, UINT Height, UINT Levels, 
                          DWORD Usage, D3DFORMAT Format, 
                          PVOID pContiguousMemory,
                          D3DTexture* pTex);
HRESULT D3D_CreateCubeTexture(UINT EdgeLength, UINT Levels, 
                              D3DFORMAT Format, PVOID pContiguousMemory,
                              D3DCubeTexture* pTex);

#ifdef __cplusplus
}
#endif // __cplusplus
