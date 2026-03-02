#include <assert.h>
#include <stdlib.h>
#include <stdcountof.h>

#include <hal/video.h>
#include <pbkit/pbkit.h>
#include <xboxkrnl/xboxkrnl.h>

#include "d3d8.h"
#include "d3d8_private.h"
#include "d3d8_resource.h"
#include "d3d8_device.h"

// TODO: set this in build process
#ifndef NXDK_DEBUG
#define NXDK_DEBUG 1
#endif // NXDK_DEBUG

PVOID D3D_AllocContiguousMemory(DWORD Size, DWORD Alignment) {
    return MmAllocateContiguousMemoryEx(Size, 0, 0x03FFB000, Alignment,
                                        PAGE_READWRITE | PAGE_WRITECOMBINE);
}

void D3D_FreeContiguousMemory(PVOID Base) {
    MmFreeContiguousMemory(Base);
}

typedef struct Direct3D IMPLEMENTS(IDirect3D8) {
    IDirect3D8      iface;
    DWORD           KickOffSize;
    DWORD           PushBufferSize;
    D3DDISPLAYMODE  DisplayModes[51];
    UINT            DisplayModeCount;
    UINT            CurrentDisplayMode;
#ifdef __cplusplus
    ULONG AddRef() override {
        return Direct3D_AddRef();
    }

    ULONG Release() override {
        return Direct3D_Release();
    }

    UINT GetAdapterCount() override { 
        return Direct3D_GetAdapterCount(); 
    }

    HRESULT GetAdapterIdentifier(
        UINT Adapter, DWORD Flags, 
        D3DADAPTER_IDENTIFIER8* pIdentifier) override 
    { 
        return Direct3D_GetAdapterIdentifier(Adapter, Flags, pIdentifier);
    }

    UINT GetAdapterModeCount(UINT Adapter) override { 
        return Direct3D_GetAdapterModeCount(Adapter); 
    }
    HRESULT EnumAdapterModes(UINT Adapter, UINT Mode, 
                             D3DDISPLAYMODE* pMode) override 
    { 
        return Direct3D_EnumAdapterModes(Adapter, Mode, pMode);
    }

    HRESULT GetAdapterDisplayMode(UINT Adapter, 
                                  D3DDISPLAYMODE* pMode) override 
    {
        return Direct3D_GetAdapterDisplayMode(Adapter, pMode);
    }

    HRESULT CheckDeviceType(UINT Adapter, D3DDEVTYPE CheckType, 
                            D3DFORMAT DisplayFormat, 
                            D3DFORMAT BackBufferFormat, 
                            BOOL Windowed) override 
    { 
       return IDirect3D_CheckDeviceType(&this->iface, Adapter, CheckDeviceType,
                                        DisplayFormat, BackBufferFormat, 
                                        Windowed); 

    }

    HRESULT CheckDeviceFormat(UINT Adapter, D3DDEVTYPE DeviceType, 
                              D3DFORMAT AdapterFormat, DWORD Usage, 
                              D3DRESOURCETYPE RType, 
                              D3DFORMAT CheckFormat) override 
    { 
        return Direct3D_CheckDeviceFormat(Adapter, DeviceType, AdapterFormat,
                                          Usage, RType, CheckFormat); 
    }

    HRESULT CheckDeviceMultiSampleType(
        UINT Adapter, D3DDEVTYPE DeviceType, 
        D3DFORMAT SurfaceFormat, BOOL Windowed, 
        D3DMULTISAMPLE_TYPE MultiSampleType) override 
    {
        return Direct3D_CheckDeviceMultiSampleType(Adapter, DeviceType, 
                                                   SurfaceFormat, Windowed,
                                                   MultiSampleType);
    }

    HRESULT CheckDepthStencilMatch(UINT Adapter, D3DDEVTYPE DeviceType,
                                   D3DFORMAT AdapterFormat, 
                                   D3DFORMAT RenderTargetFormat, 
                                   D3DFORMAT DepthStencilFormat) override 
    {
        return Direct3D_CheckDepthStencilMatch(Adapter, DeviceType, 
                                               AdapterFormat,
                                               RenderTargetFormat, 
                                               DepthStencilFormat); 
    }

    HRESULT GetDeviceCaps(UINT Adapter, D3DDEVTYPE DeviceType, 
                          D3DCAPS8* pCaps) override 
    { 
        return Direct3D_GetDeviceCaps(Adapter, DeviceType, pCaps);

    }

    HMONITOR GetAdapterMonitor(UINT Adapter) override {
        return Direct3D_GetAdapterMonitor(Adapter);
    }

    HRESULT CreateDevice(
        UINT Adapter, D3DDEVTYPE DeviceType, HWND hFocusWindow, 
        DWORD BehaviorFlags, D3DPRESENT_PARAMETERS* pPresentationParameters, 
        LPDIRECT3DDEVICE8* ppReturnedDeviceInterface) override 
    { 
        return Direct3D_CreateDevice(Adapter, DeviceType, hFocusWindow, 
                                     BehaviorFlags, pPresentationParameters,
                                     ppReturnedDeviceInterface); 
    }
#endif // __cplusplus
} Direct3D;

