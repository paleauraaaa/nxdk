#include <assert.h>
#include <stdlib.h>

#include <hal/video.h>
#include <pbkit/pbkit.h>
#include <xboxkrnl/xboxkrnl.h>

#include "d3d8.h"

#define MASK(mask, val) (((val) << (__builtin_ffs(mask)-1)) & (mask))

static IDirect3DResourceVtbl8     g_d3dResourceVtbl;
static IDirect3DBaseTextureVtbl8  g_d3dBaseTextureVtbl;
static IDirect3DTextureVtbl8      g_d3dTextureVtbl;
static IDirect3DSurfaceVtbl8      g_d3dSurfaceVtbl;
static IDirect3DVertexBufferVtbl8 g_d3dVertexBufferVtbl;
static IDirect3DPushBufferVtbl8   g_d3dPushBufferVtbl;

PVOID D3D_AllocContiguousMemory(DWORD Size, DWORD Alignment) {
    return MmAllocateContiguousMemoryEx(Size, 0, MAXRAM, Alignment,
                                        PAGE_READWRITE | PAGE_WRITECOMBINE);
}

void D3D_FreeContiguousMemory(PVOID Base) {
    MmFreeContiguousMemory(Base);
}

// ============================================================================
typedef struct D3DResourceInner {
    ULONG              refcount;
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
    D3DResource* resource = (D3DResource*)pThis;
    resource->inner.refcount++;
    return resource->inner.refcount;
}

ULONG IDirect3DResource8_Release(LPDIRECT3DRESOURCE8 pThis) {
    D3DResource* resource = (D3DResource*)pThis;
    if(resource->inner.refcount == 0)
        return 0;
    resource->inner.refcount--;
    if (resource->inner.refcount > 0)
        return resource->inner.refcount;
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
    D3DPushBuffer* pb = (D3DPushBuffer*)pThis;
    pb->resource.refcount++;
    return pb->resource.refcount;
}

ULONG IDirect3DPushBuffer8_Release(LPDIRECT3DPUSHBUFFER8 pThis) {
    D3DPushBuffer* pb = (D3DPushBuffer*)pThis;
    if(pb->resource.refcount == 0)
        return 0;
    pb->resource.refcount--;
    if (pb->resource.refcount > 0)
        return pb->resource.refcount;

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
    if (pb->bCpu == TRUE)
        return pb->SizeNeeded >= pb->Size;
    
    if (pb->p + pb->SizeNeeded >= 
        ((PDWORD)pb->resource.pContiguousMemory + pb->Size))
    {
        return TRUE;
    }

    return FALSE;
}

HRESULT D3DPushBuffer_Push1(
    D3DPushBuffer* pThis, DWORD dwData)
{
    if (pThis->SizeNeeded > pThis->Size - 1) {
        pThis->SizeNeeded += 1;
        return D3DERR_BUFFERTOOSMALL;
    }

    *(pThis->p) = dwData;
    pThis->p++;
    pThis->SizeNeeded += 1;
    return D3D_OK;
}

HRESULT D3DPushBuffer_PushCmd(
    D3DPushBuffer* pThis, DWORD cmd, DWORD dwData)
{
    if (pThis->SizeNeeded > pThis->Size - 2) {
        pThis->SizeNeeded += 2;
        return D3DERR_BUFFERTOOSMALL;
    }

    pThis->p = (PDWORD)pb_push1((uint32_t*)pThis->p, cmd, dwData);
    pThis->SizeNeeded += 2;
    return D3D_OK;
}

HRESULT D3DPushBuffer_PushN(
    D3DPushBuffer* pThis, DWORD dwData, SIZE_T n)
{
    if (pThis->SizeNeeded > pThis->Size - n) {
        pThis->SizeNeeded += n;
        return D3DERR_BUFFERTOOSMALL;
    }

    memset(pThis->p, dwData, n * sizeof(DWORD));
    pThis->p += n;
    pThis->SizeNeeded += n;
    return D3D_OK;
}

HRESULT D3DPushBuffer_PushA(
    D3DPushBuffer* pThis, CONST DWORD* pdwData, SIZE_T n)
{
    if (pThis->SizeNeeded > pThis->Size - n) {
        pThis->SizeNeeded += n;
        return D3DERR_BUFFERTOOSMALL;
    }

    memcpy(pThis->p, pdwData, n * sizeof(DWORD));
    pThis->p += n;
    pThis->SizeNeeded += n;
    return D3D_OK;
}
// ============================================================================

struct D3DSurface;
typedef struct D3DSurface D3DSurface;

