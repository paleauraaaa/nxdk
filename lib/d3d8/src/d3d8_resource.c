#include "d3d8.h"
#include "d3d8_private.h"
#include "d3d8_device.h"
#include "winerror.h"
#include "d3d8_resource.h"

#include <assert.h>
#include <stdlib.h>
#include <stdbool.h>

#include <pbkit/pbkit_pushbuffer.h>

IDirect3DResourceVtbl8*         g_pResourceVtbl     = NULL;
IDirect3DBaseTextureVtbl8*      g_pBaseTextureVtbl  = NULL;
IDirect3DTextureVtbl8*          g_pTextureVtbl      = NULL;
IDirect3DCubeTextureVtbl8*      g_pCubeTextureVtbl  = NULL;
IDirect3DSurfaceVtbl8*          g_pSurfaceVtbl      = NULL;
IDirect3DVertexBufferVtbl8*     g_pVertexBufferVtbl = NULL;
IDirect3DPushBufferVtbl8*       g_pPushBufferVtbl   = NULL;

// ============================================================================
ULONG D3DRefcount_AddRef(D3DRefcount* pThis) {
    // prevent overflow
    if (pThis->c == ~((ULONG)0)) {
        assert(false);
        return pThis->c;
    }
    return pThis->c++;
}

ULONG D3DRefcount_Release(D3DRefcount *pThis) {
    // prevent underflow
    if (pThis->c == 0) {
        assert(false);
        return 0;
    }

    return pThis->c--;
}
// ============================================================================

// ============================================================================
D3DAPI ULONG D3DResource_AddRef(LPDIRECT3DRESOURCE8 pThis) {
    return D3DRefcount_AddRef(&((D3DResource*)pThis)->inner.refcount);
}

D3DAPI ULONG D3DResource_Release(LPDIRECT3DRESOURCE8 pThis) {
    D3DResource* resource = (D3DResource*)pThis;
    ULONG refcount = D3DRefcount_Release(&resource->inner.refcount);
    if (refcount > 0)
        return refcount;

    if (resource->inner.bManuallyRegistered != TRUE)
        D3D_FreeContiguousMemory(resource->inner.pContiguousMemory);

    free(resource);
    return 0;
}

D3DAPI D3DRESOURCETYPE D3DResource_GetType(LPDIRECT3DRESOURCE8 pThis) {
    return ((D3DResource*)pThis)->inner.type;
}

D3DAPI VOID D3DResource_Register(LPDIRECT3DRESOURCE8 pThis, PVOID pBase) {
    D3DResource* resource = (D3DResource*)pThis;
    resource->inner.pContiguousMemory =
        (PVOID)((DWORD)pBase + resource->inner.Data);
    resource->inner.Data += (DWORD)MmGetPhysicalAddress(pBase);
}

D3DAPI BOOL D3DResource_IsBusy(LPDIRECT3DRESOURCE8 pThis) {
    D3DResource* resource = (D3DResource*)pThis;
    if (D3DDevice_GetLastCompletedFence() >= resource->inner.Fence) {
        resource->inner.Fence = 0;
        return FALSE;
    }
    return TRUE;
}

D3DAPI VOID D3DResource_BlockUntilNotBusy(LPDIRECT3DRESOURCE8 pThis) {
    D3DResource* resource = (D3DResource*)pThis;
    D3DDevice_BlockOnFence(resource->inner.Fence);
}
// ============================================================================

// ============================================================================
ULONG D3DPushBuffer_AddRef(LPDIRECT3DPUSHBUFFER8 pThis) {
    return D3DResource_AddRef((LPDIRECT3DRESOURCE8)pThis);
}

ULONG D3DPushBuffer_Release(LPDIRECT3DPUSHBUFFER8 pThis) {
    D3DPushBuffer* pb = (D3DPushBuffer*)pThis;
    ULONG refcount = D3DRefcount_Release(&pb->resource.refcount);
    if (refcount > 0)
        return refcount;

    if (pb->resource.bManuallyRegistered != TRUE) {
        if (pb->bCpu == TRUE) {
            free(pb->p);
        }
        else {
            D3D_FreeContiguousMemory(pb->resource.pContiguousMemory);
        }
    }

    free(pb);
    return 0;
}

D3DRESOURCETYPE D3DPushBuffer_GetType(
    LPDIRECT3DPUSHBUFFER8 pThis)
{
    return D3DResource_GetType((LPDIRECT3DRESOURCE8)pThis);
}

VOID D3DPushBuffer_Register(LPDIRECT3DPUSHBUFFER8 pThis,
                                          PVOID pBase)
{
    D3DResource_Register((LPDIRECT3DRESOURCE8)pThis, pBase);
}

VOID D3DPushBuffer_BlockUntilNotBusy(
    LPDIRECT3DPUSHBUFFER8 pThis)
{
    return;
}

BOOL D3DPushBuffer_IsBusy(LPDIRECT3DPUSHBUFFER8 pThis) {
    return FALSE;
}

HRESULT D3DPushBuffer_GetSize(LPDIRECT3DPUSHBUFFER8 pThis,
                              UINT* pSize)
{
    D3DPushBuffer* pPB = (D3DPushBuffer*)pThis;
    *pSize = ((D3DPushBuffer*)pThis)->SizeNeeded;
    return D3D_OK;
}

