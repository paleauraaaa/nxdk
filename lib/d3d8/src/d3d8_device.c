// This file contains the implementation of D3DDevice. As IDirect3DDevice8
// accounts for the majority of D3D8's API surface and encapsulates the entire
// rendering pipeline, D3DDevice contains the bulk of the D3D implementation.
//
// It might at some point in the future be desirable to split D3DDevice into
// multiple objects (e.g. a D3DFixedFunctionShaderPipeline,
// a D3DProgrammableShaderPipeline, a D3DTextureStateManager and
// D3DRenderStateManager, etc.) and just link them together under
// IDirect3DDevice8, but for now, the entire implementation is contained in
// D3DDevice.

#include "d3d8_device.h"

#include <assert.h>
#include <stdcountof.h>

#include <pbkit/pbkit.h>

D3DDevice*            g_pDevice     = NULL;
IDirect3DDeviceVtbl8* g_pDeviceVtbl = NULL;

HRESULT D3DDevice_Nop() {
    return D3DDevice_Push1(NV097_NO_OPERATION);
}

D3DAPI HRESULT Direct3DDevice8_Nop(LPDIRECT3DDEVICE8 pThis) {
    assert(pThis == &g_pDevice->iface);
    return D3DDevice_Nop();
}

// Sets the device's current push buffer to the supplied push buffer.
// All subsequent GPU commands will be sent to this push buffer until
// D3DDevce_EndPushBuffer is called.
//
// If pPushBuffer == NULL, the default push buffer is targeted.
//
// This "resets" the push buffer, clearing all commands and setting its
// write head to the beginning of the buffer.
//
// The current push buffer is kicked before the new buffer is targeted.
HRESULT D3DDevice_BeginPushBuffer(D3DPushBuffer* pPushBuffer) {
    if (g_pDevice->pCurrentPB == &g_pDevice->DefaultPB)
        D3DDevice_KickPushBuffer();

    pPushBuffer->resource.Fence = D3DFENCE_IN_USE;
    if (pPushBuffer == NULL) {
        pb_reset();
        g_pDevice->pCurrentPB = &g_pDevice->DefaultPB;
        g_pDevice->DefaultPB.p = (PDWORD)pb_begin();
        g_pDevice->DefaultPB.SizeNeeded = 0;
        g_pDevice->DefaultPB.pLastPush = g_pDevice->DefaultPB.p;
        return D3D_OK;
    }

    g_pDevice->pCurrentPB = pPushBuffer;
    g_pDevice->pCurrentPB->p =
            g_pDevice->pCurrentPB->resource.pContiguousMemory;
    pPushBuffer->SizeNeeded = 0;
    return D3D_OK;
}

D3DAPI HRESULT Direct3DDevice8_BeginPushBuffer(
    LPDIRECT3DDEVICE8 pThis, LPDIRECT3DPUSHBUFFER8 pPushBuffer)
{
    assert(pThis == &g_pDevice->iface);
    return D3DDevice_BeginPushBuffer((D3DPushBuffer*)pPushBuffer);
}

// Stops sending commands to the currently targeted push buffer and resets
// the current push buffer to the default push buffer.
HRESULT D3DDevice_EndPushBuffer(void) {
    // We insert a NOP here for D3DDevice_RunPushBuffer to "fix up" with a jump
    // back to the default push buffer.
    D3DDevice_Nop();
    HRESULT hr = D3D_OK;
    if (g_pDevice->pCurrentPB->SizeNeeded > g_pDevice->pCurrentPB->Size)
        hr = D3DERR_BUFFERTOOSMALL;

    g_pDevice->pCurrentPB->resource.Fence = D3DDevice_GetCurrentFence();
    g_pDevice->pCurrentPB = &g_pDevice->DefaultPB;
    return hr;
}

D3DAPI HRESULT Direct3DDevice8_EndPushBuffer(LPDIRECT3DDEVICE8 pThis) {
    assert(pThis == &g_pDevice->iface);
    return D3DDevice_EndPushBuffer();
}

BOOL D3DDevice_IsKickoffReady(void) {
    if (g_pDevice->pCurrentPB == NULL)
        return FALSE;

    if (g_pDevice->pCurrentPB->SizeNeeded >= g_pDevice->KickOffSize)
        return TRUE;
    else
        return FALSE;
}

VOID D3DDevice_KickPushBuffer(void) {
    assert(g_pDevice->pCurrentPB == &g_pDevice->DefaultPB);
    assert(g_pDevice->DefaultPB.bCpu == FALSE);

    pb_end((uint32_t*)g_pDevice->DefaultPB.p);
    g_pDevice->DefaultPB.SizeNeeded = 0;
    g_pDevice->DefaultPB.pLastPush = g_pDevice->DefaultPB.p;
    g_pDevice->DefaultPB.p = (PDWORD)pb_begin();
}

VOID D3DDevice_SyncPushBuffer(void) {
    if (g_pDevice->pCurrentPB != &g_pDevice->DefaultPB)
        return;

    g_pDevice->DefaultPB.p = (PDWORD)pb_begin();
    D3DDevice_KickPushBuffer();
}

D3DAPI VOID Direct3DDevice8_SyncPushBuffer(LPDIRECT3DDEVICE8 pThis) {
    assert(pThis == &g_pDevice->iface);
    D3DDevice_SyncPushBuffer();
}

VOID D3DDevice_ResetPushBuffer(void) {
    pb_reset();
    g_pDevice->DefaultPB.p = (PDWORD)pb_begin();
}

D3DAPI VOID Direct3DDevice8_KickPushBuffer(LPDIRECT3DDEVICE8 pThis) {
    assert(pThis == &g_pDevice->iface);
    D3DDevice_KickPushBuffer();
}

HRESULT D3DDevice_PushJump(PVOID vaddr) {
    BOOL bLoop = FALSE;
    if (g_pDevice->pCurrentPB == &g_pDevice->DefaultPB)
        bLoop = TRUE;
    return D3DPushBuffer_PushJump(&g_pDevice->DefaultPB, vaddr, bLoop);
}

HRESULT D3DDevice_RunPushBuffer(D3DPushBuffer* pPushBuffer) {
    if (pPushBuffer->bCpu == TRUE) {
        if (pPushBuffer->SizeNeeded + g_pDevice->DefaultPB.SizeNeeded >
            g_pDevice->DefaultPB.Size)
        {
            return D3DERR_BUFFERTOOSMALL;
        }

        memcpy(g_pDevice->DefaultPB.p,
               pPushBuffer->resource.pContiguousMemory,
               pPushBuffer->SizeNeeded * sizeof(DWORD));
        g_pDevice->DefaultPB.p += pPushBuffer->SizeNeeded;
        D3DDevice_KickPushBuffer();
        return D3D_OK;
    }

    HRESULT hr = D3DDevice_PushJump(pPushBuffer->resource.pContiguousMemory);
    if (FAILED(hr)) return hr;

    // D3DDevice_EndPushBuffer inserted a NOP at the end of the push buffer.
    // That NOP is "fixed up" here as a jump back to the default push buffer.
    *(pPushBuffer->p) = D3D_NV2A_PFIFO_ENCODE_JUMP(g_pDevice->DefaultPB.p);
    return D3D_OK;
}

HRESULT D3DDevice_Push1(DWORD dwData) {
    if (g_pDevice->pCurrentPB == NULL)
        return D3DERR_NOTAVAILABLE;

    if (D3DDevice_IsKickoffReady() == TRUE)
        D3DDevice_KickPushBuffer();

    BOOL bLoop = FALSE;
    if (g_pDevice->pCurrentPB == &g_pDevice->DefaultPB)
        bLoop = TRUE;
    return D3DPushBuffer_Push1(g_pDevice->pCurrentPB, dwData, bLoop);
}

HRESULT D3DDevice_PushCmd(DWORD cmd, DWORD dwData) {
#if NXDK_DEBUG
    if (g_pDevice->pCurrentPB == NULL)
        return D3DERR_NOTAVAILABLE;
    if (cmd == 0)
        return D3DERR_INVALIDCALL;
#endif // NXDK_DEBUG
    if (D3DDevice_IsKickoffReady() == TRUE)
        D3DDevice_KickPushBuffer();

    BOOL bLoop = FALSE;
    if (g_pDevice->pCurrentPB == &g_pDevice->DefaultPB)
        bLoop = TRUE;
    return D3DPushBuffer_PushCmd(g_pDevice->pCurrentPB, cmd, dwData, bLoop);
}

HRESULT D3DDevice_PushCmdA(DWORD cmd, CONST DWORD* pdwData,
                           SIZE_T n, BOOL bIncrement)
{
#if NXDK_DEBUG
    if (g_pDevice->pCurrentPB == NULL)
        return D3DERR_NOTAVAILABLE;
    if (cmd == 0)
        return D3DERR_INVALIDCALL;
#endif // NXDK_DEBUG
    if (D3DDevice_IsKickoffReady() == TRUE)
        D3DDevice_KickPushBuffer();

    BOOL bLoop = FALSE;
    if (g_pDevice->pCurrentPB == &g_pDevice->DefaultPB)
        bLoop = TRUE;
    D3DPushBuffer_PushCmdA(g_pDevice->pCurrentPB, cmd,
                           pdwData, n, bIncrement, bLoop);
    return D3D_OK;
}

HRESULT D3DDevice_PushCmd2(DWORD cmd, DWORD dwData1,
                           DWORD dwData2, BOOL bIncrement)
{
#if NXDK_DEBUG
    if (g_pDevice->pCurrentPB == NULL)
        return D3DERR_NOTAVAILABLE;
#endif // NXDK_DEBUG
    if (D3DDevice_IsKickoffReady() == TRUE)
        D3DDevice_KickPushBuffer();

    BOOL bLoop = FALSE;
    if (g_pDevice->pCurrentPB == &g_pDevice->DefaultPB)
        bLoop = TRUE;
    return D3DPushBuffer_PushCmd2(g_pDevice->pCurrentPB, cmd,
                                  dwData1, dwData2, bIncrement, bLoop);
}

HRESULT D3DDevice_PushCmd3(DWORD cmd, DWORD dwData1,
                           DWORD dwData2, DWORD dwData3,
                           BOOL bIncrement)
{
#if NXDK_DEBUG
    if (g_pDevice->pCurrentPB == NULL)
        return D3DERR_NOTAVAILABLE;
#endif // NXDK_DEBUG
    if (D3DDevice_IsKickoffReady() == TRUE)
        D3DDevice_KickPushBuffer();

    BOOL bLoop = FALSE;
    if (g_pDevice->pCurrentPB == &g_pDevice->DefaultPB)
        bLoop = TRUE;
    return D3DPushBuffer_PushCmd3(g_pDevice->pCurrentPB, cmd,
                                  dwData1, dwData2, dwData3,
                                  bIncrement, bLoop);
}

HRESULT D3DDevice_PushCmd4(DWORD cmd,
                           DWORD dwData1, DWORD dwData2,
                           DWORD dwData3, DWORD dwData4,
                           BOOL bIncrement)
{
#if NXDK_DEBUG
    if (g_pDevice->pCurrentPB == NULL)
        return D3DERR_NOTAVAILABLE;
#endif // NXDK_DEBUG
    if (D3DDevice_IsKickoffReady() == TRUE)
        D3DDevice_KickPushBuffer();

    BOOL bLoop = FALSE;
    if (g_pDevice->pCurrentPB == &g_pDevice->DefaultPB)
        bLoop = TRUE;
    return D3DPushBuffer_PushCmd4(g_pDevice->pCurrentPB, cmd,
                                  dwData1, dwData2, dwData3, dwData4,
                                  bIncrement, bLoop);
}

HRESULT D3DDevice_PushCmdf(DWORD cmd, float fData) {
#if NXDK_DEBUG
    if (g_pDevice->pCurrentPB == NULL)
        return D3DERR_NOTAVAILABLE;
    if (cmd == 0)
        return D3DERR_INVALIDCALL;
#endif // NXDK_DEBUG
    if (D3DDevice_IsKickoffReady() == TRUE)
        D3DDevice_KickPushBuffer();

    BOOL bLoop = FALSE;
    if (g_pDevice->pCurrentPB == &g_pDevice->DefaultPB)
        bLoop = TRUE;
    return D3DPushBuffer_PushCmdf(g_pDevice->pCurrentPB, cmd, fData, bLoop);
}

HRESULT D3DDevice_PushCmd2f(DWORD cmd, float fData1,
                            float fData2, BOOL bIncrement)
{
#if NXDK_DEBUG
    if (g_pDevice->pCurrentPB == NULL)
        return D3DERR_NOTAVAILABLE;
    if (cmd == 0)
        return D3DERR_INVALIDCALL;
#endif // NXDK_DEBUG
    if (D3DDevice_IsKickoffReady() == TRUE)
        D3DDevice_KickPushBuffer();

    BOOL bLoop = FALSE;
    if (g_pDevice->pCurrentPB == &g_pDevice->DefaultPB)
        bLoop = TRUE;
    return D3DPushBuffer_PushCmd2f(g_pDevice->pCurrentPB, cmd,
                                   fData1, fData2, bIncrement, bLoop);
}

HRESULT D3DDevice_PushCmd3f(DWORD cmd, float fData1,
                            float fData2, float fData3,
                            BOOL bIncrement)
{
#if NXDK_DEBUG
    if (g_pDevice->pCurrentPB == NULL)
        return D3DERR_NOTAVAILABLE;
    if (cmd == 0)
        return D3DERR_INVALIDCALL;
#endif // NXDK_DEBUG
    if (D3DDevice_IsKickoffReady() == TRUE)
        D3DDevice_KickPushBuffer();

    BOOL bLoop = FALSE;
    if (g_pDevice->pCurrentPB == &g_pDevice->DefaultPB)
        bLoop = TRUE;
    return D3DPushBuffer_PushCmd3f(g_pDevice->pCurrentPB, cmd,
                                   fData1, fData2, fData3, bIncrement, bLoop);
}

HRESULT D3DDevice_PushCmd4f(DWORD cmd,
                            float fData1, float fData2,
                            float fData3, float fData4,
                            BOOL bIncrement)
{
#if NXDK_DEBUG
    if (g_pDevice->pCurrentPB == NULL)
        return D3DERR_NOTAVAILABLE;
    if (cmd == 0)
        return D3DERR_INVALIDCALL;
#endif // NXDK_DEBUG
    if (D3DDevice_IsKickoffReady() == TRUE)
        D3DDevice_KickPushBuffer();

    BOOL bLoop = FALSE;
    if (g_pDevice->pCurrentPB == &g_pDevice->DefaultPB)
        bLoop = TRUE;
    return D3DPushBuffer_PushCmd4f(g_pDevice->pCurrentPB, cmd,
                                   fData1, fData2, fData3, fData4,
                                   bIncrement, bLoop);
}