// ============================================================================
typedef struct D3DDevice IMPLEMENTS(IDirect3DDevice8) {
    IDirect3DDevice8 iface;
    ULONG refcount;
    DWORD KickOffSize;
    D3DSurface* surfaces[3];
    DWORD current_surface;
    DWORD max_surfaces;
    D3DSurface* depth_stencil_surface;
    D3DPushBuffer default_pb;
    D3DPushBuffer* current_pb;
    BOOL in_scene;
#ifdef __cplusplus
    ULONG AddRef() override {
        return iface->lpVtbl->AddRef(&this->iface);
    }

    ULONG Release() override {
        return iface->lpVtbl->Release(&this->iface);
    }

    HRESULT CreateImageSurface(UINT Width, UINT Height,
                                   D3DFORMAT Format, 
                                   LPDIRECT3DSURFACE8* ppSurface) override 
    {
        return iface->lpVtbl->CreateImageSurface(&this->iface, Width, Height,
                                                 Format, ppSurface);
    }

    HRESULT CreateDepthStencilSurface(
        UINT Width, UINT Height, 
        D3DFORMAT Format, 
        D3DMULTISAMPLE_TYPE MultiSampleType,
        LPDIRECT3DSURFACE8* ppSurface) override 
    {
        return iface->lpVtbl->CreateDepthStencilSurface(&this->iface, Width,
                                                        Height, Format,
                                                        MultiSampleType,
                                                        ppSurface);
    }

    HRESULT CreateVertexBuffer(
        UINT Length, DWORD Usage, DWORD FVF, 
        D3DPOOL Pool, LPDIRECT3DVERTEXBUFFER8* ppVertexBuffer) override 
    {
        return iface->lpVtbl->CreateVertexBuffer(&this->iface, Length, Usage,
                                                 FVF, Pool, ppVertexBuffer);
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
        return iface->lpVtbl->CreateTexture(&this->iface, Width, Height, 
                                            Levels, Usage, Format, Pool, 
                                            ppTexture);
    }

    HRESULT CreatePushBuffer(
        UINT Size, BOOL RunUsingCpuCopy, 
        LPDIRECT3DPUSHBUFFER8* ppPushBuffer) override
    {
        return iface->lpVtbl->CreatePushBuffer(&this->iface, Size, 
                                               RunUsingCpuCopy, ppPushBuffer);
    }

    HRESULT BeginScene() override {
        return iface->lpVtbl->BeginScene(&this->iface);
    }

    HRESULT EndScene() override {
        return iface->lpVtbl->EndScene(&this->iface);
    }

    HRESULT Present(CONST RECT* pSourceRect, CONST RECT* pDestRect) override {
        return iface->lpVtbl->Present(&this->iface, pSourceRect, pDestRect);
    }

    HRESULT DrawPrimitive(
        D3DPRIMITIVETYPE PrimitiveType, 
        UINT StartVertex, UINT PrimitiveCount) override 
    {
        return iface->lpVtbl->DrawPrimitive(&this->iface, PrimitiveType,
                                           StartVertex, PrimitiveCount);
    }

    HRESULT SetViewport(CONST D3DVIEWPORT8* pViewport) override {
        return iface->lpVtbl->SetViewport(&this->iface, pViewport);
    }

    HRESULT SetVertexShaderInputDirect(
        D3DVERTEXATTRIBUTEFORMAT *pVAF,
        UINT StreamCount, 
        D3DSTREAM_INPUT *pStreamInputs) override 
    {
        return iface->lpVtbl->SetVertexShaderInputDirect(&this->iface, pVAF,
                                                         StreamCount,
                                                         pStreamInputs);
    }

    HRESULT SetTextureStageState(
        DWORD Stage,
        D3DTEXTURESTAGESTATETYPE Type,
        DWORD Value) override 
    {
        return iface->lpVtbl->SetTextureStageState(&this->iface, Stage, Type,
                                                   Value);
    }

    HRESULT SetRenderState(D3DRENDERSTATETYPE Type, DWORD Value) override {
        return iface->lpVtbl->SetRenderState(&this->iface, Type, Value);
    }

    HRESULT LoadVertexShaderProgram(CONST DWORD *pFunction, 
                                    DWORD Address) override 
    {
        return iface->lpVtbl->LoadVertexShaderProgram(&this->iface, 
                                                      pFunction, Address);
    }

    HRESULT SetPixelShaderProgram(CONST D3DPIXELSHADERDEF *pPSDef) override {
        return iface->lpVtbl->LoadVertexShaderProgram(&this->iface, pPSDef);
    }

    HRESULT Clear(DWORD Count, CONST D3DRECT* pRects, DWORD Flags,
                  D3DCOLOR Color, float Z, DWORD Stencil) override {
        return iface->lpVtbl->Clear(&this->iface, Count, pRects, Flags, Color, Z, Stencil);
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
        return iface->lpVtbl->AddRef(&this->iface);
    }

    ULONG Release() override {
        return iface->lpVtbl->Release(&this->iface);
    }

    UINT GetAdapterCount() override { 
        return iface->lpVtbl->GetAdapterCount(&this->iface); 
    }
    HRESULT GetAdapterIdentifier(
        UINT Adapter, DWORD Flags, 
        D3DADAPTER_IDENTIFIER8* pIdentifier) override 
    { 
        return iface->lpVtbl->GetAdapterIdentifier(&this->iface, Adapter, 
                                                   Flags, pIdentifier);
    }
    UINT GetAdapterModeCount(UINT Adapter) override { 
        return iface->lpVtbl->GetAdapterModeCount(&this->iface, Adapter); }
    HRESULT EnumAdapterModes(UINT Adapter, UINT Mode, 
                             D3DDISPLAYMODE* pMode) override 
    { 
        return iface->lpVtbl->EnumAdapterModes(&this->iface, Adapter, 
                                               Mode, pMode);
    }
    HRESULT GetAdapterDisplayMode(UINT Adapter, 
                                  D3DDISPLAYMODE* pMode) override 
    {
        return iface->lpVtbl->GetAdapterDisplayMode(&this->iface, Adapter, 
                                                    pMode);
    }
    HRESULT CheckDeviceType(UINT Adapter, D3DDEVTYPE CheckType, 
                            D3DFORMAT DisplayFormat, 
                            D3DFORMAT BackBufferFormat, 
                            BOOL Windowed) override 
    { 
       return iface->lpVtbl->CheckDeviceType(&this->iface, Adapter, 
                                             CheckDeviceType, DisplayFormat,
                                             BackBufferFormat, Windowed); 

    }
    HRESULT CheckDeviceFormat(UINT Adapter, D3DDEVTYPE DeviceType, 
                              D3DFORMAT AdapterFormat, DWORD Usage, 
                              D3DRESOURCETYPE RType, 
                              D3DFORMAT CheckFormat) override 
    { 
        return iface->lpVtbl->CheckDeviceFormat(&this->iface, Adapter,
                                                DeviceType, AdapterFormat,
                                                Usage, RType, CheckFormat); 
    }
    HRESULT CheckDeviceMultiSampleType(
        UINT Adapter, D3DDEVTYPE DeviceType, 
        D3DFORMAT SurfaceFormat, BOOL Windowed, 
        D3DMULTISAMPLE_TYPE MultiSampleType) override 
    {
        return iface->lpVtbl->CheckDeviceMultiSampleType(&this->iface, Adapter,
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
        return iface->lpVtbl->CheckDepthStencilMatch(&this->iface, Adapter, 
                                                     DeviceType, AdapterFormat,
                                                     RenderTargetFormat, 
                                                     DepthStencilFormat); 
    }

    HRESULT GetDeviceCaps(UINT Adapter, D3DDEVTYPE DeviceType, 
                          D3DCAPS8* pCaps) override 
    { 
        return iface->lpVtbl->GetDeviceCaps(&this->iface, Adapter, 
                                            DeviceType, pCaps);

    }
    HMONITOR GetAdapterMonitor(UINT Adapter) override {
        return iface->lpVtbl->GetAdapterMonitor(&this->iface, Adapter);
    }
    HRESULT CreateDevice(
        UINT Adapter, D3DDEVTYPE DeviceType, HWND hFocusWindow, 
        DWORD BehaviorFlags, D3DPRESENT_PARAMETERS* pPresentationParameters, 
        LPDIRECT3DDEVICE8* ppReturnedDeviceInterface) override 
    { 
        return iface->lpVtbl->CreateDevice(&this->iface, Adapter, DeviceType,
                                           hFocusWindow, BehaviorFlags,
                                           pPresentationParameters,
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
        return D3DERR_INVALIDDEVICE;
    return Direct3D_CheckDepthStencilMatch(AdapterFormat, RenderTargetFormat,
                                           DepthStencilFormat);
}

HRESULT Direct3D_SetPushBufferSize(DWORD PushBufferSize, DWORD KickOffSize) {
    if (PushBufferSize < 64*1024)
        return D3DERR_INVALIDCALL;
    
    pb_size(PushBufferSize);
    g_d3d.KickOffSize = KickOffSize;
    g_d3d.PushBufferSize = PushBufferSize;
    return D3D_OK;
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
    D3DResourceInner  resource;
    D3DSURFACE_DESC   desc;
    D3DLock           lock;
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

static VOID D3D_CreateResource(D3DRESOURCETYPE type, DWORD Data, 
                               PVOID pContiguousMemory, 
                               D3DResourceInner* pResource) 
{
    pResource->refcount            = 1;
    pResource->type                = type;
    pResource->bManuallyRegistered = FALSE;
    pResource->Data                = Data;
    pResource->pContiguousMemory   = pContiguousMemory;
}

static VOID D3D_CreateSurface(UINT Width, UINT Height, D3DFORMAT Format,
                              DWORD Usage, 
                              D3DMULTISAMPLE_TYPE MultiSampleType, 
                              PVOID pContiguousMemory,
                              D3DSurface* pSurf) 
{
    pSurf->iface.lpVtbl = &g_d3dSurfaceVtbl;
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
    if (bCpu == TRUE && pContiguousMemory != NULL) {
        return D3DERR_INVALIDCALL;
    }
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
        pPB->p = NULL;
    }
    return D3D_OK;
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
        pb_set_ds_addr(pDSSurf->resource.pContiguousMemory);
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
            g_d3ddev.depth_stencil_surface);
    }
    pb_show_front_screen();

    g_d3ddev.refcount = 1;
    g_d3ddev.iface.lpVtbl = &g_d3ddevVtbl;
    g_d3ddev.KickOffSize = g_d3d.KickOffSize;
    g_d3ddev.in_scene = FALSE;
    g_d3ddev.max_surfaces = BackBufferCount;
    g_d3ddev.current_surface = 0;
    g_d3ddev.default_pb.iface.lpVtbl = &g_d3dPushBufferVtbl;
    D3D_CreateResource(D3DRTYPE_PUSHBUFFER, 0, NULL, 
                                    &g_d3ddev.default_pb.resource);
    HRESULT hr = D3D_CreatePushBuffer(g_d3d.PushBufferSize, FALSE, 
                              NULL, &g_d3ddev.default_pb);
    if (FAILED(hr))
        return hr;
    g_d3ddev.current_pb = &g_d3ddev.default_pb;

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
        return D3DERR_INVALIDDEVICE;
    if (hFocusWindow != NULL) 
        return D3DERR_INVALIDCALL;

    HRESULT hr = Direct3D_CreateDevice(Adapter, BehaviorFlags, 
                                       pPresentationParameters);
    *ppReturnedDevice = &g_d3ddev.iface;
    return hr;
}

IDirect3DVtbl8 g_d3dvtbl;

BOOL g_firstCreate = TRUE;

// ============================================================================
ULONG IDirect3DSurface8_AddRef(LPDIRECT3DSURFACE8 pThis) {
    D3DSurface* surface = (D3DSurface*)pThis;
    surface->resource.refcount++;
    return surface->resource.refcount;
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
        return D3DERR_INVALIDCALL;
    }
}