HRESULT D3DPushBuffer_GetData(LPDIRECT3DPUSHBUFFER8 pThis,
                              CONST DWORD** ppData)
{
    *ppData = ((D3DPushBuffer*)pThis)->pLastPush;
    return D3D_OK;
}

BOOL D3DPushBuffer_IsFull(LPDIRECT3DPUSHBUFFER8 pThis) {
    D3DPushBuffer* pb = (D3DPushBuffer*)pThis;
    return pb->SizeNeeded >= pb->Size;
}

typedef union _D3DPushData {
    DWORD d;
    float f;
} D3DPushData, *PD3DPUSHDATA;

HRESULT D3DPushBuffer_Push(D3DPushBuffer* pThis, PD3DPUSHDATA pData, SIZE_T n, BOOL bLoop) {
    pThis->SizeNeeded += n;
    if (pThis->SizeNeeded > pThis->Size)
        return D3DERR_BUFFERTOOSMALL;

    PDWORD pTail = (PDWORD)pThis->resource.pContiguousMemory + pThis->Size - 1;
    if (pThis->p + n >= pTail) {
        if (bLoop == TRUE) {
            while (pThis->p != pTail) {
                *pThis->p++ =
                    D3D_NV2A_PFIFO_ENCODE_3D_METHOD(NV097_NO_OPERATION, 0);
            }
            pThis->p = pThis->resource.pContiguousMemory;
        }
        else {
            return D3DERR_BUFFERTOOSMALL;
        }
    }

    memcpy(pThis->p, pData, n * sizeof(DWORD));
    pThis->p += n;
    return D3D_OK;
}

HRESULT D3DPushBuffer_Push1(
    D3DPushBuffer* pThis, DWORD dwData, BOOL bLoop)
{
    D3DPushData data[] = { { .d = dwData} };
    return D3DPushBuffer_Push(pThis, &data[0], 1, bLoop);
}

HRESULT D3DPushBuffer_PushA(
    D3DPushBuffer* pThis, CONST DWORD* pdwData, SIZE_T n, BOOL bLoop)
{
    return D3DPushBuffer_Push(pThis, (PD3DPUSHDATA)pdwData, n, bLoop);
}

HRESULT D3DPushBuffer_PushCmd(
    D3DPushBuffer* pThis, DWORD cmd, DWORD dwData, BOOL bLoop)
{
    DWORD method = D3D_NV2A_PFIFO_ENCODE_3D_METHOD(cmd, 1);

    D3DPushData data[2] = { { .d = method }, { .d = dwData } };

    return D3DPushBuffer_Push(pThis, &data[0], 2, bLoop);
}

HRESULT D3DPushBuffer_PushCmd2(
    D3DPushBuffer* pThis, DWORD cmd, DWORD dwData1,
    DWORD dwData2, BOOL bIncrement, BOOL bLoop)
{
    if (bIncrement == FALSE)
        cmd = NV2A_SUPPRESS_COMMAND_INCREMENT(cmd);

    DWORD method = D3D_NV2A_PFIFO_ENCODE_3D_METHOD(cmd, 2);

    D3DPushData data[3] = {
        { .d = method }, { .d = dwData1 }, { .d = dwData2 }
    };

    return D3DPushBuffer_Push(pThis, &data[0], 3, bLoop);
}

HRESULT D3DPushBuffer_PushCmd3(
    D3DPushBuffer* pThis, DWORD cmd, DWORD dwData1,
    DWORD dwData2, DWORD dwData3, BOOL bIncrement, BOOL bLoop)
{
    if (bIncrement == FALSE)
        cmd = NV2A_SUPPRESS_COMMAND_INCREMENT(cmd);

    DWORD method = D3D_NV2A_PFIFO_ENCODE_3D_METHOD(cmd, 3);
    D3DPushData data[4] = {
        { .d = method }, { .d = dwData1 }, { .d = dwData2 }, { .d = dwData3 }
    };

    return D3DPushBuffer_Push(pThis, &data[0], 4, bLoop);
}

HRESULT D3DPushBuffer_PushCmd4(
    D3DPushBuffer* pThis, DWORD cmd, DWORD dwData1,
    DWORD dwData2, DWORD dwData3, DWORD dwData4, BOOL bIncrement, BOOL bLoop)
{
    if (bIncrement == FALSE)
        cmd = NV2A_SUPPRESS_COMMAND_INCREMENT(cmd);

    DWORD method = D3D_NV2A_PFIFO_ENCODE_3D_METHOD(cmd, 4);
    D3DPushData data[5] = {
        { .d = method },
        { .d = dwData1 }, { .d = dwData2 },
        { .d = dwData3 }, { .d = dwData4 }
    };

    return D3DPushBuffer_Push(pThis, &data[0], 5, bLoop);
}