static Direct3D g_d3d;

ULONG IDirect3D8_AddRef(LPDIRECT3D8 pThis) {
    assert(pThis == &g_d3d.iface);
    return 1;
}

ULONG IDirect3D8_Release(LPDIRECT3D8 pThis) {
    assert(pThis == &g_d3d.iface);
    return 1;
}

UINT IDirect3D8_GetAdapterCount(LPDIRECT3D8 pThis) {
    assert(pThis == &g_d3d.iface);
    return 1;
}

HMONITOR IDirect3D8_GetAdapterMonitor(LPDIRECT3D8 pThis, UINT Adapter) {
    assert(pThis == &g_d3d.iface);
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

HRESULT IDirect3D8_CheckDeviceType(LPDIRECT3D8 pThis, UINT Adapter, 
                                   D3DDEVTYPE CheckType, 
                                   D3DFORMAT DisplayFormat,
                                   D3DFORMAT BackBufferFormat, BOOL Windowed)
{
    assert(pThis == &g_d3d.iface);
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
    return g_d3d.DisplayModeCount;
}

UINT IDirect3D8_GetAdapterModeCount(LPDIRECT3D8 pThis, UINT Adapter) {
    assert(pThis == &g_d3d.iface);
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
    if (Mode >= g_d3d.DisplayModeCount) {
        assert(false);
        return D3DERR_INVALIDCALL;
    }
#endif // NXDK_DEBUG
    memcpyp(pMode, &g_d3d.DisplayModes[Mode]);
    return D3D_OK;
}

HRESULT IDirect3D8_EnumAdapterModes(LPDIRECT3D8 pThis, UINT Adapter, 
                                    UINT Mode, D3DDISPLAYMODE* pMode) 
{
    assert(pThis == &g_d3d.iface);
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
    memcpyp(pMode, &g_d3d.DisplayModes[g_d3d.CurrentDisplayMode]);
    return D3D_OK;
}

HRESULT IDirect3D8_GetAdapterDisplayMode(LPDIRECT3D8 pThis, UINT Adapter, 
                                         D3DDISPLAYMODE* pMode)
{
    assert(pThis == &g_d3d.iface);
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

HRESULT IDirect3D8_CheckDepthStencilMatch(LPDIRECT3D8 pThis, UINT Adapter, 
                                          D3DDEVTYPE DeviceType, 
                                          D3DFORMAT AdapterFormat, 
                                          D3DFORMAT RenderTargetFormat, 
                                          D3DFORMAT DepthStencilFormat)
{
    assert(pThis == &g_d3d.iface);
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
    
    pb_size(PushBufferSize);
    g_d3d.KickOffSize       = KickOffSize;
    g_d3d.PushBufferSize    = PushBufferSize;
    return D3D_OK;
}

HRESULT IDirect3D8_SetPushBufferSize(LPDIRECT3D8 pThis, 
                                     DWORD PushBufferSize, 
                                     DWORD KickOffSize) 
{
    assert(pThis == &g_d3d.iface);
    return Direct3D_SetPushBufferSize(PushBufferSize, KickOffSize);
}

BOOL Direct3D_IsDisplayModeValid(D3DDISPLAYMODE* pMode, int* pIndex) {
#if NXDK_DEBUG
    if (pMode == NULL)
        return FALSE;
#endif // NXDK_DEBUG
    if (pMode->RefreshRate == 0) {
        // Any refresh rate is acceptable, use first matching mode.
        for (int i = 0; i < g_d3d.DisplayModeCount; i++) {
            if (pMode->Width  == g_d3d.DisplayModes[i].Width  &&
                pMode->Height == g_d3d.DisplayModes[i].Height &&
                pMode->Format == g_d3d.DisplayModes[i].Format &&
                pMode->Flags  == g_d3d.DisplayModes[i].Flags) 
            {
                if (pIndex)
                    *pIndex = i;
                return TRUE;
            }
        }
        return FALSE;
    }

    // If refresh rate is specified, memcmp works just fine.
    for (int i = 0; i < g_d3d.DisplayModeCount; i++) {
        if (memcmp(pMode, &g_d3d.DisplayModes[i], sizeof(*pMode)) == 0) {
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
if (pPresentationParameters == NULL)
        return D3DERR_INVALIDCALL;

    if (pPresentationParameters->Windowed != FALSE)
        return D3DERR_INVALIDCALL;

    if (pPresentationParameters->BackBufferCount > 2)
        return D3DERR_INVALIDCALL;

    if (pPresentationParameters->Flags & 
        (D3DPRESENTFLAG_FIELD | D3DPRESENTFLAG_10X11PIXELASPECTRATIO |
         D3DPRESENTFLAG_EMULATE_REFRESH_RATE)
    ) {
        return D3DERR_INVALIDCALL;
    }

    if ((pPresentationParameters->Flags & D3DPRESENTFLAG_PROGRESSIVE) && 
        (pPresentationParameters->Flags & D3DPRESENTFLAG_INTERLACED)) 
    {
        return D3DERR_INVALIDCALL;
    }

    // D3DPRESENT_INTERVAL_FOUR is invalid on Xbox, 
    // and any higher values aren't valid D3DPRESENT values.
    if (pPresentationParameters->FullScreen_PresentationInterval > 
        D3DPRESENT_INTERVAL_THREE)
    {
        return D3DERR_INVALIDCALL;
    }
    
    if (pPresentationParameters->EnableAutoDepthStencil == TRUE) {
        if (FAILED(Direct3D_CheckDepthStencilMatch(
            pPresentationParameters->AutoDepthStencilFormat))) 
        {
            return D3DERR_INVALIDCALL;
        }

        if (pPresentationParameters->DepthStencilSurface != NULL)
            return D3DERR_INVALIDCALL;
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
        if (pPresentationParameters->BufferSurfaces[1] == NULL)
            return D3DERR_INVALIDCALL;

        D3DSurface* pBufferSurface0 = 
            (D3DSurface*)pPresentationParameters->BufferSurfaces[0];
        if (pBufferSurface0->desc.Size == 0)
            return D3DERR_INVALIDCALL; 
        if (pBufferSurface0->desc.Width  != Width ||
            pBufferSurface0->desc.Height != Height)
        {
            return D3DERR_INVALIDCALL;
        }
        if (pBufferSurface0->desc.Format != Format)
            return D3DERR_INVALIDCALL;

        D3DSurface* pBufferSurface1 = 
            (D3DSurface*)pPresentationParameters->BufferSurfaces[1];
        if (pBufferSurface1->desc.Size == 0)
            return D3DERR_INVALIDCALL; 
        if (pBufferSurface1->desc.Width  != Width ||
            pBufferSurface1->desc.Height != Height)
        {
            return D3DERR_INVALIDCALL;
        }
        if (pBufferSurface1->desc.Format != Format)
            return D3DERR_INVALIDCALL;
        if ((DWORD)pBufferSurface0->resource.pContiguousMemory 
                + pBufferSurface0->desc.Size 
            != (DWORD)pBufferSurface1->resource.pContiguousMemory)
        {
            return D3DERR_INVALIDCALL;
        }

        if (pPresentationParameters->BackBufferCount == 2) {
            if(pPresentationParameters->BufferSurfaces[2] == NULL)
                return D3DERR_INVALIDCALL;
            
            D3DSurface* pBufferSurface2 = 
                (D3DSurface*)pPresentationParameters->BufferSurfaces[2];
            if (pBufferSurface2->desc.Size == 0)
                return D3DERR_INVALIDCALL; 
            if (pBufferSurface2->desc.Width  != Width ||
                pBufferSurface2->desc.Height != Height)
            {
                return D3DERR_INVALIDCALL;
            }
            if (pBufferSurface2->desc.Format != Format)
                return D3DERR_INVALIDCALL;
            return D3DERR_INVALIDCALL;
            if ((DWORD)pBufferSurface1->resource.pContiguousMemory 
                + pBufferSurface1->desc.Size 
            != (DWORD)pBufferSurface2->resource.pContiguousMemory)
            {
                return D3DERR_INVALIDCALL;
            }
        }
    } 
    else {
        if (pPresentationParameters->BufferSurfaces[1] != NULL)
            return D3DERR_INVALIDCALL;

        if (pPresentationParameters->BackBufferCount == 2 &&
            pPresentationParameters->BufferSurfaces[2] != NULL)
        {
            return D3DERR_INVALIDCALL;
        }
    }

    if (pPresentationParameters->DepthStencilSurface != NULL) {
        D3DSurface* pDSSurf = 
            (D3DSurface*)pPresentationParameters->DepthStencilSurface;
        if (pDSSurf->desc.Width  < Width ||
            pDSSurf->desc.Height < Height)
        {
            return D3DERR_INVALIDCALL;
        }

        HRESULT hr = Direct3D_CheckDepthStencilMatch(
            pDSSurf->desc.Format);
        if (FAILED(hr)) return hr;
    }

    return D3D_OK;
}

HRESULT Direct3D_CreateDevice_Validate(
    DWORD BehaviorFlags,
    D3DPRESENT_PARAMETERS* pPresentationParameters)
{
    if (BehaviorFlags !=
        (D3DCREATE_HARDWARE_VERTEXPROCESSING | D3DCREATE_PUREDEVICE))
    {
        return D3DERR_INVALIDCALL;
    }

    return D3D_PresentParameters_Validate(pPresentationParameters);
}
#endif // NXDK_DEBUG

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
        g_d3d.CurrentDisplayMode = i;
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
        g_d3ddev.pDepthStencilSurface = (D3DSurface*)pDSSurf;
        pb_set_ds_addr(pDSSurf->resource.pContiguousMemory);
    }
    
    UINT BackBufferCount = pPresentationParameters->BackBufferCount;
    if (BackBufferCount == 0)
        BackBufferCount = 1;
    pb_back_buffer_count(BackBufferCount);
    pb_set_color_format(pPresentationParameters->BackBufferFormat, false);

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
    assert(SUCCEEDED(hr));
    if (FAILED(hr)) return hr;
#else 
    HRESULT hr = D3D_OK;
#endif // NXDK_DEBUG
    if (g_d3ddev.refcount.c != 0)
        return D3DERR_DEVICENOTRESET;

    hr = Direct3D_ResetDevice(pPresentationParameters);

    int ret = pb_init();
    if (ret != 0) {
        switch (ret) {
        case -8:
            return D3DERR_DEVICENOTRESET;
        case -2:
        case -3:
        case -11:
            return D3DERR_OUTOFVIDEOMEMORY;
        default:
            return D3DERR_DRIVERINTERNALERROR;
        }
    }

    UINT BackBufferCount = pPresentationParameters->BackBufferCount;
    if (BackBufferCount == 0)
        BackBufferCount = 1;

    for (int i = 0; i < BackBufferCount + 1; i++) {
        if (pPresentationParameters->BufferSurfaces[i] != NULL) {
            g_d3ddev.pSurfaces[i] = 
                (D3DSurface*)pPresentationParameters->BufferSurfaces[i];
            continue;
        }

        g_d3ddev.pSurfaces[i] = malloc(sizeof(D3DSurface));
        if (!g_d3ddev.pSurfaces[i])
            return E_OUTOFMEMORY;

        D3D_CreateSurface(
            pPresentationParameters->BackBufferWidth, 
            pPresentationParameters->BackBufferHeight, 
            pPresentationParameters->BackBufferFormat,
            0, 
            pPresentationParameters->MultiSampleType,
            (PVOID)pb_fb_addr(i),
            NULL,
            g_d3ddev.pSurfaces[i]);
    }
        
    if (pPresentationParameters->EnableAutoDepthStencil) {
        g_d3ddev.pDepthStencilSurface = malloc(sizeof(D3DSurface));
        if (!g_d3ddev.pDepthStencilSurface)
            return E_OUTOFMEMORY;

        D3D_CreateSurface(
            pPresentationParameters->BackBufferWidth, 
            pPresentationParameters->BackBufferHeight, 
            pPresentationParameters->AutoDepthStencilFormat,
            D3DUSAGE_DEPTHSTENCIL, 
            pPresentationParameters->MultiSampleType,
            (PVOID)pb_ds_addr(),
            NULL,
            g_d3ddev.pDepthStencilSurface);
    }

    pb_show_front_screen();

    g_d3ddev.refcount.c              = 1;
    g_d3ddev.iface.lpVtbl            = &g_d3ddevVtbl;
    g_d3ddev.KickOffSize             = g_d3d.KickOffSize;
    g_d3ddev.bInScene                = FALSE;
    g_d3ddev.MaxSurfaces             = BackBufferCount;
    g_d3ddev.CurrentSurface          = 0;
    g_d3ddev.pDefaultPB.iface.lpVtbl = &g_d3dPushBufferVtbl;
    g_d3ddev.LastPresentVBlankCount  = 0;
    g_d3ddev.PresentInterval = 
        pPresentationParameters->FullScreen_PresentationInterval;

    hr = D3D_CreatePushBuffer(g_d3d.PushBufferSize, FALSE, 
                              NULL, &g_d3ddev.pDefaultPB);

    if (FAILED(hr)) return hr;
    g_d3ddev.pCurrentPB = &g_d3ddev.pDefaultPB;
    DWORD ZFormat = D3D_IsDepthStencilFormatFixed(
        g_d3ddev.pDepthStencilSurface->desc.Format) ? 
                    NV097_SET_CONTROL0_Z_FORMAT_FIXED : 
                    NV097_SET_CONTROL0_Z_FORMAT_FLOAT;
    g_d3ddev.Control0 = 
        NV097_SET_CONTROL0_TEXTURE_PERSPECTIVE_ENABLE | ZFormat;
    hr = D3DDevice_PushCmd(NV097_SET_CONTROL0, g_d3ddev.Control0);
    if (FAILED(hr)) return hr;

    for (UINT Stage = 0; Stage < D3DTSS_MAXSTAGES; Stage++) {
        g_d3ddev.pTexture[Stage] = NULL;
        memzrop(&g_d3ddev.TextureStageState[Stage]);
    }

    hr = D3DDevice_PushCmd(NV097_SET_TRANSFORM_EXECUTION_MODE, 
            MASK(NV097_SET_TRANSFORM_EXECUTION_MODE_MODE, 
                 NV097_SET_TRANSFORM_EXECUTION_MODE_MODE_PROGRAM)
                | MASK(NV097_SET_TRANSFORM_EXECUTION_MODE_RANGE_MODE, 
                  NV097_SET_TRANSFORM_EXECUTION_MODE_RANGE_MODE_PRIV));
    if (FAILED(hr)) return hr;

    hr = D3DDevice_InitDeviceState();
    if (FAILED(hr)) return hr;

    return D3D_OK;
}

HRESULT IDirect3D8_CreateDevice(
    LPDIRECT3D8 pThis, UINT Adapter, D3DDEVTYPE DeviceType, 
    HWND hFocusWindow, DWORD BehaviorFlags, 
    D3DPRESENT_PARAMETERS* pPresentationParameters,
    LPDIRECT3DDEVICE8* ppReturnedDevice)
{
    assert(pThis == &g_d3d.iface);
#if NXDK_DEBUG
    if (Adapter != D3DADAPTER_DEFAULT) {
        assert(false);
        return 0;
    }

    if (DeviceType != D3DDEVTYPE_HAL) {
        assert(false);
        return D3DERR_INVALIDDEVICE;
    }

    if (hFocusWindow != NULL) {
        assert(false);
        return D3DERR_INVALIDCALL;
    }
#endif // NXDK_DEBUG
    HRESULT hr = Direct3D_CreateDevice(BehaviorFlags, 
                                       pPresentationParameters);
    *ppReturnedDevice = &g_d3ddev.iface;
    return hr;
}

IDirect3DVtbl8 g_d3dvtbl;

BOOL g_firstCreate = TRUE;

UINT Direct3D_EnumDisplayModes(int bpp) {
    VIDEO_MODE vm;
    void* p = NULL;
    while(XVideoListModes(&vm, bpp, REFRESH_DEFAULT, &p)) {
        D3DDISPLAYMODE* pDisplayMode = 
            &g_d3d.DisplayModes[g_d3d.DisplayModeCount];
        DWORD dwFlags = (vm.interlaced == TRUE ? D3DPRESENTFLAG_INTERLACED 
                                               : D3DPRESENTFLAG_PROGRESSIVE)
                        | (vm.width == 640 ? 0 : D3DPRESENTFLAG_WIDESCREEN);
        pDisplayMode->Width       = vm.width;
        pDisplayMode->Height      = vm.height;
        pDisplayMode->RefreshRate = vm.refresh;
        pDisplayMode->Format      = D3DFMT_A1R5G5B5;
        pDisplayMode->Flags       = dwFlags;
        g_d3d.DisplayModeCount++;
    }
    return g_d3d.DisplayModeCount;
}

static BOOL g_d3dCreated = FALSE;

LPDIRECT3D8 Direct3DCreate8(UINT SDKVersion) {
    if (SDKVersion != D3D_SDK_VERSION)
        return NULL;

    if (g_d3dCreated == TRUE)
        return &g_d3d.iface;

    g_d3dvtbl.AddRef                   = IDirect3D8_AddRef;
    g_d3dvtbl.Release                  = IDirect3D8_Release;
    g_d3dvtbl.GetAdapterCount          = IDirect3D8_GetAdapterCount;
    g_d3dvtbl.GetAdapterMonitor        = IDirect3D8_GetAdapterMonitor;
    g_d3dvtbl.GetAdapterModeCount      = IDirect3D8_GetAdapterModeCount;
    g_d3dvtbl.GetAdapterDisplayMode    = IDirect3D8_GetAdapterDisplayMode;
    g_d3dvtbl.EnumAdapterModes         = IDirect3D8_EnumAdapterModes;
    g_d3dvtbl.CheckDepthStencilMatch   = IDirect3D8_CheckDepthStencilMatch;
    g_d3dvtbl.SetPushBufferSize        = IDirect3D8_SetPushBufferSize;
    g_d3dvtbl.CreateDevice             = IDirect3D8_CreateDevice;
    g_d3d.iface.lpVtbl = &g_d3dvtbl;

    g_d3ddevVtbl.AddRef                = IDirect3DDevice8_AddRef;
    g_d3ddevVtbl.Release               = IDirect3DDevice8_Release;
    g_d3ddevVtbl.BlockUntilVerticalBlank = 
        IDirect3DDevice8_BlockUntilVerticalBlank;
    g_d3ddevVtbl.CreateImageSurface = IDirect3DDevice8_CreateImageSurface;
    g_d3ddevVtbl.CreateDepthStencilSurface = 
        IDirect3DDevice8_CreateDepthStencilSurface;
    g_d3ddevVtbl.CreateVertexBuffer    = IDirect3DDevice8_CreateVertexBuffer;
    g_d3ddevVtbl.CreatePushBuffer      = IDirect3DDevice8_CreatePushBuffer;
    g_d3ddevVtbl.BeginScene            = IDirect3DDevice8_BeginScene;
    g_d3ddevVtbl.EndScene              = IDirect3DDevice8_EndScene;
    g_d3ddevVtbl.Present               = IDirect3DDevice8_Present;
    g_d3ddevVtbl.DrawVertices          = IDirect3DDevice8_DrawVertices;
    g_d3ddevVtbl.DrawPrimitive         = IDirect3DDevice8_DrawPrimitive;
    g_d3ddevVtbl.SetViewport           = IDirect3DDevice8_SetViewport;
    g_d3ddevVtbl.CreateTexture         = IDirect3DDevice8_CreateTexture;
    g_d3ddevVtbl.CreateCubeTexture     = IDirect3DDevice8_CreateCubeTexture;
    g_d3ddevVtbl.SetRenderState        = IDirect3DDevice8_SetRenderState;
    g_d3ddevVtbl.SetTextureStageState  = IDirect3DDevice8_SetTextureStageState;
    g_d3ddevVtbl.SetVertexShaderInputDirect = 
        IDirect3DDevice8_SetVertexShaderInputDirect;
    g_d3ddevVtbl.LoadVertexShaderProgram = 
        IDirect3DDevice8_LoadVertexShaderProgram;
    g_d3ddevVtbl.SetPixelShaderProgram = 
        IDirect3DDevice8_SetPixelShaderProgram;
    g_d3ddevVtbl.Clear                 = IDirect3DDevice8_Clear;
    g_d3ddevVtbl.SetScissors           = IDirect3DDevice8_SetScissors;
    g_d3ddevVtbl.SetTile               = IDirect3DDevice8_SetTile;
    g_d3ddevVtbl.SetTexture            = IDirect3DDevice8_SetTexture;
    g_d3ddevVtbl.InsertCallback        = IDirect3DDevice8_InsertCallback;
    g_d3ddevVtbl.Reset                 = IDirect3DDevice8_Reset;
    g_d3ddev.iface.lpVtbl = &g_d3ddevVtbl;

    g_d3dResourceVtbl.AddRef           = IDirect3DResource8_AddRef;
    g_d3dResourceVtbl.Release          = IDirect3DResource8_Release;
    g_d3dResourceVtbl.GetType          = IDirect3DResource8_GetType;
    g_d3dResourceVtbl.Register         = IDirect3DResource8_Register;

    g_d3dBaseTextureVtbl.AddRef        = IDirect3DBaseTexture8_AddRef;
    g_d3dBaseTextureVtbl.Release       = IDirect3DBaseTexture8_Release;
    g_d3dBaseTextureVtbl.GetType       = IDirect3DBaseTexture8_GetType;
    g_d3dBaseTextureVtbl.Register      = IDirect3DBaseTexture8_Register;
    g_d3dBaseTextureVtbl.GetLevelCount = IDirect3DBaseTexture8_GetLevelCount;

    g_d3dTextureVtbl.AddRef            = IDirect3DTexture8_AddRef;
    g_d3dTextureVtbl.Release           = IDirect3DTexture8_Release;
    g_d3dTextureVtbl.GetType           = IDirect3DTexture8_GetType;
    g_d3dTextureVtbl.Register          = IDirect3DTexture8_Register;
    g_d3dTextureVtbl.GetLevelCount     = IDirect3DTexture8_GetLevelCount;
    g_d3dTextureVtbl.GetSurfaceLevel   = IDirect3DTexture8_GetSurfaceLevel;
    g_d3dTextureVtbl.GetLevelDesc      = IDirect3DTexture8_GetLevelDesc;
    g_d3dTextureVtbl.LockRect          = IDirect3DTexture8_LockRect;
    g_d3dTextureVtbl.UnlockRect        = IDirect3DTexture8_UnlockRect;

    g_d3dCubeTextureVtbl.AddRef        = IDirect3DCubeTexture8_AddRef;
    g_d3dCubeTextureVtbl.Release       = IDirect3DCubeTexture8_Release;
    g_d3dCubeTextureVtbl.GetType       = IDirect3DCubeTexture8_GetType;
    g_d3dCubeTextureVtbl.Register      = IDirect3DCubeTexture8_Register;
    g_d3dCubeTextureVtbl.GetLevelCount = IDirect3DCubeTexture8_GetLevelCount;
    g_d3dCubeTextureVtbl.GetLevelDesc  = IDirect3DCubeTexture8_GetLevelDesc;
    g_d3dCubeTextureVtbl.LockRect      = IDirect3DCubeTexture8_LockRect;
    g_d3dCubeTextureVtbl.UnlockRect    = IDirect3DCubeTexture8_UnlockRect;
    g_d3dCubeTextureVtbl.GetCubeMapSurface = 
        IDirect3DCubeTexture8_GetCubeMapSurface;

    g_d3dSurfaceVtbl.AddRef            = IDirect3DSurface8_AddRef;
    g_d3dSurfaceVtbl.Release           = IDirect3DSurface8_Release;
    g_d3dSurfaceVtbl.GetType           = IDirect3DSurface8_GetType;
    g_d3dSurfaceVtbl.Register          = IDirect3DSurface8_Register;
    g_d3dSurfaceVtbl.GetContainer      = IDirect3DSurface8_GetContainer;
    g_d3dSurfaceVtbl.GetDesc           = IDirect3DSurface8_GetDesc;
    g_d3dSurfaceVtbl.LockRect          = IDirect3DSurface8_LockRect;
    g_d3dSurfaceVtbl.UnlockRect        = IDirect3DSurface8_UnlockRect;
     
    g_d3dVertexBufferVtbl.AddRef      = IDirect3DVertexBuffer8_AddRef;
    g_d3dVertexBufferVtbl.Release     = IDirect3DVertexBuffer8_Release;
    g_d3dVertexBufferVtbl.GetType     = IDirect3DVertexBuffer8_GetType;
    g_d3dVertexBufferVtbl.Register    = IDirect3DVertexBuffer8_Register;
    g_d3dVertexBufferVtbl.GetDesc     = IDirect3DVertexBuffer8_GetDesc;
    g_d3dVertexBufferVtbl.Lock        = IDirect3DVertexBuffer8_Lock;
    g_d3dVertexBufferVtbl.Unlock      = IDirect3DVertexBuffer8_Unlock;

    g_d3dPushBufferVtbl.AddRef        = IDirect3DPushBuffer8_AddRef;
    g_d3dPushBufferVtbl.Release       = IDirect3DPushBuffer8_Release;
    g_d3dPushBufferVtbl.GetType       = IDirect3DPushBuffer8_GetType;
    g_d3dPushBufferVtbl.Register      = IDirect3DPushBuffer8_Register;
    g_d3dPushBufferVtbl.BlockUntilNotBusy = 
        IDirect3DPushBuffer8_BlockUntilNotBusy;
    g_d3dPushBufferVtbl.IsBusy        = IDirect3DPushBuffer8_IsBusy;
    g_d3dPushBufferVtbl.GetSize       = IDirect3DPushBuffer8_GetSize;

    g_d3d.DisplayModeCount = 0;
    
    Direct3D_EnumDisplayModes(15);
    Direct3D_EnumDisplayModes(16);
    Direct3D_EnumDisplayModes(32);

    if (g_d3d.DisplayModeCount == 0) {
        assert(false);
        return NULL;
    }

    g_d3d.CurrentDisplayMode = g_d3d.DisplayModeCount - 1;
    D3DDISPLAYMODE* pDM = &g_d3d.DisplayModes[g_d3d.CurrentDisplayMode];
    // set the current mode to whatever the last retrieved mode was
    if (XVideoSetMode(pDM->Width, pDM->Height, 
                      D3D_FormatBPP(pDM->Format), pDM->RefreshRate) == FALSE)
    {
        assert(false);
        return NULL;
    }

    g_d3d.KickOffSize = 32 * 1024 / sizeof(DWORD);
    return &g_d3d.iface;
}