HRESULT IDirect3DSurface8_UnlockRect(LPDIRECT3DSURFACE8 pThis) {
    ((D3DSurface*)pThis)->lock.bLocked = FALSE;
    return D3D_OK;
}
// ============================================================================

// ============================================================================
ULONG IDirect3DVertexBuffer8_AddRef(LPDIRECT3DVERTEXBUFFER8 pThis) {
    D3DVertexBuffer* vb = (D3DVertexBuffer*)pThis;
    vb->resource.refcount++;
    return vb->resource.refcount;
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

typedef struct D3DBaseTexture IMPLEMENTS(IDirect3DBaseTexture8) {
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
} D3DBaseTexture;

ULONG IDirect3DBaseTexture8_AddRef(LPDIRECT3DBASETEXTURE8 pThis) {
    D3DBaseTexture* tex = (D3DBaseTexture*)pThis;
    tex->inner.resource.refcount++;
    return tex->inner.resource.refcount;
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
    ULONG refcount = texture->base.resource.refcount; 
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
    if (Level >= texture->base.dwLevelCount)
        return D3DERR_INVALIDCALL;
    *ppSurfaceLevel = &texture->pLevels[Level].iface;
    IDirect3DSurface8_AddRef(*ppSurfaceLevel);
    return D3D_OK;
}

HRESULT IDirect3DTexture8_GetLevelDesc(LPDIRECT3DTEXTURE8 pThis, UINT Level, 
                                       D3DSURFACE_DESC* pDesc) 
{
    D3DTexture* texture = (D3DTexture*)pThis;
    if (Level >= texture->base.dwLevelCount)
        return D3DERR_INVALIDCALL;
    *pDesc = texture->pLevels[Level].desc;
    return D3D_OK;
}

HRESULT IDirect3DTexture8_LockRect(
    LPDIRECT3DTEXTURE8 pThis, UINT Level, 
    D3DLOCKED_RECT* pLockedRect, CONST RECT* pRect, DWORD Flags)
{
    D3DTexture* texture = (D3DTexture*)pThis;
    if (Level >= texture->base.dwLevelCount)
        return D3DERR_INVALIDCALL;
        
    texture->pLevels[Level].lock.bLocked = TRUE;
    texture->pLevels[Level].lock.Flags   = Flags;
    if (pRect)
        texture->pLevels[Level].lock.Rect = *pRect;
    else
        memset(&texture->pLevels[Level].lock.Rect, 0, 
               sizeof(texture->pLevels[Level].lock.Rect));
    if (Level >= texture->base.dwLevelCount)
        return D3DERR_INVALIDCALL;

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
        return D3DERR_INVALIDCALL;
    }

    return D3D_OK;
}