HRESULT D3DPushBuffer_PushCmdA(
    D3DPushBuffer* pThis, DWORD cmd, CONST DWORD* pdwData,
    SIZE_T n, BOOL bIncrement, BOOL bLoop)
{
    if (bIncrement == FALSE)
        cmd = NV2A_SUPPRESS_COMMAND_INCREMENT(cmd);

    // TODO: don't allocate here.
    DWORD method = D3D_NV2A_PFIFO_ENCODE_3D_METHOD(cmd, n);
    PD3DPUSHDATA pData = malloc((n + 1) * sizeof(*pData));
    D3D_ASSERT_IF(E_OUTOFMEMORY, pData == NULL);
    pData[0].d = method;
    memcpy(&pData[1], pdwData, n * sizeof(DWORD));
    HRESULT hr = D3DPushBuffer_Push(pThis, pData, n + 1, bLoop);
    free(pData);
    return hr;
}

HRESULT D3DPushBuffer_PushCmdf(D3DPushBuffer* pThis, DWORD cmd,
                               float fData, BOOL bLoop)
{
    DWORD method = D3D_NV2A_PFIFO_ENCODE_3D_METHOD(cmd, 1);

    D3DPushData data[2] = { { .d = method }, { .f = fData } };

    return D3DPushBuffer_Push(pThis, &data[0], 2, bLoop);
}

HRESULT D3DPushBuffer_PushCmd2f(D3DPushBuffer* pThis, DWORD cmd,
                                float fData1, float fData2,
                                BOOL bIncrement, BOOL bLoop)
{
    if (bIncrement == FALSE)
        cmd = NV2A_SUPPRESS_COMMAND_INCREMENT(cmd);

    DWORD method = D3D_NV2A_PFIFO_ENCODE_3D_METHOD(cmd, 2);

    D3DPushData data[3] = {
        { .d = method }, { .f = fData1 }, { .f = fData2 }
    };

    return D3DPushBuffer_Push(pThis, &data[0], 3, bLoop);
}

HRESULT D3DPushBuffer_PushCmd3f(D3DPushBuffer* pThis, DWORD cmd, float fData1,
                                float fData2, float fData3, BOOL bIncrement,
                                BOOL bLoop)
{
    if (bIncrement == FALSE)
        cmd = NV2A_SUPPRESS_COMMAND_INCREMENT(cmd);

    DWORD method = D3D_NV2A_PFIFO_ENCODE_3D_METHOD(cmd, 3);

    D3DPushData data[4] = {
        { .d = method }, { .f = fData1 }, { .f = fData2 }, { .f = fData3 }
    };

    return D3DPushBuffer_Push(pThis, &data[0], 4, bLoop);
}

HRESULT D3DPushBuffer_PushCmd4f(D3DPushBuffer* pThis, DWORD cmd, float fData1,
                                float fData2, float fData3, float fData4,
                                BOOL bIncrement, BOOL bLoop)
{
    if (bIncrement == FALSE)
        cmd = NV2A_SUPPRESS_COMMAND_INCREMENT(cmd);

    DWORD method = D3D_NV2A_PFIFO_ENCODE_3D_METHOD(cmd, 4);

    D3DPushData data[5] = {
        { .d = method },
        { .f = fData1 }, { .f = fData2 },
        { .f = fData3 }, { .f = fData3 },
    };

    return D3DPushBuffer_Push(pThis, &data[0], 5, bLoop);
}

HRESULT D3DPushBuffer_PushJump(D3DPushBuffer* pThis, PVOID vaddr, BOOL bLoop) {
    return D3DPushBuffer_Push1(
        pThis, D3D_NV2A_PFIFO_ENCODE_JUMP(vaddr), bLoop);
}

DWORD D3DPushBuffer_BytesRemaining(LPDIRECT3DPUSHBUFFER8 pThis) {
    D3DPushBuffer* pPB = (D3DPushBuffer*)pThis;
    assert((DWORD)pPB->p > (DWORD)pPB->resource.pContiguousMemory);
    DWORD offset = (DWORD)pPB->p - (DWORD)pPB->resource.pContiguousMemory;
    return pPB->Size * sizeof(DWORD) - offset;
}

BOOL D3D_VerifyMethod(DWORD method) {
    if (method == 0)
        return FALSE;

    // Malformed command. Bit 0 indicates a jump, but for now those will fail
    // verification.
    if (method & 0x00000003)
        return FALSE;

    // Malformed subchannel.
    if (method & 0x00030000)
        return FALSE;

    // Bit 31 is reserved for methods.
    // Bit 29 indicates an "old" jump, which D3D doesn't use.
    if (method & 0xA0000000)
        return FALSE;

    return TRUE;
}

HRESULT D3DPushBuffer_Verify(LPDIRECT3DPUSHBUFFER8 pThis, PDWORD pdwPos) {
    D3DPushBuffer* pPB = (D3DPushBuffer*)pThis;
    if (pPB->SizeNeeded == 0)
        return 0;

    PDWORD pHead = pPB->resource.pContiguousMemory;
    PDWORD pEnd  = pHead + pPB->SizeNeeded;
    PDWORD p = pHead;
    do {
        DWORD method = *p;
        DWORD pos = (DWORD)((DWORD)p - (DWORD)pHead);
        if (D3D_VerifyMethod(method) == FALSE) {
            if (pdwPos)
                *pdwPos = pos;
            return D3DERR_DRIVERINTERNALERROR;
        }
        DWORD nparam = D3D_NV2A_PFIFO_METHOD_NPARAM(method);
        if ((nparam == 0 &&
             D3D_NV2A_PFIFO_METHOD_CMD(method) != NV097_NO_OPERATION) ||
            nparam > 16)
        {
            if (pdwPos)
                *pdwPos = pos;
            return D3DERR_DRIVERINTERNALERROR;
        }
        p += nparam + 1;
    } while (p < pEnd);
    return D3D_OK;
}
// ============================================================================

