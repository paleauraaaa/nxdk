#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include <hal/debug.h>
#include <hal/video.h>
#include <pbkit/pbkit.h>
#include <xboxkrnl/xboxkrnl.h>

#include "d3d8.h"
#include "d3d8_private.h"
#include "d3d8_resource.h"
#include "d3d8_device.h"

PVOID D3D_AllocContiguousMemory(DWORD Size, DWORD Alignment) {
    return MmAllocateContiguousMemoryEx(Size, 0, 0x03FFB000, Alignment,
                                        PAGE_READWRITE | PAGE_WRITECOMBINE);
}

VOID D3D_FreeContiguousMemory(PVOID Base) {
    MmFreeContiguousMemory(Base);
}

int D3D_FormatBPP(D3DFORMAT fmt) {
    switch (fmt) {
    case D3DFMT_A8R8G8B8:
    case D3DFMT_X8R8G8B8:
    case D3DFMT_LIN_A8R8G8B8:
    case D3DFMT_D24S8:
        return 32;
    case D3DFMT_A1R5G5B5:
    case D3DFMT_X1R5G5B5:
    case D3DFMT_LIN_X1R5G5B5:
        return 15;
    case D3DFMT_R5G6B5:
    case D3DFMT_LIN_R5G6B5:
        return 16;
    default:
        assert(false);
        return 0; 
    }
}

int D3D_FormatBytesPerPixel(D3DFORMAT fmt) {
    return (D3D_FormatBPP(fmt)+7)/8;
}

void D3D_DebugPrintf(const char* fmt, ...) {
    char buf[4096];
    va_list va;
    va_start(va, fmt);
    vsnprintf(buf, sizeof(buf), fmt, va);
    OutputDebugStringA(buf);
    debugPrint("%s", buf);
    va_end(va);
}

static Direct3D* g_pD3D = NULL;

D3DAPI ULONG Direct3D8_AddRef(LPDIRECT3D8 pThis) {
    assert(pThis == &g_pD3D->iface);
    return 1;
}

D3DAPI ULONG Direct3D8_Release(LPDIRECT3D8 pThis) {
    assert(pThis == &g_pD3D->iface);
    return 1;
}

D3DAPI UINT Direct3D8_GetAdapterCount(LPDIRECT3D8 pThis) {
    assert(pThis == &g_pD3D->iface);
    return 1;
}

D3DAPI HMONITOR Direct3D8_GetAdapterMonitor(LPDIRECT3D8 pThis, UINT Adapter) {
    assert(pThis == &g_pD3D->iface);
    return NULL;
}

static BOOL D3D_IsFormatRenderTarget(D3DFORMAT fmt) {
    switch (fmt) {
    case D3DFMT_A8R8G8B8:
    case D3DFMT_X8R8G8B8:
    case D3DFMT_A1R5G5B5:
    case D3DFMT_X1R5G5B5:
    case D3DFMT_R5G6B5:
        return TRUE;
    default:
        return FALSE;
    }
}

HRESULT Direct3D_CheckDeviceType(D3DFORMAT DisplayFormat,
                                 D3DFORMAT BackBufferFormat)
{
    if (DisplayFormat != BackBufferFormat)
        return D3DERR_INVALIDCALL;

    if (D3D_IsFormatRenderTarget(DisplayFormat) == FALSE)
        return D3DERR_NOTAVAILABLE;

    return D3D_OK;
}

D3DAPI HRESULT Direct3D8_CheckDeviceType(LPDIRECT3D8 pThis, UINT Adapter, 
                                   D3DDEVTYPE CheckType, 
                                   D3DFORMAT DisplayFormat,
                                   D3DFORMAT BackBufferFormat, BOOL Windowed)
{
    assert(pThis == &g_pD3D->iface);
#if NXDK_DEBUG
    if (Adapter != D3DADAPTER_DEFAULT) {
        assert(false);
        return D3DERR_INVALIDCALL;
    }

    if (CheckType != D3DDEVTYPE_HAL) {
        assert(false);
        return D3DERR_INVALIDDEVICE;
    }

    if (Windowed != FALSE) {
        assert(false);
        return D3DERR_INVALIDCALL;
    }
#endif // NXDK_DEBUG
    return Direct3D_CheckDeviceType(DisplayFormat, BackBufferFormat);
}

UINT Direct3D_GetAdapterModeCount(void) {
    return g_pD3D->DisplayModeCount;
}

D3DAPI UINT Direct3D8_GetAdapterModeCount(LPDIRECT3D8 pThis, UINT Adapter) {
    assert(pThis == &g_pD3D->iface);
#if NXDK_DEBUG
    if (Adapter != D3DADAPTER_DEFAULT) {
        assert(false);
        return 0;
    }
#endif // NXDK_DEBUG
    return Direct3D_GetAdapterModeCount();
}

HRESULT Direct3D_EnumAdapterModes(UINT Mode, D3DDISPLAYMODE* pMode)  {
#if NXDK_DEBUG
    if (Mode >= g_pD3D->DisplayModeCount) {
        assert(false);
        return D3DERR_INVALIDCALL;
    }
#endif // NXDK_DEBUG
    memcpyp(pMode, &g_pD3D->DisplayModes[Mode]);
    return D3D_OK;
}

D3DAPI HRESULT Direct3D8_EnumAdapterModes(LPDIRECT3D8 pThis, UINT Adapter, 
                                          UINT Mode, D3DDISPLAYMODE* pMode) 
{
    assert(pThis == &g_pD3D->iface);
#if NXDK_DEBUG
    if (Adapter != D3DADAPTER_DEFAULT) {
        assert(false);
        return 0;
    }
#endif // NXDK_DEBUG
    return Direct3D_EnumAdapterModes(Mode, pMode);
}

HRESULT Direct3D_GetAdapterDisplayMode(D3DDISPLAYMODE* pMode) 
{
    memcpyp(pMode, &g_pD3D->DisplayModes[g_pD3D->CurrentDisplayMode]);
    return D3D_OK;
}