HRESULT D3DDevice_PushA(CONST DWORD* pdwData, SIZE_T n) {
    if (g_pDevice->pCurrentPB == NULL)
        return D3DERR_NOTAVAILABLE;

    if (D3DDevice_IsKickoffReady() == TRUE)
        D3DDevice_KickPushBuffer();

    BOOL bLoop = FALSE;
    if (g_pDevice->pCurrentPB == &g_pDevice->DefaultPB)
        bLoop = TRUE;
    return D3DPushBuffer_PushA(g_pDevice->pCurrentPB, pdwData, n, bLoop);
}

HRESULT D3DDevice_CreateImageSurface(UINT Width, UINT Height, D3DFORMAT Format,
                                     D3DSurface** ppSurface)
{
    D3DSurface* surf = malloc(sizeof(*surf));
    surf->iface.lpVtbl = g_pSurfaceVtbl;
    D3D_CreateSurface(
        Width, Height, Format, 0, 0, NULL, NULL, surf);
    surf->resource.pContiguousMemory =
        D3D_AllocContiguousMemory(surf->desc.Size, D3DSURFACE_ALIGNMENT);
    if (surf->resource.pContiguousMemory == NULL) {
        free(surf);
        return D3DERR_OUTOFVIDEOMEMORY;
    }
    *ppSurface = surf;
    return D3D_OK;
}

D3DAPI HRESULT Direct3DDevice8_CreateImageSurface(
    LPDIRECT3DDEVICE8 pThis, UINT Width, UINT Height,
    D3DFORMAT Format, LPDIRECT3DSURFACE8* ppSurface)
{
    return D3DDevice_CreateImageSurface(Width, Height, Format,
                                        (D3DSurface**)ppSurface);
}


HRESULT D3DDevice_CreateDepthStencilSurface(
    UINT Width, UINT Height, D3DFORMAT Format, D3DSurface** ppSurface)
{
    return D3DDevice_CreateImageSurface(Width, Height, Format, ppSurface);
}

D3DAPI HRESULT Direct3DDevice8_CreateDepthStencilSurface(
    LPDIRECT3DDEVICE8 pThis, UINT Width, UINT Height, D3DFORMAT Format,
    D3DMULTISAMPLE_TYPE MultiSample, LPDIRECT3DSURFACE8* ppSurface)
{
    return D3DDevice_CreateDepthStencilSurface(Width,
                                               Height,
                                               Format,
                                               (D3DSurface**)ppSurface);
}

HRESULT D3DDevice_SetScissorRect(DWORD Index, CONST D3DRECT* pRect) {
    // TODO: does WINDOW_CLIP use point/point or point/length? The other
    HRESULT hr = D3DDevice_PushCmd(
        NV097_SET_WINDOW_CLIP_HORIZONTAL(Index),
            MASK(NV097_SET_WINDOW_CLIP_HORIZONTAL_X1, pRect->x1) |
            MASK(NV097_SET_WINDOW_CLIP_HORIZONTAL_X2, pRect->x2));
    if (FAILED(hr)) return hr;
    return D3DDevice_PushCmd(
        NV097_SET_WINDOW_CLIP_VERTICAL(Index),
            MASK(NV097_SET_WINDOW_CLIP_VERTICAL_Y1, pRect->y1) |
            MASK(NV097_SET_WINDOW_CLIP_VERTICAL_Y2, pRect->y2));
}

HRESULT D3DDevice_SetScissors(DWORD Count, BOOL Exclusive,
                              CONST D3DRECT *pRects)
{
    D3D_DebugPrintf("Setting scissors.\n");
#if NXDK_DEBUG
    if (Count >= D3DSCISSORS_MAX)
        return D3DERR_INVALIDCALL;

    if (Exclusive > TRUE)
        return D3DERR_INVALIDCALL;
#endif // NXDK_DEBUG

    if (Count == 0) {
        HRESULT hr = D3DDevice_PushCmd(
            NV097_SET_WINDOW_CLIP_TYPE, NV097_SET_WINDOW_CLIP_TYPE_INCLUSIVE);
        D3DVIEWPORT8* pViewport = &g_pDevice->Viewport;
        D3DRECT rect;
        rect.x1 = pViewport->X;
        rect.y1 = pViewport->Y;
        rect.x2 = pViewport->X + pViewport->Width;
        rect.y2 = pViewport->Y + pViewport->Height;
        D3D_DebugPrintf("Setting default scissor rect.\n");
        return D3DDevice_SetScissorRect(0, &rect);
    }

    HRESULT hr = D3DDevice_PushCmd(NV097_SET_WINDOW_CLIP_TYPE, Exclusive);
    if (FAILED(hr)) return hr;

    for (int i = 0; i < Count; i++) {
        CONST D3DRECT* pRect = &pRects[i];
        hr = D3DDevice_SetScissorRect(i, pRect);
        if (FAILED(hr)) return hr;
    }

    // Setting the first clip region causes the hardware to set the others to
    // match. If that's the only one we set, no need to explicitly handle the
    // remaining ones.
    if (Count == 1)
        return D3D_OK;

    // Otherwise we set them to either exclude nothing in exclusive mode or
    // include everything in inclusive mode.
    for (int i = Count; i < D3DSCISSORS_MAX; i++) {
        D3DRECT rect;
        rect.x1 = 0;
        rect.y1 = 0;
        rect.x2 = 0;
        rect.y2 = 0;
        if (Exclusive == FALSE) {
            rect.x2 = 0xFFFFFFFF;
            rect.y2 = 0xFFFFFFFF;
        }

        hr = D3DDevice_SetScissorRect(i, &rect);
        if (FAILED(hr)) return hr;
    }

    return hr;
}

D3DAPI HRESULT Direct3DDevice8_SetScissors(LPDIRECT3DDEVICE8 pThis,
                                            DWORD Count, BOOL Exclusive,
                                            CONST D3DRECT *pRects)
{
    assert(pThis == &g_pDevice->iface);
    return D3DDevice_SetScissors(Count, Exclusive, pRects);
}

HRESULT D3DDevice_InsertInterrupt(DWORD Subprog, DWORD ParamA) {
    HRESULT hr = D3DDevice_PushCmd(NV097_WAIT_FOR_IDLE, 0);
    if (FAILED(hr)) return hr;
    hr = D3DDevice_PushCmd(NV097_SET_INTERRUPT_PARAM_A, ParamA);
    if (FAILED(hr)) return hr;
    g_pDevice->ZStencilClearDirty = TRUE;
    return D3DDevice_PushCmd(NV097_FIRE_INTERRUPT, Subprog);
}

HRESULT D3DDevice_InsertInterrupt2(DWORD Subprog, DWORD ParamA, DWORD ParamB) {
    HRESULT hr = D3DDevice_PushCmd(NV097_WAIT_FOR_IDLE, 0);
    if (FAILED(hr)) return hr;
    hr = D3DDevice_PushCmd(NV097_SET_INTERRUPT_PARAM_A, ParamA);
    if (FAILED(hr)) return hr;
    g_pDevice->ZStencilClearDirty = TRUE;
    hr = D3DDevice_PushCmd(NV097_SET_INTERRUPT_PARAM_B, ParamB);
    if (FAILED(hr)) return hr;
    g_pDevice->ColorClearDirty = TRUE;
    return D3DDevice_PushCmd(NV097_FIRE_INTERRUPT, Subprog);
}

HRESULT D3DDevice_InitRenderState(void) {
    HRESULT hr = D3DDevice_SetRenderState(D3DRS_DEPTHCLIPCONTROL,
        D3DDCC_CULLPRIMITIVE | D3DDCC_CLAMP | D3DDCC_IGNORE_W_SIGN);
    if (FAILED(hr)) return hr;

    hr = D3DDevice_SetRenderState(D3DRS_TWOSIDEDLIGHTING, FALSE);
    if (FAILED(hr)) return hr;

    hr = D3DDevice_SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
    if (FAILED(hr)) return hr;
    hr = D3DDevice_SetRenderState(D3DRS_BACKFILLMODE, D3DFILL_SOLID);
    if (FAILED(hr)) return hr;

    hr = D3DDevice_SetRenderState(D3DRS_PSDOTMAPPING, 0);
    if (FAILED(hr)) return hr;

    hr = D3DDevice_SetRenderState(D3DRS_SHADEMODE, D3DSHADE_GOURAUD);
    if (FAILED(hr)) return hr;

    D3DRenderState* pRenderState = &g_pDevice->RenderState;
    DWORD RopZ = 0x45EAD10E;
    if (VIDEOREG(NV_PBUS_ROM_VERSION) & NV_PBUS_ROM_VERSION_MASK)
        RopZ = 0x45EAD10F;
    pRenderState->RopZ = RopZ;
    hr = D3DDevice_InsertInterrupt2(PB_SETOUTER,
                                    NV_PGRAPH_UNKNOWN_400B80,
                                    g_pDevice->RenderState.RopZ);
    if (FAILED(hr)) return hr;

    return D3D_OK;
}

HRESULT D3DDevice_InitTextureStageState(DWORD Stage) {
#if NXDK_DEBUG
    if (Stage >= D3DTSS_MAXSTAGES) {
        assert(false);
        return D3DERR_INVALIDCALL;
    }
#endif // NXDK_DEBUG

    return D3D_OK;
}

void dump_push_buffer(LPDIRECT3DDEVICE8 pDevice);
HRESULT D3DDevice_InitDeviceState(void) {
    HRESULT hr = D3DDevice_PushCmd4f(NV097_SET_EYE_POSITION,
                                     0.0f, 0.0f, 0.0f, 1.0f,
                                     FALSE);
    if (FAILED(hr)) return hr;

    D3DSURFACE_DESC desc;
    hr = D3DSurface_GetDesc(
        (LPDIRECT3DSURFACE8)g_pDevice->pSurfaces[0], &desc);
    if (FAILED(hr)) return hr;
    UINT ColorPitch = desc.Width * D3D_FormatBytesPerPixel(desc.Format);
    UINT Width = desc.Width;
    UINT Height = desc.Height;
    hr = D3DSurface_GetDesc(
        (LPDIRECT3DSURFACE8)g_pDevice->pDepthStencilSurface, &desc);
    if (FAILED(hr)) return hr;
    UINT DepthPitch = desc.Width * D3D_FormatBytesPerPixel(desc.Format);
    hr = D3DDevice_PushCmd(NV097_SET_SURFACE_PITCH,
        MASK(NV097_SET_SURFACE_PITCH_COLOR, ColorPitch) |
        MASK(NV097_SET_SURFACE_PITCH_ZETA,  DepthPitch));
    if (FAILED(hr)) return hr;

    hr = D3DDevice_PushCmd(NV097_SET_SURFACE_CLIP_HORIZONTAL,
        MASK(NV097_SET_SURFACE_CLIP_HORIZONTAL_X, 0) |
        MASK(NV097_SET_SURFACE_CLIP_HORIZONTAL_WIDTH, Width));
    if (FAILED(hr)) return hr;

    hr = D3DDevice_PushCmd(NV097_SET_SURFACE_CLIP_VERTICAL,
        MASK(NV097_SET_SURFACE_CLIP_VERTICAL_Y, 0) |
        MASK(NV097_SET_SURFACE_CLIP_VERTICAL_HEIGHT, Height));
    if (FAILED(hr)) return hr;

    hr = D3DDevice_PushCmd(NV097_SET_LIGHTING_ENABLE, 0);
    if (FAILED(hr)) return hr;

    hr = D3DDevice_PushCmd(NV097_SET_SPECULAR_ENABLE, 0);
    if (FAILED(hr)) return hr;

    hr = D3DDevice_PushCmd(NV097_SET_LIGHT_CONTROL,
        NV097_SET_LIGHT_CONTROL_V_ALPHA_FROM_MATERIAL_SPECULAR |
        NV097_SET_LIGHT_CONTROL_V_SEPARATE_SPECULAR);
    if (FAILED(hr)) return hr;

    hr = D3DDevice_PushCmd(NV097_SET_LIGHT_ENABLE_MASK,
                           NV097_SET_LIGHT_ENABLE_MASK_LIGHT0_OFF);
    if (FAILED(hr)) return hr;

    hr = D3DDevice_PushCmd(NV097_SET_COLOR_MATERIAL,
                           NV097_SET_COLOR_MATERIAL_ALL_FROM_MATERIAL);
    if (FAILED(hr)) return hr;

    hr = D3DDevice_PushCmd3(NV097_SET_SCENE_AMBIENT_COLOR, 0, 0, 0, TRUE);
    if (FAILED(hr)) return hr;

    hr = D3DDevice_PushCmd3f(NV097_SET_MATERIAL_EMISSION,
                             0.0f, 0.0f, 0.0f, TRUE);
    if (FAILED(hr)) return hr;

    hr = D3DDevice_PushCmdf(NV097_SET_MATERIAL_ALPHA, 1.0f);
    if (FAILED(hr)) return hr;

    hr = D3DDevice_PushCmdf(NV097_SET_BACK_MATERIAL_ALPHA, 1.0f);
    if (FAILED(hr)) return hr;

    hr = D3DDevice_PushCmd(NV097_SET_POINT_PARAMS_ENABLE, 0);
    if (FAILED(hr)) return hr;

    hr = D3DDevice_PushCmd(NV097_SET_POINT_SMOOTH_ENABLE, 0);
    if (FAILED(hr)) return hr;

    hr = D3DDevice_PushCmd(NV097_SET_POINT_SIZE, 8);
    if (FAILED(hr)) return hr;

    hr = D3DDevice_PushCmd(NV097_SET_LINE_WIDTH, 8);
    if (FAILED(hr)) return hr;

    hr = D3DDevice_PushCmd(NV097_SET_FLAT_SHADE_OP,
                           NV097_SET_FLAT_SHADE_OP_VERTEX_LAST);
    if (FAILED(hr)) return hr;

    hr = D3DDevice_PushCmd4f(NV097_SET_FOG_PLANE,
                             0.0f, 0.0f, 1.0f, 0.0f, TRUE);
    if (FAILED(hr)) return hr;

    hr = D3DDevice_PushCmd(NV097_SET_FOG_GEN_MODE,
                           NV097_SET_FOG_GEN_MODE_V_PLANAR);
    if (FAILED(hr)) return hr;

    hr = D3DDevice_PushCmd4(NV097_SET_TEXTURE_MATRIX_ENABLE, 0, 0, 0, 0, TRUE);
    if (FAILED(hr)) return hr;

    hr = D3DDevice_PushCmd4f(NV097_SET_TEXCOORD0_4F, 0.f, 0.f, 0.f, 0.f, TRUE);
    if (FAILED(hr)) return hr;
    hr = D3DDevice_PushCmd4f(NV097_SET_TEXCOORD1_4F, 0.f, 0.f, 0.f, 0.f, TRUE);
    if (FAILED(hr)) return hr;
    hr = D3DDevice_PushCmd4f(NV097_SET_TEXCOORD2_4F, 0.f, 0.f, 0.f, 0.f, TRUE);
    if (FAILED(hr)) return hr;
    hr = D3DDevice_PushCmd4f(NV097_SET_TEXCOORD3_4F, 0.f, 0.f, 0.f, 0.f, TRUE);
    if (FAILED(hr)) return hr;
    hr = D3DDevice_PushCmd(
        NV097_SET_VERTEX_DATA4UB + (4 * NV2A_VERTEX_ATTR_BACK_DIFFUSE),
        0xFFFFFFFF);
    if (FAILED(hr)) return hr;
    hr = D3DDevice_PushCmd(
        NV097_SET_VERTEX_DATA4UB + (4 * NV2A_VERTEX_ATTR_BACK_SPECULAR), 0);
    if (FAILED(hr)) return hr;

    const float specular_params[] = {-0.803673, -2.7813,  2.97762,
                                     -0.64766,  -2.36199, 2.71433};
    for (uint32_t i = 0, offset = 0; i < 6; ++i, offset += 4) {
        hr = D3DDevice_PushCmdf(
            NV097_SET_SPECULAR_PARAMS + offset, specular_params[i]);
        if (FAILED(hr)) return hr;
        hr = D3DDevice_PushCmdf(NV097_SET_SPECULAR_PARAMS_BACK + offset, 0);
        if (FAILED(hr)) return hr;
    }

    D3D_DebugPrintf("Initing render state.\n");
    hr = D3DDevice_InitRenderState();
    if (FAILED(hr)) return hr;


    for (DWORD Stage = 0; Stage < D3DTSS_MAXSTAGES; Stage++) {
        D3D_DebugPrintf("Initing texture stage state %d.\n", Stage);
        hr = D3DDevice_InitTextureStageState(Stage);
        if (FAILED(hr)) return hr;
    }

    return D3DDevice_SetScissors(0, FALSE, NULL);
}