// ============================================================================
ULONG D3DSurface_AddRef(LPDIRECT3DSURFACE8 pThis) {
    return D3DResource_AddRef((LPDIRECT3DRESOURCE8)pThis);
}

ULONG D3DSurface_Release(LPDIRECT3DSURFACE8 pThis) {
    return D3DResource_Release((LPDIRECT3DRESOURCE8)pThis);
}

D3DRESOURCETYPE D3DSurface_GetType(LPDIRECT3DSURFACE8 pThis) {
    return D3DResource_GetType((LPDIRECT3DRESOURCE8) pThis);
}

VOID D3DSurface_Register(LPDIRECT3DSURFACE8 pThis, PVOID pBase) {
    D3DResource_Register((LPDIRECT3DRESOURCE8)pThis, pBase);
}

BOOL D3DSurface_IsBusy(LPDIRECT3DSURFACE8 pThis) {
    return D3DResource_IsBusy((LPDIRECT3DRESOURCE8)pThis);
}

VOID D3DSurface_BlockUntilNotBusy(LPDIRECT3DSURFACE8 pThis) {
    D3DResource_BlockUntilNotBusy((LPDIRECT3DRESOURCE8)pThis);
}

HRESULT D3DSurface_GetContainer(
    LPDIRECT3DSURFACE8 pThis,
    LPDIRECT3DBASETEXTURE8* ppContainer,
    REFIID riid)
{
    *ppContainer = NULL;
    assert(false); // TODO: implement
    return E_FAIL;
}
HRESULT D3DSurface_GetDesc(LPDIRECT3DSURFACE8 pThis,
                                         D3DSURFACE_DESC* pDesc)
{
    memcpyp(pDesc, &((D3DSurface*)pThis)->desc);
    return D3D_OK;
}
HRESULT D3DSurface_LockRect(LPDIRECT3DSURFACE8 pThis,
                            D3DLOCKED_RECT* pLockedRect,
                            CONST RECT* pRect, DWORD Flags)
{
    D3DSurface* surface    = (D3DSurface*)pThis;
    if (Flags != (D3DLOCK_READONLY | D3DLOCK_NOOVERWRITE) &&
        D3DResource_IsBusy((LPDIRECT3DRESOURCE8)pThis))
    {
        D3DResource_BlockUntilNotBusy((LPDIRECT3DRESOURCE8)pThis);
    }

    surface->resource.Fence    =  D3DDevice_GetCurrentFence();
    surface->lock.Flags        =  Flags;
    if (pRect)
        surface->lock.Rect = *pRect;
    else
        memzrop(&surface->lock.Rect);

    DWORD bytesPerPixel = D3D_FormatBytesPerPixel(surface->desc.Format);
    DWORD width;
    if (pRect)
        width = pRect->right - pRect->left;
    else
        width = surface->desc.Width;


    if (pRect == NULL || (pRect->left == 0 && pRect->top == 0 &&
        pRect->right == surface->desc.Width &&
        pRect->bottom == surface->desc.Height))
    {
        pLockedRect->pBits = surface->resource.pContiguousMemory;
        pLockedRect->Pitch = width * bytesPerPixel;
        return D3D_OK;
    }
    else {
        // TODO: implement partial locks
        surface->resource.Fence = 0;
        return E_NOTIMPL;
    }
}

HRESULT D3DSurface_UnlockRect(LPDIRECT3DSURFACE8 pThis) {
    return D3D_OK;
}
// ============================================================================

// ============================================================================
ULONG D3DVertexBuffer_AddRef(LPDIRECT3DVERTEXBUFFER8 pThis) {
    return D3DResource_AddRef((LPDIRECT3DRESOURCE8)pThis);
}

ULONG D3DVertexBuffer_Release(LPDIRECT3DVERTEXBUFFER8 pThis) {
    return D3DResource_Release((LPDIRECT3DRESOURCE8)pThis);
}

D3DRESOURCETYPE D3DVertexBuffer_GetType(
    LPDIRECT3DVERTEXBUFFER8 pThis)
{
    return D3DResource_GetType((LPDIRECT3DRESOURCE8)pThis);
}

VOID D3DVertexBuffer_Register(LPDIRECT3DVERTEXBUFFER8 pThis,
                                            PVOID pBase)
{
    D3DResource_Register((LPDIRECT3DRESOURCE8)pThis, pBase);
}

BOOL D3DVertexBuffer_IsBusy(LPDIRECT3DVERTEXBUFFER8 pThis) {
    return D3DResource_IsBusy((LPDIRECT3DRESOURCE8)pThis);
}

VOID D3DVertexBuffer_BlockUntilNotBusy(
    LPDIRECT3DVERTEXBUFFER8 pThis)
{
    D3DResource_BlockUntilNotBusy((LPDIRECT3DRESOURCE8)pThis);
}