D3DAPI HRESULT Direct3D8_GetAdapterDisplayMode(LPDIRECT3D8 pThis, UINT Adapter, 
                                               D3DDISPLAYMODE* pMode)
{
    assert(pThis == &g_pD3D->iface);
#if NXDK_DEBUG
    if (Adapter != D3DADAPTER_DEFAULT) {
        assert(false);
        return 0;
    }
#endif // NXDK_DEBUG
    return Direct3D_GetAdapterDisplayMode(pMode);
}

HRESULT Direct3D_CheckDepthStencilMatch(D3DFORMAT DepthStencilFormat)
{
    return DepthStencilFormat == D3DFMT_D24S8 ? D3D_OK : D3DERR_NOTAVAILABLE;
}

D3DAPI HRESULT Direct3D8_CheckDepthStencilMatch(LPDIRECT3D8 pThis, UINT Adapter, 
                                          D3DDEVTYPE DeviceType, 
                                          D3DFORMAT AdapterFormat, 
                                          D3DFORMAT RenderTargetFormat, 
                                          D3DFORMAT DepthStencilFormat)
{
    assert(pThis == &g_pD3D->iface);
#if NXDK_DEBUG
    if (Adapter != D3DADAPTER_DEFAULT) {
        assert(false);
        return D3DERR_INVALIDCALL;
    }

    if (DeviceType != D3DDEVTYPE_HAL) {
        assert(false);
        return D3DERR_INVALIDDEVICE;
    }
#endif // NXDK_DEBUG
    return Direct3D_CheckDepthStencilMatch(DepthStencilFormat);
}

HRESULT Direct3D_GetDeviceCaps(UINT Adapter, D3DDEVTYPE DeviceType, 
                               D3DCAPS8* pCaps)
{
#if NXDK_DEBUG
    if (Adapter != D3DADAPTER_DEFAULT) {
        assert(false);
        return D3DERR_INVALIDCALL;
    }

    if (DeviceType != D3DDEVTYPE_HAL) {
        assert(false);
        return D3DERR_INVALIDDEVICE;
    }

    if (pCaps == NULL) {
        assert(false);
        return D3DERR_INVALIDCALL;
    }
#endif // NXDK_DEBUG
    memzrop(pCaps);
    pCaps->DeviceType     = DeviceType;
    pCaps->AdapterOrdinal = D3DADAPTER_DEFAULT;

    return E_NOTIMPL;
}

D3DAPI HRESULT Direct3D8_GetDeviceCaps(LPDIRECT3D8 pThis, UINT Adapter, D3DDEVTYPE DeviceType, 
                                 D3DCAPS8* pCaps)
{
    assert(pThis == &g_pD3D->iface);
    return Direct3D_GetDeviceCaps(Adapter, DeviceType, pCaps);
}

HRESULT Direct3D_SetPushBufferSize(DWORD PushBufferSize, DWORD KickOffSize) {
#if NXDK_DEBUG
    if (PushBufferSize < 64*1024)
        return D3DERR_INVALIDCALL;

    if ((PushBufferSize % KickOffSize != 0) || 
        (PushBufferSize / KickOffSize < 4))
    {
        return D3DERR_INVALIDCALL;
    }
#endif // NXDK_DEBUG
    g_pD3D->KickOffSize       = KickOffSize;
    g_pD3D->PushBufferSize    = PushBufferSize;
    return D3D_OK;
}

D3DAPI HRESULT Direct3D8_SetPushBufferSize(LPDIRECT3D8 pThis, 
                                     DWORD PushBufferSize, 
                                     DWORD KickOffSize) 
{
    assert(pThis == &g_pD3D->iface);
    return Direct3D_SetPushBufferSize(PushBufferSize, KickOffSize);
}

BOOL Direct3D_IsDisplayModeValid(D3DDISPLAYMODE* pMode, int* pIndex) {
#if NXDK_DEBUG
    if (pMode == NULL)
        return FALSE;
#endif // NXDK_DEBUG
    if (pMode->RefreshRate == 0) {
        // Any refresh rate is acceptable, use first matching mode.
        for (int i = 0; i < g_pD3D->DisplayModeCount; i++) {
            if (pMode->Width  == g_pD3D->DisplayModes[i].Width  &&
                pMode->Height == g_pD3D->DisplayModes[i].Height &&
                pMode->Format == g_pD3D->DisplayModes[i].Format &&
                pMode->Flags  == g_pD3D->DisplayModes[i].Flags) 
            {
                if (pIndex)
                    *pIndex = i;
                return TRUE;
            }
        }
        return FALSE;
    }

    // If refresh rate is specified, memcmp works just fine.
    for (int i = 0; i < g_pD3D->DisplayModeCount; i++) {
        if (memcmp(pMode, &g_pD3D->DisplayModes[i], sizeof(*pMode)) == 0) {
            if (pIndex)
                *pIndex = i;
            return TRUE;
        }
    }
    return FALSE;
}