ULONG D3DDevice_AddRef(void) {
    g_pDevice->refcount.c = 1;
    return 1;
}

D3DAPI ULONG Direct3DDevice8_AddRef(LPDIRECT3DDEVICE8 pThis) {
    assert(pThis == &g_pDevice->iface);
    return D3DDevice_AddRef();
}

ULONG D3DDevice_Release(void) {
    return g_pDevice->refcount.c;
}

D3DAPI ULONG Direct3DDevice8_Release(LPDIRECT3DDEVICE8 pThis) {
    assert(pThis == &g_pDevice->iface);
    return D3DDevice_Release();
}

HRESULT D3DDevice_Reset(D3DPRESENT_PARAMETERS* pPresentationParameters) {
#if NXDK_DEBUG
    HRESULT hr = D3D_PresentParameters_Validate(pPresentationParameters);
    assert(SUCCEEDED(hr));
    if (FAILED(hr)) return hr;
#endif // NXDK_DEBUG
    return Direct3D_ResetDevice(pPresentationParameters);
}

D3DAPI HRESULT Direct3DDevice8_Reset(
    LPDIRECT3DDEVICE8 pThis,
    D3DPRESENT_PARAMETERS* pPresentationParameters)
{
    assert(pThis == &g_pDevice->iface);
    return D3DDevice_Reset(pPresentationParameters);
}

HRESULT D3DDevice_CreateVertexBuffer(
    UINT Length, D3DVertexBuffer** ppVertexBuffer)
{
    D3DVertexBuffer* vb = malloc(sizeof(*vb));
    if (vb == NULL)
        return E_OUTOFMEMORY;

    PVOID pContiguousMemory =
        D3D_AllocContiguousMemory(Length, D3DVERTEXBUFFER_ALIGNMENT);
    if (pContiguousMemory == NULL) {
        free(vb);
        return E_OUTOFMEMORY;
    }

    vb->iface.lpVtbl                = g_pVertexBufferVtbl;
    D3D_CreateResource(D3DRTYPE_VERTEXBUFFER, 0,
                       pContiguousMemory, &vb->resource);
    vb->desc.Format                 = D3DFMT_VERTEXDATA;
    vb->desc.Type                   = D3DRTYPE_VERTEXBUFFER;
    *ppVertexBuffer                 = vb;
    return D3D_OK;
}

D3DAPI HRESULT Direct3DDevice8_CreateVertexBuffer(
    LPDIRECT3DDEVICE8 pThis, UINT Length, DWORD Usage, DWORD FVF,
    DWORD Pool, LPDIRECT3DVERTEXBUFFER8* ppVertexBuffer)
{
    assert(pThis == &g_pDevice->iface);
    return D3DDevice_CreateVertexBuffer(
        Length, (D3DVertexBuffer**)ppVertexBuffer);
}

void D3DDevice_BlockUntilVerticalBlank(void) {
    pb_wait_for_vbl();
}

D3DAPI VOID Direct3DDevice8_BlockUntilVerticalBlank(LPDIRECT3DDEVICE8 pThis) {
    assert(pThis == &g_pDevice->iface);
    return D3DDevice_BlockUntilVerticalBlank();
}

// This is a hack to prevent pb_target_back_buffer from writing past the end of
// the push buffer. By default, pbkit functions will simply write their
// commands to the push buffer and hope there's space.
//
// We handle potential out-of-bounds writes in the D3DPushBuffer_Push*
// functions, but pbkit functions bypass those protections, so here we figure
// out how much space in the push buffer the function will take on its first
// run, and then jump back to the beginning of the push buffer on subsequent
// runs if there isn't enough space.
//
// We could just calculate the size as a constant based on the source for these
// functions, but that approach breaks if more commands are added later. The
// biggest problem with this approach is that the value is only calculated
// once, so in theory if the function pushes differing amounts of commands
// per run, we run the risk of the value here being too low, but there's
// really no good way to insulate against that. Most likely, this will have
// to suffice until we gut these pbkit functions out and implement their
// functionality directly in D3D.
static int g_pb_target_back_buffer_size = 0;
HRESULT D3DDevice_BeginScene(void) {
    if (g_pDevice->bInScene == TRUE)
        return D3DERR_INVALIDCALL;

    g_pDevice->bInScene = TRUE;

    PVOID p = g_pDevice->DefaultPB.p;
    if (g_pb_target_back_buffer_size > 0 &&
        D3DPushBuffer_BytesRemaining(&g_pDevice->DefaultPB) - 1
            <= g_pb_target_back_buffer_size)
    {
        HRESULT hr = D3DPushBuffer_PushJump(
            &g_pDevice->DefaultPB,
            g_pDevice->DefaultPB.resource.pContiguousMemory,
            FALSE);
        // D3DERR_BUFFERTOOSMALL means we're at the end of the push buffer,
        // but Direct3D::CreateDevice already inserted a jump back to the head
        // of the push buffer at the end, so we don't have to insert one
        // here.
        if (FAILED(hr) && hr != D3DERR_BUFFERTOOSMALL) return hr;
        D3DDevice_ResetPushBuffer();

    }
    pb_target_back_buffer();
    D3DDevice_SyncPushBuffer();
    if (g_pb_target_back_buffer_size == 0) {
        g_pb_target_back_buffer_size =
            (DWORD)g_pDevice->DefaultPB.p - (DWORD)p;
        // pbkit might have reset back to the beginning of the push buffer. If
        // so, this value is definitely high, but better safe than sorry.
        if (g_pb_target_back_buffer_size < 0)
            g_pb_target_back_buffer_size = -g_pb_target_back_buffer_size;
    }
    return D3D_OK;
}

D3DAPI HRESULT Direct3DDevice8_BeginScene(LPDIRECT3DDEVICE8 pThis) {
    assert(pThis == &g_pDevice->iface);
    return D3DDevice_BeginScene();
}

HRESULT D3DDevice_EndScene(void) {
    if (g_pDevice->bInScene == FALSE)
        return D3DERR_INVALIDCALL;

    g_pDevice->bInScene = FALSE;
    return D3D_OK;
}

D3DAPI HRESULT Direct3DDevice8_EndScene(LPDIRECT3DDEVICE8 pThis) {
    assert(pThis == &g_pDevice->iface);
    return D3DDevice_EndScene();
}

// See g_pb_target_back_buffer_size/D3DDevice_BeginScene for an explanation.
static int g_pb_finished_size = 0;
HRESULT D3DDevice_Present(CONST RECT* pSourceRect, CONST RECT* pDestRect) {
    // TODO: implement
    if (pSourceRect || pDestRect)
        return E_NOTIMPL;

    D3DDevice_KickPushBuffer();
    PVOID p = g_pDevice->DefaultPB.p;
    if (g_pb_finished_size > 0 &&
        D3DPushBuffer_BytesRemaining(&g_pDevice->DefaultPB) - 1
            <= g_pb_finished_size)
    {
        HRESULT hr = D3DPushBuffer_PushJump(
            &g_pDevice->DefaultPB,
            g_pDevice->DefaultPB.resource.pContiguousMemory,
            FALSE);
        if (FAILED(hr) && hr != D3DERR_BUFFERTOOSMALL) return hr;
        D3DDevice_ResetPushBuffer();

    }
    while (pb_finished()) {
        /* Not ready to swap yet */
    }
    D3DDevice_SyncPushBuffer();
    if (g_pb_finished_size == 0) {
        g_pb_finished_size =
            (DWORD)g_pDevice->DefaultPB.p - (DWORD)p;
        if (g_pb_finished_size < 0)
            g_pb_finished_size = -g_pb_finished_size;
    }

    g_pDevice->CurrentSurface =
        (g_pDevice->CurrentSurface + 1) % g_pDevice->MaxSurfaces;
    UINT lastVBL = g_pDevice->LastPresentVBlankCount;
    g_pDevice->LastPresentVBlankCount = pb_get_vbl_counter();
    UINT diff = g_pDevice->LastPresentVBlankCount - lastVBL;

    // If the interval is immediate, or the interval is *_OR_IMMEDIATE and
    // we've surpassed the interval, there's no need to wait for a vblank.
    if (g_pDevice->PresentInterval  == D3DPRESENT_INTERVAL_IMMEDIATE ||
        (g_pDevice->PresentInterval ==
            D3DPRESENT_INTERVAL_ONE_OR_IMMEDIATE && diff >= 1) ||
        (g_pDevice->PresentInterval ==
            D3DPRESENT_INTERVAL_TWO_OR_IMMEDIATE && diff >= 2) ||
        (g_pDevice->PresentInterval ==
            D3DPRESENT_INTERVAL_THREE_OR_IMMEDIATE && diff >= 3))
    {
        return D3D_OK;
    }

    // Otherwise we need to wait for the interval.
    if (g_pDevice->PresentInterval == D3DPRESENT_INTERVAL_ONE ||
        g_pDevice->PresentInterval == D3DPRESENT_INTERVAL_ONE_OR_IMMEDIATE)
    {
        D3DDevice_BlockUntilVerticalBlank();
        return D3D_OK;
    }

    UINT Interval = 0;
    if (g_pDevice->PresentInterval == D3DPRESENT_INTERVAL_TWO ||
        g_pDevice->PresentInterval == D3DPRESENT_INTERVAL_TWO_OR_IMMEDIATE)
    {
        Interval = 2;
    } else if (g_pDevice->PresentInterval == D3DPRESENT_INTERVAL_THREE ||
        g_pDevice->PresentInterval == D3DPRESENT_INTERVAL_THREE_OR_IMMEDIATE)
    {
       Interval = 3;
    }

    while(pb_get_vbl_counter() < g_pDevice->LastPresentVBlankCount + Interval)
        D3DDevice_BlockUntilVerticalBlank();

    return D3D_OK;
}

D3DAPI HRESULT Direct3DDevice8_Present(LPDIRECT3DDEVICE8 pThis,
                                 CONST RECT* pSourceRect,
                                 CONST RECT* pDestRect)
{
    assert(pThis == &g_pDevice->iface);
    return D3DDevice_Present(pSourceRect, pDestRect);
}

HRESULT D3DDevice_DrawVertices(D3DPRIMITIVETYPE PrimitiveType,
                               UINT StartVertex, UINT VertexCount)
{
    HRESULT hr = D3D_OK;
    for (UINT Stage = 0; Stage < D3DTSS_MAXSTAGES; Stage++) {
        if (g_pDevice->pTexture[Stage] == NULL ||
            g_pDevice->TextureStageState[Stage].Dirty == 0)
        {
            continue;
        }

        DWORD Dirty = g_pDevice->TextureStageState[Stage].Dirty;
        g_pDevice->TextureStageState[Stage].Dirty = 0;

        if (Dirty & D3DTSS_ADDRESS_DIRTY) {
            hr = D3DDevice_PushCmd(NV097_SET_TEXTURE_ADDRESS(Stage),
                                   g_pDevice->TextureStageState[Stage].Address);
            if (FAILED(hr)) return hr;
        }
        if (Dirty & D3DTSS_FILTER_DIRTY) {
            hr = D3DDevice_PushCmd(NV097_SET_TEXTURE_FILTER(Stage),
                                   g_pDevice->TextureStageState[Stage].Filter);
            if (FAILED(hr)) return hr;
        }
        if (Dirty & D3DTSS_CONTROL0_DIRTY) {
            hr = D3DDevice_PushCmd(NV097_SET_TEXTURE_CONTROL0(Stage),
                g_pDevice->TextureStageState[Stage].Control0);
            if (FAILED(hr)) return hr;
        }
    }

    hr = D3DDevice_PushCmd(NV097_SET_BEGIN_END, PrimitiveType);
    if (FAILED(hr)) return hr;
    for (int i = 0; i < VertexCount; i += 256) {
        DWORD Count = VertexCount - i;
        if (Count >= 256) Count = 256;
        hr = D3DDevice_PushCmd(
            NV2A_SUPPRESS_COMMAND_INCREMENT(NV097_DRAW_ARRAYS),
            MASK(NV097_DRAW_ARRAYS_COUNT, Count - 1) |
            MASK(NV097_DRAW_ARRAYS_START_INDEX, StartVertex));
        if (FAILED(hr)) return hr;
    }

    return D3DDevice_PushCmd(NV097_SET_BEGIN_END, NV097_SET_BEGIN_END_OP_END);
}

D3DAPI HRESULT Direct3DDevice8_DrawVertices(LPDIRECT3DDEVICE8 pThis,
                                             D3DPRIMITIVETYPE PrimitiveType,
                                             UINT StartVertex,
                                             UINT VertexCount)
{
    assert(pThis == &g_pDevice->iface);
    return D3DDevice_DrawVertices(PrimitiveType, StartVertex, VertexCount);
}