HRESULT D3DVertexBuffer_GetDesc(LPDIRECT3DVERTEXBUFFER8 pThis,
                                              D3DVERTEXBUFFER_DESC* pDesc)
{
    memcpyp(pDesc, &((D3DVertexBuffer*)pThis)->desc);
    return D3D_OK;
}

HRESULT D3DVertexBuffer_Lock(LPDIRECT3DVERTEXBUFFER8 pThis,
                                           UINT OffsetToLock, UINT SizeToLock,
                                           BYTE** ppbData, DWORD Flags)
{
    D3DVertexBuffer* vb = (D3DVertexBuffer*)pThis;
    if (Flags != (D3DLOCK_READONLY | D3DLOCK_NOOVERWRITE) &&
        D3DResource_IsBusy((LPDIRECT3DRESOURCE8)pThis))
    {
        D3DResource_BlockUntilNotBusy((LPDIRECT3DRESOURCE8)pThis);
    }

    vb->resource.Fence = D3DDevice_GetCurrentFence();
    *ppbData  = (BYTE*)vb->resource.pContiguousMemory;

    return D3D_OK;
}

HRESULT D3DVertexBuffer_Unlock(LPDIRECT3DVERTEXBUFFER8 pThis) {
    return D3D_OK;
}
// ============================================================================

// ============================================================================
ULONG D3DBaseTexture_AddRef(LPDIRECT3DBASETEXTURE8 pThis) {
    return D3DResource_AddRef((LPDIRECT3DRESOURCE8)pThis);
}

ULONG D3DBaseTexture_Release(LPDIRECT3DBASETEXTURE8 pThis) {
    return D3DResource_Release((LPDIRECT3DRESOURCE8)pThis);
}

D3DRESOURCETYPE D3DBaseTexture_GetType(
    LPDIRECT3DBASETEXTURE8 pThis)
{
    return D3DResource_GetType((LPDIRECT3DRESOURCE8)pThis);
}

VOID D3DBaseTexture_Register(LPDIRECT3DBASETEXTURE8 pThis,
                                           PVOID pBase)
{
    D3DResource_Register((LPDIRECT3DRESOURCE8)pThis, pBase);
}

BOOL D3DBaseTexture_IsBusy(LPDIRECT3DBASETEXTURE8 pThis) {
    return D3DResource_IsBusy((LPDIRECT3DRESOURCE8)pThis);
}

VOID D3DBaseTexture_BlockUntilNotBusy(
    LPDIRECT3DBASETEXTURE8 pThis)
{
    D3DResource_BlockUntilNotBusy((LPDIRECT3DRESOURCE8)pThis);
}

DWORD D3DBaseTexture_GetLevelCount(
    LPDIRECT3DBASETEXTURE8 pThis)
{
    return ((D3DBaseTexture*)pThis)->inner.dwLevelCount;
}
// ============================================================================

// ============================================================================
ULONG D3DTexture_AddRef(LPDIRECT3DTEXTURE8 pThis) {
    return D3DBaseTexture_AddRef((LPDIRECT3DBASETEXTURE8)pThis);
}

ULONG D3DTexture_Release(LPDIRECT3DTEXTURE8 pThis) {
    D3DTexture* texture = (D3DTexture*)pThis;
    ULONG refcount = texture->base.resource.refcount.c;
    if (refcount == 1)
        free(texture->pLevels);

    D3DBaseTexture_Release((LPDIRECT3DBASETEXTURE8)pThis);

    return refcount - 1;
}

D3DRESOURCETYPE D3DTexture_GetType(LPDIRECT3DTEXTURE8 pThis) {
    return D3DBaseTexture_GetType((LPDIRECT3DBASETEXTURE8)pThis);
}

VOID D3DTexture_Register(LPDIRECT3DTEXTURE8 pThis, PVOID pBase) {
    return D3DBaseTexture_Register((LPDIRECT3DBASETEXTURE8)pThis,
                                          pBase);
}

BOOL D3DTexture_IsBusy(LPDIRECT3DTEXTURE8 pThis) {
    return D3DBaseTexture_IsBusy((LPDIRECT3DBASETEXTURE8)pThis);
}

VOID D3DTexture_BlockUntilNotBusy(LPDIRECT3DTEXTURE8 pThis) {
    D3DBaseTexture_BlockUntilNotBusy((LPDIRECT3DBASETEXTURE8)pThis);
}

DWORD D3DTexture_GetLevelCount(LPDIRECT3DTEXTURE8 pThis) {
    return D3DBaseTexture_GetLevelCount((LPDIRECT3DBASETEXTURE8)pThis);
}

HRESULT D3DTexture_GetSurfaceLevel(
    LPDIRECT3DTEXTURE8 pThis, UINT Level,
    LPDIRECT3DSURFACE8* ppSurfaceLevel)
{
    D3DTexture* texture = (D3DTexture*)pThis;
#if NXDK_DEBUG
    if (Level >= texture->base.dwLevelCount)
        return D3DERR_INVALIDCALL;
#endif // NXDK_DEBUG
    *ppSurfaceLevel = &texture->pLevels[Level].iface;
    D3DSurface_AddRef(*ppSurfaceLevel);
    return D3D_OK;
}