#if NXDK_DEBUG
HRESULT D3D_PresentParameters_Validate(
    D3DPRESENT_PARAMETERS* pPresentationParameters) 
{
    D3D_ASSERT_IF(D3DERR_INVALIDCALL, (pPresentationParameters == NULL));
    D3D_ASSERT_IF(D3DERR_INVALIDCALL, 
        (pPresentationParameters->Windowed != FALSE));

    D3D_ASSERT_IF(D3DERR_INVALIDCALL, 
        (pPresentationParameters->BackBufferCount > 2))

    D3D_ASSERT_IF(D3DERR_INVALIDCALL, pPresentationParameters->Flags & 
        (D3DPRESENTFLAG_FIELD | D3DPRESENTFLAG_10X11PIXELASPECTRATIO |
         D3DPRESENTFLAG_EMULATE_REFRESH_RATE));

    D3D_ASSERT_IF(D3DERR_INVALIDCALL, 
        ((pPresentationParameters->Flags & D3DPRESENTFLAG_PROGRESSIVE) && 
         (pPresentationParameters->Flags & D3DPRESENTFLAG_INTERLACED)));

    // D3DPRESENT_INTERVAL_FOUR is invalid on Xbox, 
    // and any higher values aren't valid D3DPRESENT values.
    D3D_ASSERT_IF(D3DERR_INVALIDCALL, 
        (pPresentationParameters->FullScreen_PresentationInterval > 
         D3DPRESENT_INTERVAL_THREE_OR_IMMEDIATE));
    

    if (pPresentationParameters->EnableAutoDepthStencil == TRUE) {
        HRESULT hr = Direct3D_CheckDepthStencilMatch(
            pPresentationParameters->AutoDepthStencilFormat);
        D3D_ASSERT_IF(hr, FAILED(hr));

        D3D_ASSERT_IF(D3DERR_INVALIDCALL, 
            (pPresentationParameters->DepthStencilSurface != NULL));
    }

    DWORD Width = pPresentationParameters->BackBufferWidth;
    if (Width == 0) Width = 640;
    DWORD Height = pPresentationParameters->BackBufferHeight;
    if (Height == 0) Height = 480;
    D3DFORMAT Format = pPresentationParameters->BackBufferFormat;
    if (Format == D3DFMT_UNKNOWN) Format = D3DFMT_A8R8G8B8;
    // Ensure the BufferSurfaces are all either NULL, or non-NULL 
    // *and* contiguous.
    if (pPresentationParameters->BufferSurfaces[0] != NULL) {
         D3D_ASSERT_IF(D3DERR_INVALIDCALL, 
            (pPresentationParameters->BufferSurfaces[1] == NULL));

        D3DSurface* pBufferSurface0 = 
            (D3DSurface*)pPresentationParameters->BufferSurfaces[0];
        D3D_ASSERT_IF(D3DERR_INVALIDCALL, (pBufferSurface0->desc.Size == 0));
        D3D_ASSERT_IF(D3DERR_INVALIDCALL, 
            (pBufferSurface0->desc.Width  != Width ||
             pBufferSurface0->desc.Height != Height));
        D3D_ASSERT_IF(D3DERR_INVALIDCALL, (pBufferSurface0->desc.Format != Format))

        D3DSurface* pBufferSurface1 = 
            (D3DSurface*)pPresentationParameters->BufferSurfaces[1];
        D3D_ASSERT_IF(D3DERR_INVALIDCALL, (pBufferSurface1->desc.Size == 0))
        D3D_ASSERT_IF(D3DERR_INVALIDCALL, 
            (pBufferSurface1->desc.Width  != Width ||
             pBufferSurface1->desc.Height != Height));
        D3D_ASSERT_IF(D3DERR_INVALIDCALL, (pBufferSurface1->desc.Format != Format));
        D3D_ASSERT_IF(D3DERR_INVALIDCALL, 
            (((DWORD)pBufferSurface0->resource.pContiguousMemory 
                + pBufferSurface0->desc.Size 
            != (DWORD)pBufferSurface1->resource.pContiguousMemory)));


        if (pPresentationParameters->BackBufferCount == 2) {
            D3D_ASSERT_IF(D3DERR_INVALIDCALL, 
                (pPresentationParameters->BufferSurfaces[2] == NULL));
            
            D3DSurface* pBufferSurface2 = 
                (D3DSurface*)pPresentationParameters->BufferSurfaces[2];
            D3D_ASSERT_IF(D3DERR_INVALIDCALL, 
                (pBufferSurface2->desc.Size == 0));
            D3D_ASSERT_IF(D3DERR_INVALIDCALL, 
                (pBufferSurface2->desc.Width  != Width ||
                 pBufferSurface2->desc.Height != Height));
            D3D_ASSERT_IF(D3DERR_INVALIDCALL, 
                (pBufferSurface2->desc.Format != Format));
            D3D_ASSERT_IF(D3DERR_INVALIDCALL, 
                (((DWORD)pBufferSurface1->resource.pContiguousMemory 
                    + pBufferSurface1->desc.Size 
                != (DWORD)pBufferSurface2->resource.pContiguousMemory)));
        }
    } 
    else {
        D3D_ASSERT_IF(D3DERR_INVALIDCALL, 
            (pPresentationParameters->BufferSurfaces[1] != NULL))

        D3D_ASSERT_IF(D3DERR_INVALIDCALL, 
            (pPresentationParameters->BackBufferCount == 2 &&
             pPresentationParameters->BufferSurfaces[2] != NULL));
    }

    if (pPresentationParameters->DepthStencilSurface != NULL) {
        D3DSurface* pDSSurf = 
            (D3DSurface*)pPresentationParameters->DepthStencilSurface;
        D3D_ASSERT_IF(D3DERR_INVALIDCALL, (pDSSurf->desc.Width  < Width ||
            pDSSurf->desc.Height < Height));

        HRESULT hr = Direct3D_CheckDepthStencilMatch(
            pDSSurf->desc.Format);
        D3D_ASSERT_IF(hr, FAILED(hr));
    }

    return D3D_OK;
}

HRESULT Direct3D_CreateDevice_Validate(
    DWORD BehaviorFlags,
    D3DPRESENT_PARAMETERS* pPresentationParameters)
{
    D3D_ASSERT_IF(D3DERR_INVALIDCALL, (BehaviorFlags !=
        (D3DCREATE_HARDWARE_VERTEXPROCESSING | D3DCREATE_PUREDEVICE)));

    return D3D_PresentParameters_Validate(pPresentationParameters);
}
#endif // NXDK_DEBUG

DWORD D3D_FormatToColorFormat(D3DFORMAT Format) {
    switch (Format) {
    case D3DFMT_LIN_A8R8G8B8:
        return NV097_SET_SURFACE_FORMAT_COLOR_LE_A8R8G8B8;
    case D3DFMT_LIN_R5G6B5:
        return NV097_SET_SURFACE_FORMAT_COLOR_LE_R5G6B5;
    case D3DFMT_LIN_X1R5G5B5:
        return NV097_SET_SURFACE_FORMAT_COLOR_LE_X1R5G5B5_O1R5G5B5;
    default:
        D3D_ASSERT_IF(0, true);
    }
}