HRESULT D3DDevice_DrawPrimitive(D3DPRIMITIVETYPE PrimitiveType,
                                UINT StartVertex, UINT PrimitiveCount)
{
    DWORD VertexCount = 0;
    switch (PrimitiveType) {
    case D3DPT_QUADSTRIP:
        VertexCount = (PrimitiveCount - 1) * 2 + 4;
        break;
    case D3DPT_QUADLIST:
        VertexCount = PrimitiveCount * 4;
        break;
    case D3DPT_TRIANGLELIST:
        VertexCount = PrimitiveCount * 3;
        break;
    case D3DPT_TRIANGLESTRIP:
    case D3DPT_TRIANGLEFAN:
        VertexCount = PrimitiveCount + 2;
        break;
    case D3DPT_LINELIST:
        VertexCount = PrimitiveCount * 2;
        break;
    case D3DPT_LINESTRIP:
        VertexCount = PrimitiveCount + 1;
        break;
    case D3DPT_LINELOOP:
    case D3DPT_POINTLIST:
        VertexCount = PrimitiveCount;
        break;
    default:
        assert(false);
        return D3DERR_INVALIDCALL;
    }

    return D3DDevice_DrawVertices(PrimitiveType, StartVertex, VertexCount);
}

D3DAPI HRESULT Direct3DDevice8_DrawPrimitive(LPDIRECT3DDEVICE8 pThis,
                                              D3DPRIMITIVETYPE PrimitiveType,
                                              UINT StartVertex,
                                              UINT PrimitiveCount)
{
    assert(pThis == &g_pDevice->iface);
    return D3DDevice_DrawPrimitive(PrimitiveType, StartVertex, PrimitiveCount);
}

// See g_pb_target_back_buffer_size/D3DDevice_BeginScene for an explanation.
static int g_pb_set_viewport_size = 0;
HRESULT D3DDevice_SetViewport(CONST D3DVIEWPORT8* pViewport) {
    PVOID p = g_pDevice->DefaultPB.p;
    if (g_pb_set_viewport_size > 0 &&
        D3DPushBuffer_BytesRemaining(&g_pDevice->DefaultPB) - 1
            <= g_pb_set_viewport_size)
    {
        HRESULT hr = D3DPushBuffer_PushJump(
            &g_pDevice->DefaultPB,
            g_pDevice->DefaultPB.resource.pContiguousMemory,
            FALSE);
        if (FAILED(hr) && hr != D3DERR_BUFFERTOOSMALL) return hr;
        D3DDevice_ResetPushBuffer();

    }
    pb_set_viewport(pViewport->X, pViewport->Y, pViewport->Width,
                    pViewport->Height, pViewport->MinZ, pViewport->MaxZ);
    D3DDevice_SyncPushBuffer();
    if (g_pb_set_viewport_size == 0) {
        g_pb_set_viewport_size = (DWORD)g_pDevice->DefaultPB.p - (DWORD)p;
        if (g_pb_set_viewport_size < 0)
            g_pb_set_viewport_size = -g_pb_set_viewport_size;
    }
    g_pDevice->Viewport = *pViewport;
    return D3D_OK;
}

D3DAPI HRESULT Direct3DDevice8_SetViewport(LPDIRECT3DDEVICE8 pThis,
                                            CONST D3DVIEWPORT8* pViewport)
{
    assert(pThis == &g_pDevice->iface);
    return D3DDevice_SetViewport(pViewport);
}

HRESULT D3DDevice_SetVertexShaderInputDirect(
    D3DVERTEXATTRIBUTEFORMAT *pVAF,
    UINT StreamCount,
    D3DSTREAM_INPUT *pStreamInputs)
{
    for (int i = 0, j = 0; i < StreamCount; i++) {
        D3DVERTEXSHADERINPUT* pInput = &pVAF->Input[i];
        if (pInput->Format == 0)
            continue;

        D3DDevice_PushCmd(NV097_SET_VERTEX_DATA_ARRAY_FORMAT(i),
            MASK(NV097_SET_VERTEX_DATA_ARRAY_FORMAT_TYPE |
                NV097_SET_VERTEX_DATA_ARRAY_FORMAT_SIZE,
                pInput->Format) |
            MASK(NV097_SET_VERTEX_DATA_ARRAY_FORMAT_STRIDE,
                pStreamInputs[i].Stride));
        D3DVertexBuffer* vb = (D3DVertexBuffer*)pStreamInputs[i].VertexBuffer;
        BYTE* pData = vb->resource.pContiguousMemory;
        pData += pStreamInputs[i].Offset;
        pData += pInput->Offset;
        D3DDevice_PushCmd(NV097_SET_VERTEX_DATA_ARRAY_OFFSET(i),
                          (uint32_t)pData & 0x03ffffff);
    }

    return D3D_OK;
}

D3DAPI HRESULT Direct3DDevice8_SetVertexShaderInputDirect(
    LPDIRECT3DDEVICE8 pThis,
    D3DVERTEXATTRIBUTEFORMAT *pVAF,
    UINT StreamCount,
    D3DSTREAM_INPUT *pStreamInputs)
{
    assert(pThis == &g_pDevice->iface);
    return D3DDevice_SetVertexShaderInputDirect(pVAF, StreamCount,
                                                pStreamInputs);
}

HRESULT D3DDevice_CreateTexture(
    UINT Width, UINT Height, UINT Levels, DWORD Usage,
    D3DFORMAT Format, D3DTexture** ppTexture)
{
#if NXDK_DEBUG
    if (Usage != 0 && Usage != D3DUSAGE_BORDERSOURCE_TEXTURE)
        return D3DERR_INVALIDCALL;
#endif // NXDK_DEBUG

    D3DTexture* pTex = malloc(sizeof(*pTex));
    if (pTex == NULL)
        return E_OUTOFMEMORY;

    HRESULT hr = D3D_CreateTexture(Width, Height, Levels,
                                   Usage, Format, NULL, pTex);
    if (FAILED(hr)) return hr;
    *ppTexture = pTex;
    return D3D_OK;
}

D3DAPI HRESULT Direct3DDevice8_CreateTexture(
    LPDIRECT3DDEVICE8 pThis, UINT Width, UINT Height,
    UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool,
    LPDIRECT3DTEXTURE8* ppTexture)
{
    assert(pThis == &g_pDevice->iface);
    return D3DDevice_CreateTexture(Width, Height, Levels, Usage,
                                   Format, (D3DTexture**)ppTexture);
}

HRESULT D3DDevice_CreateCubeTexture(
    UINT EdgeLength, UINT Levels, D3DFORMAT Format,
    D3DCubeTexture** ppCubeTexture)
{
    D3DCubeTexture* pTex = malloc(sizeof(*pTex));
    if (pTex == NULL)
        return E_OUTOFMEMORY;

    HRESULT hr = D3D_CreateCubeTexture(EdgeLength, Levels, Format, NULL, pTex);
    if (FAILED(hr)) return hr;
    *ppCubeTexture = pTex;
    return D3D_OK;
}

D3DAPI HRESULT Direct3DDevice8_CreateCubeTexture(
    LPDIRECT3DDEVICE8 pThis, UINT EdgeLength, UINT Levels,
    DWORD Usage, D3DFORMAT Format, D3DPOOL Pool,
    LPDIRECT3DCUBETEXTURE8* ppCubeTexture)
{
    assert(pThis == &g_pDevice->iface);
    return D3DDevice_CreateCubeTexture(EdgeLength, Levels, Format,
                                       (D3DCubeTexture**)ppCubeTexture);
}

HRESULT D3DDevice_SetTextureStageState_Immediate(
    DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD Value)
{
#if NXDK_DEBUG
    if (Type < D3DTSS_DEFERRED_MAX) {
        assert(false);
        return D3DERR_INVALIDCALL;
    }
#endif // NXDK_DEBUG
    switch (Type) {
    case D3DTSS_BUMPENVMAT00:
    case D3DTSS_BUMPENVMAT01:
    case D3DTSS_BUMPENVMAT10:
    case D3DTSS_BUMPENVMAT11:
        DWORD n = Type - D3DTSS_BUMPENVMAT00;
        return D3DDevice_PushCmd(
            NV097_SET_TEXTURE_SET_BUMP_ENV_MAT(Stage) + n * sizeof(DWORD),
            Value);
    case D3DTSS_BUMPENVLSCALE:
        return D3DDevice_PushCmd(
            NV097_SET_TEXTURE_SET_BUMP_ENV_SCALE(Stage), Value);
    case D3DTSS_BUMPENVLOFFSET:
        return D3DDevice_PushCmd(
            NV097_SET_TEXTURE_SET_BUMP_ENV_OFFSET(Stage), Value);
    case D3DTSS_BORDERCOLOR:
        return D3DDevice_PushCmd(NV097_SET_TEXTURE_BORDER_COLOR(Stage), Value);
    case D3DTSS_COLORKEYCOLOR:
        return D3DDevice_PushCmd(
            NV20_TCL_PRIMITIVE_3D_RC_COLOR_KEY_COLOR(Stage), Value);
    default:
        // TODO: implement remaining texture stage states
        return E_NOTIMPL;
    }
}

#define MASK_INPLACE(a, mask, value) \
    (a) = ((a & (~(mask))) | MASK(mask, value))

HRESULT D3DDevice_SetTextureStageState_Deferred(
    DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD Value)
{
#if NXDK_DEBUG
    if (Type >= D3DTSS_DEFERRED_MAX) {
        assert(false);
        return D3DERR_INVALIDCALL;
    }
#endif // NXDK_DEBUG
    switch (Type) {
    case D3DTSS_ADDRESSU:
        MASK_INPLACE(g_pDevice->TextureStageState[Stage].Address,
                     NV097_SET_TEXTURE_ADDRESS_U, Value);
        g_pDevice->TextureStageState[Stage].Dirty |= D3DTSS_ADDRESS_DIRTY;
        return D3D_OK;
    case D3DTSS_ADDRESSV:
        MASK_INPLACE(g_pDevice->TextureStageState[Stage].Address,
                     NV097_SET_TEXTURE_ADDRESS_V, Value);
        g_pDevice->TextureStageState[Stage].Dirty |= D3DTSS_ADDRESS_DIRTY;
        return D3D_OK;
    case D3DTSS_ADDRESSW:
        MASK_INPLACE(g_pDevice->TextureStageState[Stage].Address,
                     NV097_SET_TEXTURE_ADDRESS_P, Value);
        g_pDevice->TextureStageState[Stage].Dirty |= D3DTSS_ADDRESS_DIRTY;
        return D3D_OK;
    case D3DTSS_MAGFILTER:
    case D3DTSS_MINFILTER: {
        // TODO: figure out how to handle the min and mag filters' convolution
        // kernel differing.
        if (Value < D3DTEXF_QUINCUNX) {
            MASK_INPLACE(g_pDevice->TextureStageState[Stage].Filter,
                         NV097_SET_TEXTURE_FILTER_MIN, Value);
        } else {
            if (Value == D3DTEXF_QUINCUNX) {
                MASK_INPLACE(g_pDevice->TextureStageState[Stage].Filter,
                    NV097_SET_TEXTURE_FILTER_CONVOLUTION_KERNEL,
                    NV097_SET_TEXTURE_FILTER_CONVOLUTION_KERNEL_QUINCUNX);
            } else {
                 MASK_INPLACE(g_pDevice->TextureStageState[Stage].Filter,
                    NV097_SET_TEXTURE_FILTER_CONVOLUTION_KERNEL,
                    NV097_SET_TEXTURE_FILTER_CONVOLUTION_KERNEL_GAUSSIAN_CUBIC
                );
            }
            DWORD Filter = NV097_SET_TEXTURE_FILTER_MIN;
            if (Type == D3DTSS_MAGFILTER)
                Filter = NV097_SET_TEXTURE_FILTER_MAG;
            MASK_INPLACE(g_pDevice->TextureStageState[Stage].Filter,
                         Filter,
                         NV097_SET_TEXTURE_FILTER_MIN_CONVOLUTION_2D_LOD0);
        }
        g_pDevice->TextureStageState[Stage].Dirty |= D3DTSS_FILTER_DIRTY;
        return D3D_OK;
    }
    case D3DTSS_MIPMAPLODBIAS:
        MASK_INPLACE(g_pDevice->TextureStageState[Stage].Filter,
                     NV097_SET_TEXTURE_FILTER_MIPMAP_LOD_BIAS, Value);
        g_pDevice->TextureStageState[Stage].Dirty |= D3DTSS_FILTER_DIRTY;
        return D3D_OK;
    case D3DTSS_MAXANISOTROPY:
        MASK_INPLACE(g_pDevice->TextureStageState[Stage].Control0,
                     NV097_SET_TEXTURE_CONTROL0_ANISOTROPY, Value);
        g_pDevice->TextureStageState[Stage].Dirty |= D3DTSS_CONTROL0_DIRTY;
        return D3D_OK;
    case D3DTSS_COLORKEYOP:
        MASK_INPLACE(g_pDevice->TextureStageState[Stage].Control0,
                     NV097_SET_TEXTURE_CONTROL0_COLOR_KEY_MODE, Value);
        g_pDevice->TextureStageState[Stage].Dirty |= D3DTSS_CONTROL0_DIRTY;
        return D3D_OK;
    case D3DTSS_COLORSIGN:
        g_pDevice->TextureStageState[Stage].Control0 =
            (g_pDevice->TextureStageState[Stage].Control0 &
                ~NV097_SET_TEXTURE_FILTER_SIGN) | Value;
        g_pDevice->TextureStageState[Stage].Dirty |= D3DTSS_CONTROL0_DIRTY;
        return D3D_OK;
    case D3DTSS_ALPHAKILL:
        g_pDevice->TextureStageState[Stage].Control0 =
            (g_pDevice->TextureStageState[Stage].Control0 &
                 ~NV097_SET_TEXTURE_CONTROL0_ALPHA_KILL_ENABLE) | Value;
        g_pDevice->TextureStageState[Stage].Dirty |= D3DTSS_CONTROL0_DIRTY;
        return D3D_OK;
    default:
        // TODO: implement remaining texture stage states
        return E_NOTIMPL;
    }
}