HRESULT IDirect3DTexture8_UnlockRect(LPDIRECT3DTEXTURE8 pThis, UINT Level) {
    D3DTexture* texture = (D3DTexture*)pThis;
    if (Level >= (texture->base).dwLevelCount)
        return D3DERR_INVALIDCALL;

    texture->pLevels[0].lock.bLocked = FALSE;
    return D3D_OK;
}
// ============================================================================

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
    D3D_CreateSurface(
        Width, Height, Format, 0, 0, NULL, surf);
    surf->resource.pContiguousMemory = 
        D3D_AllocContiguousMemory(surf->desc.Size, D3DSURFACE_ALIGNMENT);
    if (surf->resource.pContiguousMemory == NULL) {
        free(surf);
        return D3DERR_OUTOFVIDEOMEMORY;
    }
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
    if (g_d3ddev.current_pb == NULL)
        return D3DERR_INVALIDCALL;

    if (g_d3ddev.current_pb->bCpu == TRUE) {
        uint32_t* p = pb_begin();
        memcpy(p, g_d3ddev.current_pb->p, 
               g_d3ddev.current_pb->SizeNeeded * sizeof(DWORD));
        p += g_d3ddev.current_pb->SizeNeeded;
        pb_end(p);
        return D3D_OK;
    }
    
    HRESULT hr = D3DDevice_EndPushBuffer();
    if (FAILED(hr)) return hr;
    return D3DDevice_BeginPushBuffer(g_d3ddev.current_pb);
}