HRESULT Direct3D_ResetDevice(
    D3DPRESENT_PARAMETERS* pPresentationParameters) 
{
    // TODO: This function's current implementation doesn't free the old
    // framebuffers if they were user-supplied, nor account for switching
    // between user-supplied and default framebuffers if the previous
    // CreateDevice call's pPresentationParameters and this function's 
    // pPresentationParameters differ in that respect, nor account
    // for resizing the framebuffers in either case.
    //
    // The easy solution here would be to call pb_kill and then execute
    // CreateDevice again, but beyond being inefficient, this will result in
    // a new call to pb_init, and pb_init will clobber the existing render 
    // state, which resetting the device should not do.
    //
    // tl;dr the current implementation is unsafe to call in any circumstance
    // except when the presentation parameters are identical.

    int bpp = D3D_FormatBPP(pPresentationParameters->BackBufferFormat);
#if NXDK_DEBUG
    if (bpp == 0)
        return D3DERR_INVALIDCALL;
#endif // NXDK_DEBUG
    
    D3DDISPLAYMODE mode;
    mode.Width          = pPresentationParameters->BackBufferWidth;
    mode.Height         = pPresentationParameters->BackBufferHeight;
    mode.RefreshRate    = pPresentationParameters->FullScreen_RefreshRateInHz;
    mode.Format         = pPresentationParameters->BackBufferFormat;
    mode.Flags          = pPresentationParameters->Flags;
    int i = 0;
    if (Direct3D_IsDisplayModeValid(&mode, &i) == TRUE) {
        int refresh = pPresentationParameters->FullScreen_RefreshRateInHz;
        BOOL b = XVideoSetMode(
            pPresentationParameters->BackBufferWidth,
            pPresentationParameters->BackBufferHeight, 
            bpp, refresh);
#if NXDK_DEBUG
        if (b  == FALSE)
            return D3DERR_INVALIDCALL;
#else
        (void)b;
#endif // NXDK_DEBUG
        g_pD3D->CurrentDisplayMode = i;
        D3D_DebugPrintf("Set video mode %ux%u%c@%uHz (%dbpp).\n", 
                        mode.Width, mode.Height, 
                        (mode.Flags & D3DPRESENTFLAG_INTERLACED) ? 
                            'i' : 'p', mode.RefreshRate, bpp);
    }
#if NXDK_DEBUG
    else {
        return D3DERR_INVALIDCALL;
    }
#endif // NXDK_DEBUG
    LPDIRECT3DSURFACE8 pBackBuffer0 = pPresentationParameters->BufferSurfaces[0];
    if (pBackBuffer0) {
        // pb_init targets the second surface as the initial backbuffer,
        // but D3D expects pPresentationParameters->BufferSurfaces[0] to
        // be targeted first. 
        pPresentationParameters->BufferSurfaces[0] = 
            pPresentationParameters->BufferSurfaces[1];
        pPresentationParameters->BufferSurfaces[1] = pBackBuffer0;
        D3DSurface* pBufferSurface = (D3DSurface*)pBackBuffer0;
        pb_set_fbs_addr(pBufferSurface->resource.pContiguousMemory);
    }

    if (pPresentationParameters->DepthStencilSurface) {
        D3DSurface* pDSSurf = 
            (D3DSurface*)pPresentationParameters->DepthStencilSurface;
        g_pDevice->pDepthStencilSurface = (D3DSurface*)pDSSurf;
        pb_set_ds_addr(pDSSurf->resource.pContiguousMemory);
    }
    
    UINT BackBufferCount = pPresentationParameters->BackBufferCount;
    if (BackBufferCount == 0)
        BackBufferCount = 1;
    pb_back_buffer_count(BackBufferCount);
    DWORD ColorFormat = D3D_FormatToColorFormat(mode.Format);
    pb_set_color_format(ColorFormat, false);

    g_pDevice->dwLastFenceCompleted = 0;
    g_pDevice->dwNextFence = 1;
    if (g_pDevice->hFenceEvent != NULL)
        CloseHandle(g_pDevice->hFenceEvent);

    if (!NT_SUCCESS(NtCreateEvent(&g_pDevice->hFenceEvent, NULL, SynchronizationEvent, FALSE))) {
        g_pDevice->hFenceEvent = NULL;
        return E_FAIL;
    }

    if (g_pDevice->hFenceEvent == NULL)
        return E_FAIL;

    return D3D_OK;
}

BOOL D3D_IsDepthStencilFormatFixed(D3DFORMAT Format) {
    if (Format == D3DFMT_D24S8 || D3DFMT_D16)
        return TRUE;
    if (Format == D3DFMT_F24S8 || D3DFMT_F16)
        return FALSE;

    assert(false);
    return -1;
}

