#pragma once

#include "d3d8.h"

extern IDirect3DResourceVtbl8       g_d3dResourceVtbl;
extern IDirect3DBaseTextureVtbl8    g_d3dBaseTextureVtbl;
extern IDirect3DTextureVtbl8        g_d3dTextureVtbl;
extern IDirect3DCubeTextureVtbl8    g_d3dCubeTextureVtbl;
extern IDirect3DSurfaceVtbl8        g_d3dSurfaceVtbl;
extern IDirect3DVertexBufferVtbl8   g_d3dVertexBufferVtbl;
extern IDirect3DPushBufferVtbl8     g_d3dPushBufferVtbl;

typedef struct D3DLock {
    BOOL  bLocked;
    RECT  Rect;
    DWORD Flags;
} D3DLock;

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
} D3DResourceInner;

typedef struct D3DResource IMPLEMENTS(IDirect3DResource8) {
    IDirect3DResource8 iface;
    D3DResourceInner   inner;
#ifdef __cplusplus
    ULONG AddRef() override {
        return IDirect3DResource8_AddRef(this->iface);
    }

    ULONG Release() override {
        return IDirect3DResource8_Release(this->iface);
    }

    D3DRESOURCETYPE GetType() override {
        return IDirect3DResource8_GetType(this->iface);
    }

    VOID Register(PVOID pBase) override {
        IDirect3DResource8_Register(this->iface, pBase);
    }

    BOOL IsBusy() override {
        return IDirect3DResource8_IsBusy(this->iface);
    }

    VOID BlockUntilNotBusy() override {
        IDirect3DResource8_BlockUntilNotBusy(this->iface);
    }
#endif // __cplusplus
} D3DResource;
// ============================================================================

// ============================================================================

typedef struct D3DPushBuffer IMPLEMENTS(IDirect3DPushBuffer8) {
    IDirect3DPushBuffer8 iface;
    D3DResourceInner     resource;
    DWORD                Size;       // in DWORDs
    DWORD                SizeNeeded; // in DWORDs
    BOOL                 bCpu;
    PDWORD               p;

#ifdef __cplusplus
    ULONG AddRef() override {
        return IDirect3DPushBuffer8_AddRef(this->iface);
    }

    ULONG Release() override {
        return IDirect3DPushBuffer8_Release(this->iface);
    }

    D3DRESOURCETYPE GetType() override {
        return IDirect3DPushBuffer8_GetType(this->iface);
    }

    VOID Register(PVOID pBase) override {
        IDirect3DPushBuffer8_Register(this->iface, pBase);
    }

    VOID BlockUntilNotBusy() override {
        IDirect3DPushBuffer8_BlockUntilNotBusy(this->iface);
    }

    BOOL IsBusy() override {
        return IDirect3DPushBuffer8_IsBusy(this->iface);
    }

    DWORD GetSize() override {
        return IDirect3DPushBuffer8_GetSize(this->iface);
    }
#endif // __cplusplus
} D3DPushBuffer;

BOOL D3DPushBuffer_IsFull(LPDIRECT3DPUSHBUFFER8 pThis);
HRESULT D3DPushBuffer_Push1(
    D3DPushBuffer* pThis, DWORD dwData);
HRESULT D3DPushBuffer_PushCmd(
    D3DPushBuffer* pThis, DWORD cmd, DWORD dwData);
HRESULT D3DPushBuffer_PushCmd2(
    D3DPushBuffer* pThis, DWORD cmd, DWORD dwData1, DWORD dwData2);
HRESULT D3DPushBuffer_PushCmd3(
    D3DPushBuffer* pThis, DWORD cmd, DWORD dwData1, 
    DWORD dwData2, DWORD dwData3);
HRESULT D3DPushBuffer_PushCmd4(
    D3DPushBuffer* pThis, DWORD cmd, DWORD dwData1, 
    DWORD dwData2, DWORD dwData3, DWORD dwData4);
HRESULT D3DPushBuffer_PushCmdf(D3DPushBuffer* pThis, DWORD cmd, float fData);
HRESULT D3DPushBuffer_PushCmd2f(D3DPushBuffer* pThis, DWORD cmd, 
                                float fData1, float fData2);
HRESULT D3DPushBuffer_PushCmd3f(D3DPushBuffer* pThis, DWORD cmd, 
                                float fData1, float fData2, float fData3);
HRESULT D3DPushBuffer_PushCmd4f(D3DPushBuffer* pThis, DWORD cmd, 
                                float fData1, float fData2, 
                                float fData3, float fData4);
HRESULT D3DPushBuffer_PushN(
    D3DPushBuffer* pThis, DWORD dwData, SIZE_T n);
HRESULT D3DPushBuffer_PushA(
    D3DPushBuffer* pThis, CONST DWORD* pdwData, SIZE_T n);