HRESULT D3DDevice_Push1(DWORD dwData) {
    if (D3DDevice_IsKickoffReady() == TRUE) {
        HRESULT hr = D3DDevice_SendKickoff();
        if (FAILED(hr))
            return hr;
    }
    return D3DPushBuffer_Push1(g_d3ddev.current_pb, dwData);
}

HRESULT D3DDevice_PushCmd(DWORD cmd, DWORD dwData) {
    if (D3DDevice_IsKickoffReady() == TRUE) {
        HRESULT hr = D3DDevice_SendKickoff();
        if (FAILED(hr))
            return hr;
    }
    return D3DPushBuffer_PushCmd(g_d3ddev.current_pb, cmd, dwData);
}

HRESULT D3DDevice_PushN(DWORD dwData, SIZE_T n) {
    if (D3DDevice_IsKickoffReady() == TRUE) {
        HRESULT hr = D3DDevice_SendKickoff();
        if (FAILED(hr))
            return hr;
    }
    return D3DPushBuffer_PushN(g_d3ddev.current_pb, dwData, n);
}

HRESULT D3DDevice_PushA(CONST DWORD* pdwData, SIZE_T n) {
    if (D3DDevice_IsKickoffReady() == TRUE) {
        HRESULT hr = D3DDevice_SendKickoff();
        if (FAILED(hr))
            return hr;
    }
    return D3DPushBuffer_PushA(g_d3ddev.current_pb, pdwData, n);
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
    /* Clear depth & stencil buffers */
    D3DSURFACE_DESC ds_desc;
    HRESULT hr = IDirect3DSurface8_GetDesc(
        &g_d3ddev.depth_stencil_surface->iface, &ds_desc);
    pb_erase_depth_stencil_buffer(0, 0, ds_desc.Width, ds_desc.Height);
    pb_fill(0, 0, ds_desc.Width, ds_desc.Height, 0x00000000);
    while(pb_busy()) {
        /* Wait for completion... */
    }
    D3DDevice_BeginPushBuffer(g_d3ddev.current_pb);
    D3DDevice_PushCmd(NV097_SET_VERTEX_DATA_ARRAY_FORMAT, 16);
    D3DDevice_PushN(NV097_SET_VERTEX_DATA_ARRAY_FORMAT_TYPE_F, 16);
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

    D3DDevice_EndPushBuffer();
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

    g_d3ddev.current_surface = 
        (g_d3ddev.current_surface + 1) % g_d3ddev.max_surfaces;
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

    DWORD Count = 0;
    switch (PrimitiveType) {
    // TODO: is D3DPT_QUADSTRIP right?
    case D3DPT_QUADSTRIP:
        Count = PrimitiveCount + 3;
        break;
    case D3DPT_QUADLIST:
        Count = PrimitiveCount * 4;
        break;
    case D3DPT_TRIANGLELIST:
        Count = PrimitiveCount * 3;
        break;
    case D3DPT_TRIANGLESTRIP:
    case D3DPT_TRIANGLEFAN:
        Count = PrimitiveCount + 2;
        break;
    case D3DPT_LINELIST:
        Count = PrimitiveCount * 2;
        break;
    case D3DPT_LINESTRIP:
        Count = PrimitiveCount + 1;
        break;
    case D3DPT_LINELOOP:
    case D3DPT_POINTLIST:
        Count = PrimitiveCount;
        break;
    default:
        return D3DERR_INVALIDCALL;
    }
    
    D3DDevice_PushCmd(NV097_SET_BEGIN_END, PrimitiveType);
    D3DDevice_PushCmd(0x40000000|NV097_DRAW_ARRAYS,
                 MASK(NV097_DRAW_ARRAYS_COUNT, Count) | 
                 MASK(NV097_DRAW_ARRAYS_START_INDEX, StartVertex));

    D3DDevice_PushCmd(NV097_SET_BEGIN_END, NV097_SET_BEGIN_END_OP_END);
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
    // TODO: implement mipmaps
    if (Levels > 1)
        return D3DERR_INVALIDCALL;

    if (Levels == 0) {
        // Calculate number of levels
        Levels = 1;
        // UINT w = Width;
        // UINT h = Height;
        // while (w > 1 || h > 1) {
        //     Levels++;
        //     if (w > 1) w >>= 1;
        //     if (h > 1) h >>= 1;
        // }
    }

    if (Usage != 0 && Usage != D3DUSAGE_BORDERSOURCE_TEXTURE)
        return D3DERR_INVALIDCALL;

    D3DSurface* pLevels = malloc(sizeof(*pLevels) * Levels);
    if (!pLevels)
        return E_OUTOFMEMORY;

    D3DTexture* texture = malloc(sizeof(*texture));
    if (!texture)
        return E_OUTOFMEMORY;

    pLevels[0].resource.refcount = 1;
    pLevels[0].resource.bManuallyRegistered = FALSE;
    pLevels[0].resource.Data = 0;
    pLevels[0].desc.Width  = Width;
    pLevels[0].desc.Height = Height;
    pLevels[0].desc.Size   = Width * Height * D3D_FormatBytesPerPixel(Format);
    pLevels[0].resource.pContiguousMemory = 
        D3D_AllocContiguousMemory(pLevels[0].desc.Size, D3DTEXTURE_ALIGNMENT);
    
    if (pLevels[0].resource.pContiguousMemory == NULL) {
        free(pLevels);
        free(texture);
        return E_OUTOFMEMORY;
    }

    texture->iface.lpVtbl                      = &g_d3dTextureVtbl;
    texture->base.resource.bManuallyRegistered = FALSE;
    texture->base.resource.Data                = 0;
    texture->base.dwLevelCount                 = Levels;
    texture->desc.Type                         = D3DRTYPE_TEXTURE;
    texture->desc.Format                       = Format;
    texture->desc.Usage                        = Usage;
    texture->desc.Width                        = Width;
    texture->desc.Height                       = Height;
    texture->desc.Size = Width * Height * D3D_FormatBytesPerPixel(Format);
    texture->pLevels                           = pLevels;
    *ppTexture = &texture->iface;
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

HRESULT D3DDevice_SetTextureStageState(
    DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD Value)
{
    if (Stage >= 4)
        return D3DERR_INVALIDCALL;

    switch (Type) {
    // TODO: implement texture stage states
    default:
        return D3DERR_INVALIDCALL;
    }
    return D3D_OK;
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

    pb->iface.lpVtbl = &g_d3dPushBufferVtbl;
    pb->resource.type = D3DRTYPE_PUSHBUFFER;
    pb->resource.bManuallyRegistered = FALSE;
    pb->resource.refcount = 1;
    pb->Size = Size;
    pb->SizeNeeded = 0;
    if (RunUsingCpuCopy == TRUE) {
        pb->p = malloc(Size);
        pb->bCpu = TRUE;
        if (pb->p == NULL) {
            free(pb);
            return E_OUTOFMEMORY;
        }
    }
    else {
        pb->resource.pContiguousMemory = 
            D3D_AllocContiguousMemory(Size, D3DPUSHBUFFER_ALIGNMENT);
        pb->bCpu = FALSE;
        if (pb->resource.pContiguousMemory == NULL) {
            free(pb);
            return D3DERR_OUTOFVIDEOMEMORY;
        }
    }

    pb->p = pb->resource.pContiguousMemory;
    *ppPushBuffer = &pb->iface;
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
    if(FAILED(hr))
        return hr;

    for (int i = 0; pFunction[i] != D3DVS_END() && Address + i < 136; i +=4) {
        hr = D3DDevice_PushCmd(NV097_SET_TRANSFORM_PROGRAM, 4);
        if(FAILED(hr))
            return hr;
        hr = D3DDevice_PushA(&pFunction[i], 4);
        if(FAILED(hr))
            return hr;
    }
    return D3D_OK;
}

HRESULT IDirect3DDevice8_LoadVertexShaderProgram(
    LPDIRECT3DDEVICE8 pThis, CONST DWORD *pFunction, DWORD Address)
{
    assert(pThis == &g_d3ddev.iface);
    return D3DDevice_LoadVertexShaderProgram(pFunction, Address);
}


HRESULT D3DDevice_SetRenderState(D3DRENDERSTATETYPE Type, DWORD Value) {
    switch (Type) {
    case D3DRS_PSALPHAINPUTS0:
    case D3DRS_PSALPHAINPUTS1:
    case D3DRS_PSALPHAINPUTS2:
    case D3DRS_PSALPHAINPUTS3:
    case D3DRS_PSALPHAINPUTS4:
    case D3DRS_PSALPHAINPUTS5:
    case D3DRS_PSALPHAINPUTS6:
    case D3DRS_PSALPHAINPUTS7:
        DWORD n = Type - D3DRS_PSALPHAINPUTS0;
        return D3DDevice_PushCmd(NV20_TCL_PRIMITIVE_3D_RC_IN_ALPHA(n), Value);
    case D3DRS_PSRGBINPUTS0:
    case D3DRS_PSRGBINPUTS1:
    case D3DRS_PSRGBINPUTS2:
    case D3DRS_PSRGBINPUTS3:
    case D3DRS_PSRGBINPUTS4:
    case D3DRS_PSRGBINPUTS5:
    case D3DRS_PSRGBINPUTS6:
    case D3DRS_PSRGBINPUTS7:
        n = Type - D3DRS_PSRGBINPUTS0;
        return D3DDevice_PushCmd(NV20_TCL_PRIMITIVE_3D_RC_IN_RGB(n), Value);
    // TODO: are these the right regs?
    case D3DRS_PSFINALCOMBINERCONSTANT0:
        return D3DDevice_PushCmd(NV20_TCL_PRIMITIVE_3D_RC_COLOR0, Value);
    case D3DRS_PSFINALCOMBINERCONSTANT1:
        return D3DDevice_PushCmd(NV20_TCL_PRIMITIVE_3D_RC_COLOR1, Value);
    case D3DRS_PSCONSTANT0_0:
    case D3DRS_PSCONSTANT0_1:
    case D3DRS_PSCONSTANT0_2:
    case D3DRS_PSCONSTANT0_3:
    case D3DRS_PSCONSTANT0_4:
    case D3DRS_PSCONSTANT0_5:
    case D3DRS_PSCONSTANT0_6:
    case D3DRS_PSCONSTANT0_7:
        n = Type - D3DRS_PSCONSTANT0_0;
        return D3DDevice_PushCmd(
            NV20_TCL_PRIMITIVE_3D_RC_CONSTANT_COLOR0(n), Value);
    case D3DRS_PSCONSTANT1_0:
    case D3DRS_PSCONSTANT1_1:
    case D3DRS_PSCONSTANT1_2:
    case D3DRS_PSCONSTANT1_3:
    case D3DRS_PSCONSTANT1_4:
    case D3DRS_PSCONSTANT1_5:
    case D3DRS_PSCONSTANT1_6:
    case D3DRS_PSCONSTANT1_7:
        n = Type - D3DRS_PSCONSTANT1_0;
        return D3DDevice_PushCmd(
            NV20_TCL_PRIMITIVE_3D_RC_CONSTANT_COLOR1(n), Value);
    case D3DRS_PSALPHAOUTPUTS0:
    case D3DRS_PSALPHAOUTPUTS1:
    case D3DRS_PSALPHAOUTPUTS2:
    case D3DRS_PSALPHAOUTPUTS3:
    case D3DRS_PSALPHAOUTPUTS4:
    case D3DRS_PSALPHAOUTPUTS5:
    case D3DRS_PSALPHAOUTPUTS6:
    case D3DRS_PSALPHAOUTPUTS7:
        n = Type - D3DRS_PSALPHAOUTPUTS0;
        return D3DDevice_PushCmd(NV20_TCL_PRIMITIVE_3D_RC_OUT_ALPHA(n), Value);
    case D3DRS_PSRGBOUTPUTS0:
    case D3DRS_PSRGBOUTPUTS1:
    case D3DRS_PSRGBOUTPUTS2:
    case D3DRS_PSRGBOUTPUTS3:
    case D3DRS_PSRGBOUTPUTS4:
    case D3DRS_PSRGBOUTPUTS5:
    case D3DRS_PSRGBOUTPUTS6:
    case D3DRS_PSRGBOUTPUTS7:
        n = Type - D3DRS_PSRGBOUTPUTS0;
        return D3DDevice_PushCmd(NV20_TCL_PRIMITIVE_3D_RC_OUT_RGB(n), Value);
    case D3DRS_PSFINALCOMBINERINPUTSABCD:
        return D3DDevice_PushCmd(NV20_TCL_PRIMITIVE_3D_RC_FINAL0, Value);
    case D3DRS_PSFINALCOMBINERINPUTSEFG:
        return D3DDevice_PushCmd(NV20_TCL_PRIMITIVE_3D_RC_FINAL1, Value);
    case D3DRS_PSCOMPAREMODE:
        return D3DDevice_PushCmd(NV097_SET_SHADER_CLIP_PLANE_MODE, Value);
    case D3DRS_PSCOMBINERCOUNT:
        return D3DDevice_PushCmd(NV097_SET_COMBINER_CONTROL, Value);
    case D3DRS_PSTEXTUREMODES:
        return D3DDevice_PushCmd(NV097_SET_SHADER_STAGE_PROGRAM, Value);
    case D3DRS_PSDOTMAPPING:
        return D3DDevice_PushCmd(NV097_SET_DOT_RGBMAPPING, Value);
    case D3DRS_PSINPUTTEXTURE:
        return D3DDevice_PushCmd(NV097_SET_SHADER_OTHER_STAGE_INPUT, Value);
    // TODO: implement remaining render states
    default:
        return D3DERR_INVALIDCALL;
    }
}

HRESULT IDirect3DDevice8_SetRenderState(
    LPDIRECT3DDEVICE8 pThis, D3DRENDERSTATETYPE Type, DWORD Value)
{
    assert(pThis == &g_d3ddev.iface);
    return D3DDevice_SetRenderState(Type, Value);
}

HRESULT D3DDevice_SetPixelShaderProgram(CONST D3DPIXELSHADERDEF *pPSDef) {
    for (int i = 0; i < 8; i++) {
        HRESULT hr = D3DDevice_SetRenderState(D3DRS_PSALPHAINPUTS0 + i, 
                                              pPSDef->PSAlphaInputs[i]);
        if (FAILED(hr))
            return hr;
    }
    HRESULT hr = D3DDevice_SetRenderState(D3DRS_PSFINALCOMBINERINPUTSABCD, 
                                          pPSDef->PSFinalCombinerInputsABCD);
    if(FAILED(hr))
        return hr;
    hr = D3DDevice_SetRenderState(D3DRS_PSFINALCOMBINERINPUTSEFG, 
                                  pPSDef->PSFinalCombinerInputsEFG);
    if(FAILED(hr))
        return hr;
    for (int i = 0; i < 8; i++) {
        hr = D3DDevice_SetRenderState(D3DRS_PSCONSTANT0_0 + i, 
                                      pPSDef->PSConstant0[i]);
        if(FAILED(hr))
            return hr;
    }
    for (int i = 0; i < 8; i++) {
        hr = D3DDevice_SetRenderState(D3DRS_PSCONSTANT1_0 + i, 
                                      pPSDef->PSConstant1[i]);
        if(FAILED(hr))
            return hr;
    }
    for (int i = 0; i < 8; i++) {
        hr = D3DDevice_SetRenderState(D3DRS_PSALPHAOUTPUTS0 + i, 
                                      pPSDef->PSAlphaOutputs[i]);
        if (FAILED(hr))
            return hr;
    }
    for (int i = 0; i < 8; i++) {
        hr = D3DDevice_SetRenderState(D3DRS_PSRGBINPUTS0 + i, 
                                      pPSDef->PSRGBInputs[i]);
        if (FAILED(hr))
            return hr;
    }
    hr = D3DDevice_SetRenderState(D3DRS_PSCOMPAREMODE, pPSDef->PSCompareMode);
    if (FAILED(hr))
        return hr;
    hr = D3DDevice_SetRenderState(D3DRS_PSFINALCOMBINERCONSTANT0,
                                  pPSDef->PSFinalCombinerConstant0);
    if (FAILED(hr))
        return hr;
    hr = D3DDevice_SetRenderState(D3DRS_PSFINALCOMBINERCONSTANT1, 
                                  pPSDef->PSFinalCombinerConstant1);
    if (FAILED(hr))
        return hr;
    for (int i = 0; i < 8; i++) {
        hr = D3DDevice_SetRenderState(D3DRS_PSRGBOUTPUTS0 + i, 
                                      pPSDef->PSRGBOutputs[i]);
        if (FAILED(hr))
            return hr;
    }
    hr = D3DDevice_SetRenderState(D3DRS_PSCOMBINERCOUNT, 
                                  pPSDef->PSCombinerCount);
    if (FAILED(hr))
        return hr;
    hr = D3DDevice_SetRenderState(D3DRS_PSTEXTUREMODES, 
                                  pPSDef->PSTextureModes);
    if (FAILED(hr))
        return hr;
    hr = D3DDevice_SetRenderState(D3DRS_PSDOTMAPPING, pPSDef->PSDotMapping);
    if (FAILED(hr))
        return hr;
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
    HRESULT hr = D3DDevice_PushCmd(NV097_SET_CLEAR_RECT_HORIZONTAL, 
                                   ((pRect->x1 - 1) << 16) | 
                                    (pRect->x2 & 0xFFFF));
    if (FAILED(hr)) return hr;
    hr = D3DDevice_PushCmd(NV097_SET_CLEAR_RECT_VERTICAL, 
        ((pRect->y1 - 1) << 16) | (pRect->y2 & 0xFFFF));
    if (FAILED(hr)) return hr;
    return D3DDevice_PushCmd(NV097_CLEAR_SURFACE, Flags);
}

HRESULT D3DDevice_Clear(DWORD Count, CONST D3DRECT* pRects, DWORD Flags,
                        D3DCOLOR Color, float Z, DWORD Stencil)
{
    HRESULT hr = D3DDevice_PushCmd(NV097_SET_ZSTENCIL_CLEAR_VALUE, 
        (((DWORD)(Z * D3DZ_MAX_D24S8) & 0x00FFFFFF) << 8) | Stencil);
    if (FAILED(hr)) return hr;
    hr = D3DDevice_PushCmd(NV097_SET_COLOR_CLEAR_VALUE, Color);
    if (FAILED(hr)) return hr;
    if (pRects == NULL) {
        if (Count > 0)
            return D3DERR_INVALIDCALL;

        D3DSURFACE_DESC desc;
        hr = IDirect3DSurface8_GetDesc(
            &g_d3ddev.surfaces[g_d3ddev.current_surface]->iface, &desc);
        if (FAILED(hr)) return hr;
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

HRESULT IDirect3DDevice8_Clear(LPDIRECT3DDEVICE8 pThis, DWORD Count, 
                               CONST D3DRECT* pRects, DWORD Flags,
                               D3DCOLOR Color, float Z, DWORD Stencil)
{
    assert(pThis == &g_d3ddev.iface);
    return D3DDevice_Clear(Count, pRects, Flags, Color, Z, Stencil);
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
    g_d3ddevVtbl.DrawPrimitive              = IDirect3DDevice8_DrawPrimitive;
    g_d3ddevVtbl.SetViewport                = IDirect3DDevice8_SetViewport;
    g_d3ddevVtbl.CreateTexture              = IDirect3DDevice8_CreateTexture;
    g_d3ddevVtbl.SetRenderState             = IDirect3DDevice8_SetRenderState;
    g_d3ddevVtbl.SetTextureStageState       = IDirect3DDevice8_SetTextureStageState;
    g_d3ddevVtbl.SetVertexShaderInputDirect = IDirect3DDevice8_SetVertexShaderInputDirect;
    g_d3ddevVtbl.LoadVertexShaderProgram    = IDirect3DDevice8_LoadVertexShaderProgram;
    g_d3ddevVtbl.SetPixelShaderProgram      = IDirect3DDevice8_SetPixelShaderProgram;
    g_d3ddevVtbl.Clear                      = IDirect3DDevice8_Clear;
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