#if NXDK_DEBUG
HRESULT D3DDevice_SetTextureStageState_Validate(
    DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD Value)
{
    if (Stage >= D3DTSS_MAXSTAGES)
        return D3DERR_INVALIDCALL;
    if (Type  >= D3DTSS_MAX)
        return D3DERR_INVALIDCALL;

    switch (Type) {
    case D3DTSS_ADDRESSU:
    case D3DTSS_ADDRESSV:
    case D3DTSS_ADDRESSW:
        if (Value < D3DTADDRESS_WRAP || Value >= D3DTADDRESS_MAX)
            return D3DERR_INVALIDCALL;
        return D3D_OK;
    case D3DTSS_MAGFILTER:
    case D3DTSS_MINFILTER:
        if (Value == D3DTEXF_NONE)
            return D3DERR_INVALIDCALL;
        D3D_FALLTHROUGH();
    case D3DTSS_MIPFILTER:
        if (Value >= D3DTEXF_MAX)
            return D3DERR_INVALIDCALL;
        return D3D_OK;
    case D3DTSS_MAXMIPLEVEL:
        if (Value > 15)
            return D3DERR_INVALIDCALL;
        return D3D_OK;
    case D3DTSS_MAXANISOTROPY:
        if (Value < 1 || Value > 4)
            return D3DERR_INVALIDCALL;
        return D3D_OK;
    case D3DTSS_COLORKEYOP:
        if (Value >= D3DTCOLORKEYOP_MAX)
            return D3DERR_INVALIDCALL;
        return D3D_OK;
    case D3DTSS_COLORSIGN:
        if (Value & 0x0FFFFFFF)
            return D3DERR_INVALIDCALL;
        return D3D_OK;
    case D3DTSS_ALPHAKILL:
        if (Value != D3DTALPHAKILL_DISABLE && Value != D3DTALPHAKILL_ENABLE)
            return D3DERR_INVALIDCALL;
        return D3D_OK;
    case D3DTSS_COLORARG0:
    case D3DTSS_COLORARG1:
    case D3DTSS_COLORARG2:
    case D3DTSS_ALPHAARG0:
    case D3DTSS_ALPHAARG1:
    case D3DTSS_ALPHAARG2:
        if (Value & 0xFFFFFFC0)
            return D3DERR_INVALIDCALL;
        return D3D_OK;
    default:
        return D3D_OK;
    }
}
#endif // NXDK_DEBUG

HRESULT D3DDevice_SetTextureStageState(
    DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD Value)
{
#if NXDK_DEBUG
    HRESULT hr = D3DDevice_SetTextureStageState_Validate(Stage, Type, Value);
    assert(SUCCEEDED(hr));
    if (FAILED(hr)) return hr;
#endif // NXDK_DEBUG
    if (Type >= D3DTSS_DEFERRED_MAX)
        return D3DDevice_SetTextureStageState_Immediate(Stage, Type, Value);
    else
        return D3DDevice_SetTextureStageState_Deferred(Stage, Type, Value);
}

D3DAPI HRESULT Direct3DDevice8_SetTextureStageState(
    LPDIRECT3DDEVICE8 pThis, DWORD Stage,
    D3DTEXTURESTAGESTATETYPE Type, DWORD Value)
{
    assert(pThis == &g_pDevice->iface);
    return D3DDevice_SetTextureStageState(Stage, Type, Value);
}

HRESULT D3DDevice_CreatePushBuffer(
    UINT Size, BOOL RunUsingCpuCopy, D3DPushBuffer** ppPushBuffer)
{
    D3DPushBuffer* pb = malloc(sizeof(*pb));
    if (!pb)
        return E_OUTOFMEMORY;

    D3D_CreatePushBuffer(Size, RunUsingCpuCopy, NULL, pb);
    *ppPushBuffer = pb;
    return D3D_OK;
}

D3DAPI HRESULT Direct3DDevice8_CreatePushBuffer(
    LPDIRECT3DDEVICE8 pThis, UINT Size,
    BOOL RunUsingCpuCopy, LPDIRECT3DPUSHBUFFER8* ppPushBuffer)
{
    assert(pThis == &g_pDevice->iface);
    return D3DDevice_CreatePushBuffer(Size, RunUsingCpuCopy,
                                      (D3DPushBuffer**)ppPushBuffer);
}

HRESULT D3DDevice_LoadVertexShaderProgram(
    CONST DWORD *pFunction, DWORD Address)
{
    HRESULT hr = D3DDevice_PushCmd(NV097_SET_TRANSFORM_PROGRAM_LOAD, Address);
    if(FAILED(hr)) return hr;

    int i = 0;
    for (; pFunction[i] != D3DVS_END() && Address + i < 136; i += 4) {
        hr = D3DDevice_PushCmd4f(NV097_SET_TRANSFORM_PROGRAM,
                                 pFunction[i],   pFunction[i+1],
                                 pFunction[i+2], pFunction[i+3], TRUE);
        if(FAILED(hr)) return hr;
    }

    // If we overran, this means pFunction did not contain a D3DVS_END() token.
    // Either pFunction does not point to a valid vertex shader program, or
    // that program is too long to be supported by the hardware (max length
    // supported is 136 DWORDs). Either way, this is an error on the caller's
    // part.
#if NXDK_DEBUG
    if (Address + i >= 136) {
        assert(false);
        return D3DERR_INVALIDCALL;
    }
#endif // NXDK_DEBUG

    return D3D_OK;
}

D3DAPI HRESULT Direct3DDevice8_LoadVertexShaderProgram(
    LPDIRECT3DDEVICE8 pThis, CONST DWORD *pFunction, DWORD Address)
{
    assert(pThis == &g_pDevice->iface);
    return D3DDevice_LoadVertexShaderProgram(pFunction, Address);
}

DWORD D3D_SimpleRenderState[D3DRS_MAX] = {
    [D3DRS_PSALPHAINPUTS0]            = NV20_TCL_PRIMITIVE_3D_RC_IN_ALPHA(0),
    [D3DRS_PSALPHAINPUTS1]            = NV20_TCL_PRIMITIVE_3D_RC_IN_ALPHA(1),
    [D3DRS_PSALPHAINPUTS2]            = NV20_TCL_PRIMITIVE_3D_RC_IN_ALPHA(2),
    [D3DRS_PSALPHAINPUTS3]            = NV20_TCL_PRIMITIVE_3D_RC_IN_ALPHA(3),
    [D3DRS_PSALPHAINPUTS4]            = NV20_TCL_PRIMITIVE_3D_RC_IN_ALPHA(4),
    [D3DRS_PSALPHAINPUTS5]            = NV20_TCL_PRIMITIVE_3D_RC_IN_ALPHA(5),
    [D3DRS_PSALPHAINPUTS6]            = NV20_TCL_PRIMITIVE_3D_RC_IN_ALPHA(6),
    [D3DRS_PSALPHAINPUTS7]            = NV20_TCL_PRIMITIVE_3D_RC_IN_ALPHA(7),
    [D3DRS_PSRGBINPUTS0]              = NV20_TCL_PRIMITIVE_3D_RC_IN_RGB(0),
    [D3DRS_PSRGBINPUTS1]              = NV20_TCL_PRIMITIVE_3D_RC_IN_RGB(1),
    [D3DRS_PSRGBINPUTS2]              = NV20_TCL_PRIMITIVE_3D_RC_IN_RGB(2),
    [D3DRS_PSRGBINPUTS3]              = NV20_TCL_PRIMITIVE_3D_RC_IN_RGB(3),
    [D3DRS_PSRGBINPUTS4]              = NV20_TCL_PRIMITIVE_3D_RC_IN_RGB(4),
    [D3DRS_PSRGBINPUTS5]              = NV20_TCL_PRIMITIVE_3D_RC_IN_RGB(5),
    [D3DRS_PSRGBINPUTS6]              = NV20_TCL_PRIMITIVE_3D_RC_IN_RGB(6),
    [D3DRS_PSRGBINPUTS7]              = NV20_TCL_PRIMITIVE_3D_RC_IN_RGB(7),
    [D3DRS_PSFINALCOMBINERCONSTANT0]  = NV20_TCL_PRIMITIVE_3D_RC_COLOR0,
    [D3DRS_PSFINALCOMBINERCONSTANT1]  = NV20_TCL_PRIMITIVE_3D_RC_COLOR1,
    [D3DRS_PSCONSTANT0_0]       = NV20_TCL_PRIMITIVE_3D_RC_CONSTANT_COLOR0(0),
    [D3DRS_PSCONSTANT0_1]       = NV20_TCL_PRIMITIVE_3D_RC_CONSTANT_COLOR0(1),
    [D3DRS_PSCONSTANT0_2]       = NV20_TCL_PRIMITIVE_3D_RC_CONSTANT_COLOR0(2),
    [D3DRS_PSCONSTANT0_3]       = NV20_TCL_PRIMITIVE_3D_RC_CONSTANT_COLOR0(3),
    [D3DRS_PSCONSTANT0_4]       = NV20_TCL_PRIMITIVE_3D_RC_CONSTANT_COLOR0(4),
    [D3DRS_PSCONSTANT0_5]       = NV20_TCL_PRIMITIVE_3D_RC_CONSTANT_COLOR0(5),
    [D3DRS_PSCONSTANT0_6]       = NV20_TCL_PRIMITIVE_3D_RC_CONSTANT_COLOR0(6),
    [D3DRS_PSCONSTANT0_7]       = NV20_TCL_PRIMITIVE_3D_RC_CONSTANT_COLOR0(7),
    [D3DRS_PSCONSTANT1_0]       = NV20_TCL_PRIMITIVE_3D_RC_CONSTANT_COLOR1(0),
    [D3DRS_PSCONSTANT1_1]       = NV20_TCL_PRIMITIVE_3D_RC_CONSTANT_COLOR1(1),
    [D3DRS_PSCONSTANT1_2]       = NV20_TCL_PRIMITIVE_3D_RC_CONSTANT_COLOR1(2),
    [D3DRS_PSCONSTANT1_3]       = NV20_TCL_PRIMITIVE_3D_RC_CONSTANT_COLOR1(3),
    [D3DRS_PSCONSTANT1_4]       = NV20_TCL_PRIMITIVE_3D_RC_CONSTANT_COLOR1(4),
    [D3DRS_PSCONSTANT1_5]       = NV20_TCL_PRIMITIVE_3D_RC_CONSTANT_COLOR1(5),
    [D3DRS_PSCONSTANT1_6]       = NV20_TCL_PRIMITIVE_3D_RC_CONSTANT_COLOR1(6),
    [D3DRS_PSCONSTANT1_7]       = NV20_TCL_PRIMITIVE_3D_RC_CONSTANT_COLOR1(7),
    [D3DRS_PSALPHAOUTPUTS0]           = NV20_TCL_PRIMITIVE_3D_RC_OUT_ALPHA(0),
    [D3DRS_PSALPHAOUTPUTS1]           = NV20_TCL_PRIMITIVE_3D_RC_OUT_ALPHA(1),
    [D3DRS_PSALPHAOUTPUTS2]           = NV20_TCL_PRIMITIVE_3D_RC_OUT_ALPHA(2),
    [D3DRS_PSALPHAOUTPUTS3]           = NV20_TCL_PRIMITIVE_3D_RC_OUT_ALPHA(3),
    [D3DRS_PSALPHAOUTPUTS4]           = NV20_TCL_PRIMITIVE_3D_RC_OUT_ALPHA(4),
    [D3DRS_PSALPHAOUTPUTS5]           = NV20_TCL_PRIMITIVE_3D_RC_OUT_ALPHA(5),
    [D3DRS_PSALPHAOUTPUTS6]           = NV20_TCL_PRIMITIVE_3D_RC_OUT_ALPHA(6),
    [D3DRS_PSALPHAOUTPUTS7]           = NV20_TCL_PRIMITIVE_3D_RC_OUT_ALPHA(7),
    [D3DRS_PSRGBOUTPUTS0]             = NV20_TCL_PRIMITIVE_3D_RC_OUT_RGB(0),
    [D3DRS_PSRGBOUTPUTS1]             = NV20_TCL_PRIMITIVE_3D_RC_OUT_RGB(1),
    [D3DRS_PSRGBOUTPUTS2]             = NV20_TCL_PRIMITIVE_3D_RC_OUT_RGB(2),
    [D3DRS_PSRGBOUTPUTS3]             = NV20_TCL_PRIMITIVE_3D_RC_OUT_RGB(3),
    [D3DRS_PSRGBOUTPUTS4]             = NV20_TCL_PRIMITIVE_3D_RC_OUT_RGB(4),
    [D3DRS_PSRGBOUTPUTS5]             = NV20_TCL_PRIMITIVE_3D_RC_OUT_RGB(5),
    [D3DRS_PSRGBOUTPUTS6]             = NV20_TCL_PRIMITIVE_3D_RC_OUT_RGB(6),
    [D3DRS_PSRGBOUTPUTS7]             = NV20_TCL_PRIMITIVE_3D_RC_OUT_RGB(7),
    [D3DRS_PSFINALCOMBINERINPUTSABCD] = NV20_TCL_PRIMITIVE_3D_RC_FINAL0,
    [D3DRS_PSFINALCOMBINERINPUTSEFG]  = NV20_TCL_PRIMITIVE_3D_RC_FINAL1,
    [D3DRS_PSCOMPAREMODE]             = NV097_SET_SHADER_CLIP_PLANE_MODE,
    [D3DRS_PSCOMBINERCOUNT]           = NV097_SET_COMBINER_CONTROL,
    [D3DRS_PSTEXTUREMODES]            = NV097_SET_SHADER_STAGE_PROGRAM,
    [D3DRS_PSDOTMAPPING]              = NV097_SET_DOT_RGBMAPPING,
    [D3DRS_PSINPUTTEXTURE]            = NV097_SET_SHADER_OTHER_STAGE_INPUT,
    [D3DRS_ZFUNC]                     = NV097_SET_DEPTH_FUNC,
    [D3DRS_ALPHAFUNC]                 = NV097_SET_ALPHA_FUNC,
    [D3DRS_ALPHABLENDENABLE]          = NV097_SET_BLEND_ENABLE,
    [D3DRS_ALPHATESTENABLE]           = NV097_SET_ALPHA_TEST_ENABLE,
    [D3DRS_ALPHAREF]                  = NV097_SET_ALPHA_REF,
    [D3DRS_SRCBLEND]                  = NV097_SET_BLEND_FUNC_SFACTOR,
    [D3DRS_DESTBLEND]                 = NV097_SET_BLEND_FUNC_DFACTOR,
    [D3DRS_ZWRITEENABLE]              = NV097_SET_DEPTH_MASK,
    [D3DRS_DITHERENABLE]              = NV097_SET_DITHER_ENABLE,
    [D3DRS_SHADEMODE]                 = NV097_SET_SHADE_MODEL,
    [D3DRS_COLORWRITEENABLE]          = NV097_SET_COLOR_MASK,
    [D3DRS_STENCILZFAIL]              = NV097_SET_STENCIL_OP_ZFAIL,
    [D3DRS_STENCILPASS]               = NV097_SET_STENCIL_OP_ZPASS,
    [D3DRS_STENCILFAIL]               = NV097_SET_STENCIL_OP_FAIL,
    [D3DRS_STENCILREF]                = NV097_SET_STENCIL_FUNC_REF,
    [D3DRS_STENCILMASK]               = NV097_SET_STENCIL_MASK,
    [D3DRS_SWATHWIDTH]                = NV097_SET_SWATH_WIDTH,
    [D3DRS_BLENDOP]                   = NV097_SET_BLEND_EQUATION,
    [D3DRS_BLENDCOLOR]                = NV097_SET_BLEND_COLOR,
    [D3DRS_POLYGONOFFSETZSLOPESCALE]  = NV097_SET_POLYGON_OFFSET_SCALE_FACTOR,
    [D3DRS_POLYGONOFFSETZOFFSET]      = NV097_SET_POLYGON_OFFSET_BIAS,
    [D3DRS_POINTOFFSETENABLE]         = NV097_SET_POLY_OFFSET_POINT_ENABLE,
    [D3DRS_WIREFRAMEOFFSETENABLE]     = NV097_SET_POLY_OFFSET_LINE_ENABLE,
    [D3DRS_SOLIDOFFSETENABLE]         = NV097_SET_POLY_OFFSET_FILL_ENABLE,
    [D3DRS_DEPTHCLIPCONTROL]          = NV097_SET_ZMIN_MAX_CONTROL,
    [D3DRS_STIPPLEENABLE]             = NV097_SET_STIPPLE_ENABLE,
};

