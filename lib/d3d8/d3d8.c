#include <assert.h>
#include <stdlib.h>

#include <hal/video.h>
#include <pbkit/pbkit.h>
#include <xboxkrnl/xboxkrnl.h>
#include <strings.h>

#include "d3d8.h"

#define MASK(mask, val) (((val) << (ffs(mask)-1)) & (mask))

static Direct3DResourceVtbl8     g_d3dResourceVtbl;
static Direct3DBaseTextureVtbl8  g_d3dBaseTextureVtbl;
static Direct3DSurfaceVtbl8      g_d3dSurfaceVtbl;
static Direct3DVertexBufferVtbl8 g_d3dVertexBufferVtbl;

PVOID D3D_AllocContiguousMemory(DWORD Size, DWORD Alignment) {
    return MmAllocateContiguousMemoryEx(Size, 0, MAXRAM, Alignment, PAGE_READWRITE | PAGE_WRITECOMBINE);
}

void D3D_FreeContiguousMemory(PVOID Base) {
    MmFreeContiguousMemory(Base);
}

struct D3DSurface;
typedef struct D3DSurface D3DSurface;

typedef struct D3DDevice IMPLEMENTS(IDirect3DDevice8) {
    IDirect3DDevice8 iface;
    ULONG refcount;
    D3DSurface* surfaces[3];
    D3DSurface* depth_stencil_surface;
} D3DDevice;

typedef struct Direct3D IMPLEMENTS(IDirect3D8) {
    IDirect3D8 iface;
    D3DDISPLAYMODE displayModes[51];
    DWORD dwDisplayModeCount;
    DWORD dwCurrentDisplayMode;
#ifdef __cplusplus
    UINT GetAdapterCount() override { 
        return iface->lpVtbl->GetAdapterCount(this->iface); 
    }
    HRESULT GetAdapterIdentifier(
        UINT Adapter, DWORD Flags, 
        D3DADAPTER_IDENTIFIER8* pIdentifier) override 
    { 
        return iface->lpVtbl->GetAdapterIdentifier(this->iface, Adapter, 
                                                   Flags, pIdentifier);
    }
    UINT GetAdapterModeCount(UINT Adapter) override { 
        return iface->lpVtbl->GetAdapterModeCount(this->iface, Adapter); }
    HRESULT EnumAdapterModes(UINT Adapter, UINT Mode, 
                             D3DDISPLAYMODE* pMode) override 
    { 
        return iface->lpVtbl->EnumAdapterModes(this->iface, Adapter, 
                                               Mode, pMode);
    }
    HRESULT GetAdapterDisplayMode(UINT Adapter, 
                                  D3DDISPLAYMODE* pMode) override 
    {
        return iface->lpVtbl->GetAdapterDisplayMode(this->iface, Adapter, 
                                                    pMode);
    }
    HRESULT CheckDeviceType(UINT Adapter, D3DDEVTYPE CheckType, 
                            D3DFORMAT DisplayFormat, 
                            D3DFORMAT BackBufferFormat, 
                            BOOL Windowed) override 
    { 
       return iface->lpVtbl->CheckDeviceType(this->iface, Adapter, 
                                             CheckDeviceType, DisplayFormat,
                                             BackBufferFormat, Windowed); 

    }
    HRESULT CheckDeviceFormat(UINT Adapter, D3DDEVTYPE DeviceType, 
                              D3DFORMAT AdapterFormat, DWORD Usage, 
                              D3DRESOURCETYPE RType, 
                              D3DFORMAT CheckFormat) override 
    { 
        return iface->lpVtbl->CheckDeviceFormat(this->iface, Adapter, 
                                                DeviceType, AdapterFormat,
                                                Usage, RType, CheckFormat); 
    }
    HRESULT CheckDeviceMultiSampleType(
        UINT Adapter, D3DDEVTYPE DeviceType, 
        D3DFORMAT SurfaceFormat, BOOL Windowed, 
        D3DMULTISAMPLE_TYPE MultiSampleType) override 
    {
        return iface->lpVtbl->CheckDeviceMultiSampleType(this->iface, Adapter,
                                                         DeviceType, 
                                                         SurfaceFormat,
                                                         Windowed, 
                                                         MultiSampleType);
    }
    HRESULT CheckDepthStencilMatch(UINT Adapter, D3DDEVTYPE DeviceType,
                                   D3DFORMAT AdapterFormat, 
                                   D3DFORMAT RenderTargetFormat, 
                                   D3DFORMAT DepthStencilFormat) override 
    {
        return iface->lpVtbl->CheckDepthStencilMatch(this->iface, Adapter, 
                                                     DeviceType, AdapterFormat,
                                                     RenderTargetFormat, 
                                                     DepthStencilFormat); 
    }
    HRESULT GetDeviceCaps(UINT Adapter, D3DDEVTYPE DeviceType, 
                          D3DCAPS8* pCaps) override 
    { 
        return iface->lpVtbl->GetDeviceCaps(this->iface, Adapter, 
                                            DeviceType, pCaps);

    }
    HMONITOR GetAdapterMonitor(UINT Adapter) override {
        return iface->lpVtbl->GetAdapterMonitor(this->iface, Adapter);
    }
    HRESULT CreateDevice(
        UINT Adapter, D3DDEVTYPE DeviceType, HWND hFocusWindow, 
        DWORD BehaviorFlags, D3DPRESENT_PARAMETERS* pPresentationParameters, 
        LPDIRECT3DDEVICE8* ppReturnedDeviceInterface) override 
    { 
        return iface->lpVtbl->CreateDevice(this->iface, Adapter, DeviceType,
                                           hFocusWindow, BehaviorFlags,
                                           pPresentationParameters,
                                           ppReturnedDeviceInterface); 
    }
#endif
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

HRESULT Direct3D_CheckDeviceType(UINT Adapter, 
                                 D3DDEVTYPE CheckType, 
                                 D3DFORMAT DisplayFormat,
                                 D3DFORMAT BackBufferFormat)
{
    if (Adapter != D3DADAPTER_DEFAULT)
        return D3DERR_INVALIDCALL;

    if (CheckType != D3DDEVTYPE_HAL)
        return D3DERR_INVALIDDEVICE;

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
    if (Windowed != FALSE)
        return D3DERR_INVALIDCALL;

    return Direct3D_CheckDeviceType(Adapter, CheckType, 
                                    DisplayFormat, BackBufferFormat);
}

UINT Direct3D_GetAdapterModeCount(UINT Adapter) {
    if (Adapter != D3DADAPTER_DEFAULT)
        return 0;
    
    return g_d3d.dwDisplayModeCount;
}

UINT IDirect3D8_GetAdapterModeCount(LPDIRECT3D8 pThis, UINT Adapter) {
    assert(pThis == &g_d3d.iface);
    return Direct3D_GetAdapterModeCount(Adapter);
}

HRESULT Direct3D_EnumAdapterModes(UINT Adapter, UINT Mode, 
                                  D3DDISPLAYMODE* pMode) 
{
    if (Adapter != D3DADAPTER_DEFAULT)
        return D3DERR_INVALIDCALL;

    if (Mode >= g_d3d.dwDisplayModeCount)
        return D3DERR_INVALIDCALL;

    memcpy(pMode, &g_d3d.displayModes[Mode], sizeof(*pMode));
    return D3D_OK;
}

HRESULT IDirect3D8_EnumAdapterModes(LPDIRECT3D8 pThis, UINT Adapter, 
                                    UINT Mode, D3DDISPLAYMODE* pMode) 
{
    assert(pThis == &g_d3d.iface);
    return Direct3D_EnumAdapterModes(Adapter, Mode, pMode);
}

HRESULT Direct3D_GetAdapterDisplayMode(UINT Adapter, 
                                       D3DDISPLAYMODE* pMode) 
{
    if (Adapter != D3DADAPTER_DEFAULT)
        return D3DERR_INVALIDCALL;

    memcpy(pMode, &g_d3d.displayModes[g_d3d.dwCurrentDisplayMode], sizeof(*pMode));
    return D3D_OK;
}

HRESULT IDirect3D8_GetAdapterDisplayMode(LPDIRECT3D8 pThis, UINT Adapter, 
                                         D3DDISPLAYMODE* pMode)
{
    assert(pThis == &g_d3d.iface);
    return Direct3D_GetAdapterDisplayMode(Adapter, pMode);
}

HRESULT Direct3D_SetPushBufferSize(DWORD PushBufferSize) {
    if (PushBufferSize < 64*1024)
        return D3DERR_INVALIDCALL;
    
    pb_size(PushBufferSize);
    return D3D_OK;
}

HRESULT Direct3D_CheckDepthStencilMatch(D3DFORMAT AdapterFormat, 
                                        D3DFORMAT RenderTargetFormat, 
                                        D3DFORMAT DepthStencilFormat)
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
    if (Adapter != D3DADAPTER_DEFAULT)
        return D3DERR_INVALIDCALL;
    if (DeviceType != D3DDEVTYPE_HAL)
        return D3DERR_INVALIDCALL;
    return Direct3D_CheckDepthStencilMatch(AdapterFormat, RenderTargetFormat, DepthStencilFormat);
}