// ============================================================================

struct D3DBaseTexture;
typedef struct D3DBaseTexture D3DBaseTexture;

// ============================================================================
typedef struct D3DSurface IMPLEMENTS(IDirect3DSurface8) {
    IDirect3DSurface8 iface;
    D3DResourceInner  resource;
    D3DSURFACE_DESC   desc;
    D3DLock           lock;
    D3DBaseTexture*   pContainer;
#ifdef __cplusplus
    UINT AddRef() override {
        return IDirect3DSurface8_AddRef(&this->iface);
    }

    UINT Release() override {
        return IDirect3DSurface8_Release(&this->iface);
    }

    D3DRESOURCETYPE GetType() override {
        return IDirect3DSurface8_GetType(&this->iface);
    }

    VOID Register(PVOID pBase) override {
        IDirect3DSurface8_Register(&this->iface, pBase);
    }

    VOID BlockUntilNotBusy() override {
        IDirect3DSurface8_BlockUntilNotBusy(&this->iface);
    }

    BOOL IsBusy() override {
        return IDirect3DSurface8_IsBusy(&this->iface);
    }

    HRESULT LockRect(
        D3DLOCKED_RECT* pLockedRect, 
        CONST RECT* pRect, DWORD Flags) override 
    {
        return iface->lpVtbl->LockRect(this->iface, pLockedRect, pRect,
                                       Flags);
    }

    HRESULT UnlockRect() override {
        return iface->lpVtbl->UnlockRect(this->iface);
    }

    HRESULT GetDesc(D3DSURFACE_DESC* pDesc) override {
        return iface->lpVtbl->GetDesc(this->iface, pDesc);
    }

    HRESULT GetContainer(
        REFIID riid, 
        PVOID* ppContainer) override 
    {
        return iface->lpVtbl->GetContainer(this->iface, riid, ppContainer);
    }
#endif // __cplusplus
} D3DSurface;
// ============================================================================

// ============================================================================
typedef struct D3DVertexBuffer IMPLEMENTS(IDirect3DVertexBuffer8) {
    IDirect3DVertexBuffer8 iface;
    D3DResourceInner       resource;
    D3DVERTEXBUFFER_DESC   desc;
    struct {
        BOOL bLocked;
        UINT Offset;
        UINT Size;
        DWORD Flags;
    } lock;
#ifdef __cplusplus
    UINT AddRef() override {
        return IDirect3DVertexBuffer8_AddRef(&this->iface);
    }

    UINT Release() override {
        return IDirect3DVertexBuffer8_Release(&this->iface);
    }

    D3DRESOURCETYPE GetType() override {
        return IDirect3DVertexBuffer8_GetType(&this->iface);
    }

    VOID Register(PVOID pBase) override {
        IDirect3DVertexBuffer8_Register(&this->iface, pBase);
    }

    VOID BlockUntilNotBusy() override {
        IDirect3DVertexBuffer8_BlockUntilNotBusy(&this->iface);
    }

    BOOL IsBusy() override {
        return IDirect3DVertexBuffer8_IsBusy(&this->iface);
    }

    HRESULT Lock(
        UINT OffsetToLock, UINT SizeToLock, 
        VOID** ppbData, DWORD Flags) override 
    {
        return iface->lpVtbl->Lock(&this->iface, OffsetToLock, SizeToLock,
                                   ppbData, Flags);
    }

    HRESULT Unlock() override {
        return iface->lpVtbl->Unlock(&this->iface);
    }

    HRESULT GetDesc(D3DVERTEXBUFFER_DESC* pDesc) override {
        return iface->lpVtbl->GetDesc(&this->iface, pDesc);
    }
#endif // __cplusplus
} D3DVertexBuffer;
// ============================================================================

// ============================================================================
typedef struct D3DBaseTextureInner {
    D3DResourceInner resource;
    DWORD            dwLevelCount;
} D3DBaseTextureInner;

struct D3DBaseTexture IMPLEMENTS(IDirect3DBaseTexture8) {
    IDirect3DBaseTexture8 iface;
    D3DBaseTextureInner   inner;
#ifdef __cplusplus
    UINT AddRef() override {
        return IDirect3DBaseTexture8_AddRef(&this->iface);
    }

    UINT Release() override {
        return IDirect3DBaseTexture8_Release(&this->iface);
    }

    D3DRESOURCETYPE GetType() override {
        return IDirect3DBaseTexture8_GetType(&this->iface);
    }

    VOID Register(PVOID pBase) override {
        IDirect3DBaseTexture8_Register(&this->iface, pBase);
    }

    VOID BlockUntilNotBusy() override {
        IDirect3DBaseTexture8_BlockUntilNotBusy(&this->iface);
    }