#if NXDK_DEBUG
HRESULT D3DDevice_SetRenderState_Simple_Validate(D3DRENDERSTATETYPE Type,
                                                 DWORD Value)
{
    if (Type >= D3DRS_MAX)
        return D3DERR_INVALIDCALL;

    if (D3D_SimpleRenderState[Type] == 0)
        return E_NOTIMPL;

    switch (Type) {
    case D3DRS_ZFUNC:
    case D3DRS_ALPHAFUNC:
    case D3DRS_STENCILFUNC:
        if (Value < D3DCMP_NEVER || Value > D3DCMP_ALWAYS)
            return D3DERR_INVALIDCALL;
        return D3D_OK;
    case D3DRS_ALPHABLENDENABLE:
    case D3DRS_ALPHATESTENABLE:
    case D3DRS_ZWRITEENABLE:
    case D3DRS_DITHERENABLE:
    case D3DRS_POINTOFFSETENABLE:
    case D3DRS_WIREFRAMEOFFSETENABLE:
    case D3DRS_SOLIDOFFSETENABLE:
    case D3DRS_STIPPLEENABLE:
        if (Value > TRUE)
            return D3DERR_INVALIDCALL;
        return D3D_OK;
    case D3DRS_SRCBLEND:
    case D3DRS_DESTBLEND:
        if (Value <= D3DBLEND_ONE)
            return D3D_OK;
        if (Value >= D3DBLEND_SRCCOLOR && Value <= D3DBLEND_SRCALPHASAT)
            return D3D_OK;
        if (Value >= D3DBLEND_CONSTANTCOLOR &&
            Value <= D3DBLEND_INVCONSTANTALPHA)
        {
            return D3D_OK;
        }
        return D3DERR_INVALIDCALL;
    case D3DRS_SHADEMODE:
        if (Value != D3DSHADE_FLAT && Value != D3DSHADE_GOURAUD)
            return D3DERR_INVALIDCALL;
        return D3D_OK;
    case D3DRS_COLORWRITEENABLE:
        if (Value & ~D3DCOLORWRITEENABLE_ALL)
            return D3DERR_INVALIDCALL;
        return D3D_OK;
    case D3DRS_STENCILZFAIL:
    case D3DRS_STENCILPASS:
        if (Value == D3DSTENCILOP_ZERO)
            return D3D_OK;
        if (Value >= D3DSTENCILOP_KEEP && Value <= D3DSTENCILOP_DECRSAT)
            return D3D_OK;
        if (Value == D3DSTENCILOP_INVERT)
            return D3D_OK;
        if (Value == D3DSTENCILOP_INCR || Value == D3DSTENCILOP_DECR)
            return D3D_OK;
        return D3DERR_INVALIDCALL;
    case D3DRS_BLENDOP:
        if (Value >= D3DBLENDOP_ADD && Value <= D3DBLENDOP_MAX)
            return D3D_OK;
        if (Value == D3DBLENDOP_SUBTRACT || Value == D3DBLENDOP_REVSUBTRACT)
            return D3D_OK;
        if (Value == D3DBLENDOP_REVSUBTRACTSIGNED ||
            Value == D3DBLENDOP_ADDSIGNED)
        {
            return D3D_OK;
        }
        return D3DERR_INVALIDCALL;
    case D3DRS_SWATHWIDTH:
        if (Value >= D3DSWATH_8 && Value <= D3DSWATH_128)
            return D3D_OK;
        if (Value == D3DSWATH_OFF)
            return D3D_OK;
        return D3DERR_INVALIDCALL;
    case D3DRS_DEPTHCLIPCONTROL:
        if (Value &
            ~(D3DDCC_CULLPRIMITIVE | D3DDCC_CLAMP | D3DDCC_IGNORE_W_SIGN))
        {
            return D3DERR_INVALIDCALL;
        }
        return D3D_OK;
    default:
        return D3D_OK;
    }
}
#endif // NXDK_DEBUG

HRESULT D3DDevice_SetRenderState_Simple(D3DRENDERSTATETYPE Type, DWORD Value) {
#if NXDK_DEBUG
    HRESULT hr = D3DDevice_SetRenderState_Simple_Validate(Type, Value);
    assert(SUCCEEDED(hr));
    if (FAILED(hr)) return hr;
#endif // NXDK_DEBUG
    if (Type == D3DRS_ZWRITEENABLE) {
        DWORD Control0 = g_pDevice->Control0;
        MASK_INPLACE(g_pDevice->Control0,
                     NV097_SET_CONTROL0_STENCIL_WRITE_ENABLE, Value);

        if (g_pDevice->Control0 == Control0)
            return D3D_OK;

        Value = g_pDevice->Control0;
    }
    DWORD Cmd = D3D_SimpleRenderState[Type];
    return D3DDevice_PushCmd(Cmd, Value);
}

#if NXDK_DEBUG
HRESULT D3DDevice_SetRenderState_Deferred_Validate(D3DRENDERSTATETYPE Type,
                                                   DWORD Value)
{
    switch (Type) {
    case D3DRS_FOGENABLE:
        if (Value > TRUE)
            return D3DERR_INVALIDCALL;
        return D3D_OK;
    case D3DRS_PRESENTATIONINTERVAL:
        if (Value > D3DPRESENT_INTERVAL_THREE_OR_IMMEDIATE)
            return D3DERR_INVALIDCALL;
        return D3D_OK;
    default:
        return D3D_OK;
    }
}
#endif // NXDK_DEBUG

HRESULT D3DDevice_SetRenderState_Deferred(D3DRENDERSTATETYPE Type, DWORD Value)
{
#if NXDK_DEBUG
    HRESULT hr = D3DDevice_SetRenderState_Deferred_Validate(Type, Value);
    assert(SUCCEEDED(hr));
    if (FAILED(hr)) return hr;
#endif // NXDK_DEBUG
    switch(Type) {
    case D3DRS_FOGENABLE:
        return D3DDevice_PushCmd(NV097_SET_FOG_ENABLE, Value);
    case D3DRS_PRESENTATIONINTERVAL:
        g_pDevice->PresentInterval = Value;
        return D3D_OK;
    default:
        // TODO: implement remaining render states
        assert(false);
        return E_NOTIMPL;
    }
}

HRESULT D3DDevice_SetRenderState_Complex_Validate(D3DRENDERSTATETYPE Type,
                                                  DWORD Value)
{
    switch (Type) {
    case D3DRS_PSTEXTUREMODES:
        if (MASK(NV097_SET_SHADER_STAGE_PROGRAM_STAGE0, Value) >
            NV097_SET_SHADER_STAGE_PROGRAM_STAGE0_CLIP_PLANE)
        {
            return D3DERR_INVALIDCALL;
        }

        DWORD Stage1 = MASK(NV097_SET_SHADER_STAGE_PROGRAM_STAGE1, Value);
        if (Stage1 >
            NV097_SET_SHADER_STAGE_PROGRAM_STAGE1_BUMPENVMAP_LUMINANCE &&
            Stage1 < NV097_SET_SHADER_STAGE_PROGRAM_STAGE1_DEPENDENT_AR)
        {
            return D3DERR_INVALIDCALL;
        }

        if (Stage1 > NV097_SET_SHADER_STAGE_PROGRAM_STAGE1_DOT_PRODUCT)
            return D3DERR_INVALIDCALL;

        DWORD Stage2 = MASK(NV097_SET_SHADER_STAGE_PROGRAM_STAGE2, Value);
        if (Stage2 >
            NV097_SET_SHADER_STAGE_PROGRAM_STAGE2_DOT_REFLECT_DIFFUSE &&
            Stage2 < NV097_SET_SHADER_STAGE_PROGRAM_STAGE2_DEPENDENT_AR)
        {
            return D3DERR_INVALIDCALL;
        }

        if (Stage2 > NV097_SET_SHADER_STAGE_PROGRAM_STAGE2_DOT_PRODUCT)
            return D3DERR_INVALIDCALL;

        DWORD Stage3 = MASK(NV097_SET_SHADER_STAGE_PROGRAM_STAGE3, Value);
        if (Stage3 >
            NV097_SET_SHADER_STAGE_PROGRAM_STAGE3_DOT_REFLECT_SPECULAR_CONST)
        {
            return D3DERR_INVALIDCALL;
        }
        return D3D_OK;
    case D3DRS_VERTEXBLEND:
        if (Value >= D3DVBF_MAX)
            return D3DERR_INVALIDCALL;
        return D3D_OK;
    case D3DRS_FILLMODE:
    case D3DRS_BACKFILLMODE:
        if (Value < D3DFILL_POINT || Value > D3DFILL_SOLID)
            return D3DERR_INVALIDCALL;
        return D3D_OK;
    case D3DRS_TWOSIDEDLIGHTING:
    case D3DRS_NORMALIZENORMALS:
    case D3DRS_STENCILENABLE:
    case D3DRS_EDGEANTIALIAS:
    case D3DRS_MULTISAMPLEANTIALIAS:
    case D3DRS_DXT1NOISEENABLE:
    case D3DRS_YUVENABLE:
    case D3DRS_OCCLUSIONCULLENABLE:
    case D3DRS_STENCILCULLENABLE:
    case D3DRS_ROPZCMPALWAYSREAD:
    case D3DRS_ROPZREAD:
    case D3DRS_DONOTCULLUNCOMPRESSED:
        if (Value > TRUE)
            return D3DERR_INVALIDCALL;
        return D3D_OK;
    case D3DRS_STENCILFAIL:
        if (Value == D3DSTENCILOP_ZERO)
            return D3D_OK;
        if (Value >= D3DSTENCILOP_KEEP && Value <= D3DSTENCILOP_DECRSAT)
            return D3D_OK;
        if (Value == D3DSTENCILOP_INVERT)
            return D3D_OK;
        if (Value == D3DSTENCILOP_INCR || Value == D3DSTENCILOP_DECR)
            return D3D_OK;
        return D3DERR_INVALIDCALL;
    case D3DRS_CULLMODE:
        if (Value == D3DCULL_NONE)
            return D3D_OK;
        D3D_FALLTHROUGH();
    case D3DRS_FRONTFACE:
        if (Value == D3DFRONT_CW || Value == D3DFRONT_CCW)
            return D3D_OK;
        return D3DERR_INVALIDCALL;
    case D3DRS_LOGICOP:
        if (Value == D3DLOGICOP_NONE)
            return D3D_OK;
        if (Value >= D3DLOGICOP_CLEAR && Value <= D3DLOGICOP_SET)
            return D3D_OK;
        return D3DERR_INVALIDCALL;
    case D3DRS_MULTISAMPLEMODE:
    case D3DRS_MULTISAMPLERENDERTARGETMODE:
        if (Value > D3DMULTISAMPLEMODE_4X)
            return D3DERR_INVALIDCALL;
        return D3D_OK;
    case D3DRS_SHADOWFUNC:
        if (Value > D3DCMP_ALWAYS)
            return D3D_OK;
        return D3DERR_INVALIDCALL;
    default:
        return D3D_OK;
    }
}

