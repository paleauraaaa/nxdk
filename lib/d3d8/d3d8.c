#include <assert.h>
#include <stdlib.h>

#include <hal/video.h>
#include <pbkit/pbkit.h>
#include <xboxkrnl/xboxkrnl.h>

#include "d3d8.h"

// TODO: set this in build process
#ifndef NXDK_DEBUG
#define NXDK_DEBUG 1
#endif // NXDK_DEBUG
  
#define MASK(mask, val) (((val) << (__builtin_ffs(mask)-1)) & (mask))

static IDirect3DResourceVtbl8     g_d3dResourceVtbl;
static IDirect3DBaseTextureVtbl8  g_d3dBaseTextureVtbl;
static IDirect3DTextureVtbl8      g_d3dTextureVtbl;
static IDirect3DSurfaceVtbl8      g_d3dSurfaceVtbl;
static IDirect3DVertexBufferVtbl8 g_d3dVertexBufferVtbl;
static IDirect3DPushBufferVtbl8   g_d3dPushBufferVtbl;

PVOID D3D_AllocContiguousMemory(DWORD Size, DWORD Alignment) {
    return MmAllocateContiguousMemoryEx(Size, 0, 0x03FFB000, Alignment,
                                        PAGE_READWRITE | PAGE_WRITECOMBINE);
}

void D3D_FreeContiguousMemory(PVOID Base) {
    MmFreeContiguousMemory(Base);
}

// ============================================================================
typedef struct D3DRefcount {
    ULONG c;
} D3DRefcount;

ULONG D3DRefcount_AddRef(D3DRefcount* pThis) {
    return pThis->c++;
}

ULONG D3DRefcount_Release(D3DRefcount *pThis) {
    if (pThis->c == 0)
        return 0;

    return pThis->c--;
}
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

ULONG IDirect3DResource8_AddRef(LPDIRECT3DRESOURCE8 pThis) {
    return D3DRefcount_AddRef(&((D3DResource*)pThis)->inner.refcount);
}

ULONG IDirect3DResource8_Release(LPDIRECT3DRESOURCE8 pThis) {
    D3DResource* resource = (D3DResource*)pThis;
    ULONG refcount = D3DRefcount_Release(&resource->inner.refcount);
    if (refcount > 0)
        return refcount;

    if (resource->inner.bManuallyRegistered != TRUE) 
        D3D_FreeContiguousMemory(resource->inner.pContiguousMemory);

    free(resource);
    return 0;
}

D3DRESOURCETYPE IDirect3DResource8_GetType(LPDIRECT3DRESOURCE8 pThis) {
    return ((D3DResource*)pThis)->inner.type;
}

VOID IDirect3DResource8_Register(LPDIRECT3DRESOURCE8 pThis, PVOID pBase) {
    D3DResource* resource = (D3DResource*)pThis;
    resource->inner.pContiguousMemory = 
        (PVOID)((DWORD)pBase + resource->inner.Data);
    resource->inner.Data += (DWORD)MmGetPhysicalAddress(pBase);
}

BOOL IDirect3DResource8_IsBusy(LPDIRECT3DRESOURCE8 pThis) {
    D3DResource* resource = (D3DResource*)pThis;
    assert(false); // TODO: implement
    return FALSE;
}

VOID IDirect3DResource8_BlockUntilNotBusy(LPDIRECT3DRESOURCE8 pThis) {
    assert(false); // TODO: implement
}
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

ULONG IDirect3DPushBuffer8_AddRef(LPDIRECT3DPUSHBUFFER8 pThis) {
    return IDirect3DResource8_AddRef((LPDIRECT3DRESOURCE8)pThis);
}