HRESULT D3DTexture_GetLevelDesc(LPDIRECT3DTEXTURE8 pThis,
                                              UINT Level,
                                              D3DSURFACE_DESC* pDesc)
{
    D3DTexture* texture = (D3DTexture*)pThis;
#if NXDK_DEBUG
    if (Level >= texture->base.dwLevelCount)
        return D3DERR_INVALIDCALL;
#endif // NXDK_DEBUG
    *pDesc = texture->pLevels[Level].desc;
    return D3D_OK;
}

HRESULT D3DTexture_LockRect(
    LPDIRECT3DTEXTURE8 pThis, UINT Level,
    D3DLOCKED_RECT* pLockedRect, CONST RECT* pRect, DWORD Flags)
{
    D3DTexture* texture = (D3DTexture*)pThis;
#if NXDK_DEBUG
    if (Level >= texture->base.dwLevelCount)
        return D3DERR_INVALIDCALL;
#endif // NXDK_DEBUG
    D3DSurface* pLevel = &texture->pLevels[Level];
    if (Flags != (D3DLOCK_READONLY | D3DLOCK_NOOVERWRITE) &&
        D3DSurface_IsBusy((LPDIRECT3DSURFACE8)pLevel))
    {
        D3DSurface_BlockUntilNotBusy((LPDIRECT3DSURFACE8)pLevel);
    }

    pLevel->resource.Fence = D3DDevice_GetCurrentFence();
    pLevel->lock.Flags    =  Flags;
    if (pRect) {
        pLevel->lock.Rect = *pRect;
    }
    else {
        memzrop(&pLevel->lock.Rect);
    }

    if (pRect == NULL || (pRect->left == 0 && pRect->top == 0 &&
        pRect->right == pLevel->desc.Width &&
        pRect->bottom == pLevel->desc.Height))
    {
        pLockedRect->Pitch = pLevel->desc.Width *
                             D3D_FormatBytesPerPixel(texture->pLevels[0].desc.Format);
        pLockedRect->pBits = pLevel->resource.pContiguousMemory;
    }
    else {
        // TODO: implement partial locks
        texture->pLevels[Level].resource.Fence = 0;
        return E_NOTIMPL;
    }

    return D3D_OK;
}

HRESULT D3DTexture_UnlockRect(LPDIRECT3DTEXTURE8 pThis,
                                            UINT Level)
{
    D3DTexture* texture = (D3DTexture*)pThis;
#if NXDK_DEBUG
    if (Level >= (texture->base).dwLevelCount)
        return D3DERR_INVALIDCALL;
#endif // NXDK_DEBUG

    return D3D_OK;
}
// ============================================================================

// ============================================================================
ULONG D3DCubeTexture_AddRef(LPDIRECT3DCUBETEXTURE8 pThis) {
    return D3DBaseTexture_AddRef((LPDIRECT3DBASETEXTURE8)pThis);
}

ULONG D3DCubeTexture_Release(LPDIRECT3DCUBETEXTURE8 pThis) {
    D3DCubeTexture* texture = (D3DCubeTexture*)pThis;
    ULONG refcount = texture->base.resource.refcount.c;
    if (refcount == 1) {
        for (int i = 0; i < D3DCUBEMAP_FACE_MAX; i++)
            D3DTexture_Release(&texture->pCubeSurfaces[i].iface);
    }

    D3DBaseTexture_Release((LPDIRECT3DBASETEXTURE8)pThis);
    return refcount - 1;
}

D3DRESOURCETYPE D3DCubeTexture_GetType(
    LPDIRECT3DCUBETEXTURE8 pThis)
{
    return D3DBaseTexture_GetType((LPDIRECT3DBASETEXTURE8)pThis);
}

VOID D3DCubeTexture_Register(
    LPDIRECT3DCUBETEXTURE8 pThis, PVOID pBase)
{
    return D3DBaseTexture_Register((LPDIRECT3DBASETEXTURE8)pThis,
                                          pBase);
}

BOOL D3DCubeTexture_IsBusy(LPDIRECT3DCUBETEXTURE8 pThis) {
    return D3DBaseTexture_IsBusy((LPDIRECT3DBASETEXTURE8)pThis);
}

VOID D3DCubeTexture_BlockUntilNotBusy(
    LPDIRECT3DCUBETEXTURE8 pThis)
{
    D3DBaseTexture_BlockUntilNotBusy((LPDIRECT3DBASETEXTURE8)pThis);
}

DWORD D3DCubeTexture_GetLevelCount(LPDIRECT3DCUBETEXTURE8 pThis)
{
    return D3DBaseTexture_GetLevelCount((LPDIRECT3DBASETEXTURE8)pThis);
}

HRESULT D3DCubeTexture_GetLevelDesc(LPDIRECT3DCUBETEXTURE8 pThis,
                                                  UINT Level,
                                                  D3DSURFACE_DESC* pDesc)
{
    D3DCubeTexture* texture = (D3DCubeTexture*)pThis;
    return D3DTexture_GetLevelDesc(&texture->pCubeSurfaces[0].iface,
                                          Level, pDesc);
}