HRESULT D3DDevice_SetRenderState_Complex(D3DRENDERSTATETYPE Type, DWORD Value)
{
#if NXDK_DEBUG
    HRESULT hr = D3DDevice_SetRenderState_Complex_Validate(Type, Value);
    assert(SUCCEEDED(hr));
    if (FAILED(hr)) return hr;
#else
    HRESULT hr = D3D_OK;
#endif // NXDK_DEBUG
    switch (Type) {
    case D3DRS_PSTEXTUREMODES:
        return D3DDevice_PushCmd(NV097_SET_SHADER_STAGE_PROGRAM, Value);
    case D3DRS_VERTEXBLEND:
        return D3DDevice_PushCmd(NV097_SET_SKIN_MODE, Value);
    case D3DRS_FOGCOLOR:
        return D3DDevice_PushCmd(NV097_SET_FOG_COLOR, Value);
    case D3DRS_FILLMODE:
        return D3DDevice_PushCmd(NV097_SET_FRONT_POLYGON_MODE, Value);
    case D3DRS_BACKFILLMODE:
        return D3DDevice_PushCmd(NV097_SET_BACK_POLYGON_MODE, Value);
    case D3DRS_TWOSIDEDLIGHTING:
        return D3DDevice_PushCmd(NV097_SET_LIGHT_TWO_SIDE_ENABLE, Value);
    case D3DRS_NORMALIZENORMALS:
        return D3DDevice_PushCmd(NV097_SET_NORMALIZATION_ENABLE, Value);
    case D3DRS_ZENABLE:
        return D3DDevice_PushCmd(NV097_SET_DEPTH_TEST_ENABLE, Value);
    case D3DRS_STENCILENABLE:
        return D3DDevice_PushCmd(NV097_SET_STENCIL_TEST_ENABLE, Value);
    case D3DRS_STENCILFAIL:
        return D3DDevice_PushCmd(NV097_SET_STENCIL_OP_FAIL, Value);
    case D3DRS_CULLMODE:
        D3DCULL FaceCull = g_pDevice->RenderState.FaceCull;
        BOOL bWasFaceCullingEnabled = FaceCull == D3DCULL_NONE;
        BOOL bIsFaceCullingEnabled = Value == D3DCULL_NONE;
        if (bWasFaceCullingEnabled != bIsFaceCullingEnabled) {
            hr = D3DDevice_PushCmd(NV097_SET_CULL_FACE, bIsFaceCullingEnabled);
            if (FAILED(hr)) return hr;
        }
        D3D_FALLTHROUGH();
    case D3DRS_FRONTFACE:
        return D3DDevice_PushCmd(NV097_SET_FRONT_FACE, Value);
    case D3DRS_TEXTUREFACTOR:
        return D3DDevice_PushCmd(NV097_SET_BLEND_COLOR, Value);
    case D3DRS_LOGICOP: {
        DWORD LogicOp = g_pDevice->RenderState.LogicOp;
        BOOL bWasLogicOpEnabled = LogicOp != D3DLOGICOP_NONE;
        g_pDevice->RenderState.LogicOp = Value;
        BOOL bIsLogicOpEnabled =
            g_pDevice->RenderState.LogicOp != D3DLOGICOP_NONE;
        if (bWasLogicOpEnabled != bIsLogicOpEnabled) {
            HRESULT hr = D3DDevice_PushCmd(NV097_SET_LOGIC_OP_ENABLE,
                                           bIsLogicOpEnabled);
            if (FAILED(hr)) return hr;
        }

        if (LogicOp != g_pDevice->RenderState.LogicOp)
            return D3DDevice_PushCmd(NV097_SET_LOGIC_OP, LogicOp);

        return D3D_OK;
    }
    case D3DRS_EDGEANTIALIAS:
        return D3DDevice_PushCmd(NV097_SET_LINE_SMOOTH_ENABLE, Value);
    case D3DRS_MULTISAMPLEANTIALIAS:
    case D3DRS_MULTISAMPLEMASK: {
        DWORD MultiSample = g_pDevice->RenderState.MultiSample;
        DWORD Mask = NV097_SET_MULTISAMPLE_ENABLE;
        if (Type == D3DRS_MULTISAMPLEMASK)
            Mask = NV097_SET_MULTISAMPLE_MASK;

        MASK_INPLACE(g_pDevice->RenderState.MultiSample, Mask, Value);

        if (MultiSample != g_pDevice->RenderState.MultiSample) {
            return D3DDevice_PushCmd(NV097_SET_MULTISAMPLE,
                                     g_pDevice->RenderState.MultiSample);
        }

        return D3D_OK;
    }
    case D3DRS_SHADOWFUNC:
        return D3DDevice_PushCmd(NV097_SET_SHADOW_COMPARE_FUNC, Value);
    case D3DRS_LINEWIDTH:
        return D3DDevice_PushCmd(NV097_SET_LINE_WIDTH, Value);
    case D3DRS_DXT1NOISEENABLE:
        return D3DDevice_InsertInterrupt(PB_SETNOISE, Value);
    case D3DRS_OCCLUSIONCULLENABLE:
    case D3DRS_STENCILCULLENABLE: {
        DWORD CullMode = g_pDevice->RenderState.CullMode;
        BOOL bWasCullingEnabled = CullMode == 0;

        DWORD Mask = NV097_SET_CULL_MODE_OCCLUSION;
        if (Type == D3DRS_STENCILCULLENABLE)
            Mask = NV097_SET_CULL_MODE_STENCIL;

        MASK_INPLACE(g_pDevice->RenderState.CullMode, Mask, Value);

        BOOL bIsCullingEnabled = g_pDevice->RenderState.CullMode == 0;
        if (bWasCullingEnabled != bIsCullingEnabled) {
            hr = D3DDevice_PushCmd(NV097_SET_CULL_FACE_ENABLE, CullMode);
            if (FAILED(hr)) return hr;
        }

        if (CullMode != g_pDevice->RenderState.CullMode)
            return D3DDevice_PushCmd(NV097_SET_CULL_MODE, CullMode);

        return D3D_OK;
    }
    case D3DRS_ROPZCMPALWAYSREAD:
    case D3DRS_ROPZREAD: {
        DWORD RopZ = g_pDevice->RenderState.RopZ;

        DWORD Mask = NV_PGRAPH_UNKNOWN_400B80_ROPZ_CMP_ALWAYS_READ;
        if (Type == D3DRS_ROPZREAD)
            Mask = NV_PGRAPH_UNKNOWN_400B80_ROPZ_READ;

        MASK_INPLACE(g_pDevice->RenderState.RopZ, Mask, Value);
        if (RopZ != g_pDevice->RenderState.RopZ) {
            return D3DDevice_InsertInterrupt2(PB_SETOUTER,
                                              NV_PGRAPH_UNKNOWN_400B80,
                                              g_pDevice->RenderState.RopZ);
        }

        return D3D_OK;
    }
    case D3DRS_DONOTCULLUNCOMPRESSED:
        DWORD Debug5 = g_pDevice->RenderState.Debug5;
        MASK_INPLACE(g_pDevice->RenderState.Debug5,
                     NV_PGRAPH_DEBUG_5_ZCULL_SPARE2_ENABLED, Value);
        if (Debug5 != g_pDevice->RenderState.Debug5) {
            return D3DDevice_InsertInterrupt2(PB_SETOUTER,
                                              NV_PGRAPH_DEBUG_5, Debug5);
        }

        return D3D_OK;
    default:
        // TODO: implement remaining render states
        assert(false);
        return E_NOTIMPL;
    }
}


HRESULT D3DDevice_SetRenderState(D3DRENDERSTATETYPE Type, DWORD Value) {
    // Render states can be categorized into three groups:
    // 1) Simple render states that can be directly translated to a single GPU
    //    command.
    // 2) Deferred render states that will only be processed after the next
    //    draw call due to interdependencies with other render states.
    // 3) Complex states that don't correspond to a single GPU command and
    //    require more complex handling.
    if (Type < D3DRS_SIMPLE_MAX)
        return D3DDevice_SetRenderState_Simple(Type, Value);
    else if (Type < D3DRS_DEFERRED_MAX)
        return D3DDevice_SetRenderState_Deferred(Type, Value);
    else
        return D3DDevice_SetRenderState_Complex(Type, Value);
}

D3DAPI HRESULT Direct3DDevice8_SetRenderState(
    LPDIRECT3DDEVICE8 pThis, D3DRENDERSTATETYPE Type, DWORD Value)
{
    assert(pThis == &g_pDevice->iface);
    return D3DDevice_SetRenderState(Type, Value);
}

HRESULT D3DDevice_SetPixelShaderProgram(CONST D3DPIXELSHADERDEF *pPSDef) {
    // TODO: Optimize this function by setting the hardware registers directly
    // without going through D3DDevice_SetRenderState for each register.

    HRESULT hr = D3D_OK;
    for (int i = 0; i < countof(pPSDef->PSAlphaInputs); i++) {
        hr = D3DDevice_SetRenderState(D3DRS_PSALPHAINPUTS0 + i,
                                      pPSDef->PSAlphaInputs[i]);
        if (FAILED(hr)) return hr;
    }
    hr = D3DDevice_SetRenderState(D3DRS_PSFINALCOMBINERINPUTSABCD,
                                          pPSDef->PSFinalCombinerInputsABCD);
    if(FAILED(hr)) return hr;
    hr = D3DDevice_SetRenderState(D3DRS_PSFINALCOMBINERINPUTSEFG,
                                  pPSDef->PSFinalCombinerInputsEFG);
    if(FAILED(hr)) return hr;
    for (int i = 0; i < countof(pPSDef->PSConstant0); i++) {
        hr = D3DDevice_SetRenderState(D3DRS_PSCONSTANT0_0 + i,
                                      pPSDef->PSConstant0[i]);
        if(FAILED(hr)) return hr;
    }
    for (int i = 0; i < countof(pPSDef->PSConstant1); i++) {
        hr = D3DDevice_SetRenderState(D3DRS_PSCONSTANT1_0 + i,
                                      pPSDef->PSConstant1[i]);
        if(FAILED(hr)) return hr;
    }
    for (int i = 0; i < countof(pPSDef->PSAlphaOutputs); i++) {
        hr = D3DDevice_SetRenderState(D3DRS_PSALPHAOUTPUTS0 + i,
                                      pPSDef->PSAlphaOutputs[i]);
        if (FAILED(hr)) return hr;
    }
    for (int i = 0; i < countof(pPSDef->PSRGBInputs); i++) {
        hr = D3DDevice_SetRenderState(D3DRS_PSRGBINPUTS0 + i,
                                      pPSDef->PSRGBInputs[i]);
        if (FAILED(hr)) return hr;
    }
    hr = D3DDevice_SetRenderState(D3DRS_PSCOMPAREMODE, pPSDef->PSCompareMode);
    if (FAILED(hr)) return hr;
    hr = D3DDevice_SetRenderState(D3DRS_PSFINALCOMBINERCONSTANT0,
                                  pPSDef->PSFinalCombinerConstant0);
    if (FAILED(hr)) return hr;
    hr = D3DDevice_SetRenderState(D3DRS_PSFINALCOMBINERCONSTANT1,
                                  pPSDef->PSFinalCombinerConstant1);
    if (FAILED(hr)) return hr;
    for (int i = 0; i < countof(pPSDef->PSRGBOutputs); i++) {
        hr = D3DDevice_SetRenderState(D3DRS_PSRGBOUTPUTS0 + i,
                                      pPSDef->PSRGBOutputs[i]);
        if (FAILED(hr)) return hr;
    }
    hr = D3DDevice_SetRenderState(D3DRS_PSCOMBINERCOUNT,
                                  pPSDef->PSCombinerCount);
    if (FAILED(hr)) return hr;
    hr = D3DDevice_SetRenderState(D3DRS_PSTEXTUREMODES,
                                  pPSDef->PSTextureModes);
    if (FAILED(hr)) return hr;
    hr = D3DDevice_SetRenderState(D3DRS_PSDOTMAPPING, pPSDef->PSDotMapping);
    if (FAILED(hr)) return hr;
    return D3DDevice_SetRenderState(D3DRS_PSINPUTTEXTURE,
                                    pPSDef->PSInputTexture);
}

D3DAPI HRESULT Direct3DDevice8_SetPixelShaderProgram(
    LPDIRECT3DDEVICE8 pThis, CONST D3DPIXELSHADERDEF *pPSDef)
{
    assert(pThis == &g_pDevice->iface);
    return D3DDevice_SetPixelShaderProgram(pPSDef);
}

HRESULT D3DDevice_ClearRect(CONST D3DRECT* pRect, DWORD Flags) {
    assert((Flags & 0x0C) == 0);
    HRESULT hr = D3DDevice_PushCmd(NV097_SET_CLEAR_RECT_HORIZONTAL,
        MASK(NV097_SET_CLEAR_RECT_HORIZONTAL_X2, pRect->x2) |
        MASK(NV097_SET_CLEAR_RECT_HORIZONTAL_X1, pRect->x1));

    if (FAILED(hr)) return hr;
    hr = D3DDevice_PushCmd(NV097_SET_CLEAR_RECT_VERTICAL,
        MASK(NV097_SET_CLEAR_RECT_VERTICAL_Y2, pRect->y2) |
        MASK(NV097_SET_CLEAR_RECT_VERTICAL_Y1, pRect->y1));
    if (FAILED(hr)) return hr;
    return D3DDevice_PushCmd(NV097_CLEAR_SURFACE, Flags & 0xF3);
}

HRESULT D3DDevice_Clear(DWORD Count, CONST D3DRECT* pRects, DWORD Flags,
                        D3DCOLOR Color, float Z, DWORD Stencil)
{
    HRESULT hr = D3D_OK;
    if (Stencil != g_pDevice->Stencil) {
        g_pDevice->Stencil =
            MASK(NV097_SET_ZSTENCIL_CLEAR_VALUE_DEPTH,
                 (DWORD)(Z * D3DZ_MAX_D24S8)) |
            MASK(NV097_SET_ZSTENCIL_CLEAR_VALUE_STENCIL, Stencil);
        g_pDevice->ZStencilClearDirty = TRUE;

    }

    if (Color != g_pDevice->ClearColor) {
        g_pDevice->ClearColor = Color;
        g_pDevice->ColorClearDirty = TRUE;
    }

    if ((Flags & D3DCLEAR_ZSTENCIL) && g_pDevice->ZStencilClearDirty) {
        hr = D3DDevice_PushCmd(NV097_SET_ZSTENCIL_CLEAR_VALUE,
            MASK(NV097_SET_ZSTENCIL_CLEAR_VALUE_DEPTH,
                 (DWORD)(Z * D3DZ_MAX_D24S8)) |
            MASK(NV097_SET_ZSTENCIL_CLEAR_VALUE_STENCIL, Stencil));
        if (FAILED(hr)) return hr;
    }

    if ((Flags & D3DCLEAR_TARGET) && g_pDevice->ColorClearDirty) {
        hr = D3DDevice_PushCmd(NV097_SET_COLOR_CLEAR_VALUE, Color);
        if (FAILED(hr)) return hr;
    }

    if (pRects == NULL) {
#if NXDK_DEBUG
        if (Count > 0)
            return D3DERR_INVALIDCALL;
#endif // NXDK_DEBUG
        D3DSURFACE_DESC desc;
        hr = D3DSurface_GetDesc(
            (LPDIRECT3DSURFACE8)g_pDevice->pSurfaces[g_pDevice->CurrentSurface],
             &desc);
        D3DRECT rect;
        rect.x1 = 0;
        rect.y1 = 0;
        rect.x2 = desc.Width;
        rect.y2 = desc.Height;
        return D3DDevice_ClearRect(&rect, Flags);
    }

    for (int i = 0; i < Count; i++) {
        hr = D3DDevice_ClearRect(&pRects[i], Flags);
        if (FAILED(hr)) return hr;
    }
    return hr;
}

D3DAPI HRESULT Direct3DDevice8_Clear(LPDIRECT3DDEVICE8 pThis, DWORD Count,
                                      CONST D3DRECT* pRects, DWORD Flags,
                                      D3DCOLOR Color, float Z, DWORD Stencil)
{
    assert(pThis == &g_pDevice->iface);
    return D3DDevice_Clear(Count, pRects, Flags, Color, Z, Stencil);
}

HRESULT D3DDevice_SetTile(DWORD Index, CONST D3DTILE* pTile) {
#if NXDK_DEBUG
    if (Index >= D3DTILE_INDEX_MAX)
        return D3DERR_INVALIDCALL;

    if (pTile == NULL)
        return D3DERR_INVALIDCALL;

    if (pTile->Flags & ~(D3DTILE_FLAGS_ZCOMPRESS |
                         D3DTILE_FLAGS_Z32BITS   |
                         D3DTILE_FLAGS_ZBUFFER))
    {
        return D3DERR_INVALIDCALL;
    }

    if (pTile->pMemory == NULL)
        return D3DERR_INVALIDCALL;

    if (pTile->Pitch < D3DTILE_PITCH_0200 ||
        pTile->Pitch > D3DTILE_PITCH_E000)
    {
        return D3DERR_INVALIDCALL;
    }

    if (pTile->Pitch <= D3DTILE_PITCH_0800) {
        if (pTile->Pitch & 0xF0FF)
            return D3DERR_INVALIDCALL;
    } else if (pTile->Pitch <= D3DTILE_PITCH_1000) {
        if (pTile->Pitch & 0xE1FF)
            return D3DERR_INVALIDCALL;
    } else if (pTile->Pitch <= D3DTILE_PITCH_2000) {
        if (pTile->Pitch & 0xD3FF)
            return D3DERR_INVALIDCALL;
    } else if (pTile->Pitch <= D3DTILE_PITCH_4000) {
        if (pTile->Pitch & 0xC7FF)
            return D3DERR_INVALIDCALL;
    } else if (pTile->Pitch <= D3DTILE_PITCH_E000) {
        if (pTile->Pitch & 0x0FFF)
            return D3DERR_INVALIDCALL;
    }
#endif // NXDK_DEBUG
    pb_assign_tile(Index, (DWORD)pTile->pMemory, pTile->Size, pTile->Pitch,
                   pTile->ZStartTag, pTile->ZOffset, pTile->Flags);
    return D3D_OK;
}