ULONG IDirect3DPushBuffer8_Release(LPDIRECT3DPUSHBUFFER8 pThis) {
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

D3DRESOURCETYPE IDirect3DPushBuffer8_GetType(LPDIRECT3DPUSHBUFFER8 pThis) {
    return IDirect3DResource8_GetType((LPDIRECT3DRESOURCE8)pThis);
}

VOID IDirect3DPushBuffer8_Register(LPDIRECT3DPUSHBUFFER8 pThis, PVOID pBase) {
    IDirect3DResource8_Register((LPDIRECT3DRESOURCE8)pThis, pBase);
}

VOID IDirect3DPushBuffer8_BlockUntilNotBusy(LPDIRECT3DPUSHBUFFER8 pThis) {
    IDirect3DResource8_BlockUntilNotBusy((LPDIRECT3DRESOURCE8)pThis);   
}

BOOL IDirect3DPushBuffer8_IsBusy(LPDIRECT3DPUSHBUFFER8 pThis) {
    return IDirect3DResource8_IsBusy((LPDIRECT3DRESOURCE8)pThis);   
}

HRESULT IDirect3DPushBuffer8_GetSize(LPDIRECT3DPUSHBUFFER8 pThis, UINT* pSize) 
{
    *pSize = ((D3DPushBuffer*)pThis)->SizeNeeded;
    return D3D_OK;
}

BOOL D3DPushBuffer_IsFull(LPDIRECT3DPUSHBUFFER8 pThis) {
    D3DPushBuffer* pb = (D3DPushBuffer*)pThis;
    return pb->SizeNeeded >= pb->Size;
}

HRESULT D3DPushBuffer_Push1(
    D3DPushBuffer* pThis, DWORD dwData)
{
    pThis->SizeNeeded++;
    if (pThis->SizeNeeded > pThis->Size - 1)
        return D3DERR_BUFFERTOOSMALL;

    *(pThis->p++) = dwData;
    return D3D_OK;
}

HRESULT D3DPushBuffer_PushCmd(
    D3DPushBuffer* pThis, DWORD cmd, DWORD dwData)
{
    pThis->SizeNeeded += 2;
    if (pThis->SizeNeeded > pThis->Size - 2)
        return D3DERR_BUFFERTOOSMALL;

    pThis->p = (PDWORD)pb_push1((uint32_t*)pThis->p, cmd, dwData);
    return D3D_OK;
}

HRESULT D3DPushBuffer_PushN(
    D3DPushBuffer* pThis, DWORD dwData, SIZE_T n)
{
    pThis->SizeNeeded += n;
    if (pThis->SizeNeeded > pThis->Size - n)
        return D3DERR_BUFFERTOOSMALL;

    memset(pThis->p, dwData, n * sizeof(DWORD));
    pThis->p += n;
    return D3D_OK;
}

HRESULT D3DPushBuffer_PushA(
    D3DPushBuffer* pThis, CONST DWORD* pdwData, SIZE_T n)
{
    pThis->SizeNeeded += n;
    if (pThis->SizeNeeded > pThis->Size - n)
        return D3DERR_BUFFERTOOSMALL;

    memcpy(pThis->p, pdwData, n * sizeof(DWORD));
    pThis->p += n;
    return D3D_OK;
}
// ============================================================================

struct D3DSurface;
typedef struct D3DSurface D3DSurface;

struct D3DBaseTexture;
typedef struct D3DBaseTexture D3DBaseTexture;

// ============================================================================
typedef struct D3DTextureStageState {
    DWORD Address;
    BOOL  AddressDirty;
    DWORD Filter;
    BOOL  FilterDirty;
    DWORD Control0;
    BOOL  Control0Dirty;
    DWORD Format;
    BOOL  FormatDirty;
    DWORD Offset;
    BOOL  OffsetDirty;
} D3DTextureStageState;

typedef struct D3DDevice IMPLEMENTS(IDirect3DDevice8) {
    IDirect3DDevice8 iface;
    D3DRefcount refcount;
    DWORD KickOffSize;
    D3DSurface* surfaces[3];
    DWORD current_surface;
    DWORD max_surfaces;
    D3DSurface* depth_stencil_surface;
    D3DPushBuffer default_pb;
    D3DPushBuffer* current_pb;
    BOOL in_scene;
    D3DVIEWPORT8 viewport;
    UINT lastPresentVBlankCount;
    BOOL bUserSuppliedBufferSurfaces;
    BOOL bUserSuppliedDepthStencilSurface;
    DWORD Control0;
    UINT PresentInterval;
    D3DTextureStageState TextureStageState[D3DTSS_MAXSTAGES];
    BOOL TextureStageStateDirty[D3DTSS_MAXSTAGES]
                               [D3DTSS_DEFERRED_TEXTURE_STATE_MAX];
    D3DBaseTexture* pTexture[D3DTSS_MAXSTAGES];
#ifdef __cplusplus
    ULONG AddRef() override {
        return D3DDevice_AddRef();
    }

    ULONG Release() override {
        return D3DDevice_Release();
    }

    HRESULT CreateImageSurface(UINT Width, UINT Height,
                                   D3DFORMAT Format, 
                                   LPDIRECT3DSURFACE8* ppSurface) override 
    {
        return D3DDevice_CreateImageSurface(Width, Height, Format, ppSurface);
    }

    HRESULT CreateDepthStencilSurface(
        UINT Width, UINT Height, 
        D3DFORMAT Format, 
        D3DMULTISAMPLE_TYPE MultiSampleType,
        LPDIRECT3DSURFACE8* ppSurface) override 
    {
        return D3DDevice_CreateDepthStencilSurface(Width, Height, Format,
                                                   MultiSampleType, ppSurface);
    }

    HRESULT CreateVertexBuffer(
        UINT Length, DWORD Usage, DWORD FVF, 
        D3DPOOL Pool, LPDIRECT3DVERTEXBUFFER8* ppVertexBuffer) override 
    {
        return D3DDevice_CreateVertexBuffer(Length, Usage, FVF, Pool, 
                                            ppVertexBuffer);
    }

    
    HRESULT CreateTexture(
        UINT Width,
        UINT Height,
        UINT Levels,
        DWORD Usage,
        D3DFORMAT Format,
        D3DPOOL Pool,
        LPDIRECT3DTEXTURE8* ppTexture) override 
    {
        return D3DDevice_CreateTexture(Width, Height, Levels, Usage, Format,
                                       Pool, ppTexture);
    }

    HRESULT CreatePushBuffer(
        UINT Size, BOOL RunUsingCpuCopy, 
        LPDIRECT3DPUSHBUFFER8* ppPushBuffer) override
    {
        return D3DDevice_CreatePushBuffer(Size, RunUsingCpuCopy, ppPushBuffer);
    }

    HRESULT BeginScene() override {
        return D3DDevice_BeginScene();
    }

    HRESULT EndScene() override {
        return D3DDevice_EndScene();
    }

    HRESULT Present(CONST RECT* pSourceRect, CONST RECT* pDestRect) override {
        return D3DDevice_Present(pSourceRect, pDestRect);
    }

    HRESULT DrawPrimitive(
        D3DPRIMITIVETYPE PrimitiveType, 
        UINT StartVertex, UINT VertexCount) override 
    {
        return D3DDevice_DrawPrimitive(
            PrimitiveType, StartVertex, VertexCount);
    }

    HRESULT DrawVertices(
        D3DPRIMITIVETYPE PrimitiveType, 
        UINT StartVertex, UINT PrimitiveCount) override 
    {
        return D3DDevice_DrawVertices(
            PrimitiveType, StartVertex, PrimitiveCount);
    }

    HRESULT SetViewport(CONST D3DVIEWPORT8* pViewport) override {
        return D3DDevice_SetViewport(pViewport);
    }

    HRESULT SetVertexShaderInputDirect(
        D3DVERTEXATTRIBUTEFORMAT *pVAF,
        UINT StreamCount, 
        D3DSTREAM_INPUT *pStreamInputs) override 
    {
        return D3DDevice_SetVertexShaderInputDirect(pVAF, StreamCount, 
                                                    pStreamInputs);
    }

    HRESULT SetTextureStageState(
        DWORD Stage,
        D3DTEXTURESTAGESTATETYPE Type,
        DWORD Value) override 
    {
        return D3DDevice_SetTextureStageState(Stage, Type, Value);
    }

    HRESULT SetRenderState(D3DRENDERSTATETYPE Type, DWORD Value) override {
        return D3DDevice_SetRenderState(Type, Value);
    }

    HRESULT LoadVertexShaderProgram(CONST DWORD *pFunction, 
                                    DWORD Address) override 
    {
        return D3DDevice_LoadVertexShaderProgram(pFunction, Address);
    }

    HRESULT SetPixelShaderProgram(CONST D3DPIXELSHADERDEF *pPSDef) override {
        return D3DDevice_LoadVertexShaderProgram(pPSDef);
    }

    HRESULT Clear(DWORD Count, CONST D3DRECT* pRects, DWORD Flags,
                  D3DCOLOR Color, float Z, DWORD Stencil) override {
        return D3DDevice_Clear(Count, pRects, Flags, Color, Z, Stencil);
    }

    HRESULT SetScissors(DWORD Count, BOOL Exclusive, 
                        CONST D3DRECT *pRects) override 
    {
        return D3DDevice_SetScissors(&this->iface, Count, Exclusive, pRects);
    }
#endif // __cplusplus
} D3DDevice;

typedef struct Direct3D IMPLEMENTS(IDirect3D8) {
    IDirect3D8 iface;
    DWORD KickOffSize;
    DWORD PushBufferSize;
    D3DDISPLAYMODE displayModes[51];
    DWORD dwDisplayModeCount;
    DWORD dwCurrentDisplayMode;
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
       return Direct3D_CheckDeviceType(Adapter, CheckDeviceType, DisplayFormat,
                                       BackBufferFormat, Windowed); 

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
#if NXDK_DEBUG
    if (Windowed != FALSE)
        return D3DERR_INVALIDCALL;
#endif // NXDK_DEBUG

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
#if NXDK_DEBUG
    if (Adapter != D3DADAPTER_DEFAULT)
        return D3DERR_INVALIDCALL;

    if (Mode >= g_d3d.dwDisplayModeCount)
        return D3DERR_INVALIDCALL;
#endif // NXDK_DEBUG

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
#if NXDK_DEBUG
    if (Adapter != D3DADAPTER_DEFAULT)
        return D3DERR_INVALIDCALL;
#endif // NXDK_DEBUG

    memcpy(pMode, &g_d3d.displayModes[g_d3d.dwCurrentDisplayMode], 
           sizeof(*pMode));
    return D3D_OK;
}

HRESULT IDirect3D8_GetAdapterDisplayMode(LPDIRECT3D8 pThis, UINT Adapter, 
                                         D3DDISPLAYMODE* pMode)
{
    assert(pThis == &g_d3d.iface);
    return Direct3D_GetAdapterDisplayMode(Adapter, pMode);
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
    if (Adapter != D3DADAPTER_DEFAULT)
        return D3DERR_INVALIDCALL;
    if (DeviceType != D3DDEVTYPE_HAL)
        return D3DERR_INVALIDDEVICE;
#endif // NXDK_DEBUG
    return Direct3D_CheckDepthStencilMatch(DepthStencilFormat);
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
    g_d3d.KickOffSize    = KickOffSize;
    g_d3d.PushBufferSize = PushBufferSize;
    return D3D_OK;
}

HRESULT IDirect3D8_SetPushBufferSize(LPDIRECT3D8 pThis, 
                                     DWORD PushBufferSize, 
                                     DWORD KickOffSize) 
{
    assert(pThis == &g_d3d.iface);
    return Direct3D_SetPushBufferSize(PushBufferSize, KickOffSize);
}

IDirect3DDeviceVtbl8 g_d3ddevVtbl;

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

BOOL Direct3D_IsDisplayModeValid(D3DDISPLAYMODE* pMode, int* pIndex) {
#if NXDK_DEBUG
    if (pMode == NULL)
        return FALSE;
#endif // NXDK_DEBUG
    if (pMode->RefreshRate == 0) {
        // Any refresh rate is acceptable, use first matching mode.
        for (int i = 0; i < g_d3d.dwDisplayModeCount; i++) {
            if (pMode->Width  == g_d3d.displayModes[i].Width  &&
                pMode->Height == g_d3d.displayModes[i].Height &&
                pMode->Format == g_d3d.displayModes[i].Format &&
                pMode->Flags  == g_d3d.displayModes[i].Flags) 
            {
                if (pIndex)
                    *pIndex = i;
                return TRUE;
            }
        }
        return FALSE;
    }

    // If refresh rate is specified, memcmp works just fine.
    for (int i = 0; i < g_d3d.dwDisplayModeCount; i++) {
        if (memcmp(pMode, &g_d3d.displayModes[i], sizeof(*pMode)) == 0) {
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

ULONG IDirect3DSurface8_AddRef(LPDIRECT3DSURFACE8 pThis) {
    return IDirect3DResource8_AddRef((LPDIRECT3DRESOURCE8)pThis);
}

ULONG IDirect3DSurface8_Release(LPDIRECT3DSURFACE8 pThis) {
    return IDirect3DResource8_Release((LPDIRECT3DRESOURCE8)pThis);
}

D3DRESOURCETYPE IDirect3DSurface8_GetType(LPDIRECT3DSURFACE8 pThis) {
    return IDirect3DResource8_GetType((LPDIRECT3DRESOURCE8) pThis);
}

VOID IDirect3DSurface8_Register(LPDIRECT3DSURFACE8 pThis, PVOID pBase) {
    IDirect3DResource8_Register((LPDIRECT3DRESOURCE8)pThis, pBase);
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
    D3DSurface* surface    = (D3DSurface*)pThis;
    surface->lock.bLocked  =  TRUE;
    surface->lock.Flags    =  Flags;
    if (pRect)
        surface->lock.Rect = *pRect;
    else
        memset(&surface->lock.Rect, 0, sizeof(surface->lock.Rect));

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
        surface->lock.bLocked = FALSE;
        return E_NOTIMPL;
    }
}

HRESULT IDirect3DSurface8_UnlockRect(LPDIRECT3DSURFACE8 pThis) {
    ((D3DSurface*)pThis)->lock.bLocked = FALSE;
    return D3D_OK;
}
// ============================================================================

// ============================================================================
typedef struct D3DVertexBuffer IMPLEMENTS(IDirect3DVertexBuffer8) {
    IDirect3DVertexBuffer8 iface;
    D3DResourceInner       resource;
    D3DVERTEXBUFFER_DESC   desc;
    D3DLock                lock;
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

ULONG IDirect3DVertexBuffer8_AddRef(LPDIRECT3DVERTEXBUFFER8 pThis) {
    return IDirect3DResource8_AddRef((LPDIRECT3DRESOURCE8)pThis);
}

ULONG IDirect3DVertexBuffer8_Release(LPDIRECT3DVERTEXBUFFER8 pThis) {
    return IDirect3DResource8_Release((LPDIRECT3DRESOURCE8)pThis);
}

D3DRESOURCETYPE IDirect3DVertexBuffer8_GetType(LPDIRECT3DVERTEXBUFFER8 pThis) {
    return IDirect3DResource8_GetType((LPDIRECT3DRESOURCE8)pThis);
}

VOID IDirect3DVertexBuffer8_Register(LPDIRECT3DVERTEXBUFFER8 pThis, 
                                     PVOID pBase) 
{
    IDirect3DResource8_Register((LPDIRECT3DRESOURCE8)pThis, pBase);
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

ULONG IDirect3DBaseTexture8_AddRef(LPDIRECT3DBASETEXTURE8 pThis) {
    return IDirect3DResource8_AddRef((LPDIRECT3DRESOURCE8)pThis);
}

ULONG IDirect3DBaseTexture8_Release(LPDIRECT3DBASETEXTURE8 pThis) {
    return IDirect3DResource8_Release((LPDIRECT3DRESOURCE8)pThis);
}

D3DRESOURCETYPE IDirect3DBaseTexture8_GetType(LPDIRECT3DBASETEXTURE8 pThis) {
    return IDirect3DResource8_GetType((LPDIRECT3DRESOURCE8)pThis);
}

VOID IDirect3DBaseTexture8_Register(LPDIRECT3DBASETEXTURE8 pThis, 
                                    PVOID pBase) 
{
    IDirect3DResource8_Register((LPDIRECT3DRESOURCE8)pThis, pBase);
}

DWORD IDirect3DBaseTexture8_GetLevelCount(LPDIRECT3DBASETEXTURE8 pThis) {
        return ((D3DBaseTexture*)pThis)->inner.dwLevelCount;
}
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

ULONG IDirect3DTexture8_AddRef(LPDIRECT3DTEXTURE8 pThis) {
    return IDirect3DBaseTexture8_AddRef((LPDIRECT3DBASETEXTURE8)pThis);
}

ULONG IDirect3DTexture8_Release(LPDIRECT3DTEXTURE8 pThis) {
    D3DTexture* texture = (D3DTexture*)pThis;
    ULONG refcount = texture->base.resource.refcount.c; 
    if (refcount == 1)
        free(texture->pLevels);

    IDirect3DBaseTexture8_Release((LPDIRECT3DBASETEXTURE8)pThis);

    return refcount - 1;
}

D3DRESOURCETYPE IDirect3DTexture8_GetType(LPDIRECT3DTEXTURE8 pThis) {
    return IDirect3DBaseTexture8_GetType((LPDIRECT3DBASETEXTURE8)pThis);
}

VOID IDirect3DTexture8_Register(LPDIRECT3DTEXTURE8 pThis, PVOID pBase) {
    return IDirect3DBaseTexture8_Register((LPDIRECT3DBASETEXTURE8)pThis, 
                                          pBase);
}

DWORD IDirect3DTexture8_GetLevelCount(LPDIRECT3DTEXTURE8 pThis) {
    return IDirect3DBaseTexture8_GetLevelCount((LPDIRECT3DBASETEXTURE8)pThis);
}

HRESULT IDirect3DTexture8_GetSurfaceLevel(LPDIRECT3DTEXTURE8 pThis, UINT Level,
                                          LPDIRECT3DSURFACE8* ppSurfaceLevel) 
{
    D3DTexture* texture = (D3DTexture*)pThis;
#if NXDK_DEBUG
    if (Level >= texture->base.dwLevelCount)
        return D3DERR_INVALIDCALL;
#endif // NXDK_DEBUG
    *ppSurfaceLevel = &texture->pLevels[Level].iface;
    IDirect3DSurface8_AddRef(*ppSurfaceLevel);
    return D3D_OK;
}

HRESULT IDirect3DTexture8_GetLevelDesc(LPDIRECT3DTEXTURE8 pThis, UINT Level, 
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

HRESULT IDirect3DTexture8_LockRect(
    LPDIRECT3DTEXTURE8 pThis, UINT Level, 
    D3DLOCKED_RECT* pLockedRect, CONST RECT* pRect, DWORD Flags)
{
    D3DTexture* texture = (D3DTexture*)pThis;
#if NXDK_DEBUG
    if (Level >= texture->base.dwLevelCount)
        return D3DERR_INVALIDCALL;
#endif // NXDK_DEBUG
        
    texture->pLevels[Level].lock.bLocked = TRUE;
    texture->pLevels[Level].lock.Flags   = Flags;
    if (pRect) {
        texture->pLevels[Level].lock.Rect = *pRect;
    }
    else {
        memset(&texture->pLevels[Level].lock.Rect, 0, 
               sizeof(texture->pLevels[Level].lock.Rect));
    }

    D3DSurface* pLevel = &texture->pLevels[Level];
    if (pRect == NULL || (pRect->left == 0 && pRect->top == 0 && 
        pRect->right == pLevel->desc.Width && 
        pRect->bottom == pLevel->desc.Height)) 
    {
        pLockedRect->Pitch = pLevel->desc.Width * 
                             D3D_FormatBytesPerPixel(texture->desc.Format);
        pLockedRect->pBits = pLevel->resource.pContiguousMemory;
    } 
    else {
        // TODO: implement partial locks
        texture->pLevels[Level].lock.bLocked = FALSE;
        return E_NOTIMPL;
    }

    return D3D_OK;
}

HRESULT IDirect3DTexture8_UnlockRect(LPDIRECT3DTEXTURE8 pThis, UINT Level) {
    D3DTexture* texture = (D3DTexture*)pThis;
#if NXDK_DEBUG
    if (Level >= (texture->base).dwLevelCount)
        return D3DERR_INVALIDCALL;
#endif // NXDK_DEBUG

    texture->pLevels[0].lock.bLocked = FALSE;
    return D3D_OK;
}
// ============================================================================

static VOID D3D_CreateResource(D3DRESOURCETYPE type, DWORD Data, 
                               PVOID pContiguousMemory, 
                               D3DResourceInner* pResource) 
{
    pResource->refcount.c          = 1;
    pResource->type                = type;
    pResource->bManuallyRegistered = FALSE;
    pResource->Data                = Data;
    pResource->pContiguousMemory   = pContiguousMemory;
}

static VOID D3D_CreateSurface(UINT Width, UINT Height, D3DFORMAT Format,
                              DWORD Usage, 
                              D3DMULTISAMPLE_TYPE MultiSampleType, 
                              PVOID pContiguousMemory,
                              D3DBaseTexture* pContainer,
                              D3DSurface* pSurf) 
{
    pSurf->iface.lpVtbl = &g_d3dSurfaceVtbl;
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
    pSurf->lock.bLocked = FALSE;
    if (pContiguousMemory != NULL)
        IDirect3DSurface8_Register(&pSurf->iface, pContiguousMemory);
}

static HRESULT D3D_CreatePushBuffer(
    DWORD Size, BOOL bCpu, PVOID pContiguousMemory, D3DPushBuffer* pPB)
{
#if NXDK_DEBUG
    if (bCpu == TRUE && pContiguousMemory != NULL)
        return D3DERR_INVALIDCALL;
#endif // NXDK_DEBUG
    
    pPB->iface.lpVtbl = &g_d3dPushBufferVtbl;
    D3D_CreateResource(D3DRTYPE_PUSHBUFFER, 0, 
                       pContiguousMemory, &pPB->resource);
    pPB->Size = Size / sizeof(DWORD);
    pPB->SizeNeeded = 0;
    pPB->bCpu = bCpu;
    if (bCpu == TRUE) {
        pPB->p = (PDWORD)malloc(Size);
        pPB->resource.pContiguousMemory = NULL;
    }
    else {
        if (pContiguousMemory != NULL)
            pPB->resource.pContiguousMemory = (PDWORD)pContiguousMemory;
        else
            pPB->resource.pContiguousMemory = 
                (PDWORD)D3D_AllocContiguousMemory(Size,
                                                  D3DPUSHBUFFER_ALIGNMENT);
        pPB->p = pPB->resource.pContiguousMemory;
    }
    return D3D_OK;
}

static HRESULT D3D_CreateTexture(UINT Width, UINT Height, UINT Levels, 
                                 DWORD Usage, D3DFORMAT Format, 
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
    }

    D3DSurface* pLevels = malloc(sizeof(*pLevels) * Levels);
    if (!pLevels)
        return E_OUTOFMEMORY;

    D3DTexture* texture = malloc(sizeof(*texture));
    if (!texture)
        return E_OUTOFMEMORY;

    PVOID pContiguousMemory = 
        D3D_AllocContiguousMemory(SizeIncludingMips, D3DTEXTURE_ALIGNMENT);
    if (pContiguousMemory == NULL) {
        free(pLevels);
        free(texture);
        return D3DERR_OUTOFVIDEOMEMORY;
    }

    D3D_CreateResource(D3DRTYPE_SURFACE, 0, pContiguousMemory, 
                       &texture->base.resource);

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

    pTex->iface.lpVtbl                      = &g_d3dTextureVtbl;
    pTex->base.dwLevelCount                 = Levels;
    pTex->desc.Type                         = D3DRTYPE_TEXTURE;
    pTex->desc.Format                       = Format;
    pTex->desc.Usage                        = Usage;
    pTex->desc.Width                        = Width;
    pTex->desc.Height                       = Height;
    pTex->desc.Size                         = BaseSize;
    pTex->pLevels                           = pLevels;
    return D3D_OK;
}

// Sets the device's current push buffer to the supplied push buffer.
// All subsequent GPU commands will be sent to this push buffer until
// D3DDevce_EndPushBuffer is called.
// 
// By default, the device will use pbkit's internal push buffer.
// Calling this function with pPushBuffer == NULL will return to using
// pbkit's internal push buffer.
HRESULT D3DDevice_BeginPushBuffer(D3DPushBuffer* pPushBuffer) {
    if (pPushBuffer == NULL)
        pPushBuffer = &g_d3ddev.default_pb;

    g_d3ddev.current_pb = pPushBuffer;
    if (g_d3ddev.current_pb->bCpu == FALSE) {
        g_d3ddev.current_pb->p = 
            (PDWORD)pb_begin_at(pPushBuffer->resource.pContiguousMemory);
    }
    pPushBuffer->SizeNeeded = 0;
    return D3D_OK;
}
// Sends the current push buffer to the GPU.
//
// Calling this function *WILL NOT* set a new push buffer, and all data
// sent to the push buffer (e.g. via D3DDevice_Push*) will be lost.
// D3DDevice_BeginPushBuffer *MUST* be called before further data is sent.
HRESULT D3DDevice_EndPushBuffer() {
    if (g_d3ddev.current_pb->bCpu == FALSE) {
        pb_end_at((uint32_t*)(g_d3ddev.current_pb->p - 1), 
                  (uint32_t*)g_d3ddev.current_pb->p);
    }
    g_d3ddev.current_pb = NULL;
    return D3D_OK;
}

BOOL D3DDevice_IsKickoffReady() {
    if (g_d3ddev.current_pb == NULL)
        return FALSE;
    if (g_d3ddev.current_pb->SizeNeeded >= g_d3ddev.KickOffSize)
        return TRUE;
    else
        return FALSE;
}

HRESULT D3DDevice_SendKickoff() {
#if NXDK_DEBUG
    if (g_d3ddev.current_pb == NULL)
        return D3DERR_INVALIDCALL;
#endif // NXDK_DEBUG

    if (g_d3ddev.current_pb->bCpu == TRUE) {
        uint32_t* p = pb_begin();
        memcpy(p, g_d3ddev.current_pb->p, 
               g_d3ddev.current_pb->SizeNeeded * sizeof(DWORD));
        p += g_d3ddev.current_pb->SizeNeeded;
        pb_end(p);
        return D3D_OK;
    }
    
    HRESULT hr = D3DDevice_EndPushBuffer();
#if NXDK_DEBUG
    if (FAILED(hr)) return hr;
#endif // NXDK_DEBUG
    
    return D3DDevice_BeginPushBuffer(g_d3ddev.current_pb);
}

HRESULT D3DDevice_Push1(DWORD dwData) {
    if (g_d3ddev.current_pb == NULL)
        return D3DERR_NOTAVAILABLE;
    
    if (D3DDevice_IsKickoffReady() == TRUE) {
        HRESULT hr = D3DDevice_SendKickoff();
        if (FAILED(hr)) return hr;
    }
    return D3DPushBuffer_Push1(g_d3ddev.current_pb, dwData);
}

HRESULT D3DDevice_PushCmd(DWORD cmd, DWORD dwData) {
#if NXDK_DEBUG
    if (g_d3ddev.current_pb == NULL)
        return D3DERR_NOTAVAILABLE;
#endif // NXDK_DEBUG
    if (D3DDevice_IsKickoffReady() == TRUE) {
        HRESULT hr = D3DDevice_SendKickoff();
#if NXDK_DEBUG
        if (FAILED(hr)) return hr;
#endif // NXDK_DEBUG
    }
    return D3DPushBuffer_PushCmd(g_d3ddev.current_pb, cmd, dwData);
}

HRESULT D3DDevice_PushN(DWORD dwData, SIZE_T n) {
    if (g_d3ddev.current_pb == NULL)
        return D3DERR_NOTAVAILABLE;

    if (D3DDevice_IsKickoffReady() == TRUE) {
        HRESULT hr = D3DDevice_SendKickoff();
        if (FAILED(hr)) return hr;
    }
    return D3DPushBuffer_PushN(g_d3ddev.current_pb, dwData, n);
}

HRESULT D3DDevice_PushA(CONST DWORD* pdwData, SIZE_T n) {
    if (g_d3ddev.current_pb == NULL)
        return D3DERR_NOTAVAILABLE;

    if (D3DDevice_IsKickoffReady() == TRUE) {
        HRESULT hr = D3DDevice_SendKickoff();
        if (FAILED(hr)) return hr;
    }
    return D3DPushBuffer_PushA(g_d3ddev.current_pb, pdwData, n);
}

HRESULT D3DDevice_CreateImageSurface(UINT Width, UINT Height, D3DFORMAT Format,
                                     LPDIRECT3DSURFACE8* ppSurface)
{
    D3DSurface* surf = malloc(sizeof(*surf));
    surf->iface.lpVtbl = &g_d3dSurfaceVtbl;
    D3D_CreateSurface(
        Width, Height, Format, 0, 0, NULL, NULL, surf);
    surf->resource.pContiguousMemory = 
        D3D_AllocContiguousMemory(surf->desc.Size, D3DSURFACE_ALIGNMENT);
    if (surf->resource.pContiguousMemory == NULL) {
        free(surf);
        return D3DERR_OUTOFVIDEOMEMORY;
    }
    *ppSurface = &surf->iface;
    return D3D_OK;
}

HRESULT IDirect3DDevice8_CreateImageSurface(
    LPDIRECT3DDEVICE8 pThis, UINT Width, UINT Height,
    D3DFORMAT Format, LPDIRECT3DSURFACE8* ppSurface)
{
    return D3DDevice_CreateImageSurface(Width, Height, Format, ppSurface);
}


HRESULT D3DDevice_CreateDepthStencilSurface(
    UINT Width, UINT Height, D3DFORMAT Format, IDirect3DSurface8** ppSurface)
{
    return D3DDevice_CreateImageSurface(Width, Height, Format, ppSurface);
}

HRESULT IDirect3DDevice8_CreateDepthStencilSurface(
    LPDIRECT3DDEVICE8 pThis, UINT Width, UINT Height, D3DFORMAT Format,
    D3DMULTISAMPLE_TYPE MultiSample, IDirect3DSurface8** ppSurface)
{
    return D3DDevice_CreateDepthStencilSurface(Width, Height,
                                               Format, ppSurface);
}

BOOL D3D_IsDepthStencilFormatFixed(D3DFORMAT Format) {
    if (Format == D3DFMT_D24S8 || D3DFMT_D16)
        return TRUE;
    if (Format == D3DFMT_F24S8 || D3DFMT_F16)
        return FALSE;

    assert(false);
    return -1;
}

#if NXDK_DEBUG
HRESULT Direct3D_CreateDevice_Validate(
    UINT Adapter, DWORD BehaviorFlags,
    D3DPRESENT_PARAMETERS* pPresentationParameters)
{
    if (Adapter != D3DADAPTER_DEFAULT)
        return D3DERR_INVALIDCALL;

    if (BehaviorFlags !=
        (D3DCREATE_HARDWARE_VERTEXPROCESSING | D3DCREATE_PUREDEVICE))
    {
        return D3DERR_INVALIDCALL;
    }

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
#endif // NXDK_DEBUG

HRESULT Direct3D_CreateDevice(
    UINT Adapter, DWORD BehaviorFlags, 
    D3DPRESENT_PARAMETERS* pPresentationParameters)
{
#if NXDK_DEBUG
    HRESULT hr = Direct3D_CreateDevice_Validate(Adapter, BehaviorFlags, 
                                                pPresentationParameters);
    if (FAILED(hr)) return hr;
#else 
    HRESULT hr = D3D_OK;
#endif // NXDK_DEBUG
    if (g_d3ddev.refcount.c != 0)
        return D3DERR_DEVICENOTRESET;

    int bpp = D3D_FormatBPP(pPresentationParameters->BackBufferFormat);
#if NXDK_DEBUG
    if (bpp == 0)
        return D3DERR_INVALIDCALL;
#endif // NXDK_DEBUG
    
    D3DDISPLAYMODE mode;
    mode.Width       = pPresentationParameters->BackBufferWidth;
    mode.Height      = pPresentationParameters->BackBufferHeight;
    mode.RefreshRate = pPresentationParameters->FullScreen_RefreshRateInHz;
    mode.Format      = pPresentationParameters->BackBufferFormat;
    mode.Flags       = pPresentationParameters->Flags;
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
        g_d3d.dwCurrentDisplayMode = i;
    }
#if NXDK_DEBUG
    else {
        return D3DERR_INVALIDCALL;
    }
#endif // NXDK_DEBUG

    // Must do this before pb_init().
    if (pPresentationParameters->DepthStencilSurface) {
        D3DSurface* pDSSurf = 
            (D3DSurface*)pPresentationParameters->DepthStencilSurface;
        g_d3ddev.depth_stencil_surface = (D3DSurface*)pDSSurf;
        pb_set_ds_addr(pDSSurf->resource.pContiguousMemory);
    }
    
    UINT BackBufferCount = pPresentationParameters->BackBufferCount;
    if (BackBufferCount == 0)
        BackBufferCount = 1;
    // Must also do these before pb_init().
    pb_back_buffer_count(BackBufferCount);
    pb_set_color_format(pPresentationParameters->BackBufferFormat, false);

    if (pb_init() != 0)
        return D3DERR_NOTAVAILABLE;

    for (i = 0; i < BackBufferCount + 1; i++) {
        if (pPresentationParameters->BufferSurfaces[i] != NULL) {
            g_d3ddev.surfaces[i] = 
                (D3DSurface*)pPresentationParameters->BufferSurfaces[i];
            continue;
        }

        g_d3ddev.surfaces[i] = malloc(sizeof(D3DSurface));
        if (!g_d3ddev.surfaces[i])
            return E_OUTOFMEMORY;

        D3D_CreateSurface(
            pPresentationParameters->BackBufferWidth, 
            pPresentationParameters->BackBufferHeight, 
            pPresentationParameters->BackBufferFormat,
            D3DUSAGE_RENDERTARGET, 
            pPresentationParameters->MultiSampleType,
            (PVOID)pb_fb_addr(i),
            NULL,
            g_d3ddev.surfaces[i]);
    }
        
    if (pPresentationParameters->EnableAutoDepthStencil) {
        g_d3ddev.depth_stencil_surface = malloc(sizeof(D3DSurface));
        if (!g_d3ddev.depth_stencil_surface)
            return E_OUTOFMEMORY;

        D3D_CreateSurface(
            pPresentationParameters->BackBufferWidth, 
            pPresentationParameters->BackBufferHeight, 
            pPresentationParameters->AutoDepthStencilFormat,
            D3DUSAGE_DEPTHSTENCIL, 
            pPresentationParameters->MultiSampleType,
            (PVOID)pb_ds_addr(),
            NULL,
            g_d3ddev.depth_stencil_surface);
    }
    pb_show_front_screen();

    g_d3ddev.refcount.c              = 1;
    g_d3ddev.iface.lpVtbl            = &g_d3ddevVtbl;
    g_d3ddev.KickOffSize             = g_d3d.KickOffSize;
    g_d3ddev.in_scene                = FALSE;
    g_d3ddev.max_surfaces            = BackBufferCount;
    g_d3ddev.current_surface         = 0;
    g_d3ddev.default_pb.iface.lpVtbl = &g_d3dPushBufferVtbl;
    g_d3ddev.lastPresentVBlankCount  = 0;
    g_d3ddev.PresentInterval = 
        pPresentationParameters->FullScreen_PresentationInterval;

    hr = D3D_CreatePushBuffer(g_d3d.PushBufferSize, FALSE, 
                              NULL, &g_d3ddev.default_pb);
    if (FAILED(hr)) return hr;
    g_d3ddev.current_pb = &g_d3ddev.default_pb;
    DWORD ZFormat = D3D_IsDepthStencilFormatFixed(g_d3ddev.depth_stencil_surface->desc.Format) ? 
                    NV097_SET_CONTROL0_Z_FORMAT_FIXED : 
                    NV097_SET_CONTROL0_Z_FORMAT_FLOAT;
    g_d3ddev.Control0 = NV097_SET_CONTROL0_TEXTURE_PERSPECTIVE_ENABLE | ZFormat;
    hr = D3DDevice_PushCmd(NV097_SET_CONTROL0, g_d3ddev.Control0);
    if (FAILED(hr)) return hr;

    for (UINT Stage = 0; Stage < D3DTSS_MAXSTAGES; Stage++) {
        g_d3ddev.pTexture[Stage] = NULL;
        memset(&g_d3ddev.TextureStageState[Stage], 0, sizeof(g_d3ddev.TextureStageState[Stage]));
    }

    hr = D3DDevice_PushCmd(NV097_SET_TRANSFORM_EXECUTION_MODE, 
            MASK(NV097_SET_TRANSFORM_EXECUTION_MODE_MODE, 
                 NV097_SET_TRANSFORM_EXECUTION_MODE_MODE_PROGRAM)
                | MASK(NV097_SET_TRANSFORM_EXECUTION_MODE_RANGE_MODE, 
                  NV097_SET_TRANSFORM_EXECUTION_MODE_RANGE_MODE_PRIV));
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
    if (DeviceType != D3DDEVTYPE_HAL)
        return D3DERR_INVALIDDEVICE;
    if (hFocusWindow != NULL) 
        return D3DERR_INVALIDCALL;
#endif // NXDK_DEBUG

    HRESULT hr = Direct3D_CreateDevice(Adapter, BehaviorFlags, 
                                       pPresentationParameters);
    *ppReturnedDevice = &g_d3ddev.iface;
    return hr;
}

IDirect3DVtbl8 g_d3dvtbl;

BOOL g_firstCreate = TRUE;

// ============================================================================
ULONG D3DDevice_AddRef() {
    return D3DRefcount_AddRef(&g_d3ddev.refcount);
}

ULONG IDirect3DDevice8_AddRef(LPDIRECT3DDEVICE8 pThis) {
    assert(pThis == &g_d3ddev.iface);
    return D3DDevice_AddRef();
}

ULONG D3DDevice_Release() {
    ULONG refcount = D3DRefcount_Release(&g_d3ddev.refcount);
    if (refcount > 0)
        return refcount;

    pb_kill();
    return 0;
}

ULONG IDirect3DDevice8_Release(LPDIRECT3DDEVICE8 pThis) {
    assert(pThis == &g_d3ddev.iface);
    return D3DDevice_Release();
}

HRESULT D3DDevice_CreateVertexBuffer(
    UINT Length, LPDIRECT3DVERTEXBUFFER8* ppVertexBuffer) 
{
    D3DVertexBuffer* vb = malloc(sizeof(*vb));
    vb->iface.lpVtbl                 = &g_d3dVertexBufferVtbl;
    vb->resource.bManuallyRegistered = FALSE;
    vb->resource.Data                = 0;
    vb->lock.bLocked                 = FALSE;
    vb->resource.pContiguousMemory   = 
        D3D_AllocContiguousMemory(Length, D3DVERTEXBUFFER_ALIGNMENT);
    if (vb->resource.pContiguousMemory == NULL) {
        free(vb);
        return E_OUTOFMEMORY;
    }
    vb->desc.Format                 = D3DFMT_VERTEXDATA;
    vb->desc.Type                   = D3DRTYPE_VERTEXBUFFER;
    *ppVertexBuffer                 = &vb->iface;
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
    if (g_d3ddev.in_scene == TRUE) 
        return D3DERR_INVALIDCALL;

    g_d3ddev.in_scene = TRUE;

    if (g_d3ddev.current_pb == NULL)
        g_d3ddev.current_pb = &g_d3ddev.default_pb;

    if (g_d3ddev.current_pb == &g_d3ddev.default_pb)
        pb_reset();

    pb_target_back_buffer();
    while(pb_busy()) {
        /* Wait for completion... */
    }
    D3DDevice_BeginPushBuffer(g_d3ddev.current_pb);
    return D3D_OK;
}

HRESULT IDirect3DDevice8_BeginScene(LPDIRECT3DDEVICE8 pThis) {
    assert(pThis == &g_d3ddev.iface);
    return D3DDevice_BeginScene();
}

HRESULT D3DDevice_EndScene() {
    if (g_d3ddev.in_scene == FALSE) 
        return D3DERR_INVALIDCALL;

    g_d3ddev.in_scene = FALSE;

    while(pb_busy()) {
        /* Wait for completion... */
    }

    return D3DDevice_EndPushBuffer();
}

HRESULT IDirect3DDevice8_EndScene(LPDIRECT3DDEVICE8 pThis) {
    assert(pThis == &g_d3ddev.iface);
    return D3DDevice_EndScene();
}

HRESULT D3DDevice_Present(CONST RECT* pSourceRect, CONST RECT* pDestRect) {
    // TODO: implement
    if (pSourceRect || pDestRect)
        return E_NOTIMPL;

    while (pb_finished()) {
        /* Not ready to swap yet */
    }

    g_d3ddev.current_surface = 
        (g_d3ddev.current_surface + 1) % g_d3ddev.max_surfaces;
    UINT lastVBL = g_d3ddev.lastPresentVBlankCount;
    g_d3ddev.lastPresentVBlankCount = pb_get_vbl_counter();

    // If the present interval is immediate, we don't need to worry about
    // waiting for vblanks and we can bail early.
    if (g_d3ddev.PresentInterval == D3DPRESENT_INTERVAL_IMMEDIATE)
        return D3D_OK;

    // The remaining logic will wait for the number of vblanks necessary to
    // maintain the presentation interval, accounting for the number of vblanks
    // since the last Present call.

    // Highest valid presentation interval is 3. If the difference is greater
    // than 3 we don't need to wait for a vblank regardless of the interval.
    UINT intervalDiff = g_d3ddev.lastPresentVBlankCount - lastVBL;
    if (intervalDiff > 3)
        return D3D_OK;

    // If we're still on the same vblank as the last Present call, we need 
    // to wait regardless of if the interval is 1, 2, or 3.
    if (intervalDiff == 0)
        D3DDevice_BlockUntilVerticalBlank();
    
    // If the interval is 2 or 3, we need to wait if the difference 
    // was 0 or 1.
    if (g_d3ddev.PresentInterval >= D3DPRESENT_INTERVAL_TWO && 
        intervalDiff < 2)
    {
        D3DDevice_BlockUntilVerticalBlank();
    }

    // If the interval was 3, we need to wait unless the difference was
    // exactly 3.
    if (g_d3ddev.PresentInterval == D3DPRESENT_INTERVAL_THREE && 
        intervalDiff < 3)
    {
        D3DDevice_BlockUntilVerticalBlank();
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

HRESULT D3DDevice_DrawVertices(D3DPRIMITIVETYPE PrimitiveType, 
                               UINT StartVertex, UINT VertexCount)
{
    HRESULT hr = D3D_OK;
    for (UINT Stage = 0; Stage < D3DTSS_MAXSTAGES; Stage++) {
        if (g_d3ddev.TextureStageState[Stage].AddressDirty) {
            hr = D3DDevice_PushCmd(NV097_SET_TEXTURE_ADDRESS(Stage), g_d3ddev.TextureStageState[Stage].Address);
            if (FAILED(hr)) return hr;
            g_d3ddev.TextureStageState[Stage].AddressDirty = FALSE;
        }
        if (g_d3ddev.TextureStageState[Stage].FilterDirty) {
            hr = D3DDevice_PushCmd(NV097_SET_TEXTURE_FILTER(Stage), g_d3ddev.TextureStageState[Stage].Filter);
            if (FAILED(hr)) return hr;
            g_d3ddev.TextureStageState[Stage].FilterDirty = FALSE;
        }
        if (g_d3ddev.TextureStageState[Stage].Control0Dirty) {
            hr = D3DDevice_PushCmd(NV097_SET_TEXTURE_CONTROL0(Stage), g_d3ddev.TextureStageState[Stage].Control0);
            if (FAILED(hr)) return hr;
            g_d3ddev.TextureStageState[Stage].Control0Dirty = FALSE;
        }
        if (g_d3ddev.TextureStageState[Stage].FormatDirty) {
            hr = D3DDevice_PushCmd(NV097_SET_TEXTURE_FORMAT(Stage), g_d3ddev.TextureStageState[Stage].Format);
            if (FAILED(hr)) return hr;
            g_d3ddev.TextureStageState[Stage].FormatDirty = FALSE;
        }
        if (g_d3ddev.TextureStageState[Stage].OffsetDirty) {
            hr = D3DDevice_PushCmd(NV097_SET_TEXTURE_OFFSET(Stage), g_d3ddev.TextureStageState[Stage].Offset);
            if (FAILED(hr)) return hr;
            g_d3ddev.TextureStageState[Stage].OffsetDirty = FALSE;
        }
    }
    hr = D3DDevice_PushCmd(NV097_SET_BEGIN_END, PrimitiveType);
    if (FAILED(hr)) return hr;
    for (int i = 0; i < VertexCount; i += 256) {
        DWORD Count = VertexCount - i;
        if (Count >= 256) Count = 256;
        hr = D3DDevice_PushCmd(NV097_DRAW_ARRAYS,
            MASK(NV097_DRAW_ARRAYS_COUNT, Count - 1) | 
            MASK(NV097_DRAW_ARRAYS_START_INDEX, StartVertex));
        if (FAILED(hr)) return hr;
    }

    return D3DDevice_PushCmd(NV097_SET_BEGIN_END, NV097_SET_BEGIN_END_OP_END);
}

HRESULT IDirect3DDevice8_DrawVertices(LPDIRECT3DDEVICE8 pThis, 
                                      D3DPRIMITIVETYPE PrimitiveType, 
                                      UINT StartVertex, UINT VertexCount)
{
    assert(pThis == &g_d3ddev.iface);
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
    g_d3ddev.viewport = *pViewport;
    return D3D_OK;
}

HRESULT IDirect3DDevice8_SetViewport(LPDIRECT3DDEVICE8 pThis, 
                                     CONST D3DVIEWPORT8* pViewport) 
{
    assert(pThis == &g_d3ddev.iface);
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
        
        D3DDevice_PushCmd(NV097_SET_VERTEX_DATA_ARRAY_FORMAT + i*sizeof(DWORD),
                          MASK(NV097_SET_VERTEX_DATA_ARRAY_FORMAT_TYPE |
                               NV097_SET_VERTEX_DATA_ARRAY_FORMAT_SIZE,
                               pInput->Format) |
                          MASK(NV097_SET_VERTEX_DATA_ARRAY_FORMAT_STRIDE,
                               pStreamInputs[i].Stride));
        D3DVertexBuffer* vb = (D3DVertexBuffer*)pStreamInputs[i].VertexBuffer;
        BYTE* pData = vb->resource.pContiguousMemory;
        pData += pStreamInputs[i].Offset;
        pData += pInput->Offset;
        D3DDevice_PushCmd(NV097_SET_VERTEX_DATA_ARRAY_OFFSET + i*sizeof(DWORD),
                         (uint32_t)pData & 0x03ffffff);
    }

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

HRESULT D3DDevice_CreateTexture(
    UINT Width, UINT Height, UINT Levels, DWORD Usage, 
    D3DFORMAT Format, LPDIRECT3DTEXTURE8* ppTexture)
{
#if NXDK_DEBUG
    if (Usage != 0 && Usage != D3DUSAGE_BORDERSOURCE_TEXTURE)
        return D3DERR_INVALIDCALL;
#endif // NXDK_DEBUG

    D3DTexture* pTex = malloc(sizeof(*pTex));
    if (pTex == NULL)
        return E_OUTOFMEMORY;

    HRESULT hr = D3D_CreateTexture(Width, Height, Levels, Usage, Format, pTex);
    if (FAILED(hr)) return hr;
    *ppTexture = &pTex->iface;
    return D3D_OK;
}

HRESULT IDirect3DDevice8_CreateTexture(
    LPDIRECT3DDEVICE8 pThis, UINT Width, UINT Height, 
    UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool,
    LPDIRECT3DTEXTURE8* ppTexture)
{
    assert(pThis == &g_d3ddev.iface);
    return D3DDevice_CreateTexture(Width, Height, Levels, Usage, 
                                   Format, ppTexture);
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

HRESULT D3DDevice_SetTextureStageState_Simple(
    DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD Value)
{
#if NXDK_DEBUG
    if (Type >= D3DTSS_DEFERRED_MAX || Type < D3DTSS_DEFERRED_TEXTURE_STATE_MAX) {
        assert(false);
        return D3DERR_INVALIDCALL;
    }
#endif // NXDK_DEBUG
    switch (Type) {
    default:
        // TODO: implement remaining texture stage states
        return E_NOTIMPL;
    }
}

#define MASK_INPLACE(a, mask, value) \
    (a) = ((a & (~(mask))) | MASK(mask, value))

#define D3DTSS_DIRTY(stage, type) \
    g_d3ddev.TextureStageStateDirty[stage][type] = TRUE;

#define D3DTSS_CLEAN(stage, type) \
    g_d3ddev.TextureStageStateDirty[stage][type] = FALSE;

HRESULT D3DDevice_SetTextureStageState_Deferred(
    DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD Value)
{
#if NXDK_DEBUG
    if (Type >= D3DTSS_DEFERRED_MAX || Type < D3DTSS_DEFERRED_TEXTURE_STATE_MAX) {
        assert(false);
        return D3DERR_INVALIDCALL;
    }
#endif // NXDK_DEBUG
    switch (Type) {
    case D3DTSS_ADDRESSU:
        MASK_INPLACE(g_d3ddev.TextureStageState[Stage].Address, 
                     NV097_SET_TEXTURE_ADDRESS_U, Value);
        g_d3ddev.TextureStageState[Stage].AddressDirty = TRUE;
        return D3D_OK;
    case D3DTSS_ADDRESSV:
        MASK_INPLACE(g_d3ddev.TextureStageState[Stage].Address, 
                     NV097_SET_TEXTURE_ADDRESS_V, Value);
        g_d3ddev.TextureStageState[Stage].AddressDirty = TRUE;
        return D3D_OK;
    case D3DTSS_ADDRESSW:
        MASK_INPLACE(g_d3ddev.TextureStageState[Stage].Address, 
                     NV097_SET_TEXTURE_ADDRESS_P, Value);
        g_d3ddev.TextureStageState[Stage].AddressDirty = TRUE;
        return D3D_OK;
    case D3DTSS_MAGFILTER:
        MASK_INPLACE(g_d3ddev.TextureStageState[Stage].Filter, 
                     NV097_SET_TEXTURE_FILTER_MAG, Value);
        g_d3ddev.TextureStageState[Stage].FilterDirty = TRUE;
        return D3D_OK;
    case D3DTSS_MINFILTER:
        MASK_INPLACE(g_d3ddev.TextureStageState[Stage].Filter, 
                     NV097_SET_TEXTURE_FILTER_MIN, Value);
        g_d3ddev.TextureStageState[Stage].FilterDirty = TRUE;
        return D3D_OK;
    case D3DTSS_MIPMAPLODBIAS:
        MASK_INPLACE(g_d3ddev.TextureStageState[Stage].Filter, 
                     NV097_SET_TEXTURE_FILTER_MIPMAP_LOD_BIAS, Value);
        g_d3ddev.TextureStageState[Stage].FilterDirty = TRUE;
        return D3D_OK;
    case D3DTSS_MAXMIPLEVEL:
        MASK_INPLACE(g_d3ddev.TextureStageState[Stage].Format, 
                     NV097_SET_TEXTURE_FORMAT_MIPMAP_LEVELS, Value);
        g_d3ddev.TextureStageState[Stage].FormatDirty = TRUE;
        return D3D_OK;
    case D3DTSS_MAXANISOTROPY:
        MASK_INPLACE(g_d3ddev.TextureStageState[Stage].Control0, 
                     NV097_SET_TEXTURE_CONTROL0_ANISOTROPY, Value);
        g_d3ddev.TextureStageState[Stage].Control0Dirty = TRUE;
        return D3D_OK;
    case D3DTSS_COLORKEYOP:
        MASK_INPLACE(g_d3ddev.TextureStageState[Stage].Control0, 
                     NV097_SET_TEXTURE_CONTROL0_COLOR_KEY_MODE, Value);
        g_d3ddev.TextureStageState[Stage].Control0Dirty = TRUE;
        return D3D_OK;
    case D3DTSS_COLORSIGN:
        g_d3ddev.TextureStageState[Stage].Control0 = 
            (g_d3ddev.TextureStageState[Stage].Control0 & 
                ~NV097_SET_TEXTURE_FILTER_SIGN) | Value;
        g_d3ddev.TextureStageState[Stage].Control0Dirty = TRUE;
        return D3D_OK;
    case D3DTSS_ALPHAKILL:
        g_d3ddev.TextureStageState[Stage].Control0 = 
            (g_d3ddev.TextureStageState[Stage].Control0 &
                 ~NV097_SET_TEXTURE_CONTROL0_ALPHA_KILL_ENABLE) | Value;
        g_d3ddev.TextureStageState[Stage].Control0Dirty = TRUE;
        return D3D_OK;
    default:
        // TODO: implement remaining texture stage states
        return E_NOTIMPL;
    }

    D3DTSS_DIRTY(Stage, Type);
    return D3D_OK;
}

HRESULT D3DDevice_SetTextureStageState(
    DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD Value)
{
#if NXDK_DEBUG
    if (Stage >= D3DTSS_MAXSTAGES)
        return D3DERR_INVALIDCALL;
    if (Type >= D3DTSS_MAX)
        return D3DERR_INVALIDCALL;
#endif // NXDK_DEBUG
    if (Type >= D3DTSS_DEFERRED_MAX)
        return D3DDevice_SetTextureStageState_Immediate(Stage, Type, Value);
    else if (Type >= D3DTSS_DEFERRED_TEXTURE_STATE_MAX)
        return D3DDevice_SetTextureStageState_Deferred(Stage, Type, Value);
    else
        return D3DDevice_SetTextureStageState_Simple(Stage, Type, Value);
}

HRESULT IDirect3DDevice8_SetTextureStageState(
    LPDIRECT3DDEVICE8 pThis, DWORD Stage, 
    D3DTEXTURESTAGESTATETYPE Type, DWORD Value)
{
    assert(pThis == &g_d3ddev.iface);
    return D3DDevice_SetTextureStageState(Stage, Type, Value);
}

HRESULT D3DDevice_CreatePushBuffer(
    UINT Size, BOOL RunUsingCpuCopy, LPDIRECT3DPUSHBUFFER8* ppPushBuffer)
{
    D3DPushBuffer* pb = malloc(sizeof(*pb));
    if (!pb)
        return E_OUTOFMEMORY;

    D3D_CreatePushBuffer(Size, RunUsingCpuCopy, NULL, pb);
    *ppPushBuffer = (LPDIRECT3DPUSHBUFFER8)pb;
    return D3D_OK;
}

HRESULT IDirect3DDevice8_CreatePushBuffer(
    LPDIRECT3DDEVICE8 pThis, UINT Size, 
    BOOL RunUsingCpuCopy, LPDIRECT3DPUSHBUFFER8* ppPushBuffer)
{
    assert(pThis == &g_d3ddev.iface);
    return D3DDevice_CreatePushBuffer(Size, RunUsingCpuCopy, ppPushBuffer);
}

HRESULT D3DDevice_LoadVertexShaderProgram(
    CONST DWORD *pFunction, DWORD Address)
{
    HRESULT hr = D3DDevice_PushCmd(NV097_SET_TRANSFORM_PROGRAM_LOAD, Address);
    if(FAILED(hr)) return hr;

    for (int i = 0; pFunction[i] != D3DVS_END() && Address + i < 136; i +=4) {
        hr = D3DDevice_PushCmd(NV097_SET_TRANSFORM_PROGRAM, 4);
        if(FAILED(hr))
            return hr;
        hr = D3DDevice_PushA(&pFunction[i], 4);
        if(FAILED(hr)) return hr;
    }
    return D3D_OK;
}

HRESULT IDirect3DDevice8_LoadVertexShaderProgram(
    LPDIRECT3DDEVICE8 pThis, CONST DWORD *pFunction, DWORD Address)
{
    assert(pThis == &g_d3ddev.iface);
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
    [D3DRS_PSCONSTANT0_0]             = NV20_TCL_PRIMITIVE_3D_RC_CONSTANT_COLOR0(0),
    [D3DRS_PSCONSTANT0_1]             = NV20_TCL_PRIMITIVE_3D_RC_CONSTANT_COLOR0(1),
    [D3DRS_PSCONSTANT0_2]             = NV20_TCL_PRIMITIVE_3D_RC_CONSTANT_COLOR0(2),
    [D3DRS_PSCONSTANT0_3]             = NV20_TCL_PRIMITIVE_3D_RC_CONSTANT_COLOR0(3),
    [D3DRS_PSCONSTANT0_4]             = NV20_TCL_PRIMITIVE_3D_RC_CONSTANT_COLOR0(4),
    [D3DRS_PSCONSTANT0_5]             = NV20_TCL_PRIMITIVE_3D_RC_CONSTANT_COLOR0(5),
    [D3DRS_PSCONSTANT0_6]             = NV20_TCL_PRIMITIVE_3D_RC_CONSTANT_COLOR0(6),
    [D3DRS_PSCONSTANT0_7]             = NV20_TCL_PRIMITIVE_3D_RC_CONSTANT_COLOR0(7),
    [D3DRS_PSCONSTANT1_0]             = NV20_TCL_PRIMITIVE_3D_RC_CONSTANT_COLOR1(0),
    [D3DRS_PSCONSTANT1_1]             = NV20_TCL_PRIMITIVE_3D_RC_CONSTANT_COLOR1(1),
    [D3DRS_PSCONSTANT1_2]             = NV20_TCL_PRIMITIVE_3D_RC_CONSTANT_COLOR1(2),
    [D3DRS_PSCONSTANT1_3]             = NV20_TCL_PRIMITIVE_3D_RC_CONSTANT_COLOR1(3),
    [D3DRS_PSCONSTANT1_4]             = NV20_TCL_PRIMITIVE_3D_RC_CONSTANT_COLOR1(4),
    [D3DRS_PSCONSTANT1_5]             = NV20_TCL_PRIMITIVE_3D_RC_CONSTANT_COLOR1(5),
    [D3DRS_PSCONSTANT1_6]             = NV20_TCL_PRIMITIVE_3D_RC_CONSTANT_COLOR1(6),
    [D3DRS_PSCONSTANT1_7]             = NV20_TCL_PRIMITIVE_3D_RC_CONSTANT_COLOR1(7),
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
    [D3DRS_ZWRITEENABLE]              = NV097_SET_CONTROL0,
    [D3DRS_DITHERENABLE]              = NV097_SET_DITHER_ENABLE,
    [D3DRS_SHADEMODE]                 = NV097_SET_SHADE_MODEL,
    [D3DRS_COLORWRITEENABLE]          = NV097_SET_COLOR_MASK,
    [D3DRS_STENCILZFAIL]              = NV097_SET_STENCIL_OP_ZFAIL,
    [D3DRS_STENCILPASS]               = NV097_SET_STENCIL_OP_ZPASS,
    [D3DRS_STENCILFAIL]               = NV097_SET_STENCIL_OP_FAIL,
    [D3DRS_STENCILREF]                = NV097_SET_STENCIL_FUNC_REF,
    [D3DRS_STENCILMASK]               = NV097_SET_STENCIL_MASK,
    [D3DRS_BLENDOP]                   = NV097_SET_BLEND_EQUATION,
    [D3DRS_BLENDCOLOR]                = NV097_SET_BLEND_COLOR,
    [D3DRS_POLYGONOFFSETZSLOPESCALE]  = NV097_SET_POLYGON_OFFSET_SCALE_FACTOR,
    [D3DRS_POLYGONOFFSETZOFFSET]      = NV097_SET_POLYGON_OFFSET_BIAS,
    [D3DRS_POINTOFFSETENABLE]         = NV097_SET_POLY_OFFSET_POINT_ENABLE,
    [D3DRS_WIREFRAMEOFFSETENABLE]     = NV097_SET_POLY_OFFSET_LINE_ENABLE,
    [D3DRS_SOLIDOFFSETENABLE]         = NV097_SET_POLY_OFFSET_FILL_ENABLE,
    [D3DRS_CULLMODE]                  = NV097_SET_ZMIN_MAX_CONTROL,
    [D3DRS_STIPPLEENABLE]             = NV097_SET_STIPPLE_ENABLE,
};

HRESULT D3DDevice_SetRenderState_Simple(D3DRENDERSTATETYPE Type, DWORD Value) {
#if NXDK_DEBUG
    if (Type > D3DRS_STIPPLEENABLE)
        return D3DERR_INVALIDCALL;
#endif // NXDK_DEBUG
    if (Type == D3DRS_ZWRITEENABLE) {
        if (Value == TRUE)
            g_d3ddev.Control0 |= NV097_SET_CONTROL0_STENCIL_WRITE_ENABLE;
        else
            g_d3ddev.Control0 &= ~NV097_SET_CONTROL0_STENCIL_WRITE_ENABLE;
        Value = g_d3ddev.Control0;
    }
    DWORD Cmd = D3D_SimpleRenderState[Type];
#if NXDK_DEBUG
    // Cmd == 0 means the render state doesn't have a value in 
    // D3D_SimpleRenderState yet, indicating that it's unimplemented.
    if (Cmd == 0) {
        assert(false);
        return E_NOTIMPL;
    }
#endif // NXDK_DEBUG
    return D3DDevice_PushCmd(Cmd, Value);
}

HRESULT D3DDevice_SetRenderState_Deferred(D3DRENDERSTATETYPE Type, DWORD Value) {
#if NXDK_DEBUG
    if (Type < D3DRS_SIMPLE_MAX || Type > D3DRS_PRESENTATIONINTERVAL)
        return D3DERR_INVALIDCALL;
#endif // NXDK_DEBUG
    switch(Type) {
    case D3DRS_PRESENTATIONINTERVAL:
        g_d3ddev.PresentInterval = Value;
        return D3D_OK;
    default:
        // TODO: implement remaining render states
        assert(false);
        return E_NOTIMPL;
    }
}

HRESULT D3DDevice_SetRenderState_Complex(D3DRENDERSTATETYPE Type, DWORD Value) {
#if NXDK_DEBUG
    if (Type >= D3DRS_DEFERRED_MAX)
        return D3DERR_INVALIDCALL;
#endif // NXDK_DEBUG
    switch (Type) {
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

HRESULT IDirect3DDevice8_SetRenderState(
    LPDIRECT3DDEVICE8 pThis, D3DRENDERSTATETYPE Type, DWORD Value)
{
    assert(pThis == &g_d3ddev.iface);
    return D3DDevice_SetRenderState(Type, Value);
}

HRESULT D3DDevice_SetPixelShaderProgram(CONST D3DPIXELSHADERDEF *pPSDef) {
    HRESULT hr = D3D_OK;
    for (int i = 0; i < 8; i++) {
        hr = D3DDevice_SetRenderState(D3DRS_PSALPHAINPUTS0 + i, 
                                      pPSDef->PSAlphaInputs[i]);
#if NXDK_DEBUG
        if (FAILED(hr)) return hr;
#endif // NXDK_DEBUG
    }
    hr = D3DDevice_SetRenderState(D3DRS_PSFINALCOMBINERINPUTSABCD, 
                                          pPSDef->PSFinalCombinerInputsABCD);
#if NXDK_DEBUG
    if(FAILED(hr)) return hr;
#endif // NXDK_DEBUG
    hr = D3DDevice_SetRenderState(D3DRS_PSFINALCOMBINERINPUTSEFG, 
                                  pPSDef->PSFinalCombinerInputsEFG);
#if NXDK_DEBUG
    if(FAILED(hr)) return hr;
#endif // NXDK_DEBUG
    for (int i = 0; i < 8; i++) {
        hr = D3DDevice_SetRenderState(D3DRS_PSCONSTANT0_0 + i, 
                                      pPSDef->PSConstant0[i]);
#if NXDK_DEBUG
        if(FAILED(hr)) return hr;
#endif // NXDK_DEBUG
    }
    for (int i = 0; i < 8; i++) {
        hr = D3DDevice_SetRenderState(D3DRS_PSCONSTANT1_0 + i, 
                                      pPSDef->PSConstant1[i]);
#if NXDK_DEBUG
        if(FAILED(hr)) return hr;
#endif // NXDK_DEBUG
    }
    for (int i = 0; i < 8; i++) {
        hr = D3DDevice_SetRenderState(D3DRS_PSALPHAOUTPUTS0 + i, 
                                      pPSDef->PSAlphaOutputs[i]);
#if NXDK_DEBUG
        if (FAILED(hr)) return hr;
#endif // NXDK_DEBUG
    }
    for (int i = 0; i < 8; i++) {
        hr = D3DDevice_SetRenderState(D3DRS_PSRGBINPUTS0 + i, 
                                      pPSDef->PSRGBInputs[i]);
#if NXDK_DEBUG
        if (FAILED(hr)) return hr;
#endif // NXDK_DEBUG
    }
    hr = D3DDevice_SetRenderState(D3DRS_PSCOMPAREMODE, pPSDef->PSCompareMode);
#if NXDK_DEBUG
    if (FAILED(hr)) return hr;
#endif // NXDK_DEBUG
    hr = D3DDevice_SetRenderState(D3DRS_PSFINALCOMBINERCONSTANT0,
                                  pPSDef->PSFinalCombinerConstant0);
#if NXDK_DEBUG
    if (FAILED(hr)) return hr;
#endif // NXDK_DEBUG
    hr = D3DDevice_SetRenderState(D3DRS_PSFINALCOMBINERCONSTANT1, 
                                  pPSDef->PSFinalCombinerConstant1);
#if NXDK_DEBUG
    if (FAILED(hr)) return hr;
#endif // NXDK_DEBUG
    for (int i = 0; i < 8; i++) {
        hr = D3DDevice_SetRenderState(D3DRS_PSRGBOUTPUTS0 + i, 
                                      pPSDef->PSRGBOutputs[i]);
#if NXDK_DEBUG
        if (FAILED(hr)) return hr;
#endif // NXDK_DEBUG
    }
    hr = D3DDevice_SetRenderState(D3DRS_PSCOMBINERCOUNT, 
                                  pPSDef->PSCombinerCount);
#if NXDK_DEBUG
    if (FAILED(hr)) return hr;
#endif // NXDK_DEBUG
    hr = D3DDevice_SetRenderState(D3DRS_PSTEXTUREMODES, 
                                  pPSDef->PSTextureModes);
#if NXDK_DEBUG
    if (FAILED(hr)) return hr;
#endif // NXDK_DEBUG
    hr = D3DDevice_SetRenderState(D3DRS_PSDOTMAPPING, pPSDef->PSDotMapping);
#if NXDK_DEBUG
    if (FAILED(hr)) return hr;
#endif // NXDK_DEBUG
    return D3DDevice_SetRenderState(D3DRS_PSINPUTTEXTURE, 
                                    pPSDef->PSInputTexture);
}

HRESULT IDirect3DDevice8_SetPixelShaderProgram(LPDIRECT3DDEVICE8 pThis, 
                                               CONST D3DPIXELSHADERDEF *pPSDef)
{
    assert(pThis == &g_d3ddev.iface);
    return D3DDevice_SetPixelShaderProgram(pPSDef);
}

HRESULT D3DDevice_ClearRect(CONST D3DRECT* pRect, DWORD Flags) {
    assert((Flags & 0x0C) == 0);
    HRESULT hr = D3DDevice_PushCmd(NV097_SET_CLEAR_RECT_HORIZONTAL, 
                                   ((pRect->x1 - 1) << 16) | 
                                    (pRect->x2 & 0xFFFF));
    if (FAILED(hr)) return hr;
    hr = D3DDevice_PushCmd(NV097_SET_CLEAR_RECT_VERTICAL, 
        ((pRect->y1 - 1) << 16) | (pRect->y2 & 0xFFFF));
    if (FAILED(hr)) return hr;
    return D3DDevice_PushCmd(NV097_CLEAR_SURFACE, Flags & 0xF3);
}

HRESULT D3DDevice_Clear(DWORD Count, CONST D3DRECT* pRects, DWORD Flags,
                        D3DCOLOR Color, float Z, DWORD Stencil)
{
    HRESULT hr = D3D_OK;
    if (Flags & D3DCLEAR_ZSTENCIL) {
        hr = D3DDevice_PushCmd(NV097_SET_ZSTENCIL_CLEAR_VALUE, 
           (((DWORD)(Z * D3DZ_MAX_D24S8) & 0x00FFFFFF) << 8) | Stencil);
        if (FAILED(hr)) return hr;
    }

    if (Flags & D3DCLEAR_TARGET) {
        hr = D3DDevice_PushCmd(NV097_SET_COLOR_CLEAR_VALUE, Color);
        if (FAILED(hr)) return hr;
    }

    if (pRects == NULL) {
#if NXDK_DEBUG
        if (Count > 0)
            return D3DERR_INVALIDCALL;
#endif // NXDK_DEBUG

        D3DSURFACE_DESC desc;
        hr = IDirect3DSurface8_GetDesc(
            &g_d3ddev.surfaces[g_d3ddev.current_surface]->iface, &desc);
        D3DRECT rect;
        rect.x1 = 0;
        rect.y1 = 0;
        rect.x2 = desc.Width;
        rect.y2 = desc.Height;
        return D3DDevice_ClearRect(&rect, Flags);
    }

    for (int i = 0; i < Count; i++) {
        hr = D3DDevice_ClearRect(&pRects[i], Flags);
#if NXDK_DEBUG
        if (FAILED(hr)) return hr;
#endif // NXDK_DEBUG
    }
    return hr;
}

HRESULT IDirect3DDevice8_Clear(LPDIRECT3DDEVICE8 pThis, DWORD Count, 
                               CONST D3DRECT* pRects, DWORD Flags,
                               D3DCOLOR Color, float Z, DWORD Stencil)
{
    assert(pThis == &g_d3ddev.iface);
    return D3DDevice_Clear(Count, pRects, Flags, Color, Z, Stencil);
}

HRESULT D3DDevice_SetScissorRect(DWORD Index, DWORD X, DWORD Y,
                                 DWORD Width, DWORD Height) 
{
    HRESULT hr = D3DDevice_PushCmd(
        NV20_TCL_PRIMITIVE_3D_VIEWPORT_CLIP_HORIZ(Index), X | (Width << 16));
    if (FAILED(hr)) return hr;
    return D3DDevice_PushCmd(
        NV20_TCL_PRIMITIVE_3D_VIEWPORT_CLIP_VERT(Index), Y | (Height << 16));
}

HRESULT D3DDevice_SetScissors(DWORD Count, BOOL Exclusive, 
                              CONST D3DRECT *pRects)
{
#if NXDK_DEBUG
    if (Count >= D3DSCISSORS_MAX)
        return D3DERR_INVALIDCALL;
#endif // NXDK_DEBUG

    if (Count == 0) {
        HRESULT hr = D3DDevice_PushCmd(
            NV20_TCL_PRIMITIVE_3D_VIEWPORT_CLIP_MODE, 0);
        D3DVIEWPORT8* pViewport = &g_d3ddev.viewport;
        return D3DDevice_SetScissorRect(0, pViewport->X, pViewport->Y, 
                                        pViewport->Width, pViewport->Height);
    }

    if (Exclusive != 0)
        Exclusive = 1;
    
    HRESULT hr = D3DDevice_PushCmd(
        NV20_TCL_PRIMITIVE_3D_VIEWPORT_CLIP_MODE, Exclusive);
    if (FAILED(hr)) return hr;

    for (int i = 0; i < Count; i++) {
        CONST D3DRECT* pRect = &pRects[i];
        DWORD Width  = pRect->x2 - pRect->x1;
        DWORD Height = pRect->y2 - pRect->y1;
        hr = D3DDevice_SetScissorRect(i, pRect->x1, pRect->y1, Width, Height);
#if NXDK_DEBUG
        if (FAILED(hr)) return hr;
#endif // NXDK_DEBUG
    }

    for (int i = Count; i < D3DSCISSORS_MAX; i++) {
        DWORD Width = 0;
        DWORD Height = 0;
        if (Exclusive == FALSE) {
            Width  = 0xFFFFFFFF;
            Height = 0xFFFFFFFF;
        }
        hr = D3DDevice_SetScissorRect(i, 0, 0, Width, Height);
#if NXDK_DEBUG
        if (FAILED(hr)) return hr;
#endif // NXDK_DEBUG
    }

    return hr;
}

HRESULT IDirect3DDevice8_SetScissors(LPDIRECT3DDEVICE8 pThis, DWORD Count, 
                                     BOOL Exclusive, CONST D3DRECT *pRects)
{
    assert(pThis == &g_d3ddev.iface);
    return D3DDevice_SetScissors(Count, Exclusive, pRects);
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
    }
    else if (pTile->Pitch <= D3DTILE_PITCH_1000) {
        if (pTile->Pitch & 0xE1FF)
            return D3DERR_INVALIDCALL;
    }
    else if (pTile->Pitch <= D3DTILE_PITCH_2000) {
        if (pTile->Pitch & 0xD3FF)
            return D3DERR_INVALIDCALL;
    }
    else if (pTile->Pitch <= D3DTILE_PITCH_4000) {
        if (pTile->Pitch & 0xC7FF)
            return D3DERR_INVALIDCALL;
    }
    else if (pTile->Pitch <= D3DTILE_PITCH_E000) {
        if (pTile->Pitch & 0x0FFF)
            return D3DERR_INVALIDCALL;
    }
#endif // NXDK_DEBUG
    pb_assign_tile(Index, (DWORD)pTile->pMemory, pTile->Size, pTile->Pitch, 
                   pTile->ZStartTag, pTile->ZOffset, pTile->Flags);
    return D3D_OK;
}

HRESULT IDirect3DDevice8_SetTile(LPDIRECT3DDEVICE8 pThis, DWORD Index, 
                                 CONST D3DTILE* pTile) 
{
    assert(pThis == &g_d3ddev.iface);
    return D3DDevice_SetTile(Index, pTile);
}

HRESULT D3DDevice_SetTexture(DWORD Stage, LPDIRECT3DBASETEXTURE8 pTexture) {
    if (pTexture == NULL) {
        IDirect3DBaseTexture8_Release(&g_d3ddev.pTexture[Stage]->iface);
        g_d3ddev.pTexture[Stage] = NULL;
        return D3DDevice_PushCmd(NV097_SET_TEXTURE_CONTROL0(Stage), 
            MASK(NV097_SET_TEXTURE_CONTROL0_MAX_LOD_CLAMP, 0x3ffc0));
    }

    IDirect3DBaseTexture8_AddRef(&g_d3ddev.pTexture[Stage]->iface);
    D3DBaseTexture* pBase = (D3DBaseTexture*)pTexture;
    g_d3ddev.pTexture[Stage] = pBase;

    D3DRESOURCETYPE Type = IDirect3DBaseTexture8_GetType(pTexture);
    if (Type != D3DRTYPE_TEXTURE) {
        if (Type == D3DRTYPE_VOLUMETEXTURE || Type == D3DRTYPE_CUBETEXTURE)
            return E_NOTIMPL;
        else
            return D3DERR_INVALIDCALL;
    }

    D3DTexture* pTex = (D3DTexture*)pBase;
    LPDIRECT3DSURFACE8* pSurface = NULL;
    HRESULT hr = IDirect3DTexture8_GetSurfaceLevel((LPDIRECT3DTEXTURE8)pTex, 0, (LPDIRECT3DSURFACE8*)&pSurface);
#if NXDK_DEBUG
    if (FAILED(hr)) return hr;
#endif // NXDK_DEBUG

    D3DSurface* pSurf = (D3DSurface*)pSurface;
    D3DResourceInner* pResource = &pSurf->resource;
    hr = D3DDevice_PushCmd(NV097_SET_TEXTURE_OFFSET(Stage), 
                           (DWORD)pResource->pContiguousMemory);
    if (FAILED(hr)) return hr;

    D3DSURFACE_DESC desc;
    hr = IDirect3DSurface8_GetDesc((LPDIRECT3DSURFACE8)pSurface, &desc);
#if NXDK_DEBUG
    if (FAILED(hr)) return hr;
#endif // NXDK_DEBUG

    hr = D3DDevice_PushCmd(NV097_SET_TEXTURE_IMAGE_RECT(Stage), 
        MASK(NV097_SET_TEXTURE_IMAGE_RECT_WIDTH,  desc.Width) |
        MASK(NV097_SET_TEXTURE_IMAGE_RECT_HEIGHT, desc.Height));
    if (FAILED(hr)) return hr;

    hr = D3DDevice_PushCmd(NV097_SET_TEXTURE_CONTROL0(Stage), 
        NV097_SET_TEXTURE_CONTROL0_ENABLE |
        MASK(NV097_SET_TEXTURE_CONTROL0_MAX_LOD_CLAMP, 0x0003ffc0));
    if (FAILED(hr)) return hr;

    UINT Pitch = desc.Width * D3D_FormatBytesPerPixel(desc.Format);
    hr = D3DDevice_PushCmd(NV097_SET_TEXTURE_CONTROL1(Stage),
        MASK(NV097_SET_TEXTURE_CONTROL1_IMAGE_PITCH, Pitch));
    if (FAILED(hr)) return hr;

    DWORD dwLevelCount = IDirect3DBaseTexture8_GetLevelCount(pTexture);

    return D3DDevice_PushCmd(NV097_SET_TEXTURE_FORMAT(Stage), 
        MASK(NV097_SET_TEXTURE_FORMAT_MIPMAP_LEVELS, dwLevelCount) |
        MASK(NV097_SET_TEXTURE_FORMAT_COLOR, desc.Format) |
        MASK(NV097_SET_TEXTURE_FORMAT_DIMENSIONALITY, 2) |
        MASK(NV097_SET_TEXTURE_FORMAT_BORDER_SOURCE, desc.Usage) |
        MASK(NV097_SET_TEXTURE_FORMAT_CONTEXT_DMA, 2));
}

HRESULT IDirect3DDevice8_SetTexture(LPDIRECT3DDEVICE8 pThis, DWORD Stage, 
                                    LPDIRECT3DBASETEXTURE8 pTexture)
{
    assert(pThis == &g_d3ddev.iface);
    return D3DDevice_SetTexture(Stage, pTexture);
}
// ============================================================================

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
    g_d3ddevVtbl.BlockUntilVerticalBlank    = IDirect3DDevice8_BlockUntilVerticalBlank;
    g_d3ddevVtbl.CreateImageSurface         = IDirect3DDevice8_CreateImageSurface;
    g_d3ddevVtbl.CreateDepthStencilSurface  = IDirect3DDevice8_CreateDepthStencilSurface;
    g_d3ddevVtbl.CreateVertexBuffer         = IDirect3DDevice8_CreateVertexBuffer;
    g_d3ddevVtbl.CreatePushBuffer           = IDirect3DDevice8_CreatePushBuffer;
    g_d3ddevVtbl.BeginScene                 = IDirect3DDevice8_BeginScene;
    g_d3ddevVtbl.EndScene                   = IDirect3DDevice8_EndScene;
    g_d3ddevVtbl.Present                    = IDirect3DDevice8_Present;
    g_d3ddevVtbl.DrawVertices               = IDirect3DDevice8_DrawVertices;
    g_d3ddevVtbl.DrawPrimitive              = IDirect3DDevice8_DrawPrimitive;
    g_d3ddevVtbl.SetViewport                = IDirect3DDevice8_SetViewport;
    g_d3ddevVtbl.CreateTexture              = IDirect3DDevice8_CreateTexture;
    g_d3ddevVtbl.SetRenderState             = IDirect3DDevice8_SetRenderState;
    g_d3ddevVtbl.SetTextureStageState       = IDirect3DDevice8_SetTextureStageState;
    g_d3ddevVtbl.SetVertexShaderInputDirect = IDirect3DDevice8_SetVertexShaderInputDirect;
    g_d3ddevVtbl.LoadVertexShaderProgram    = IDirect3DDevice8_LoadVertexShaderProgram;
    g_d3ddevVtbl.SetPixelShaderProgram      = IDirect3DDevice8_SetPixelShaderProgram;
    g_d3ddevVtbl.Clear                      = IDirect3DDevice8_Clear;
    g_d3ddevVtbl.SetScissors                = IDirect3DDevice8_SetScissors;
    g_d3ddevVtbl.SetTile                    = IDirect3DDevice8_SetTile;
    g_d3ddevVtbl.SetTexture                 = IDirect3DDevice8_SetTexture;
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

    g_d3dTextureVtbl.AddRef                 = IDirect3DTexture8_AddRef;
    g_d3dTextureVtbl.Release                = IDirect3DTexture8_Release;
    g_d3dTextureVtbl.GetType                = IDirect3DTexture8_GetType;
    g_d3dTextureVtbl.Register               = IDirect3DTexture8_Register;
    g_d3dTextureVtbl.GetLevelCount          = IDirect3DTexture8_GetLevelCount;
    g_d3dTextureVtbl.GetSurfaceLevel        = IDirect3DTexture8_GetSurfaceLevel;
    g_d3dTextureVtbl.GetLevelDesc           = IDirect3DTexture8_GetLevelDesc;
    g_d3dTextureVtbl.LockRect               = IDirect3DTexture8_LockRect;
    g_d3dTextureVtbl.UnlockRect             = IDirect3DTexture8_UnlockRect;

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

    g_d3dPushBufferVtbl.AddRef              = IDirect3DPushBuffer8_AddRef;
    g_d3dPushBufferVtbl.Release             = IDirect3DPushBuffer8_Release;
    g_d3dPushBufferVtbl.GetType             = IDirect3DPushBuffer8_GetType;
    g_d3dPushBufferVtbl.Register            = IDirect3DPushBuffer8_Register;
    g_d3dPushBufferVtbl.BlockUntilNotBusy   = IDirect3DPushBuffer8_BlockUntilNotBusy;
    g_d3dPushBufferVtbl.IsBusy              = IDirect3DPushBuffer8_IsBusy;
    g_d3dPushBufferVtbl.GetSize             = IDirect3DPushBuffer8_GetSize;

    g_d3d.dwDisplayModeCount = 0;
    VIDEO_MODE vm;
    memset(&vm, 0, sizeof(vm));
    void* p = NULL;
    while(XVideoListModes(&vm, 15, REFRESH_DEFAULT, &p)) {
        D3DDISPLAYMODE* pDisplayMode = 
            &g_d3d.displayModes[g_d3d.dwDisplayModeCount];
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
        D3DDISPLAYMODE* pDisplayMode = 
            &g_d3d.displayModes[g_d3d.dwDisplayModeCount];
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
        D3DDISPLAYMODE* pDisplayMode =
            &g_d3d.displayModes[g_d3d.dwDisplayModeCount];
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
    g_d3d.dwCurrentDisplayMode = g_d3d.dwDisplayModeCount - 1;
    g_d3d.KickOffSize = 32 * 1024 / sizeof(DWORD);

    return &g_d3d.iface;
}