HRESULT D3DCubeTexture_LockRect(
    LPDIRECT3DCUBETEXTURE8 pThis, D3DCUBEMAP_FACES FaceType, UINT Level,
    D3DLOCKED_RECT* pLockedRect, CONST RECT* pRect, DWORD Flags)
{
#if NXDK_DEBUG
    if (FaceType >= D3DCUBEMAP_FACE_MAX)
        return D3DERR_INVALIDCALL;
#endif // NXDK_DEBUG
    D3DCubeTexture* texture = (D3DCubeTexture*)pThis;
    return D3DTexture_LockRect(&texture->pCubeSurfaces[FaceType].iface,
                               Level, pLockedRect, pRect, Flags);
}

HRESULT D3DCubeTexture_UnlockRect(LPDIRECT3DCUBETEXTURE8 pThis,
                                  D3DCUBEMAP_FACES FaceType,
                                  UINT Level)
{
#if NXDK_DEBUG
    if (FaceType >= D3DCUBEMAP_FACE_MAX)
        return D3DERR_INVALIDCALL;
#endif // NXDK_DEBUG
    D3DCubeTexture* texture = (D3DCubeTexture*)pThis;
    return D3DTexture_UnlockRect(
        &texture->pCubeSurfaces[FaceType].iface, Level);
}

HRESULT D3DCubeTexture_GetCubeMapSurface(
    LPDIRECT3DCUBETEXTURE8 pThis,
    D3DCUBEMAP_FACES FaceType, UINT Level,
    LPDIRECT3DSURFACE8* ppCubeMapSurface)
{
#if NXDK_DEBUG
    if (FaceType >= D3DCUBEMAP_FACE_MAX)
        return D3DERR_INVALIDCALL;
#endif // NXDK_DEBUG
    D3DCubeTexture* texture = (D3DCubeTexture*)pThis;
    return D3DTexture_GetSurfaceLevel(
        &texture->pCubeSurfaces[FaceType].iface, Level, ppCubeMapSurface);
}

// ============================================================================

// Does not allocate.
VOID D3D_CreateResource(D3DRESOURCETYPE type, DWORD Data,
                        PVOID pContiguousMemory,
                        D3DResourceInner* pResource)
{
    pResource->refcount.c          = 1;
    pResource->type                = type;
    pResource->bManuallyRegistered = FALSE;
    pResource->Data                = Data;
    pResource->pContiguousMemory   = pContiguousMemory;
    pResource->Fence               = 0;
}


HRESULT D3D_CreateSurface(UINT Width, UINT Height, D3DFORMAT Format,
                          DWORD Usage,
                          D3DMULTISAMPLE_TYPE MultiSampleType,
                          PVOID pContiguousMemory,
                          D3DBaseTexture* pContainer,
                          D3DSurface* pSurf)
{
    UINT Size = Width * Height *  D3D_FormatBytesPerPixel(Format);
    if (pContiguousMemory == NULL) {
        pContiguousMemory =
            D3D_AllocContiguousMemory(Size, D3DSURFACE_ALIGNMENT);

        if (pContiguousMemory == NULL)
            return D3DERR_OUTOFVIDEOMEMORY;
    }
    pSurf->iface.lpVtbl = g_pSurfaceVtbl;
    pSurf->pContainer = pContainer;
    D3D_CreateResource(D3DRTYPE_SURFACE, 0, pContiguousMemory,
                       &pSurf->resource);
    pSurf->desc.Format = Format;
    pSurf->desc.Type = D3DRTYPE_SURFACE;
    pSurf->desc.Usage = Usage;
    pSurf->desc.Size =
        Width *
        Height *
        D3D_FormatBytesPerPixel(Format);
    pSurf->desc.MultiSampleType = MultiSampleType;
    pSurf->desc.Width = Width;
    pSurf->desc.Height = Height;
    if (pContiguousMemory != NULL)
        D3DSurface_Register(&pSurf->iface, pContiguousMemory);

    return D3D_OK;
}

// Creates a push buffer, allocating if pContiguousMemory == NULL. Infallible
// if pContiguousMemory != NULL.
//
// Parameters:
// * Size - Size of the push buffer in DWORDs. If pContiguousMemory != NULL, it
// is the caller's responsibility to ensure the allocation is large enough.
// * bCpu - Determines whether the push buffer will be run by being copied
// directly into the active push buffer (TRUE), or by jumping to the push
// buffer and then back (FALSE). For small push buffers, bCpu == TRUE is
// likely to be faster, but for larger push buffers and/or push buffers that
// are commonly reused, bCpu == FALSE is likely to be faster.
// * pContiguousMemory - Optional pointer to contiguous memory where the push
// buffer is to reside. Must be NULL if bCpu == TRUE. If non-NULL, it is the
// caller's responsibility to ensure the allocation is large enough.
// * pPB - Out parameter to receive the push buffer.
HRESULT D3D_CreatePushBuffer(
    DWORD Size, BOOL bCpu, PVOID pContiguousMemory, D3DPushBuffer* pPB)
{
#if NXDK_DEBUG
    if (bCpu == TRUE && pContiguousMemory != NULL)
        return D3DERR_INVALIDCALL;
#endif // NXDK_DEBUG
    pPB->iface.lpVtbl = g_pPushBufferVtbl;
    D3D_CreateResource(D3DRTYPE_PUSHBUFFER, 0,
                       pContiguousMemory, &pPB->resource);
    pPB->Size = Size / sizeof(DWORD);
    pPB->SizeNeeded = 0;
    pPB->bCpu = bCpu;
    if (bCpu == TRUE) {
        pPB->p = (PDWORD)malloc(Size);
        D3D_ASSERT_IF(E_OUTOFMEMORY, pPB->p == NULL);
        pPB->resource.pContiguousMemory = pPB->p;
    } else {
        if (pContiguousMemory != NULL) {
            pPB->resource.pContiguousMemory = (PDWORD)pContiguousMemory;
        } else {
            pPB->resource.pContiguousMemory =
                (PDWORD)D3D_AllocContiguousMemory(Size,
                                                  D3DPUSHBUFFER_ALIGNMENT);
            D3D_ASSERT_IF(D3DERR_OUTOFVIDEOMEMORY, pPB->resource.pContiguousMemory == NULL)
        }
        pPB->p = pPB->resource.pContiguousMemory;
    }
    pPB->pLastPush = pPB->p;
    return D3D_OK;
}