D3DAPI HRESULT Direct3DDevice8_SetTile(LPDIRECT3DDEVICE8 pThis, DWORD Index,
                                        CONST D3DTILE* pTile)
{
    assert(pThis == &g_pDevice->iface);
    return D3DDevice_SetTile(Index, pTile);
}

HRESULT D3DDevice_SetTexture(DWORD Stage, D3DBaseTexture* pTexture) {
    if (pTexture == NULL) {
        g_pDevice->pTexture[Stage]->inner.resource.Fence =
            D3DDevice_GetCurrentFence();

        D3DBaseTexture_Release(
            (LPDIRECT3DBASETEXTURE8)g_pDevice->pTexture[Stage]);

        g_pDevice->pTexture[Stage] = NULL;
        g_pDevice->TextureStageState[Stage].Control0 &=
            ~NV097_SET_TEXTURE_CONTROL0_ENABLE;
        return D3DDevice_PushCmd(NV097_SET_TEXTURE_CONTROL0(Stage),
                                 g_pDevice->TextureStageState[Stage].Control0);
    }

    D3DBaseTexture_AddRef((LPDIRECT3DBASETEXTURE8)g_pDevice->pTexture[Stage]);
    D3DBaseTexture* pOldBase = g_pDevice->pTexture[Stage];
    g_pDevice->pTexture[Stage] = pTexture;

    if (pOldBase != NULL)
        pOldBase->inner.resource.Fence = D3DDevice_GetCurrentFence();

    D3DRESOURCETYPE Type = D3DBaseTexture_GetType(
        (LPDIRECT3DBASETEXTURE8)pTexture);
    if (Type != D3DRTYPE_TEXTURE && Type != D3DRTYPE_CUBETEXTURE) {
        if (Type == D3DRTYPE_VOLUMETEXTURE)
            return E_NOTIMPL;
        else
            return D3DERR_INVALIDCALL;
    }

    D3DResourceInner* pResource = &pTexture->inner.resource;
    HRESULT hr = D3DDevice_PushCmd(NV097_SET_TEXTURE_OFFSET(Stage),
                           (DWORD)pResource->pContiguousMemory);
    if (FAILED(hr)) return hr;

    LPDIRECT3DSURFACE8 pSurface = NULL;
    if (Type == D3DRTYPE_TEXTURE) {
        HRESULT hr = D3DTexture_GetSurfaceLevel(
            (LPDIRECT3DTEXTURE8)pTexture, 0, &pSurface);
#if NXDK_DEBUG
            if (FAILED(hr)) return hr;
#endif // NXDK_DEBUG
    }

    D3DSURFACE_DESC desc;
    if (Type == D3DRTYPE_TEXTURE) {
        hr = D3DSurface_GetDesc((LPDIRECT3DSURFACE8)pSurface, &desc);
#if NXDK_DEBUG
        if (FAILED(hr)) return hr;
#endif // NXDK_DEBUG
    } else {
        hr = D3DCubeTexture_GetLevelDesc(
            (LPDIRECT3DCUBETEXTURE8)pTexture, 0, &desc);
#if NXDK_DEBUG
        if (FAILED(hr)) return hr;
#endif // NXDK_DEBUG
    }

    D3DSURFACE_DESC oldDesc;
    LPDIRECT3DSURFACE8 pOldSurface = NULL;
    D3DRESOURCETYPE oldType = D3DBaseTexture_GetType(
        (LPDIRECT3DBASETEXTURE8)pOldBase);
    if (pOldBase != NULL && oldType == D3DRTYPE_TEXTURE) {
        hr = D3DTexture_GetSurfaceLevel(
            (LPDIRECT3DTEXTURE8)pOldBase, 0, &pOldSurface);
#if NXDK_DEBUG
        if (FAILED(hr)) return hr;
#endif // NXDK_DEBUG
    }

    if (pOldSurface != NULL) {
        if (oldType == D3DRTYPE_TEXTURE) {
            hr = D3DSurface_GetDesc(pOldSurface, &oldDesc);
        } else {
            hr = D3DCubeTexture_GetLevelDesc(
                (LPDIRECT3DCUBETEXTURE8)pOldBase, 0, &oldDesc);
        }
#if NXDK_DEBUG
        if (FAILED(hr)) return hr;
#endif // NXDK_DEBUG
    } else {
        memzrop(&oldDesc);
    }

    if (desc.Width != oldDesc.Width || desc.Height != oldDesc.Height) {
        hr = D3DDevice_PushCmd(NV097_SET_TEXTURE_IMAGE_RECT(Stage),
            MASK(NV097_SET_TEXTURE_IMAGE_RECT_WIDTH,  desc.Width) |
            MASK(NV097_SET_TEXTURE_IMAGE_RECT_HEIGHT, desc.Height));
        if (FAILED(hr)) return hr;
    }

    if ((g_pDevice->TextureStageState[Stage].Control0 &
        NV097_SET_TEXTURE_CONTROL0_ENABLE) == 0)
    {
        g_pDevice->TextureStageState[Stage].Control0 |=
            NV097_SET_TEXTURE_CONTROL0_ENABLE;
        hr = D3DDevice_PushCmd(NV097_SET_TEXTURE_CONTROL0(Stage),
                               g_pDevice->TextureStageState[Stage].Control0);
        if (FAILED(hr)) return hr;
    }

    UINT Pitch = desc.Width * D3D_FormatBytesPerPixel(desc.Format);
    UINT OldPitch = oldDesc.Width * D3D_FormatBytesPerPixel(oldDesc.Format);
    if (Pitch != OldPitch) {
        hr = D3DDevice_PushCmd(NV097_SET_TEXTURE_CONTROL1(Stage),
            MASK(NV097_SET_TEXTURE_CONTROL1_IMAGE_PITCH, Pitch));
        if (FAILED(hr)) return hr;
    }

    DWORD dwLevelCount = D3DBaseTexture_GetLevelCount(
        (LPDIRECT3DBASETEXTURE8)pTexture);
    DWORD dwOldLevelCount = 0;
    if (pOldBase != NULL) {
        dwOldLevelCount = D3DBaseTexture_GetLevelCount(
            (LPDIRECT3DBASETEXTURE8)pOldBase);
    }

    if (desc.Format  != oldDesc.Format  ||
        dwLevelCount != dwOldLevelCount ||
        Type != oldType)
    {
        DWORD CubemapEnable = 0;
        if (Type == D3DRTYPE_CUBETEXTURE)
            CubemapEnable = NV097_SET_TEXTURE_FORMAT_CUBEMAP_ENABLE;
        hr = D3DDevice_PushCmd(NV097_SET_TEXTURE_FORMAT(Stage),
            MASK(NV097_SET_TEXTURE_FORMAT_MIPMAP_LEVELS, dwLevelCount) |
            MASK(NV097_SET_TEXTURE_FORMAT_COLOR, desc.Format) |
            MASK(NV097_SET_TEXTURE_FORMAT_DIMENSIONALITY, 2) |
            MASK(NV097_SET_TEXTURE_FORMAT_CONTEXT_DMA, 2) |
            desc.Usage | CubemapEnable
        );
        if (FAILED(hr)) return hr;
    }

    return hr;
}

D3DAPI HRESULT Direct3DDevice8_SetTexture(LPDIRECT3DDEVICE8 pThis,
                                           DWORD Stage,
                                           LPDIRECT3DBASETEXTURE8 pTexture)
{
    assert(pThis == &g_pDevice->iface);
    return D3DDevice_SetTexture(Stage, (D3DBaseTexture*)pTexture);
}

HRESULT D3DDevice_InsertCallback(D3DCALLBACKTYPE Type,
                                 D3DCALLBACK pCallback,
                                 DWORD Context)
{
    // TODO: avoid pipeline flush if Type == D3DCALLBACKTYPE_READ
    return D3DDevice_InsertInterrupt2(PB_CALLBACK, (DWORD)pCallback, Context);
}

D3DAPI HRESULT Direct3DDevice8_InsertCallback(LPDIRECT3DDEVICE8 pThis,
                                               D3DCALLBACKTYPE Type,
                                               D3DCALLBACK pCallback,
                                               DWORD Context)
{
    assert(pThis == &g_pDevice->iface);
    return D3DDevice_InsertCallback(Type, pCallback, Context);
}

DWORD D3DDevice_InsertFence() {
    if (g_pDevice->dwNextFence == ~0) {
        assert(false && "Fence counter overflow");
        return 0;
    }

    DWORD Fence = g_pDevice->dwNextFence++;
    if (FAILED(D3DDevice_InsertInterrupt(PB_FENCE, Fence)))
        return 0;

    return Fence;
}

D3DAPI DWORD Direct3DDevice8_InsertFence(LPDIRECT3DDEVICE8 pThis) {
    assert(pThis == &g_pDevice->iface);
    return D3DDevice_InsertFence();
}

BOOL D3DDevice_IsFencePending(DWORD Fence) {
    if (Fence == 0)
        return FALSE;

    if (Fence >= g_pDevice->dwLastFenceCompleted)
        return TRUE;

    return FALSE;
}

D3DAPI BOOL Direct3DDevice8_IsFencePending(LPDIRECT3DDEVICE8 pThis,
                                           DWORD Fence)
{
    assert(pThis == &g_pDevice->iface);
    return D3DDevice_IsFencePending(Fence);
}

VOID D3DDevice_BlockOnFence(DWORD Fence) {
    while (D3DDevice_IsFencePending(Fence) == TRUE) {
        if (!NT_SUCCESS(NtWaitForSingleObject(g_pDevice->hFenceEvent,
                                              FALSE, NULL)))
        {
            assert(false);
        }
    }
}

D3DAPI VOID Direct3DDevice8_BlockOnFence(LPDIRECT3DDEVICE8 pThis, DWORD Fence)
{
    assert(pThis == &g_pDevice->iface);
    D3DDevice_BlockOnFence(Fence);
}

BOOL D3DDevice_SetFence(DWORD Fence) {
    if (Fence <= g_pDevice->dwLastFenceCompleted) {
        assert(false);
        return FALSE;
    }

    if (!NT_SUCCESS(NtSetEvent(g_pDevice->hFenceEvent, NULL))) {
        assert(false);
        return FALSE;
    }

    g_pDevice->dwLastFenceCompleted = Fence;
    return TRUE;
}

DWORD D3DDevice_GetCurrentFence(void) {
    return g_pDevice->dwLastFenceCompleted + 1;
}

DWORD D3DDevice_GetLastCompletedFence(void) {
    return g_pDevice->dwLastFenceCompleted;
}

HRESULT D3DDevice_SetVertexShaderConstant(INT Register, CONST VOID* pConstantData,
                                          DWORD ConstantCount)
{
#if NXDK_DEBUG
    if (Register < 0 ||
        Register + ConstantCount > 96 ||
        ConstantCount == 0 ||
        pConstantData == NULL)
    {
        return D3DERR_INVALIDCALL;
    }
#endif // NXDK_DEBUG
    HRESULT hr = D3DDevice_PushCmd(NV097_SET_TRANSFORM_CONSTANT_LOAD,
                                   Register + 96);
    if (FAILED(hr)) return hr;

    for (int i = 0; i < ConstantCount; i++) {
        hr = D3DDevice_PushCmdA(NV097_SET_TRANSFORM_CONSTANT,
                                pConstantData,
                                4,
                                TRUE);
        if (FAILED(hr)) return hr;
    }
    return hr;
}

HRESULT Direct3DDevice8_SetVertexShaderConstant(
    LPDIRECT3DDEVICE8 pThis, INT Register, CONST VOID* pConstantData,
    DWORD ConstantCount)
{
    assert(pThis == &g_pDevice->iface);
    return D3DDevice_SetVertexShaderConstant(Register, pConstantData,
                                             ConstantCount);
}

HRESULT D3DDevice_SetPixelShaderConstant(DWORD Register,
                                         CONST VOID* pConstantData,
                                         DWORD ConstantCount)
{
    // TODO: Optimize this function by setting the hardware registers directly
    // without going through D3DDevice_SetRenderState for each register.
#if NXDK_DEBUG
    if (Register + ConstantCount > 16 ||
        ConstantCount == 0 ||
        pConstantData == NULL)
    {
        return D3DERR_INVALIDCALL;
    }
#endif // NXDK_DEBUG

    // Zero out all registers before Register.
    int i = 0;
    for (; i < Register; i++) {
        HRESULT hr = D3DDevice_SetRenderState(D3DRS_PSCONSTANT0_0 + i, 0);
        if (FAILED(hr)) return hr;
    }
    for (; i < ConstantCount; i++) {
        DWORD Data = ((CONST DWORD*)pConstantData)[i];
        HRESULT hr = D3DDevice_SetRenderState(D3DRS_PSCONSTANT0_0 + i,
                                              Data);
        if (FAILED(hr)) return hr;
    }
    // Also zero out all registers after (Register + ConstantCount).
    for (; i < 16; i++) {
        HRESULT hr = D3DDevice_SetRenderState(D3DRS_PSCONSTANT0_0 + i, 0);
        if (FAILED(hr)) return hr;
    }
    return D3D_OK;
}

D3DAPI HRESULT Direct3DDevice8_SetPixelShaderConstant(
    LPDIRECT3DDEVICE8 pThis, DWORD Register, CONST VOID* pConstantData,
    DWORD ConstantCount)
{
    assert(pThis == &g_pDevice->iface);
    return D3DDevice_SetPixelShaderConstant(Register, pConstantData,
                                            ConstantCount);
}

HRESULT D3DDevice_GetPushBuffer(D3DPushBuffer** ppPushBuffer) {
    *ppPushBuffer = g_pDevice->pCurrentPB;
    return D3D_OK;
}

D3DAPI HRESULT Direct3DDevice8_GetPushBuffer(
    LPDIRECT3DDEVICE8 pThis, LPDIRECT3DPUSHBUFFER8* ppPushBuffer)
{
    assert(pThis == &g_pDevice->iface);
    return D3DDevice_GetPushBuffer((D3DPushBuffer**)ppPushBuffer);
}