HRESULT IDirect3D8_SetPushBufferSize(LPDIRECT3D8 pThis, 
                                     DWORD PushBufferSize, 
                                     DWORD KickOffSize) 
{
    assert(pThis == &g_d3d.iface);

    if ((PushBufferSize % KickOffSize != 0) || 
        (PushBufferSize / KickOffSize < 4))
    {
        return D3DERR_INVALIDCALL;
    }
    return Direct3D_SetPushBufferSize(PushBufferSize);
}

Direct3DDeviceVtbl8 g_d3ddevVtbl;

static int D3D_FormatBPP(D3DFORMAT fmt) {
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

static int D3D_FormatBytesPerPixel(D3DFORMAT fmt) {
    return (D3D_FormatBPP(fmt)+7)/8;
}

BOOL Direct3D_IsDisplayModeValid(D3DDISPLAYMODE* mode, int* pIndex) {
    for (int i = 0; i < g_d3d.dwDisplayModeCount; i++) {
        if (memcmp(mode, &g_d3d.displayModes[i], sizeof(*mode)) == 0) {
            if (pIndex)
                *pIndex = i;
            return TRUE;
        }
    }
    return FALSE;
}

D3DDevice g_d3ddev;

typedef struct D3DLock {
    BOOL  bLocked;
    RECT  Rect;
    DWORD Flags;
} D3DLock;

typedef struct D3DSurface IMPLEMENTS(IDirect3DSurface8) {
    IDirect3DSurface8 iface;
    ULONG             refcount;
    BOOL              bManuallyRegistered;
    DWORD             Data;
    PVOID             pContiguousMemory;
    D3DSURFACE_DESC   desc;
    D3DLock           lock;
} D3DSurface;

typedef struct D3DVertexBuffer IMPLEMENTS(IDirect3DVertexBuffer8) {
    IDirect3DVertexBuffer8 iface;
    ULONG                  refcount;
    BOOL                   bManuallyRegistered;
    DWORD                  Data;
    PVOID                  pContiguousMemory;
    D3DVERTEXBUFFER_DESC   desc;
    D3DLock                lock;
} D3DVertexBuffer;

static D3DSurface* D3D_CreateSurface(UINT Width, UINT Height, D3DFORMAT Format,
                                     DWORD Usage, 
                                     D3DMULTISAMPLE_TYPE MultiSampleType, 
                                     PVOID pContiguousMemory) 
{
    D3DSurface* pSurf = malloc(sizeof(*pSurf));
    if (!pSurf)
        return NULL;
    pSurf->iface.lpVtbl = &g_d3dSurfaceVtbl;
    pSurf->refcount = 1;
    pSurf->bManuallyRegistered = TRUE;
    pSurf->Data = 0;
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
    pSurf->lock.bLocked = FALSE;
    IDirect3DSurface8_Register(&pSurf->iface, pContiguousMemory);
    return pSurf;
}

HRESULT Direct3D_CreateDevice(
    UINT Adapter, DWORD BehaviorFlags, 
    D3DPRESENT_PARAMETERS* pPresentationParameters)
{
    if (g_d3ddev.refcount != 0)
        return D3DERR_DEVICENOTRESET;

    if (Adapter != D3DADAPTER_DEFAULT)
        return D3DERR_INVALIDCALL;

    if (BehaviorFlags != D3DCREATE_HARDWARE_VERTEXPROCESSING)
        return D3DERR_INVALIDCALL;

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

    if (pPresentationParameters->FullScreen_PresentationInterval == D3DPRESENT_INTERVAL_FOUR)
        return D3DERR_INVALIDCALL;

    for (int i = 0; i < 3; i++) {
        if (pPresentationParameters->BufferSurfaces[i] != NULL)
            return D3DERR_INVALIDCALL;
    }

    if (pPresentationParameters->BackBufferWidth == 0 || pPresentationParameters->BackBufferHeight == 0)
        return D3DERR_INVALIDCALL;

    int bpp = D3D_FormatBPP(pPresentationParameters->BackBufferFormat);
    if (bpp == 0)
        return D3DERR_INVALIDCALL;
    
    D3DDISPLAYMODE mode;
    mode.Width       = pPresentationParameters->BackBufferWidth;
    mode.Height      = pPresentationParameters->BackBufferHeight;
    mode.RefreshRate = pPresentationParameters->FullScreen_RefreshRateInHz;
    mode.Format      = pPresentationParameters->BackBufferFormat;
    mode.Flags       = pPresentationParameters->Flags;
    int i = 0;
    if (Direct3D_IsDisplayModeValid(&mode, &i) == 0) {
        int refresh = pPresentationParameters->FullScreen_RefreshRateInHz;
        if (pPresentationParameters->FullScreen_RefreshRateInHz == D3DPRESENT_RATE_DEFAULT)
            refresh = REFRESH_DEFAULT;
        if (XVideoSetMode(
            pPresentationParameters->BackBufferWidth,
            pPresentationParameters->BackBufferHeight, 
            bpp, refresh
        ) == FALSE)
        {
            return D3DERR_INVALIDCALL;
        }
        g_d3d.dwCurrentDisplayMode = i;
    }
    else {
        return D3DERR_INVALIDCALL;
    }

    if (pPresentationParameters->EnableAutoDepthStencil == TRUE && 
        Direct3D_CheckDepthStencilMatch(
            g_d3d.displayModes[g_d3d.dwCurrentDisplayMode].Format,
            pPresentationParameters->BackBufferFormat,
            pPresentationParameters->AutoDepthStencilFormat
        ) != D3D_OK) 
    {
        return D3DERR_INVALIDCALL;
    }

    if (pPresentationParameters->DepthStencilSurface) {
        D3DSurface* pDSSurf = (D3DSurface*)pPresentationParameters->DepthStencilSurface;
        g_d3ddev.depth_stencil_surface = 
            (D3DSurface*)pDSSurf;
        pb_set_ds_addr(pDSSurf->pContiguousMemory);
    } else if (pPresentationParameters->EnableAutoDepthStencil) {
        HRESULT hr = IDirect3DDevice8_CreateDepthStencilSurface(
            NULL,
            pPresentationParameters->BackBufferWidth, 
            pPresentationParameters->BackBufferHeight, 
            pPresentationParameters->AutoDepthStencilFormat, 
            0, (LPDIRECT3DSURFACE8*)&g_d3ddev.depth_stencil_surface);
        if (hr != D3D_OK)
            return hr;
    }
    
    pb_back_buffer_count(pPresentationParameters->BackBufferCount);
    pb_set_color_format(pPresentationParameters->BackBufferFormat, false);
    if (pb_init() != 0)
        return D3DERR_NOTAVAILABLE;

    UINT BackBufferCount = pPresentationParameters->BackBufferCount;
    if (BackBufferCount == 0)
        BackBufferCount = 1;
    for (i = 0; i < BackBufferCount + 1; i++) {
        if (pPresentationParameters->BufferSurfaces[i] != NULL)
            return D3DERR_INVALIDCALL;

        g_d3ddev.surfaces[i] = D3D_CreateSurface(
            pPresentationParameters->BackBufferWidth, 
            pPresentationParameters->BackBufferHeight, 
            pPresentationParameters->BackBufferFormat,
            D3DUSAGE_RENDERTARGET, 
            pPresentationParameters->MultiSampleType,
            (PVOID)pb_fb_addr(i));

        if (!g_d3ddev.surfaces[i])
            return E_OUTOFMEMORY;
    }
        
    if (pPresentationParameters->EnableAutoDepthStencil) {
        D3DSurface* pDSSurf = D3D_CreateSurface(
            pPresentationParameters->BackBufferWidth, 
            pPresentationParameters->BackBufferHeight, 
            pPresentationParameters->AutoDepthStencilFormat,
            D3DUSAGE_DEPTHSTENCIL, 
            pPresentationParameters->MultiSampleType,
            (PVOID)pb_ds_addr());
        
        if (!pDSSurf)
            return E_OUTOFMEMORY;
        
        g_d3ddev.depth_stencil_surface = pDSSurf;
    }
    pb_show_front_screen();

    g_d3ddev.refcount = 1;
    g_d3ddev.iface.lpVtbl = &g_d3ddevVtbl;

    return D3D_OK;
}

HRESULT IDirect3D8_CreateDevice(
    LPDIRECT3D8 pThis, UINT Adapter, D3DDEVTYPE DeviceType, 
    HWND hFocusWindow, DWORD BehaviorFlags, 
    D3DPRESENT_PARAMETERS* pPresentationParameters,
    LPDIRECT3DDEVICE8* ppReturnedDevice)
{
    assert(pThis == &g_d3d.iface);
    if (DeviceType != D3DDEVTYPE_HAL)
        return D3DERR_INVALIDCALL;
    if (hFocusWindow != NULL) 
        return D3DERR_INVALIDCALL;

    HRESULT hr = Direct3D_CreateDevice(Adapter, BehaviorFlags, 
                                       pPresentationParameters);
    *ppReturnedDevice = &g_d3ddev.iface;
    return hr;
}

DIRECT3DVTBL8 g_d3dvtbl;

BOOL g_firstCreate = TRUE;

// ============================================================================
ULONG IDirect3DSurface8_AddRef(LPDIRECT3DSURFACE8 pThis) {
    D3DSurface* surface = (D3DSurface*)pThis;
    surface->refcount++;
    return surface->refcount;
}

ULONG IDirect3DSurface8_Release(LPDIRECT3DSURFACE8 pThis) {
    D3DSurface* surface = (D3DSurface*)pThis;
    if(surface->refcount == 0)
        return 0;
    surface->refcount--;
    if (surface->refcount > 0)
        return surface->refcount;

    if (surface->bManuallyRegistered != TRUE) 
        D3D_FreeContiguousMemory(surface->pContiguousMemory);

    return 0;
}

D3DRESOURCETYPE IDirect3DSurface8_GetType(LPDIRECT3DSURFACE8 pThis) {
    return ((D3DSurface*)pThis)->desc.Type;
}

VOID IDirect3DSurface8_Register(LPDIRECT3DSURFACE8 pThis, PVOID pBase) {
    D3DSurface* surface = (D3DSurface*)pThis;
    surface->pContiguousMemory = (PVOID)((DWORD)pBase + surface->Data);
    surface->Data += (DWORD)MmGetPhysicalAddress(pBase);
}

HRESULT IDirect3DSurface8_GetContainer(LPDIRECT3DSURFACE8 pThis, 
                                       LPDIRECT3DBASETEXTURE8* ppContainer)
{
    *ppContainer = NULL;
    return E_FAIL;
}
HRESULT IDirect3DSurface8_GetDesc(LPDIRECT3DSURFACE8 pThis, 
                                  D3DSURFACE_DESC* pDesc)
{
    memcpy(pDesc, &((D3DSurface*)pThis)->desc, sizeof(*pDesc));
    return D3D_OK;
}
HRESULT IDirect3DSurface8_LockRect(LPDIRECT3DSURFACE8 pThis, 
                                   D3DLOCKED_RECT* pLockedRect, 
                                   const RECT* pRect, DWORD Flags)
{
    D3DSurface* surface   = (D3DSurface*)pThis;
    surface->lock.Flags   =  Flags;
    surface->lock.Rect    = *pRect;
    surface->lock.bLocked =  TRUE;

    DWORD bytesPerPixel = D3D_FormatBytesPerPixel(surface->desc.Format);
    DWORD width;
    if (pRect)
        width = pRect->right - pRect->left;
    else
        width = surface->desc.Width;
    
    pLockedRect->Pitch = width * bytesPerPixel;

    if (pRect == NULL) {
        pLockedRect->pBits = surface->pContiguousMemory;
    }
    else {
        pLockedRect->pBits = (PVOID)(((DWORD)surface->pContiguousMemory + 
                                      ((pLockedRect->Pitch * pRect->top) 
                                        + pRect->left) 
                                      * bytesPerPixel)); 
    }
    return D3D_OK;
}

HRESULT IDirect3DSurface8_UnlockRect(LPDIRECT3DSURFACE8 pThis) {
    ((D3DSurface*)pThis)->lock.bLocked = FALSE;
    return D3D_OK;
}
// ============================================================================

// ============================================================================
ULONG IDirect3DVertexBuffer8_AddRef(LPDIRECT3DVERTEXBUFFER8 pThis) {
    D3DVertexBuffer* vb = (D3DVertexBuffer*)pThis;
    vb->refcount++;
    return vb->refcount;
}

ULONG IDirect3DVertexBuffer8_Release(LPDIRECT3DVERTEXBUFFER8 pThis) {
    D3DVertexBuffer* vb = (D3DVertexBuffer*)pThis;
    if(vb->refcount == 0)
        return 0;
    vb->refcount--;
    if (vb->refcount > 0)
        return vb->refcount;

    if (vb->bManuallyRegistered != TRUE) 
        D3D_FreeContiguousMemory(vb->pContiguousMemory);

    return 0;
}

D3DRESOURCETYPE IDirect3DVertexBuffer8_GetType(LPDIRECT3DVERTEXBUFFER8 pThis) {
    return ((D3DVertexBuffer*)pThis)->desc.Type;
}

VOID IDirect3DVertexBuffer8_Register(LPDIRECT3DVERTEXBUFFER8 pThis, PVOID pBase) {
    D3DVertexBuffer* vb = (D3DVertexBuffer*)pThis;
    vb->pContiguousMemory = (PVOID)((DWORD)pBase + vb->Data);
    vb->Data += (DWORD)MmGetPhysicalAddress(pBase);
}

HRESULT IDirect3DVertexBuffer8_GetContainer(LPDIRECT3DVERTEXBUFFER8 pThis, 
                                            LPDIRECT3DBASETEXTURE8* ppContainer)
{
    *ppContainer = NULL;
    return E_FAIL;
}
HRESULT IDirect3DVertexBuffer8_GetDesc(LPDIRECT3DVERTEXBUFFER8 pThis, 
                                       D3DVERTEXBUFFER_DESC* pDesc)
{
    memcpy(pDesc, &((D3DVertexBuffer*)pThis)->desc, sizeof(*pDesc));
    return D3D_OK;
}
HRESULT IDirect3DVertexBuffer8_Lock(LPDIRECT3DVERTEXBUFFER8 pThis, 
                                      UINT OffsetToLock, UINT SizeToLock,
                                      BYTE** ppbData, DWORD Flags)
{
    assert(false);
    return E_FAIL;
}

HRESULT IDirect3DVertexBuffer8_Unlock(LPDIRECT3DVERTEXBUFFER8 pThis) {
    ((D3DVertexBuffer*)pThis)->lock.bLocked = FALSE;
    return D3D_OK;
}
// ============================================================================

ULONG D3DDevice_AddRef() {
    g_d3ddev.refcount++;
    return g_d3ddev.refcount;
}

ULONG IDirect3DDevice8_AddRef(LPDIRECT3DDEVICE8 pThis) {
    assert(pThis == &g_d3ddev.iface);
    return D3DDevice_AddRef();
}

ULONG D3DDevice_Release() {
    if(g_d3ddev.refcount == 0)
        return 0;
    g_d3ddev.refcount--;
    if (g_d3ddev.refcount > 0)
        return g_d3ddev.refcount;

    pb_kill();
    return 0;
}

ULONG IDirect3DDevice8_Release(LPDIRECT3DDEVICE8 pThis) {
    assert(pThis == &g_d3ddev.iface);
    return D3DDevice_Release();
}

HRESULT IDirect3DDevice8_CreateImageSurface(LPDIRECT3DDEVICE8 pThis,
                                            UINT Width, UINT Height,
                                            D3DFORMAT Format, 
                                            LPDIRECT3DSURFACE8* ppSurface)
{
    D3DSurface* surf = malloc(sizeof(*surf));
    surf->iface.lpVtbl = &g_d3dSurfaceVtbl;
    surf->bManuallyRegistered = FALSE;
    surf->Data = 0;
    surf->lock.bLocked = FALSE;
    UINT Size = Width * Height * D3D_FormatBytesPerPixel(Format);
    surf->pContiguousMemory = D3D_AllocContiguousMemory(Size, D3DSURFACE_ALIGNMENT);
    if (surf->pContiguousMemory == NULL) {
        free(surf);
        return E_OUTOFMEMORY;
    }
    surf->desc.Format             = Format;
    surf->desc.Type               = D3DRTYPE_SURFACE;
    surf->desc.Usage              = 0;
    surf->desc.Size               = Size;
    surf->desc.MultiSampleType    = 0;
    surf->desc.Width              = Width;
    surf->desc.Height             = Height;
    *ppSurface = &surf->iface;
    return D3D_OK;
}

HRESULT IDirect3DDevice8_CreateDepthStencilSurface(
    LPDIRECT3DDEVICE8 pThis, UINT Width, UINT Height, D3DFORMAT Format,
    D3DMULTISAMPLE_TYPE MultiSample, IDirect3DSurface8** ppSurface)
{
    return IDirect3DDevice8_CreateImageSurface(pThis, Width, Height, 
                                               Format, ppSurface);
}

HRESULT D3DDevice_CreateVertexBuffer(UINT Length, LPDIRECT3DVERTEXBUFFER8* ppVertexBuffer) {
    D3DVertexBuffer* vb = malloc(sizeof(*vb));
    vb->iface.lpVtbl = &g_d3dVertexBufferVtbl;
    vb->bManuallyRegistered = FALSE;
    vb->Data = 0;
    vb->lock.bLocked = FALSE;
    vb->pContiguousMemory = D3D_AllocContiguousMemory(Length, D3DVERTEXBUFFER_ALIGNMENT);
    if (vb->pContiguousMemory == NULL) {
        free(vb);
        return E_OUTOFMEMORY;
    }
    vb->desc.Format               = D3DFMT_VERTEXDATA;
    vb->desc.Type                 = D3DRTYPE_VERTEXBUFFER;
    *ppVertexBuffer = &vb->iface;
    return D3D_OK;
}

HRESULT IDirect3DDevice8_CreateVertexBuffer(
    LPDIRECT3DDEVICE8 pThis, UINT Length, DWORD Usage, DWORD FVF, 
    D3DPOOL Pool, LPDIRECT3DVERTEXBUFFER8* ppVertexBuffer)
{
    assert(pThis == &g_d3ddev.iface);
    return D3DDevice_CreateVertexBuffer(Length, ppVertexBuffer);
}

void D3DDevice_BlockUntilVerticalBlank() {
    pb_wait_for_vbl();
}

void IDirect3DDevice8_BlockUntilVerticalBlank(LPDIRECT3DDEVICE8 pThis) {
    assert(pThis == &g_d3ddev.iface);
    return D3DDevice_BlockUntilVerticalBlank();
}

HRESULT D3DDevice_BeginScene() {
    pb_reset();
    pb_target_back_buffer();
    /* Clear depth & stencil buffers */
    D3DSURFACE_DESC ds_desc;
    HRESULT hr = IDirect3DSurface8_GetDesc(&g_d3ddev.depth_stencil_surface->iface, &ds_desc);
    pb_erase_depth_stencil_buffer(0, 0, ds_desc.Width, ds_desc.Height);
    pb_fill(0, 0, ds_desc.Width, ds_desc.Height, 0x00000000);
    while(pb_busy()) {
        /* Wait for completion... */
    }
    uint32_t* p = pb_begin();
    pb_push(p++, NV097_SET_VERTEX_DATA_ARRAY_FORMAT,16);
    for(int i = 0; i < 16; i++) {
        *(p++) = NV097_SET_VERTEX_DATA_ARRAY_FORMAT_TYPE_F;
    }
    pb_end(p);
    return D3D_OK;
}

HRESULT IDirect3DDevice8_BeginScene(LPDIRECT3DDEVICE8 pThis) {
    assert(pThis == &g_d3ddev.iface);
    return D3DDevice_BeginScene();
}

HRESULT D3DDevice_EndScene() {
    while(pb_busy()) {
        /* Wait for completion... */
    }
    return D3D_OK;
}

HRESULT IDirect3DDevice8_EndScene(LPDIRECT3DDEVICE8 pThis) {
    assert(pThis == &g_d3ddev.iface);
    return D3DDevice_EndScene();
}

HRESULT D3DDevice_Present(CONST RECT* pSourceRect, CONST RECT* pDestRect) {
    // TODO: implement
    if (pSourceRect || pDestRect)
        return D3DERR_INVALIDCALL;

    while (pb_finished()) {
        /* Not ready to swap yet */
    }
    return D3D_OK;
}

HRESULT IDirect3DDevice8_Present(LPDIRECT3DDEVICE8 pThis, 
                                 CONST RECT* pSourceRect, 
                                 CONST RECT* pDestRect) 
{
    assert(pThis == &g_d3ddev.iface);
    return D3DDevice_Present(pSourceRect, pDestRect);
}

HRESULT D3DDevice_DrawPrimitive(D3DPRIMITIVETYPE PrimitiveType, 
                      UINT StartVertex, UINT PrimitiveCount) 
{
    if (PrimitiveType != D3DPT_TRIANGLELIST)
        return D3DERR_INVALIDCALL;
    
    uint32_t *p = pb_begin();
    p = pb_push1(p, NV097_SET_BEGIN_END, PrimitiveType);

    p = pb_push1(p, 0x40000000|NV097_DRAW_ARRAYS,
                 MASK(NV097_DRAW_ARRAYS_COUNT, (PrimitiveCount-1)) | 
                 MASK(NV097_DRAW_ARRAYS_START_INDEX, StartVertex * 3));

    p = pb_push1(p, NV097_SET_BEGIN_END, NV097_SET_BEGIN_END_OP_END);
    pb_end(p);
    return D3D_OK;
}

HRESULT IDirect3DDevice8_DrawPrimitive(LPDIRECT3DDEVICE8 pThis, 
                                       D3DPRIMITIVETYPE PrimitiveType, 
                                       UINT StartVertex, UINT PrimitiveCount)
{
    assert(pThis == &g_d3ddev.iface);
    return D3DDevice_DrawPrimitive(PrimitiveType, StartVertex, PrimitiveCount);
}

HRESULT D3DDevice_SetViewport(CONST D3DVIEWPORT8* pViewport) {
    pb_set_viewport(pViewport->X, pViewport->Y, pViewport->Width, 
                    pViewport->Height, pViewport->MinZ, pViewport->MaxZ);
    return D3D_OK;
}

HRESULT IDirect3DDevice8_SetViewport(LPDIRECT3DDEVICE8 pThis, 
                                     CONST D3DVIEWPORT8* pViewport) 
{
    assert(pThis == &g_d3ddev.iface);
    return D3DDevice_SetViewport(pViewport);
}

BOOL D3D_IsVertexFormatFloat(DWORD dwFormat) {
    switch (dwFormat) {
    case D3DVSDT_FLOAT1:
    case D3DVSDT_FLOAT2:
    case D3DVSDT_FLOAT3:
    case D3DVSDT_FLOAT4:
    case D3DVSDT_D3DCOLOR:
    case D3DVSDT_FLOAT2H:
        return TRUE;
    default:
        return FALSE;
    }
}

HRESULT D3DDevice_SetVertexShaderInputDirect(
    D3DVERTEXATTRIBUTEFORMAT *pVAF,
    UINT StreamCount, 
    D3DSTREAM_INPUT *pStreamInputs)
{
    uint32_t *p = pb_begin();
    DWORD dwStride = 0;
    for (int i = 0; i < StreamCount; i++) {
        for (int j = 0; j < 16; j++) {
            D3DVERTEXSHADERINPUT* pInput = &pVAF->Input[i];
            if (pInput->StreamIndex != i)
                continue;
            if (pInput->Format == 0)
                continue;

            p = pb_push1(p, NV097_SET_VERTEX_DATA_ARRAY_FORMAT + j*4,
                     MASK(NV097_SET_VERTEX_DATA_ARRAY_FORMAT_TYPE | \
                            NV097_SET_VERTEX_DATA_ARRAY_FORMAT_SIZE, \
                          pInput->Format) | \
                     MASK(NV097_SET_VERTEX_DATA_ARRAY_FORMAT_STRIDE, pStreamInputs[i].Stride));
            BYTE* pData = ((D3DVertexBuffer*)pStreamInputs[i].VertexBuffer)->pContiguousMemory;
            pData += pStreamInputs[i].Offset;
            pData += pInput->Offset;
            p = pb_push1(p, NV097_SET_VERTEX_DATA_ARRAY_OFFSET + j*4, (uint32_t)pData & 0x03ffffff);
        }
        
    }

    pb_end(p);
    return D3D_OK;
}

HRESULT IDirect3DDevice8_SetVertexShaderInputDirect(
    LPDIRECT3DDEVICE8 pThis,
    D3DVERTEXATTRIBUTEFORMAT *pVAF,
    UINT StreamCount, 
    D3DSTREAM_INPUT *pStreamInputs) 
{
    assert(pThis == &g_d3ddev.iface);
    return D3DDevice_SetVertexShaderInputDirect(pVAF, StreamCount, 
                                                pStreamInputs);
}

static BOOL g_d3dCreated = FALSE;

LPDIRECT3D8 Direct3DCreate8(UINT SDKVersion) {
    if (SDKVersion != D3D_SDK_VERSION)
        return NULL;

    if (g_d3dCreated == TRUE)
        return &g_d3d.iface;

    g_d3dvtbl.AddRef                        = IDirect3D8_AddRef;
    g_d3dvtbl.Release                       = IDirect3D8_Release;
    g_d3dvtbl.GetAdapterCount               = IDirect3D8_GetAdapterCount;
    g_d3dvtbl.GetAdapterMonitor             = IDirect3D8_GetAdapterMonitor;
    g_d3dvtbl.GetAdapterModeCount           = IDirect3D8_GetAdapterModeCount;
    g_d3dvtbl.GetAdapterDisplayMode         = IDirect3D8_GetAdapterDisplayMode;
    g_d3dvtbl.EnumAdapterModes              = IDirect3D8_EnumAdapterModes;
    g_d3dvtbl.CheckDepthStencilMatch        = IDirect3D8_CheckDepthStencilMatch;
    g_d3dvtbl.SetPushBufferSize             = IDirect3D8_SetPushBufferSize;
    g_d3dvtbl.CreateDevice                  = IDirect3D8_CreateDevice;
    g_d3d.iface.lpVtbl = &g_d3dvtbl;

    g_d3ddevVtbl.AddRef                     = IDirect3DDevice8_AddRef;
    g_d3ddevVtbl.Release                    = IDirect3DDevice8_Release;
    g_d3ddevVtbl.CreateImageSurface         = IDirect3DDevice8_CreateImageSurface;
    g_d3ddevVtbl.CreateDepthStencilSurface  = IDirect3DDevice8_CreateDepthStencilSurface;
    g_d3ddevVtbl.CreateVertexBuffer         = IDirect3DDevice8_CreateVertexBuffer;
    g_d3ddevVtbl.BeginScene                 = IDirect3DDevice8_BeginScene;
    g_d3ddevVtbl.EndScene                   = IDirect3DDevice8_EndScene;
    g_d3ddevVtbl.Present                    = IDirect3DDevice8_Present;
    g_d3ddevVtbl.DrawPrimitive              = IDirect3DDevice8_DrawPrimitive;
    g_d3ddevVtbl.SetViewport                = IDirect3DDevice8_SetViewport;
    g_d3ddevVtbl.SetVertexShaderInputDirect = IDirect3DDevice8_SetVertexShaderInputDirect;
    g_d3ddev.iface.lpVtbl = &g_d3ddevVtbl;

    g_d3dResourceVtbl.AddRef                = IDirect3DResource8_AddRef;
    g_d3dResourceVtbl.Release               = IDirect3DResource8_Release;
    g_d3dResourceVtbl.GetType               = IDirect3DResource8_GetType;
    g_d3dResourceVtbl.Register              = IDirect3DResource8_Register;
     
    g_d3dBaseTextureVtbl.AddRef             = IDirect3DBaseTexture8_AddRef;
    g_d3dBaseTextureVtbl.Release            = IDirect3DBaseTexture8_Release;
    g_d3dBaseTextureVtbl.GetType            = IDirect3DBaseTexture8_GetType;
    g_d3dBaseTextureVtbl.Register           = IDirect3DBaseTexture8_Register;
    g_d3dBaseTextureVtbl.GetLevelCount      = IDirect3DBaseTexture8_GetLevelCount;
     
    g_d3dSurfaceVtbl.AddRef                 = IDirect3DSurface8_AddRef;
    g_d3dSurfaceVtbl.Release                = IDirect3DSurface8_Release;
    g_d3dSurfaceVtbl.GetType                = IDirect3DSurface8_GetType;
    g_d3dSurfaceVtbl.Register               = IDirect3DSurface8_Register;
    g_d3dSurfaceVtbl.GetContainer           = IDirect3DSurface8_GetContainer;
    g_d3dSurfaceVtbl.GetDesc                = IDirect3DSurface8_GetDesc;
    g_d3dSurfaceVtbl.LockRect               = IDirect3DSurface8_LockRect;
    g_d3dSurfaceVtbl.UnlockRect             = IDirect3DSurface8_UnlockRect;
     
    g_d3dVertexBufferVtbl.AddRef            = IDirect3DVertexBuffer8_AddRef;
    g_d3dVertexBufferVtbl.Release           = IDirect3DVertexBuffer8_Release;
    g_d3dVertexBufferVtbl.GetType           = IDirect3DVertexBuffer8_GetType;
    g_d3dVertexBufferVtbl.Register          = IDirect3DVertexBuffer8_Register;
    g_d3dVertexBufferVtbl.GetDesc           = IDirect3DVertexBuffer8_GetDesc;
    g_d3dVertexBufferVtbl.Lock              = IDirect3DVertexBuffer8_Lock;
    g_d3dVertexBufferVtbl.Unlock            = IDirect3DVertexBuffer8_Unlock;

    g_d3d.dwDisplayModeCount = 0;
    VIDEO_MODE vm;
    memset(&vm, 0, sizeof(vm));
    void* p = NULL;
    while(XVideoListModes(&vm, 15, REFRESH_DEFAULT, &p)) {
        D3DDISPLAYMODE* pDisplayMode = &g_d3d.displayModes[g_d3d.dwDisplayModeCount];
        DWORD dwFlags = (vm.interlaced == TRUE ? D3DPRESENTFLAG_INTERLACED 
                                               : D3DPRESENTFLAG_PROGRESSIVE)
                        | (vm.width == 640 ? 0 : D3DPRESENTFLAG_WIDESCREEN);
        pDisplayMode->Width       = vm.width;
        pDisplayMode->Height      = vm.height;
        pDisplayMode->RefreshRate = vm.refresh;
        pDisplayMode->Format      = D3DFMT_A1R5G5B5;
        pDisplayMode->Flags       = dwFlags;
        g_d3d.dwDisplayModeCount++;
    }
    p = NULL;
    while(XVideoListModes(&vm, 16, REFRESH_DEFAULT, &p)) {
        D3DDISPLAYMODE* pDisplayMode = &g_d3d.displayModes[g_d3d.dwDisplayModeCount];
        DWORD dwFlags = (vm.interlaced == TRUE ? D3DPRESENTFLAG_INTERLACED 
                                               : D3DPRESENTFLAG_PROGRESSIVE)
                        | (vm.width == 640 ? 0 : D3DPRESENTFLAG_WIDESCREEN);
        pDisplayMode->Width       = vm.width;
        pDisplayMode->Height      = vm.height;
        pDisplayMode->RefreshRate = vm.refresh;
        pDisplayMode->Format      = D3DFMT_R5G6B5;
        pDisplayMode->Flags       = dwFlags;
        g_d3d.dwDisplayModeCount++;
    }
    p = NULL;
    while(XVideoListModes(&vm, 32, REFRESH_DEFAULT, &p)) {
        D3DDISPLAYMODE* pDisplayMode = &g_d3d.displayModes[g_d3d.dwDisplayModeCount];
        DWORD dwFlags = (vm.interlaced == TRUE ? D3DPRESENTFLAG_INTERLACED 
                                               : D3DPRESENTFLAG_PROGRESSIVE)
                        | (vm.width == 640 ? 0 : D3DPRESENTFLAG_WIDESCREEN);
        pDisplayMode->Width       = vm.width;
        pDisplayMode->Height      = vm.height;
        pDisplayMode->RefreshRate = vm.refresh;
        pDisplayMode->Format      = D3DFMT_A8R8G8B8;
        pDisplayMode->Flags       = dwFlags;
        g_d3d.dwDisplayModeCount++;
    }
    // set the current mode to whatever the last retrieved mode was
    XVideoSetMode(vm.width, vm.height, vm.bpp, vm.refresh);
    g_d3d.dwCurrentDisplayMode = g_d3d.dwDisplayModeCount;

    return &g_d3d.iface;
}

// ============================================================================
typedef struct D3DResource IMPLEMENTS(IDirect3DResource8) {
    IDirect3DResource8 iface;
    ULONG              refcount;
    D3DRESOURCETYPE    type;
    BOOL               bManuallyRegistered;
    DWORD              Data;
    PVOID              pContiguousMemory;
} D3DResource;

ULONG IDirect3DResource8_AddRef(LPDIRECT3DRESOURCE8 pThis) {
    D3DResource* resource = (D3DResource*)pThis;
    resource->refcount++;
    return resource->refcount;
}

ULONG IDirect3DResource8_Release(LPDIRECT3DRESOURCE8 pThis) {
    D3DResource* resource = (D3DResource*)pThis;
    if(resource->refcount == 0)
        return 0;
    resource->refcount--;
    if (resource->refcount > 0)
        return resource->refcount;

    if (resource->bManuallyRegistered != TRUE) 
        D3D_FreeContiguousMemory(resource->pContiguousMemory);

    return 0;
}

D3DRESOURCETYPE IDirect3DResource8_GetType(LPDIRECT3DRESOURCE8 pThis) {
    return ((D3DResource*)pThis)->type;
}

VOID IDirect3DResource8_Register(LPDIRECT3DRESOURCE8 pThis, PVOID pBase) {
    D3DResource* resource = (D3DResource*)pThis;
    resource->pContiguousMemory = (PVOID)((DWORD)pBase + resource->Data);
    resource->Data += (DWORD)MmGetPhysicalAddress(pBase);
}
// ============================================================================

// ============================================================================
typedef struct D3DBaseTexture IMPLEMENTS(IDirect3DBaseTexture8) {
    IDirect3DBaseTexture8 iface;
    ULONG                 refcount;
    D3DRESOURCETYPE       type;
    BOOL                  bManuallyRegistered;
    DWORD                 Data;
    PVOID                 pContiguousMemory;
    DWORD                 dwLevelCount;
} D3DBaseTexture;

ULONG IDirect3DBaseTexture8_AddRef(LPDIRECT3DBASETEXTURE8 pThis) {
    D3DBaseTexture* tex = (D3DBaseTexture*)pThis;
    tex->refcount++;
    return tex->refcount;
}

ULONG IDirect3DBaseTexture8_Release(LPDIRECT3DBASETEXTURE8 pThis) {
    D3DBaseTexture* tex = (D3DBaseTexture*)pThis;
    if(tex->refcount == 0)
        return 0;
    tex->refcount--;
    if (tex->refcount > 0)
        return tex->refcount;

    if (tex->bManuallyRegistered != TRUE) 
        D3D_FreeContiguousMemory(tex->pContiguousMemory);

    return 0;
}

D3DRESOURCETYPE IDirect3DBaseTexture8_GetType(LPDIRECT3DBASETEXTURE8 pThis) {
    return ((D3DBaseTexture*)pThis)->type;
}

VOID IDirect3DBaseTexture8_Register(LPDIRECT3DBASETEXTURE8 pThis, PVOID pBase) {
    D3DBaseTexture* tex = (D3DBaseTexture*)pThis;
    tex->pContiguousMemory = (PVOID)((DWORD)pBase + tex->Data);
    tex->Data += (DWORD)MmGetPhysicalAddress(pBase);
}

DWORD IDirect3DBaseTexture8_GetLevelCount(LPDIRECT3DBASETEXTURE8 pThis) {
        return ((D3DBaseTexture*)pThis)->dwLevelCount;
}
// ============================================================================