    BOOL IsBusy() override {
        return IDirect3DBaseTexture8_IsBusy(&this->iface);
    }

    DWORD GetLevelCount() {
        return IDirect3DBaseTexture8_GetLevelCount(&this->iface);
    }
#endif // __cplusplus
};
// ============================================================================

// ============================================================================
typedef struct D3DTexture IMPLEMENTS(IDirect3DTexture8) {
    IDirect3DTexture8   iface;
    D3DBaseTextureInner base;
    D3DSURFACE_DESC     desc;
    D3DSurface*         pLevels;
#ifdef __cplusplus
    UINT AddRef() override {
        return IDirect3DTexture8_AddRef(&this->iface);
    }

    UINT Release() override {
        return IDirect3DTexture8_Release(&this->iface);
    }

    D3DRESOURCETYPE GetType() override {
        return IDirect3DTexture8_GetType(&this->iface);
    }

    VOID Register(PVOID pBase) override {
        IDirect3DTexture8_Register(&this->iface, pBase);
    }

    VOID BlockUntilNotBusy() override {
        IDirect3DTexture8_BlockUntilNotBusy(&this->iface);
    }

    BOOL IsBusy() override {
        return IDirect3DTexture8_IsBusy(&this->iface);
    }

    DWORD GetLevelCount() {
        return IDirect3DTexture8_GetLevelCount(&this->iface);
    }

    HRESULT GetLevelDesc(UINT Level, D3DSURFACE_DESC* pDesc) {
        return IDirect3DTexture8_GetLevelDesc(&iface, Level, pDesc);
    }

    HRESULT GetSurfaceLevel(UINT Level, LPDIRECT3DSURFACE8* ppSurfaceLevel) {
        return IDirect3DTexture8_GetSurfaceLevel(&iface, Level, 
                                                 ppSurfaceLevel);
    }

    HRESULT LockRect(
        UINT Level, D3DLOCKED_RECT* pLockedRect, 
        CONST RECT* pRect, DWORD Flags) 
    {
        return IDirect3DTexture8_LockRect(&iface, Level, 
                                          pLockedRect, pRect, Flags);
    }

    HRESULT UnlockRect(UINT Level) {
        return IDirect3DTexture8_UnlockRect(&iface, Level);
    }
#endif // __cplusplus
} D3DTexture;
// ============================================================================

// ============================================================================
typedef struct D3DCubeTexture IMPLEMENTS(IDirect3DCubeTexture8) {
    IDirect3DCubeTexture8   iface;
    D3DBaseTextureInner     base;
    D3DTexture*             pCubeSurfaces;
#ifdef __cplusplus
    UINT AddRef() override {
        return IDirect3DCubeTexture8_AddRef(&this->iface);
    }

    UINT Release() override {
        return IDirect3DCubeTexture8_Release(&this->iface);
    }

    D3DRESOURCETYPE GetType() override {
        return IDirect3DCubeTexture8_GetType(&this->iface);
    }

    VOID Register(PVOID pBase) override {
        IDirect3DCubeTexture8_Register(&this->iface, pBase);
    }

    VOID BlockUntilNotBusy() override {
        IDirect3DCubeTexture8_BlockUntilNotBusy(&this->iface);
    }

    BOOL IsBusy() override {
        return IDirect3DCubeTexture8_IsBusy(&this->iface);
    }

    DWORD GetLevelCount() {
        return IDirect3DCubeTexture8_GetLevelCount(&this->iface);
    }

    HRESULT GetLevelDesc(UINT Level, D3DSURFACE_DESC* pDesc) {
        return IDirect3DCubeTexture8_GetLevelDesc(&iface, Level, pDesc);
    }

    HRESULT GetSurfaceLevel(UINT Level, LPDIRECT3DSURFACE8* ppSurfaceLevel) {
        return IDirect3DCubeTexture8_GetSurfaceLevel(&iface, Level, 
                                                     ppSurfaceLevel);
    }

    HRESULT LockRect(
        UINT Level, D3DLOCKED_RECT* pLockedRect, 
        CONST RECT* pRect, DWORD Flags) 
    {
        return IDirect3DCubeTexture8_LockRect(&iface, Level, 
                                          pLockedRect, pRect, Flags);
    }

    HRESULT UnlockRect(UINT Level) {
        return IDirect3DCubeTexture8_UnlockRect(&iface, Level);
    }
#endif // __cplusplus
} D3DCubeTexture;
// ============================================================================

VOID D3D_CreateResource(D3DRESOURCETYPE type, DWORD Data, 
                        PVOID pContiguousMemory, 
                        D3DResourceInner* pResource);
HRESULT D3D_CreateSurface(UINT Width, UINT Height, D3DFORMAT Format,
                          DWORD Usage, 
                          D3DMULTISAMPLE_TYPE MultiSampleType, 
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