HRESULT Direct3D_CreateDevice(
    DWORD BehaviorFlags, 
    D3DPRESENT_PARAMETERS* pPresentationParameters)
{
#if NXDK_DEBUG
    HRESULT hr = Direct3D_CreateDevice_Validate(BehaviorFlags, 
                                                pPresentationParameters);
    D3D_ASSERT_IF(hr, FAILED(hr));
#else 
    HRESULT hr = D3D_OK;
#endif // NXDK_DEBUG
    D3D_ASSERT_IF(D3DERR_INVALIDCALL, (g_pDevice != NULL)) 
    if (g_pDevice)
        D3D_ASSERT_IF(D3DERR_INVALIDCALL, g_pDevice->refcount.c != 0);

    g_pDevice = malloc(sizeof(*g_pDevice));
    D3D_ASSERT_IF_NO_RETURN(g_pDevice == NULL) {
        hr = E_OUTOFMEMORY;
        goto failed;
    }

    g_pDevice->iface.lpVtbl = g_pDeviceVtbl;
    hr = Direct3D_ResetDevice(pPresentationParameters);
    if (FAILED(hr))
        goto failed;

    pb_size(g_pD3D->PushBufferSize);
    D3D_DebugPrintf("pb_init()\n");
    int ret = pb_init();
    D3D_ASSERT_IF_NO_RETURN(ret != 0) {
        D3D_DebugPrintf("pb_init failed %d.\n", ret);
        switch (ret) {
        case -8:
            hr = D3DERR_DEVICENOTRESET;
        case -2:
        case -3:
        case -11:
            hr = D3DERR_OUTOFVIDEOMEMORY;
        default:
            hr = D3DERR_DRIVERINTERNALERROR;
        }
        goto failed;
    }
    D3D_DebugPrintf("pb_init successful.\n");

    UINT BackBufferCount = pPresentationParameters->BackBufferCount;
    if (BackBufferCount == 0)
        BackBufferCount = 1;

    for (int i = 0; i < BackBufferCount + 1; i++) {
        if (pPresentationParameters->BufferSurfaces[i] != NULL) {
            g_pDevice->pSurfaces[i] = 
                (D3DSurface*)pPresentationParameters->BufferSurfaces[i];
            continue;
        }

        g_pDevice->pSurfaces[i] = malloc(sizeof(D3DSurface));
        D3D_ASSERT_IF_NO_RETURN(!g_pDevice->pSurfaces[i]) {
            hr = E_OUTOFMEMORY;
            goto failed;
        }

        D3D_CreateSurface(
            pPresentationParameters->BackBufferWidth, 
            pPresentationParameters->BackBufferHeight, 
            pPresentationParameters->BackBufferFormat,
            0, 
            pPresentationParameters->MultiSampleType,
            (PVOID)pb_fb_addr(i),
            NULL,
            g_pDevice->pSurfaces[i]);
    }
        
    if (pPresentationParameters->EnableAutoDepthStencil) {
        D3D_DebugPrintf("Creating ZStencil surface.\n");
        g_pDevice->pDepthStencilSurface = malloc(sizeof(D3DSurface));
        D3D_ASSERT_IF_NO_RETURN(!g_pDevice->pDepthStencilSurface) {
            hr = E_OUTOFMEMORY;
            goto failed;
        }

        D3D_CreateSurface(
            pPresentationParameters->BackBufferWidth, 
            pPresentationParameters->BackBufferHeight, 
            pPresentationParameters->AutoDepthStencilFormat,
            D3DUSAGE_DEPTHSTENCIL, 
            pPresentationParameters->MultiSampleType,
            (PVOID)pb_ds_addr(),
            NULL,
            g_pDevice->pDepthStencilSurface);
    }

    pb_show_front_screen();

    g_pDevice->refcount.c              = 1;
    g_pDevice->KickOffSize             = g_pD3D->KickOffSize / sizeof(DWORD);
    g_pDevice->bInScene                = FALSE;
    g_pDevice->MaxSurfaces             = BackBufferCount + 1;
    g_pDevice->CurrentSurface          = 0;

    pb_reset();
    D3D_DebugPrintf("Creating default push buffer.\n");
    g_pDevice->DefaultPB.iface.lpVtbl = g_pPushBufferVtbl;
    D3D_CreateResource(D3DRTYPE_PUSHBUFFER, 0, 
                       NULL, &g_pDevice->DefaultPB.resource);
    g_pDevice->DefaultPB.Size = pb_get_size() / sizeof(DWORD);
    g_pDevice->DefaultPB.SizeNeeded = 0;
    g_pDevice->DefaultPB.bCpu = FALSE;
    g_pDevice->DefaultPB.resource.pContiguousMemory = (PDWORD)pb_begin();
    D3D_ASSERT_IF(D3DERR_DRIVERINTERNALERROR, 
                  g_pDevice->DefaultPB.resource.pContiguousMemory == NULL);
    g_pDevice->DefaultPB.p = g_pDevice->DefaultPB.resource.pContiguousMemory;
    g_pDevice->DefaultPB.pLastPush = g_pDevice->DefaultPB.p;
    g_pDevice->pCurrentPB = &g_pDevice->DefaultPB;

    // Insert a jump back to the beginning of the push buffer at the end.
    *(g_pDevice->DefaultPB.p + g_pDevice->DefaultPB.Size - 1) = 
        D3D_NV2A_PFIFO_ENCODE_JUMP(g_pDevice->DefaultPB.p);

    g_pDevice->LastPresentVBlankCount  = 0;
    g_pDevice->PresentInterval = 
        pPresentationParameters->FullScreen_PresentationInterval;

    DWORD ZFormat = D3D_IsDepthStencilFormatFixed(
        g_pDevice->pDepthStencilSurface->desc.Format) ? 
                    NV097_SET_CONTROL0_Z_FORMAT_FIXED : 
                    NV097_SET_CONTROL0_Z_FORMAT_FLOAT;
    D3D_DebugPrintf("Setting Control0.\n");
    dump_push_buffer(g_pDevice);
    g_pDevice->Control0 = 
        NV097_SET_CONTROL0_TEXTURE_PERSPECTIVE_ENABLE | ZFormat;
    hr = D3DDevice_PushCmd(NV097_SET_CONTROL0, g_pDevice->Control0);
    D3D_ASSERT_IF_NO_RETURN(FAILED(hr)) {
        goto failed;
    }
    dump_push_buffer(g_pDevice);

    for (UINT Stage = 0; Stage < D3DTSS_MAXSTAGES; Stage++) {
        g_pDevice->pTexture[Stage] = NULL;
        memzrop(&g_pDevice->TextureStageState[Stage]);
    }

    D3D_DebugPrintf("Setting execution mode.\n");
    hr = D3DDevice_PushCmd(NV097_SET_TRANSFORM_EXECUTION_MODE, 
            MASK(NV097_SET_TRANSFORM_EXECUTION_MODE_MODE, 
                 NV097_SET_TRANSFORM_EXECUTION_MODE_MODE_PROGRAM)
                | MASK(NV097_SET_TRANSFORM_EXECUTION_MODE_RANGE_MODE, 
                  NV097_SET_TRANSFORM_EXECUTION_MODE_RANGE_MODE_PRIV));
    D3D_ASSERT_IF_NO_RETURN(FAILED(hr)) {
        goto failed;
    }
    dump_push_buffer(g_pDevice);

    D3DVIEWPORT8 Viewport;
    Viewport.X      = 0;
    Viewport.Y      = 0;
    Viewport.Width  = pPresentationParameters->BackBufferWidth;
    Viewport.Height = pPresentationParameters->BackBufferHeight;
    Viewport.MinZ   = 0.2f;
    Viewport.MaxZ   = 1000.0f;
    
    hr = IDirect3DDevice8_SetViewport(&g_pDevice->iface, &Viewport);
    D3D_ASSERT_IF_NO_RETURN(FAILED(hr)) {
        goto failed;
    }

    // D3D_DebugPrintf("Initing device state.\n");
    // hr = D3DDevice_InitDeviceState();
    // D3D_ASSERT_IF_NO_RETURN(FAILED(hr)) {
    //     debugPrint("Device state init failed %d,\n", hr);
    //     goto failed;
    // }

    // D3D_DebugPrintf("Device state init successful.\n");
    return D3D_OK;

failed:
    free(g_pDevice->DefaultPB.resource.pContiguousMemory);

    if (pPresentationParameters->EnableAutoDepthStencil)
        free(g_pDevice->pDepthStencilSurface);

    for (int i = 0; i < BackBufferCount + 1; i++)
        free(g_pDevice->pSurfaces[i]);

    free(g_pDevice);
    assert(false && "hr");
    return hr;
}