HRESULT D3D_CreateTexture(UINT Width, UINT Height, UINT Levels,
                          DWORD Usage, D3DFORMAT Format,
                          PVOID pContiguousMemory,
                          D3DTexture* pTex)
{
    int bytesPerPixel = D3D_FormatBytesPerPixel(Format);
    UINT BaseSize = Width * Height * bytesPerPixel;
    UINT SizeIncludingMips = 0;
    if (Levels == 0) {
        UINT w = Width;
        UINT h = Height;
        while (w > 1 || h > 1) {
            Levels++;
            UINT Size = w * h * bytesPerPixel;
            SizeIncludingMips += Size;
            if (w > 1) w >>= 1;
            if (h > 1) h >>= 1;
        }
    } else {
        SizeIncludingMips = BaseSize;
        Levels = 1;
    }

    D3DSurface* pLevels = malloc(sizeof(*pLevels) * Levels);
    if (!pLevels)
        return E_OUTOFMEMORY;

    if (pContiguousMemory == NULL) {
        pContiguousMemory =
            D3D_AllocContiguousMemory(SizeIncludingMips, D3DTEXTURE_ALIGNMENT);
    }

    if (pContiguousMemory == NULL) {
        free(pLevels);
        return D3DERR_OUTOFVIDEOMEMORY;
    }

    D3D_CreateResource(D3DRTYPE_TEXTURE, 0, pContiguousMemory,
                       &pTex->base.resource);

    UINT w = Width;
    UINT h = Height;
    UINT offset = 0;
    for (int i = 0; i < Levels; i++) {
        offset += w * h * bytesPerPixel;
        D3D_CreateSurface(w, h, Format, Usage, 0,
                          (PVOID)((DWORD)pContiguousMemory + offset),
                          (D3DBaseTexture*)pTex, &pLevels[i]);
        w >>= 1;
        h >>= 1;
    }

    pTex->iface.lpVtbl          = g_pTextureVtbl;
    pTex->base.dwLevelCount     = Levels;
    pTex->pLevels               = pLevels;
    return D3D_OK;
}

HRESULT D3D_CreateCubeTexture(UINT EdgeLength, UINT Levels,
                              D3DFORMAT Format, PVOID pContiguousMemory,
                              D3DCubeTexture* pTex)
{
    int bytesPerPixel = D3D_FormatBytesPerPixel(Format);
    UINT BaseSize = EdgeLength * EdgeLength * bytesPerPixel;
    UINT SizeIncludingMips = 0;
    if (Levels == 0) {
        UINT l = EdgeLength;
        while (l > 1) {
            Levels++;
            UINT Size = l * l * bytesPerPixel;
            SizeIncludingMips += Size;
            if (l > 1) l >>= 1;
        }
    } else {
        SizeIncludingMips = BaseSize;
        Levels = 1;
    }

    pTex->pCubeSurfaces = malloc(sizeof(*pTex->pCubeSurfaces) * D3DCUBEMAP_FACE_MAX);
    PVOID pcm = pContiguousMemory;
    if (pcm == NULL) {
        pcm = D3D_AllocContiguousMemory(
            SizeIncludingMips * D3DCUBEMAP_FACE_MAX,
            D3DTEXTURE_ALIGNMENT);
    }

    if (pContiguousMemory == NULL) {
        free(pTex->pCubeSurfaces);
        return D3DERR_OUTOFVIDEOMEMORY;
    }

    if (!pTex->pCubeSurfaces)
        return E_OUTOFMEMORY;
    for (int i = 0; i < D3DCUBEMAP_FACE_MAX; i++) {
        UINT offset = i * EdgeLength * EdgeLength * bytesPerPixel;
        HRESULT hr = D3D_CreateTexture(
            EdgeLength, EdgeLength, Levels, 0, Format,
            (PVOID)((DWORD)pContiguousMemory + offset),
            &pTex->pCubeSurfaces[i]);
        if (FAILED(hr)) {
            for (int j = 0; j < i; j++)
                free(pTex->pCubeSurfaces[j].pLevels);
            free(pTex->pCubeSurfaces);
            if (pContiguousMemory == NULL)
                D3D_FreeContiguousMemory(pcm);
            return hr;
        }
    }

    pTex->iface.lpVtbl = g_pCubeTextureVtbl;
    pTex->base.dwLevelCount = Levels;
    return D3D_OK;
}