D3DAPI HRESULT Direct3D8_CreateDevice(
    LPDIRECT3D8 pThis, UINT Adapter, D3DDEVTYPE DeviceType, 
    HWND hFocusWindow, DWORD BehaviorFlags, 
    D3DPRESENT_PARAMETERS* pPresentationParameters,
    LPDIRECT3DDEVICE8* ppReturnedDevice)
{
    assert(pThis == &g_pD3D->iface);
#if NXDK_DEBUG
    D3D_ASSERT_IF(D3DERR_INVALIDCALL,   (Adapter != D3DADAPTER_DEFAULT));
    D3D_ASSERT_IF(D3DERR_INVALIDDEVICE, (DeviceType != D3DDEVTYPE_HAL));
    D3D_ASSERT_IF(D3DERR_INVALIDCALL,   (hFocusWindow != NULL));
#endif // NXDK_DEBUG
    HRESULT hr = Direct3D_CreateDevice(BehaviorFlags, 
                                       pPresentationParameters);
    *ppReturnedDevice = &g_pDevice->iface;
    assert(SUCCEEDED(hr));
    return hr;
}

static IDirect3DVtbl8* g_pD3DVtbl = NULL;

BOOL g_firstCreate = TRUE;

UINT Direct3D_EnumDisplayModes(int bpp) {
    VIDEO_MODE vm;
    void* p = NULL;
    while(XVideoListModes(&vm, bpp, REFRESH_DEFAULT, &p)) {
        D3DDISPLAYMODE* pDisplayMode = 
            &g_pD3D->DisplayModes[g_pD3D->DisplayModeCount];
        DWORD dwFlags = (vm.interlaced == TRUE ? D3DPRESENTFLAG_INTERLACED 
                                               : D3DPRESENTFLAG_PROGRESSIVE)
                        | (vm.width == 640 ? 0 : D3DPRESENTFLAG_WIDESCREEN);

        D3DFORMAT Format = D3DFMT_UNKNOWN;
        if (bpp == 32)
            Format = D3DFMT_LIN_A8R8G8B8;
        else if (bpp == 16) 
            Format = D3DFMT_LIN_R5G6B5;
        else if (bpp == 15) 
            Format = D3DFMT_LIN_X1R5G5B5;

        pDisplayMode->Width       = vm.width;
        pDisplayMode->Height      = vm.height;
        pDisplayMode->RefreshRate = vm.refresh;
        pDisplayMode->Format      = Format;
        pDisplayMode->Flags       = dwFlags;
        g_pD3D->DisplayModeCount++;
    }
    return g_pD3D->DisplayModeCount;
}

static BOOL g_d3dCreated = FALSE;

D3DEXTERN D3DAPI LPDIRECT3D8 Direct3DCreate8(UINT SDKVersion) {
    if (SDKVersion != D3D_SDK_VERSION)
        return NULL;

    if (g_d3dCreated == TRUE) {
        return &g_pD3D->iface;
    } else {
        g_pD3D = malloc(sizeof(Direct3D));
        if (!g_pD3D) goto failed;
    }

    g_pD3DVtbl = malloc(sizeof(*g_pD3DVtbl));
    if (g_pD3DVtbl == NULL) goto failed;
    g_pD3DVtbl->AddRef                    = Direct3D8_AddRef;
    g_pD3DVtbl->Release                   = Direct3D8_Release;
    g_pD3DVtbl->GetAdapterCount           = Direct3D8_GetAdapterCount;
    g_pD3DVtbl->GetAdapterMonitor         = Direct3D8_GetAdapterMonitor;
    g_pD3DVtbl->GetAdapterModeCount       = Direct3D8_GetAdapterModeCount;
    g_pD3DVtbl->GetAdapterDisplayMode     = Direct3D8_GetAdapterDisplayMode;
    g_pD3DVtbl->EnumAdapterModes          = Direct3D8_EnumAdapterModes;
    g_pD3DVtbl->CheckDepthStencilMatch    = Direct3D8_CheckDepthStencilMatch;
    g_pD3DVtbl->SetPushBufferSize         = Direct3D8_SetPushBufferSize;
    g_pD3DVtbl->CreateDevice              = Direct3D8_CreateDevice;
    g_pD3DVtbl->GetDeviceCaps             = Direct3D8_GetDeviceCaps;
    g_pD3D->iface.lpVtbl = g_pD3DVtbl;

    g_pDeviceVtbl = malloc(sizeof(*g_pDeviceVtbl));
    if (g_pDeviceVtbl == NULL) goto failed;

    g_pDeviceVtbl->AddRef                    = Direct3DDevice8_AddRef;
    g_pDeviceVtbl->Release                   = Direct3DDevice8_Release;
    g_pDeviceVtbl->BlockUntilVerticalBlank = 
        Direct3DDevice8_BlockUntilVerticalBlank;
    g_pDeviceVtbl->CreateImageSurface = Direct3DDevice8_CreateImageSurface;
    g_pDeviceVtbl->CreateDepthStencilSurface = 
        Direct3DDevice8_CreateDepthStencilSurface;
    g_pDeviceVtbl->CreateVertexBuffer = Direct3DDevice8_CreateVertexBuffer;
    g_pDeviceVtbl->CreatePushBuffer          = Direct3DDevice8_CreatePushBuffer;
    g_pDeviceVtbl->BeginScene                = Direct3DDevice8_BeginScene;
    g_pDeviceVtbl->EndScene                  = Direct3DDevice8_EndScene;
    g_pDeviceVtbl->Present                   = Direct3DDevice8_Present;
    g_pDeviceVtbl->DrawVertices              = Direct3DDevice8_DrawVertices;
    g_pDeviceVtbl->DrawPrimitive             = Direct3DDevice8_DrawPrimitive;
    g_pDeviceVtbl->SetViewport               = Direct3DDevice8_SetViewport;
    g_pDeviceVtbl->CreateTexture             = Direct3DDevice8_CreateTexture;
    g_pDeviceVtbl->CreateCubeTexture = Direct3DDevice8_CreateCubeTexture;
    g_pDeviceVtbl->SetRenderState            = Direct3DDevice8_SetRenderState;
    g_pDeviceVtbl->SetTextureStageState = Direct3DDevice8_SetTextureStageState;
    g_pDeviceVtbl->SetVertexShaderInputDirect = 
        Direct3DDevice8_SetVertexShaderInputDirect;
    g_pDeviceVtbl->LoadVertexShaderProgram = 
        Direct3DDevice8_LoadVertexShaderProgram;
    g_pDeviceVtbl->SetPixelShaderProgram = 
        Direct3DDevice8_SetPixelShaderProgram;
    g_pDeviceVtbl->Clear                     = Direct3DDevice8_Clear;
    g_pDeviceVtbl->SetScissors               = Direct3DDevice8_SetScissors;
    g_pDeviceVtbl->SetTile                   = Direct3DDevice8_SetTile;
    g_pDeviceVtbl->SetTexture                = Direct3DDevice8_SetTexture;
    g_pDeviceVtbl->InsertCallback            = Direct3DDevice8_InsertCallback;
    g_pDeviceVtbl->Reset                     = Direct3DDevice8_Reset;
    g_pDeviceVtbl->InsertFence               = Direct3DDevice8_InsertFence;
    g_pDeviceVtbl->BlockOnFence              = Direct3DDevice8_BlockOnFence;
    g_pDeviceVtbl->IsFencePending            = Direct3DDevice8_IsFencePending;
    g_pDeviceVtbl->SetVertexShaderConstant = 
        Direct3DDevice8_SetVertexShaderConstant;
    g_pDeviceVtbl->SetPixelShaderConstant = 
        Direct3DDevice8_SetPixelShaderConstant;
    g_pDeviceVtbl->GetPushBuffer = Direct3DDevice8_GetPushBuffer;
    g_pDeviceVtbl->KickPushBuffer = Direct3DDevice8_KickPushBuffer;

    g_pResourceVtbl = malloc(sizeof(*g_pResourceVtbl));
    if (g_pResourceVtbl == NULL) goto failed;
    g_pResourceVtbl->AddRef               = D3DResource_AddRef;
    g_pResourceVtbl->Release              = D3DResource_Release;
    g_pResourceVtbl->GetType              = D3DResource_GetType;
    g_pResourceVtbl->Register             = D3DResource_Register;
    g_pResourceVtbl->IsBusy               = D3DResource_IsBusy;
    g_pResourceVtbl->BlockUntilNotBusy    = D3DResource_BlockUntilNotBusy;

    g_pBaseTextureVtbl = malloc(sizeof(*g_pBaseTextureVtbl));
    if (g_pBaseTextureVtbl == NULL) goto failed;
    g_pBaseTextureVtbl->AddRef            = D3DBaseTexture_AddRef;
    g_pBaseTextureVtbl->Release           = D3DBaseTexture_Release;
    g_pBaseTextureVtbl->GetType           = D3DBaseTexture_GetType;
    g_pBaseTextureVtbl->Register          = D3DBaseTexture_Register;
    g_pBaseTextureVtbl->IsBusy            = D3DBaseTexture_IsBusy;
    g_pBaseTextureVtbl->BlockUntilNotBusy = D3DBaseTexture_BlockUntilNotBusy;
    g_pBaseTextureVtbl->GetLevelCount     = D3DBaseTexture_GetLevelCount;

    g_pTextureVtbl = malloc(sizeof(*g_pTextureVtbl));
    if (g_pTextureVtbl == NULL) goto failed;
    g_pTextureVtbl->AddRef                = D3DTexture_AddRef;
    g_pTextureVtbl->Release               = D3DTexture_Release;
    g_pTextureVtbl->GetType               = D3DTexture_GetType;
    g_pTextureVtbl->Register              = D3DTexture_Register;
    g_pTextureVtbl->IsBusy                = D3DTexture_IsBusy;
    g_pTextureVtbl->BlockUntilNotBusy     = D3DTexture_BlockUntilNotBusy;
    g_pTextureVtbl->GetLevelCount         = D3DTexture_GetLevelCount;
    g_pTextureVtbl->GetSurfaceLevel       = D3DTexture_GetSurfaceLevel;
    g_pTextureVtbl->GetLevelDesc          = D3DTexture_GetLevelDesc;
    g_pTextureVtbl->LockRect              = D3DTexture_LockRect;
    g_pTextureVtbl->UnlockRect            = D3DTexture_UnlockRect;

    g_pCubeTextureVtbl = malloc(sizeof(*g_pCubeTextureVtbl));
    if (g_pCubeTextureVtbl == NULL) goto failed;
    g_pCubeTextureVtbl->AddRef            = D3DCubeTexture_AddRef;
    g_pCubeTextureVtbl->Release           = D3DCubeTexture_Release;
    g_pCubeTextureVtbl->GetType           = D3DCubeTexture_GetType;
    g_pCubeTextureVtbl->Register          = D3DCubeTexture_Register;
    g_pCubeTextureVtbl->IsBusy            = D3DCubeTexture_IsBusy;
    g_pCubeTextureVtbl->BlockUntilNotBusy = D3DCubeTexture_BlockUntilNotBusy;
    g_pCubeTextureVtbl->GetLevelCount     = D3DCubeTexture_GetLevelCount;
    g_pCubeTextureVtbl->GetLevelDesc      = D3DCubeTexture_GetLevelDesc;
    g_pCubeTextureVtbl->LockRect          = D3DCubeTexture_LockRect;
    g_pCubeTextureVtbl->UnlockRect        = D3DCubeTexture_UnlockRect;
    g_pCubeTextureVtbl->GetCubeMapSurface = 
        D3DCubeTexture_GetCubeMapSurface;

    g_pSurfaceVtbl = malloc(sizeof(*g_pSurfaceVtbl));
    if (g_pSurfaceVtbl == NULL) goto failed;
    g_pSurfaceVtbl->AddRef                = D3DSurface_AddRef;
    g_pSurfaceVtbl->Release               = D3DSurface_Release;
    g_pSurfaceVtbl->GetType               = D3DSurface_GetType;
    g_pSurfaceVtbl->Register              = D3DSurface_Register;
    g_pSurfaceVtbl->IsBusy                = D3DSurface_IsBusy;
    g_pSurfaceVtbl->BlockUntilNotBusy     = D3DSurface_BlockUntilNotBusy;
    g_pSurfaceVtbl->GetContainer          = D3DSurface_GetContainer;
    g_pSurfaceVtbl->GetDesc               = D3DSurface_GetDesc;
    g_pSurfaceVtbl->LockRect              = D3DSurface_LockRect;
    g_pSurfaceVtbl->UnlockRect            = D3DSurface_UnlockRect;
     
    g_pVertexBufferVtbl = malloc(sizeof(*g_pVertexBufferVtbl));
    if (g_pVertexBufferVtbl == NULL) goto failed;
    g_pVertexBufferVtbl->AddRef           = D3DVertexBuffer_AddRef;
    g_pVertexBufferVtbl->Release          = D3DVertexBuffer_Release;
    g_pVertexBufferVtbl->GetType          = D3DVertexBuffer_GetType;
    g_pVertexBufferVtbl->Register         = D3DVertexBuffer_Register;
    g_pVertexBufferVtbl->IsBusy           = D3DVertexBuffer_IsBusy;
    g_pVertexBufferVtbl->BlockUntilNotBusy = 
        D3DVertexBuffer_BlockUntilNotBusy;
    g_pVertexBufferVtbl->GetDesc          = D3DVertexBuffer_GetDesc;
    g_pVertexBufferVtbl->Lock             = D3DVertexBuffer_Lock;
    g_pVertexBufferVtbl->Unlock           = D3DVertexBuffer_Unlock;

    g_pPushBufferVtbl = malloc(sizeof(*g_pPushBufferVtbl));
    if (g_pPushBufferVtbl == NULL) goto failed;
    g_pPushBufferVtbl->AddRef             = D3DPushBuffer_AddRef;
    g_pPushBufferVtbl->Release            = D3DPushBuffer_Release;
    g_pPushBufferVtbl->GetType            = D3DPushBuffer_GetType;
    g_pPushBufferVtbl->Register           = D3DPushBuffer_Register;
    g_pPushBufferVtbl->IsBusy             = D3DPushBuffer_IsBusy;
    g_pPushBufferVtbl->BlockUntilNotBusy  = D3DPushBuffer_BlockUntilNotBusy;
    g_pPushBufferVtbl->GetSize            = D3DPushBuffer_GetSize;
    g_pPushBufferVtbl->GetData            = D3DPushBuffer_GetData;

    g_pD3D->DisplayModeCount = 0;
    
    Direct3D_EnumDisplayModes(15);
    Direct3D_EnumDisplayModes(16);
    Direct3D_EnumDisplayModes(32);

    D3D_ASSERT_IF(NULL, g_pD3D->DisplayModeCount == 0);

    g_pD3D->CurrentDisplayMode = 0;
    D3DDISPLAYMODE* pDM = &g_pD3D->DisplayModes[g_pD3D->CurrentDisplayMode];
    // set the current mode to whatever the last retrieved mode was
    if (XVideoSetMode(pDM->Width, pDM->Height, 
                      D3D_FormatBPP(pDM->Format), pDM->RefreshRate) == FALSE)
    {
        assert(false);
        return NULL;
    }

    g_pD3D->KickOffSize    = 16 * 1024;
    g_pD3D->PushBufferSize = 64 * 1024;
    return &g_pD3D->iface;

failed:
    free(g_pD3D);
    free(g_pD3DVtbl);
    free(g_pDeviceVtbl);
    free(g_pResourceVtbl);
    free(g_pBaseTextureVtbl);
    g_pD3D = NULL;
    g_pD3DVtbl = NULL;
    g_pDeviceVtbl = NULL;
    g_pResourceVtbl = NULL;
    g_pBaseTextureVtbl = NULL;
    return NULL;
}
