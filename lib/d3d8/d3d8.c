#include <assert.h>
#include <stdlib.h>
#include <stdcountof.h>

#include <hal/video.h>
#include <pbkit/pbkit.h>
#include <xboxkrnl/xboxkrnl.h>

#include "d3d8.h"
#include "d3d8_resource.h"
#include "d3d8_private.h"

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

// ============================================================================
typedef struct D3DRenderState {
    DWORD CullMode;
    DWORD MultiSample;
    DWORD RopZ;
    DWORD Debug5;
    DWORD LogicOp;
} D3DRenderState;

typedef struct D3DTextureStageState {
    DWORD Address;
    DWORD Filter;
    DWORD Control0;
    DWORD Dirty;
} D3DTextureStageState;

#define D3DTSS_ADDRESS_DIRTY   0x00000001
#define D3DTSS_FILTER_DIRTY    0x00000002
#define D3DTSS_CONTROL0_DIRTY  0x00000004

typedef struct D3DDevice IMPLEMENTS(IDirect3DDevice8) {
    IDirect3DDevice8        iface;
    D3DRefcount             refcount;
    DWORD                   KickOffSize;
    D3DSurface*             pSurfaces[3];
    UINT                    CurrentSurface;
    UINT                    MaxSurfaces;
    D3DSurface*             pDepthStencilSurface;
    D3DPushBuffer           pDefaultPB;
    D3DPushBuffer*          pCurrentPB;
    BOOL                    bInScene;
    D3DVIEWPORT8            Viewport;
    UINT                    LastPresentVBlankCount;
    BOOL                    bUserSuppliedBufferSurfaces;
    BOOL                    bUserSuppliedDepthStencilSurface;
    DWORD                   Control0;
    DWORD                   Stencil;
    BOOL                    ZStencilClearDirty;
    DWORD                   ClearColor;
    BOOL                    ColorClearDirty;
    D3DPRESENT_INTERVAL     PresentInterval;
    D3DRenderState          RenderState;
    D3DTextureStageState    TextureStageState[D3DTSS_MAXSTAGES];
    DWORD                   Wrap[D3DTSS_MAXSTAGES];
    DWORD                   WrapDirty;
    D3DBaseTexture*         pTexture[D3DTSS_MAXSTAGES];
#ifdef __cplusplus
    ULONG AddRef() override {
        return D3DDevice_AddRef();
    }

    ULONG Release() override {
        return D3DDevice_Release();
    }

    VOID BlockUntilVerticalBlank() {
        D3DDevice_BlockUntilVerticalBlank();
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

    HRESULT CreateCubeTexture(
        UINT EdgeLength, 
        UINT Levels, 
        DWORD Usage, 
        D3DFORMAT Format, 
        D3DPOOL Pool,
        LPDIRECT3DCUBETEXTURE8* ppCubeTexture) override
    {
        return D3DDevice_CreateCubeTexture(EdgeLength, Levels, 
                                           Format, ppCubeTexture);
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
        return D3DDevice_SetScissors(Count, Exclusive, pRects);
    }

    HRESULT SetTile(DWORD Index, CONST D3DTILE* pTile) override {
        return D3DDevice_SetTile(Index, pTile);
    }

    HRESULT SetTexture(DWORD Stage, LPDIRECT3DBASETEXTURE8 pTexture) override {
        return D3DDevice_SetTexture(Stage, pTexture);
    }

    HRESULT InsertCallback(D3DCALLBACKTYPE Type, 
                           D3DCALLBACK pCallback, 
                           DWORD Context) override 
    {
        return D3DDevice_InsertCallback(Type, pCallback, Context);
    }

    HRESULT Reset(D3DPRESENT_PARAMETERS* pPresentationParameters) override {
        return D3DDevice_Reset(pPresentationParameters);
    }
#endif // __cplusplus
} D3DDevice;

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

IDirect3DDeviceVtbl8 g_d3ddevVtbl;

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

D3DDevice g_d3ddev;

// Sets the device's current push buffer to the supplied push buffer.
// All subsequent GPU commands will be sent to this push buffer until
// D3DDevce_EndPushBuffer is called.
// 
// By default, the device will use pbkit's internal push buffer.
// Calling this function with pPushBuffer == NULL will return to using
// pbkit's internal push buffer.
HRESULT D3DDevice_BeginPushBuffer(D3DPushBuffer* pPushBuffer) {
    if (pPushBuffer == NULL)
        pPushBuffer = &g_d3ddev.pDefaultPB;

    g_d3ddev.pCurrentPB = pPushBuffer;
    if (g_d3ddev.pCurrentPB->bCpu == FALSE) {
        g_d3ddev.pCurrentPB->p = 
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
HRESULT D3DDevice_EndPushBuffer(void) {
    if (g_d3ddev.pCurrentPB->bCpu == FALSE) {
        pb_end_at((uint32_t*)(g_d3ddev.pCurrentPB->p - 1), 
                  (uint32_t*)g_d3ddev.pCurrentPB->p);
    }
    g_d3ddev.pCurrentPB = NULL;
    return D3D_OK;
}

BOOL D3DDevice_IsKickoffReady(void) {
    if (g_d3ddev.pCurrentPB == NULL)
        return FALSE;
    if (g_d3ddev.pCurrentPB->SizeNeeded >= g_d3ddev.KickOffSize)
        return TRUE;
    else
        return FALSE;
}

HRESULT D3DDevice_KickPushBuffer(void) {
#if NXDK_DEBUG
    if (g_d3ddev.pCurrentPB == NULL)
        return D3DERR_INVALIDCALL;
#endif // NXDK_DEBUG

    if (g_d3ddev.pCurrentPB->bCpu == TRUE) {
        uint32_t* p = pb_begin();
        memcpy(p, g_d3ddev.pCurrentPB->p, 
               g_d3ddev.pCurrentPB->SizeNeeded * sizeof(DWORD));
        p += g_d3ddev.pCurrentPB->SizeNeeded;
        pb_end(p);
        return D3D_OK;
    }
    
    HRESULT hr = D3DDevice_EndPushBuffer();
    if (FAILED(hr)) return hr;
    
    return D3DDevice_BeginPushBuffer(g_d3ddev.pCurrentPB);
}

HRESULT D3DDevice_Push1(DWORD dwData) {
    if (g_d3ddev.pCurrentPB == NULL)
        return D3DERR_NOTAVAILABLE;
    
    if (D3DDevice_IsKickoffReady() == TRUE) {
        HRESULT hr = D3DDevice_KickPushBuffer();
        if (FAILED(hr)) return hr;
    }
    return D3DPushBuffer_Push1(g_d3ddev.pCurrentPB, dwData);
}

HRESULT D3DDevice_PushCmd(DWORD cmd, DWORD dwData) {
#if NXDK_DEBUG
    if (g_d3ddev.pCurrentPB == NULL)
        return D3DERR_NOTAVAILABLE;
    if (cmd == 0)
        return D3DERR_INVALIDCALL;
#endif // NXDK_DEBUG
    if (D3DDevice_IsKickoffReady() == TRUE) {
        HRESULT hr = D3DDevice_KickPushBuffer();
        if (FAILED(hr)) return hr;
    }
    return D3DPushBuffer_PushCmd(g_d3ddev.pCurrentPB, cmd, dwData);
}

HRESULT D3DDevice_PushCmd2(DWORD cmd, DWORD dwData1, DWORD dwData2) {
#if NXDK_DEBUG
    if (g_d3ddev.pCurrentPB == NULL)
        return D3DERR_NOTAVAILABLE;
#endif // NXDK_DEBUG
    if (D3DDevice_IsKickoffReady() == TRUE) {
        HRESULT hr = D3DDevice_KickPushBuffer();
        if (FAILED(hr)) return hr;
    }
    return D3DPushBuffer_PushCmd2(g_d3ddev.pCurrentPB, cmd, dwData1, dwData2);
}

HRESULT D3DDevice_PushCmd3(DWORD cmd, DWORD dwData1, 
                           DWORD dwData2, DWORD dwData3) 
{
#if NXDK_DEBUG
    if (g_d3ddev.pCurrentPB == NULL)
        return D3DERR_NOTAVAILABLE;
#endif // NXDK_DEBUG
    if (D3DDevice_IsKickoffReady() == TRUE) {
        HRESULT hr = D3DDevice_KickPushBuffer();
        if (FAILED(hr)) return hr;
    }
    return D3DPushBuffer_PushCmd3(g_d3ddev.pCurrentPB, cmd, 
                                  dwData1, dwData2, dwData3);
}

HRESULT D3DDevice_PushCmd4(DWORD cmd, DWORD dwData1, DWORD dwData2, 
                                      DWORD dwData3, DWORD dwData4) 
{
#if NXDK_DEBUG
    if (g_d3ddev.pCurrentPB == NULL)
        return D3DERR_NOTAVAILABLE;
#endif // NXDK_DEBUG
    if (D3DDevice_IsKickoffReady() == TRUE) {
        HRESULT hr = D3DDevice_KickPushBuffer();
        if (FAILED(hr)) return hr;
    }
    return D3DPushBuffer_PushCmd4(g_d3ddev.pCurrentPB, cmd, 
                                  dwData1, dwData2, dwData3, dwData4);
}

HRESULT D3DDevice_PushCmdf(DWORD cmd, float fData) {
#if NXDK_DEBUG
    if (g_d3ddev.pCurrentPB == NULL)
        return D3DERR_NOTAVAILABLE;
    if (cmd == 0)
        return D3DERR_INVALIDCALL;
#endif // NXDK_DEBUG
    if (D3DDevice_IsKickoffReady() == TRUE) {
        HRESULT hr = D3DDevice_KickPushBuffer();
        if (FAILED(hr)) return hr;
    }
    return D3DPushBuffer_PushCmdf(g_d3ddev.pCurrentPB, cmd, fData);
}

HRESULT D3DDevice_PushCmd2f(DWORD cmd, float fData1, float fData2) {
#if NXDK_DEBUG
    if (g_d3ddev.pCurrentPB == NULL)
        return D3DERR_NOTAVAILABLE;
    if (cmd == 0)
        return D3DERR_INVALIDCALL;
#endif // NXDK_DEBUG
    if (D3DDevice_IsKickoffReady() == TRUE) {
        HRESULT hr = D3DDevice_KickPushBuffer();
        if (FAILED(hr)) return hr;
    }
    return D3DPushBuffer_PushCmd2f(g_d3ddev.pCurrentPB, cmd, fData1, fData2);
}

HRESULT D3DDevice_PushCmd3f(DWORD cmd, float fData1, 
                            float fData2, float fData3) 
{
#if NXDK_DEBUG
    if (g_d3ddev.pCurrentPB == NULL)
        return D3DERR_NOTAVAILABLE;
    if (cmd == 0)
        return D3DERR_INVALIDCALL;
#endif // NXDK_DEBUG
    if (D3DDevice_IsKickoffReady() == TRUE) {
        HRESULT hr = D3DDevice_KickPushBuffer();
        if (FAILED(hr)) return hr;
    }
    return D3DPushBuffer_PushCmd3f(g_d3ddev.pCurrentPB, cmd, 
                                   fData1, fData2, fData3);
}

HRESULT D3DDevice_PushCmd4f(DWORD cmd, float fData1, float fData2, 
                                       float fData3, float fData4) 
{
#if NXDK_DEBUG
    if (g_d3ddev.pCurrentPB == NULL)
        return D3DERR_NOTAVAILABLE;
    if (cmd == 0)
        return D3DERR_INVALIDCALL;
#endif // NXDK_DEBUG
    if (D3DDevice_IsKickoffReady() == TRUE) {
        HRESULT hr = D3DDevice_KickPushBuffer();
        if (FAILED(hr)) return hr;
    }
    return D3DPushBuffer_PushCmd4f(g_d3ddev.pCurrentPB, cmd, 
                                   fData1, fData2, fData3, fData4);
}

HRESULT D3DDevice_PushN(DWORD dwData, SIZE_T n) {
    if (g_d3ddev.pCurrentPB == NULL)
        return D3DERR_NOTAVAILABLE;

    if (D3DDevice_IsKickoffReady() == TRUE) {
        HRESULT hr = D3DDevice_KickPushBuffer();
        if (FAILED(hr)) return hr;
    }
    return D3DPushBuffer_PushN(g_d3ddev.pCurrentPB, dwData, n);
}

HRESULT D3DDevice_PushA(CONST DWORD* pdwData, SIZE_T n) {
    if (g_d3ddev.pCurrentPB == NULL)
        return D3DERR_NOTAVAILABLE;

    if (D3DDevice_IsKickoffReady() == TRUE) {
        HRESULT hr = D3DDevice_KickPushBuffer();
        if (FAILED(hr)) return hr;
    }
    return D3DPushBuffer_PushA(g_d3ddev.pCurrentPB, pdwData, n);
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
#if NXDK_DEBUG
    if (Count >= D3DSCISSORS_MAX)
        return D3DERR_INVALIDCALL;
    
    if (Exclusive > TRUE)
        return D3DERR_INVALIDCALL;
#endif // NXDK_DEBUG

    if (Count == 0) {
        HRESULT hr = D3DDevice_PushCmd(
            NV097_SET_WINDOW_CLIP_TYPE, NV097_SET_WINDOW_CLIP_TYPE_INCLUSIVE);
        D3DVIEWPORT8* pViewport = &g_d3ddev.Viewport;
        D3DRECT rect;
        rect.x1 = pViewport->X;
        rect.y1 = pViewport->Y;
        rect.x2 = pViewport->X + pViewport->Width;
        rect.y2 = pViewport->Y + pViewport->Height;
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

HRESULT IDirect3DDevice8_SetScissors(LPDIRECT3DDEVICE8 pThis, DWORD Count, 
                                     BOOL Exclusive, CONST D3DRECT *pRects)
{
    assert(pThis == &g_d3ddev.iface);
    return D3DDevice_SetScissors(Count, Exclusive, pRects);
}

BOOL D3D_IsDepthStencilFormatFixed(D3DFORMAT Format) {
    if (Format == D3DFMT_D24S8 || D3DFMT_D16)
        return TRUE;
    if (Format == D3DFMT_F24S8 || D3DFMT_F16)
        return FALSE;

    assert(false);
    return -1;
}

HRESULT D3DDevice_FireInterrupt(DWORD Subprog, DWORD ParamA) {
    HRESULT hr = D3DDevice_PushCmd(NV097_WAIT_FOR_IDLE, 0);
    if (FAILED(hr)) return hr;
    hr = D3DDevice_PushCmd(NV097_SET_INTERRUPT_PARAM_A, ParamA);
    if (FAILED(hr)) return hr;
    g_d3ddev.ZStencilClearDirty = TRUE;
    return D3DDevice_PushCmd(NV097_FIRE_INTERRUPT, Subprog);
}

HRESULT D3DDevice_FireInterrupt2(DWORD Subprog, DWORD ParamA, DWORD ParamB) {
    HRESULT hr = D3DDevice_PushCmd(NV097_WAIT_FOR_IDLE, 0);
    if (FAILED(hr)) return hr;
    hr = D3DDevice_PushCmd(NV097_SET_INTERRUPT_PARAM_A, ParamA);
    if (FAILED(hr)) return hr;
    g_d3ddev.ZStencilClearDirty = TRUE;
    hr = D3DDevice_PushCmd(NV097_SET_INTERRUPT_PARAM_B, ParamB);
    if (FAILED(hr)) return hr;
    g_d3ddev.ColorClearDirty = TRUE;
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

    D3DRenderState* pRenderState = &g_d3ddev.RenderState;
    DWORD RopZ = 0;
    if (VIDEOREG(NV_PBUS_ROM_VERSION) & NV_PBUS_ROM_VERSION_MASK)
        RopZ = 0x45EAD10F;
    else 
        RopZ = 0x45EAD10E;
    pRenderState->RopZ = RopZ;
    hr = D3DDevice_FireInterrupt2(PB_SETOUTER, 
                                          NV_PGRAPH_UNKNOWN_400B80, 
                                          g_d3ddev.RenderState.RopZ);
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

HRESULT D3DDevice_InitDeviceState(void) {
    HRESULT hr = D3DDevice_PushCmd4f(NV097_SET_EYE_POSITION, 
                                     0.0f, 0.0f, 0.0f, 1.0f);
    if (FAILED(hr)) return hr;

    D3DSURFACE_DESC desc;
    hr = IDirect3DSurface8_GetDesc(
        (LPDIRECT3DSURFACE8)g_d3ddev.pSurfaces[0], &desc);
    if (FAILED(hr)) return hr;
    UINT ColorPitch = desc.Width * D3D_FormatBytesPerPixel(desc.Format);
    UINT Width = desc.Width;
    UINT Height = desc.Height;
    hr = IDirect3DSurface8_GetDesc(
        (LPDIRECT3DSURFACE8)g_d3ddev.pDepthStencilSurface, &desc);
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
    
    hr = D3DDevice_PushCmd3(NV097_SET_SCENE_AMBIENT_COLOR, 0, 0, 0);
    if (FAILED(hr)) return hr;

    hr = D3DDevice_PushCmd3(NV097_SET_MATERIAL_EMISSION, 0, 0, 0);
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
    
    hr = D3DDevice_PushCmd4f(NV097_SET_FOG_PLANE, 0.0f, 0.0f, 1.0f, 0.0f);
    if (FAILED(hr)) return hr;

    hr = D3DDevice_PushCmd(NV097_SET_FOG_GEN_MODE, 
                           NV097_SET_FOG_GEN_MODE_V_PLANAR);
    if (FAILED(hr)) return hr;

    hr = D3DDevice_PushCmd4(NV097_SET_TEXTURE_MATRIX_ENABLE, 0, 0, 0, 0);
    if (FAILED(hr)) return hr;

    hr = D3DDevice_PushCmd4f(NV097_SET_TEXCOORD0_4F, 0.f, 0.f, 0.f, 0.f);
    if (FAILED(hr)) return hr;
    hr = D3DDevice_PushCmd4f(NV097_SET_TEXCOORD1_4F, 0.f, 0.f, 0.f, 0.f);
    if (FAILED(hr)) return hr;
    hr = D3DDevice_PushCmd4f(NV097_SET_TEXCOORD2_4F, 0.f, 0.f, 0.f, 0.f);
    if (FAILED(hr)) return hr;
    hr = D3DDevice_PushCmd4f(NV097_SET_TEXCOORD3_4F, 0.f, 0.f, 0.f, 0.f);
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

    hr = D3DDevice_InitRenderState();
    if (FAILED(hr)) return hr;

    for (DWORD Stage = 0; Stage < D3DTSS_MAXSTAGES; Stage++) {
        hr = D3DDevice_InitTextureStageState(Stage);
        if (FAILED(hr)) return hr;
    }

    return D3DDevice_SetScissors(0, FALSE, NULL);
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

// ============================================================================
ULONG D3DDevice_AddRef(void) {
    g_d3ddev.refcount.c = 1;
    return 1;
}

ULONG IDirect3DDevice8_AddRef(LPDIRECT3DDEVICE8 pThis) {
    assert(pThis == &g_d3ddev.iface);
    return D3DDevice_AddRef();
}

ULONG D3DDevice_Release(void) {
    return g_d3ddev.refcount.c;
}

ULONG IDirect3DDevice8_Release(LPDIRECT3DDEVICE8 pThis) {
    assert(pThis == &g_d3ddev.iface);
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

HRESULT IDirect3DDevice8_Reset(LPDIRECT3DDEVICE8 pThis, 
                               D3DPRESENT_PARAMETERS* pPresentationParameters)
{
    assert(pThis == &g_d3ddev.iface);
    return D3DDevice_Reset(pPresentationParameters);
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

void D3DDevice_BlockUntilVerticalBlank(void) {
    pb_wait_for_vbl();
}

void IDirect3DDevice8_BlockUntilVerticalBlank(LPDIRECT3DDEVICE8 pThis) {
    assert(pThis == &g_d3ddev.iface);
    return D3DDevice_BlockUntilVerticalBlank();
}

HRESULT D3DDevice_BeginScene(void) {
    if (g_d3ddev.bInScene == TRUE) 
        return D3DERR_INVALIDCALL;

    g_d3ddev.bInScene = TRUE;

    if (g_d3ddev.pCurrentPB == NULL)
        g_d3ddev.pCurrentPB = &g_d3ddev.pDefaultPB;

    if (g_d3ddev.pCurrentPB == &g_d3ddev.pDefaultPB)
        pb_reset();

    pb_target_back_buffer();
    while(pb_busy()) {
        /* Wait for completion... */
    }
    D3DDevice_BeginPushBuffer(g_d3ddev.pCurrentPB);
    return D3D_OK;
}

HRESULT IDirect3DDevice8_BeginScene(LPDIRECT3DDEVICE8 pThis) {
    assert(pThis == &g_d3ddev.iface);
    return D3DDevice_BeginScene();
}

HRESULT D3DDevice_EndScene(void) {
    if (g_d3ddev.bInScene == FALSE) 
        return D3DERR_INVALIDCALL;

    g_d3ddev.bInScene = FALSE;

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

    g_d3ddev.CurrentSurface = 
        (g_d3ddev.CurrentSurface + 1) % g_d3ddev.MaxSurfaces;
    UINT lastVBL = g_d3ddev.LastPresentVBlankCount;
    g_d3ddev.LastPresentVBlankCount = pb_get_vbl_counter();
    UINT diff = g_d3ddev.LastPresentVBlankCount - lastVBL;

    // If the interval is immediate, or the interval is *_OR_IMMEDIATE and 
    // we've surpassed the interval, there's no need to wait for a vblank.
    if (g_d3ddev.PresentInterval  == D3DPRESENT_INTERVAL_IMMEDIATE || 
        (g_d3ddev.PresentInterval == 
            D3DPRESENT_INTERVAL_ONE_OR_IMMEDIATE && diff >= 1) ||
        (g_d3ddev.PresentInterval == 
            D3DPRESENT_INTERVAL_TWO_OR_IMMEDIATE && diff >= 2) ||
        (g_d3ddev.PresentInterval == 
            D3DPRESENT_INTERVAL_THREE_OR_IMMEDIATE && diff >= 3))
    {
        return D3D_OK;
    }

    // Otherwise we need to wait for the interval.
    if (g_d3ddev.PresentInterval == D3DPRESENT_INTERVAL_ONE || 
        g_d3ddev.PresentInterval == D3DPRESENT_INTERVAL_ONE_OR_IMMEDIATE)
    {
        D3DDevice_BlockUntilVerticalBlank();
        return D3D_OK;
    }

    UINT Interval = 0;
    if (g_d3ddev.PresentInterval == D3DPRESENT_INTERVAL_TWO || 
        g_d3ddev.PresentInterval == D3DPRESENT_INTERVAL_TWO_OR_IMMEDIATE)
    {
        Interval = 2;
    } else if (g_d3ddev.PresentInterval == D3DPRESENT_INTERVAL_THREE || 
        g_d3ddev.PresentInterval == D3DPRESENT_INTERVAL_THREE_OR_IMMEDIATE)
    {
       Interval = 3;
    }

    while(pb_get_vbl_counter() < g_d3ddev.LastPresentVBlankCount + Interval)
        D3DDevice_BlockUntilVerticalBlank();
    
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
        if (g_d3ddev.pTexture[Stage] == NULL || 
            g_d3ddev.TextureStageState[Stage].Dirty == 0)
        {
            continue;
        }

        DWORD Dirty = g_d3ddev.TextureStageState[Stage].Dirty;
        g_d3ddev.TextureStageState[Stage].Dirty = 0;

        if (Dirty & D3DTSS_ADDRESS_DIRTY) {
            hr = D3DDevice_PushCmd(NV097_SET_TEXTURE_ADDRESS(Stage), 
                                   g_d3ddev.TextureStageState[Stage].Address);
            if (FAILED(hr)) return hr;
        }
        if (Dirty & D3DTSS_FILTER_DIRTY) {
            hr = D3DDevice_PushCmd(NV097_SET_TEXTURE_FILTER(Stage), 
                                   g_d3ddev.TextureStageState[Stage].Filter);
            if (FAILED(hr)) return hr;
        }
        if (Dirty & D3DTSS_CONTROL0_DIRTY) {
            hr = D3DDevice_PushCmd(NV097_SET_TEXTURE_CONTROL0(Stage), 
                g_d3ddev.TextureStageState[Stage].Control0);
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
    g_d3ddev.Viewport = *pViewport;
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

    HRESULT hr = D3D_CreateTexture(Width, Height, Levels, 
                                   Usage, Format, NULL, pTex);
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

HRESULT D3DDevice_CreateCubeTexture(
    UINT EdgeLength, UINT Levels, D3DFORMAT Format,
    LPDIRECT3DCUBETEXTURE8* ppCubeTexture)
{
    D3DCubeTexture* pTex = malloc(sizeof(*pTex));
    if (pTex == NULL)
        return E_OUTOFMEMORY;

    HRESULT hr = D3D_CreateCubeTexture(EdgeLength, Levels, Format, NULL, pTex);
    if (FAILED(hr)) return hr;
    *ppCubeTexture = &pTex->iface;
    return D3D_OK;
}

HRESULT IDirect3DDevice8_CreateCubeTexture(
    LPDIRECT3DDEVICE8 pThis, UINT EdgeLength, UINT Levels, 
    DWORD Usage, D3DFORMAT Format, D3DPOOL Pool,
    LPDIRECT3DCUBETEXTURE8* ppCubeTexture)
{
    assert(pThis == &g_d3ddev.iface);
    return D3DDevice_CreateCubeTexture(EdgeLength, Levels, Format, 
                                       ppCubeTexture);
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
        MASK_INPLACE(g_d3ddev.TextureStageState[Stage].Address, 
                     NV097_SET_TEXTURE_ADDRESS_U, Value);
        g_d3ddev.TextureStageState[Stage].Dirty |= D3DTSS_ADDRESS_DIRTY;
        return D3D_OK;
    case D3DTSS_ADDRESSV:
        MASK_INPLACE(g_d3ddev.TextureStageState[Stage].Address, 
                     NV097_SET_TEXTURE_ADDRESS_V, Value);
        g_d3ddev.TextureStageState[Stage].Dirty |= D3DTSS_ADDRESS_DIRTY;
        return D3D_OK;
    case D3DTSS_ADDRESSW:
        MASK_INPLACE(g_d3ddev.TextureStageState[Stage].Address, 
                     NV097_SET_TEXTURE_ADDRESS_P, Value);
        g_d3ddev.TextureStageState[Stage].Dirty |= D3DTSS_ADDRESS_DIRTY;
        return D3D_OK;
    case D3DTSS_MAGFILTER:
    case D3DTSS_MINFILTER: {
        // TODO: figure out how to handle the min and mag filters' convolution
        // kernel differing.
        if (Value < D3DTEXF_QUINCUNX) {
            MASK_INPLACE(g_d3ddev.TextureStageState[Stage].Filter, 
                         NV097_SET_TEXTURE_FILTER_MIN, Value);
        } else {
            if (Value == D3DTEXF_QUINCUNX) {
                MASK_INPLACE(g_d3ddev.TextureStageState[Stage].Filter, 
                    NV097_SET_TEXTURE_FILTER_CONVOLUTION_KERNEL, 
                    NV097_SET_TEXTURE_FILTER_CONVOLUTION_KERNEL_QUINCUNX);
            } else {
                 MASK_INPLACE(g_d3ddev.TextureStageState[Stage].Filter, 
                    NV097_SET_TEXTURE_FILTER_CONVOLUTION_KERNEL, 
                    NV097_SET_TEXTURE_FILTER_CONVOLUTION_KERNEL_GAUSSIAN_CUBIC
                );
            }
            DWORD Filter = NV097_SET_TEXTURE_FILTER_MIN;
            if (Type == D3DTSS_MAGFILTER)   
                Filter = NV097_SET_TEXTURE_FILTER_MAG;
            MASK_INPLACE(g_d3ddev.TextureStageState[Stage].Filter, 
                         Filter, 
                         NV097_SET_TEXTURE_FILTER_MIN_CONVOLUTION_2D_LOD0);
        } 
        g_d3ddev.TextureStageState[Stage].Dirty |= D3DTSS_FILTER_DIRTY;
        return D3D_OK;
    }
    case D3DTSS_MIPMAPLODBIAS:
        MASK_INPLACE(g_d3ddev.TextureStageState[Stage].Filter, 
                     NV097_SET_TEXTURE_FILTER_MIPMAP_LOD_BIAS, Value);
        g_d3ddev.TextureStageState[Stage].Dirty |= D3DTSS_FILTER_DIRTY;
        return D3D_OK;
    case D3DTSS_MAXANISOTROPY:
        MASK_INPLACE(g_d3ddev.TextureStageState[Stage].Control0, 
                     NV097_SET_TEXTURE_CONTROL0_ANISOTROPY, Value);
        g_d3ddev.TextureStageState[Stage].Dirty |= D3DTSS_CONTROL0_DIRTY;
        return D3D_OK;
    case D3DTSS_COLORKEYOP:
        MASK_INPLACE(g_d3ddev.TextureStageState[Stage].Control0, 
                     NV097_SET_TEXTURE_CONTROL0_COLOR_KEY_MODE, Value);
        g_d3ddev.TextureStageState[Stage].Dirty |= D3DTSS_CONTROL0_DIRTY;
        return D3D_OK;
    case D3DTSS_COLORSIGN:
        g_d3ddev.TextureStageState[Stage].Control0 = 
            (g_d3ddev.TextureStageState[Stage].Control0 & 
                ~NV097_SET_TEXTURE_FILTER_SIGN) | Value;
        g_d3ddev.TextureStageState[Stage].Dirty |= D3DTSS_CONTROL0_DIRTY;
        return D3D_OK;
    case D3DTSS_ALPHAKILL:
        g_d3ddev.TextureStageState[Stage].Control0 = 
            (g_d3ddev.TextureStageState[Stage].Control0 &
                 ~NV097_SET_TEXTURE_CONTROL0_ALPHA_KILL_ENABLE) | Value;
        g_d3ddev.TextureStageState[Stage].Dirty |= D3DTSS_CONTROL0_DIRTY;
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

    int i = 0;
    for (; pFunction[i] != D3DVS_END() && Address + i < 136; i += 4) {
        hr = D3DDevice_PushCmd(NV097_SET_TRANSFORM_PROGRAM, 4);
        if(FAILED(hr))
            return hr;
        hr = D3DDevice_PushA(&pFunction[i], 4);
        if(FAILED(hr)) return hr;
    }

    // If we overran, this means pFunction did not contain a D3DVS_END() token.
    // Either pFunction does not point to a valid vertex shader program, or 
    // that program is too long to be supported by the hardware (max length 
    // supported is 136 DWORDs). Either way, this is an error on the caller's 
    // part.
#if NXDK_DEBUG
    if (Address + i > 132) {
        assert(false);
        return D3DERR_INVALIDCALL;
    }
#endif // NXDK_DEBUG

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
        DWORD Control0 = g_d3ddev.Control0;
        if (Value == TRUE)
            g_d3ddev.Control0 |= NV097_SET_CONTROL0_STENCIL_WRITE_ENABLE;
        else
            g_d3ddev.Control0 &= ~NV097_SET_CONTROL0_STENCIL_WRITE_ENABLE;

        if (g_d3ddev.Control0 == Control0)
            return D3D_OK;
        
        Value = g_d3ddev.Control0;
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
        g_d3ddev.PresentInterval = Value;
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
        if (Value == D3DCULL_NONE) {
            g_d3ddev.RenderState.CullMode = 0;
            HRESULT hr = D3DDevice_PushCmd(NV097_SET_CULL_FACE_ENABLE, 0);
            if (FAILED(hr)) return hr;
        }
        D3D_FALLTHROUGH();
    case D3DRS_FRONTFACE:
        return D3DDevice_PushCmd(NV097_SET_FRONT_FACE, Value);
    case D3DRS_TEXTUREFACTOR:
        return D3DDevice_PushCmd(NV097_SET_BLEND_COLOR, Value);
    case D3DRS_LOGICOP: {
        DWORD LogicOp = g_d3ddev.RenderState.LogicOp;
        BOOL bWasLogicOpEnabled = LogicOp == D3DLOGICOP_NONE;
        g_d3ddev.RenderState.LogicOp = Value;
        BOOL bIsLogicOpEnabled = g_d3ddev.RenderState.LogicOp == D3DLOGICOP_NONE;
        if (bWasLogicOpEnabled != bIsLogicOpEnabled) {
            HRESULT hr = D3DDevice_PushCmd(NV097_SET_LOGIC_OP_ENABLE, 
                                           bIsLogicOpEnabled);
            if (FAILED(hr)) return hr;
        }
        
        if (LogicOp != g_d3ddev.RenderState.LogicOp) 
            return D3DDevice_PushCmd(NV097_SET_LOGIC_OP, LogicOp);
        else
            return D3D_OK;
    }
    case D3DRS_EDGEANTIALIAS:
        return D3DDevice_PushCmd(NV097_SET_LINE_SMOOTH_ENABLE, Value);
    case D3DRS_MULTISAMPLEANTIALIAS: {
        DWORD MultiSample = g_d3ddev.RenderState.MultiSample;
        if (Value == TRUE)
            g_d3ddev.RenderState.MultiSample |= NV097_SET_MULTISAMPLE_ENABLE;
        else
            g_d3ddev.RenderState.MultiSample &= ~NV097_SET_MULTISAMPLE_ENABLE;

        if (MultiSample != g_d3ddev.RenderState.MultiSample) {
            return D3DDevice_PushCmd(D3DRS_MULTISAMPLEANTIALIAS, 
                                     g_d3ddev.RenderState.MultiSample);
        } else {
            return D3D_OK;
        }
    }
    case D3DRS_MULTISAMPLEMASK: {
        DWORD MultiSample = g_d3ddev.RenderState.MultiSample;
        MASK_INPLACE(g_d3ddev.RenderState.MultiSample, 
                     NV097_SET_MULTISAMPLE_MASK, Value);
        if (MultiSample != g_d3ddev.RenderState.MultiSample) {
            return D3DDevice_PushCmd(D3DRS_MULTISAMPLEANTIALIAS, 
                                     g_d3ddev.RenderState.MultiSample);
        } else {
            return D3D_OK;
        }
    }
    case D3DRS_SHADOWFUNC:
        return D3DDevice_PushCmd(NV097_SET_SHADOW_COMPARE_FUNC, Value);
    case D3DRS_LINEWIDTH:
        return D3DDevice_PushCmd(NV097_SET_LINE_WIDTH, Value);
    case D3DRS_DXT1NOISEENABLE:
        return D3DDevice_FireInterrupt(PB_SETNOISE, Value);
    case D3DRS_OCCLUSIONCULLENABLE: {
        DWORD CullMode = g_d3ddev.RenderState.CullMode;
        BOOL bWasCullingEnabled = CullMode == 0;
        if (Value == TRUE)
            g_d3ddev.RenderState.CullMode |= NV097_SET_CULL_MODE_OCCLUSION;
        else
            g_d3ddev.RenderState.CullMode &= ~NV097_SET_CULL_MODE_OCCLUSION;
        BOOL bIsCullingEnabled = g_d3ddev.RenderState.CullMode == 0;
        if (bWasCullingEnabled != bIsCullingEnabled) {
            hr = D3DDevice_PushCmd(NV097_SET_CULL_FACE_ENABLE, CullMode);
            if (FAILED(hr)) return hr;
        }
        
        if (CullMode != g_d3ddev.RenderState.CullMode) 
            return D3DDevice_PushCmd(NV097_SET_CULL_MODE, CullMode);
        else
            return D3D_OK;
    }
    case D3DRS_STENCILCULLENABLE: {
        DWORD CullMode = g_d3ddev.RenderState.CullMode;
        BOOL bWasCullingEnabled = CullMode == 0;
        if (Value == TRUE)
            g_d3ddev.RenderState.CullMode |= NV097_SET_CULL_MODE_STENCIL;
        else
            g_d3ddev.RenderState.CullMode &= ~NV097_SET_CULL_MODE_STENCIL;
        BOOL bIsCullingEnabled = g_d3ddev.RenderState.CullMode == 0;
        if (bWasCullingEnabled != bIsCullingEnabled) {
            hr = D3DDevice_PushCmd(NV097_SET_CULL_FACE_ENABLE, CullMode);
            if (FAILED(hr)) return hr;
        }
        
        if (CullMode != g_d3ddev.RenderState.CullMode) 
            return D3DDevice_PushCmd(NV097_SET_CULL_MODE, CullMode);
        else
            return D3D_OK;
    }
    case D3DRS_ROPZCMPALWAYSREAD: {
        DWORD RopZ = g_d3ddev.RenderState.RopZ;
        if (Value == TRUE)
            g_d3ddev.RenderState.RopZ |=  0x08000000;
        else
            g_d3ddev.RenderState.RopZ &= ~0x08000000;
        if (RopZ != g_d3ddev.RenderState.RopZ) {
            return D3DDevice_FireInterrupt2(PB_SETOUTER, 
                                            NV_PGRAPH_UNKNOWN_400B80, 
                                            g_d3ddev.RenderState.RopZ);
        } else {
            return D3D_OK;
        }
    }
    case D3DRS_ROPZREAD: {
        DWORD RopZ = g_d3ddev.RenderState.RopZ;
        if (Value == TRUE)
            g_d3ddev.RenderState.RopZ |=  0x00100000;
        else
            g_d3ddev.RenderState.RopZ &= ~0x00100000;

        if (RopZ != g_d3ddev.RenderState.RopZ) {
            return D3DDevice_FireInterrupt2(PB_SETOUTER, 
                                            NV_PGRAPH_UNKNOWN_400B80,
                                            g_d3ddev.RenderState.RopZ);
        } else {
            return D3D_OK;
        }
    }
    case D3DRS_DONOTCULLUNCOMPRESSED:
        DWORD Debug5 = g_d3ddev.RenderState.Debug5;
        if (Value == TRUE)
            g_d3ddev.RenderState.Debug5 |=  NV_PGRAPH_DEBUG_5_ZCULL_SPARE2_ENABLED;
        else
            g_d3ddev.RenderState.Debug5 &= ~NV_PGRAPH_DEBUG_5_ZCULL_SPARE2_ENABLED;

        if (Debug5 != g_d3ddev.RenderState.Debug5)
            return D3DDevice_FireInterrupt2(PB_SETOUTER, NV_PGRAPH_DEBUG_5, Debug5);
        else
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

HRESULT IDirect3DDevice8_SetRenderState(
    LPDIRECT3DDEVICE8 pThis, D3DRENDERSTATETYPE Type, DWORD Value)
{
    assert(pThis == &g_d3ddev.iface);
    return D3DDevice_SetRenderState(Type, Value);
}

HRESULT D3DDevice_SetPixelShaderProgram(CONST D3DPIXELSHADERDEF *pPSDef) {
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

HRESULT IDirect3DDevice8_SetPixelShaderProgram(LPDIRECT3DDEVICE8 pThis, 
                                               CONST D3DPIXELSHADERDEF *pPSDef)
{
    assert(pThis == &g_d3ddev.iface);
    return D3DDevice_SetPixelShaderProgram(pPSDef);
}

HRESULT D3DDevice_ClearRect(CONST D3DRECT* pRect, DWORD Flags) {
    assert((Flags & 0x0C) == 0);
    HRESULT hr = D3DDevice_PushCmd(NV097_SET_CLEAR_RECT_HORIZONTAL,
        MASK(NV097_SET_CLEAR_RECT_HORIZONTAL_X2, pRect->x2 - 1) |
        MASK(NV097_SET_CLEAR_RECT_HORIZONTAL_X1, pRect->x1));

    if (FAILED(hr)) return hr;
    hr = D3DDevice_PushCmd(NV097_SET_CLEAR_RECT_VERTICAL,
        MASK(NV097_SET_CLEAR_RECT_VERTICAL_Y2, pRect->y2 - 1) |
        MASK(NV097_SET_CLEAR_RECT_VERTICAL_Y1, pRect->y1));
    if (FAILED(hr)) return hr;
    return D3DDevice_PushCmd(NV097_CLEAR_SURFACE, Flags & 0xF3);
}

HRESULT D3DDevice_Clear(DWORD Count, CONST D3DRECT* pRects, DWORD Flags,
                        D3DCOLOR Color, float Z, DWORD Stencil)
{
    HRESULT hr = D3D_OK;
    if (Stencil != g_d3ddev.Stencil) {
        g_d3ddev.Stencil = 
            MASK(NV097_SET_ZSTENCIL_CLEAR_VALUE_DEPTH, 
                 (DWORD)(Z * D3DZ_MAX_D24S8)) |
            MASK(NV097_SET_ZSTENCIL_CLEAR_VALUE_STENCIL, Stencil);
        g_d3ddev.ZStencilClearDirty = TRUE;
            
    }

    if (Color != g_d3ddev.ClearColor) {
        g_d3ddev.ClearColor = Color;
        g_d3ddev.ColorClearDirty = TRUE;
    }

    if ((Flags & D3DCLEAR_ZSTENCIL) && g_d3ddev.ZStencilClearDirty) {
        hr = D3DDevice_PushCmd(NV097_SET_ZSTENCIL_CLEAR_VALUE, 
            MASK(NV097_SET_ZSTENCIL_CLEAR_VALUE_DEPTH, 
                 (DWORD)(Z * D3DZ_MAX_D24S8)) |
            MASK(NV097_SET_ZSTENCIL_CLEAR_VALUE_STENCIL, Stencil));
        if (FAILED(hr)) return hr;
    }

    if ((Flags & D3DCLEAR_TARGET) && g_d3ddev.ColorClearDirty) {
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
            &g_d3ddev.pSurfaces[g_d3ddev.CurrentSurface]->iface, &desc);
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
        g_d3ddev.TextureStageState[Stage].Control0 &= 
            ~NV097_SET_TEXTURE_CONTROL0_ENABLE;
        return D3DDevice_PushCmd(NV097_SET_TEXTURE_CONTROL0(Stage), 
                                 g_d3ddev.TextureStageState[Stage].Control0);
    }

    IDirect3DBaseTexture8_AddRef(&g_d3ddev.pTexture[Stage]->iface);
    D3DBaseTexture* pBase = (D3DBaseTexture*)pTexture;
    D3DBaseTexture* pOldBase = g_d3ddev.pTexture[Stage];
    g_d3ddev.pTexture[Stage] = pBase;

    D3DRESOURCETYPE Type = IDirect3DBaseTexture8_GetType(pTexture);
    if (Type != D3DRTYPE_TEXTURE && Type != D3DRTYPE_CUBETEXTURE) {
        if (Type == D3DRTYPE_VOLUMETEXTURE)
            return E_NOTIMPL;
        else
            return D3DERR_INVALIDCALL;
    }

    D3DResourceInner* pResource = &pBase->inner.resource;
    HRESULT hr = D3DDevice_PushCmd(NV097_SET_TEXTURE_OFFSET(Stage), 
                           (DWORD)pResource->pContiguousMemory);
    if (FAILED(hr)) return hr;

    LPDIRECT3DSURFACE8 pSurface = NULL;
    if (Type == D3DRTYPE_TEXTURE) {
        HRESULT hr = IDirect3DTexture8_GetSurfaceLevel(
            (LPDIRECT3DTEXTURE8)pTexture, 0, &pSurface);
#if NXDK_DEBUG
            if (FAILED(hr)) return hr;
#endif // NXDK_DEBUG
    }

    D3DSURFACE_DESC desc;
    if (Type == D3DRTYPE_TEXTURE) {
        hr = IDirect3DSurface8_GetDesc((LPDIRECT3DSURFACE8)pSurface, &desc);
#if NXDK_DEBUG
        if (FAILED(hr)) return hr;
#endif // NXDK_DEBUG
    } else {
        hr = IDirect3DCubeTexture8_GetLevelDesc(
            (LPDIRECT3DCUBETEXTURE8)pTexture, 0, &desc);
#if NXDK_DEBUG
        if (FAILED(hr)) return hr;
#endif // NXDK_DEBUG
    }

    D3DSURFACE_DESC oldDesc;
    LPDIRECT3DSURFACE8 pOldSurface = NULL;
    D3DRESOURCETYPE oldType = IDirect3DBaseTexture8_GetType(
        (LPDIRECT3DBASETEXTURE8)pOldBase);
    if (pOldBase != NULL && oldType == D3DRTYPE_TEXTURE) {
        hr = IDirect3DTexture8_GetSurfaceLevel(
            (LPDIRECT3DTEXTURE8)pOldBase, 0, &pOldSurface);
#if NXDK_DEBUG
        if (FAILED(hr)) return hr;
#endif // NXDK_DEBUG
    }

    if (pOldSurface != NULL) {
        if (oldType == D3DRTYPE_TEXTURE)
            hr = IDirect3DSurface8_GetDesc(
                (LPDIRECT3DSURFACE8)pOldSurface, &oldDesc);
        else
            hr = IDirect3DCubeTexture8_GetLevelDesc(
                (LPDIRECT3DCUBETEXTURE8)pOldBase, 0, &oldDesc);
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

    if ((g_d3ddev.TextureStageState[Stage].Control0 & 
        NV097_SET_TEXTURE_CONTROL0_ENABLE) == 0) 
    {
        g_d3ddev.TextureStageState[Stage].Control0 |= 
            NV097_SET_TEXTURE_CONTROL0_ENABLE;
        hr = D3DDevice_PushCmd(NV097_SET_TEXTURE_CONTROL0(Stage), 
                               g_d3ddev.TextureStageState[Stage].Control0);
        if (FAILED(hr)) return hr;
    }

    UINT Pitch = desc.Width * D3D_FormatBytesPerPixel(desc.Format);
    UINT OldPitch = oldDesc.Width * D3D_FormatBytesPerPixel(oldDesc.Format);
    if (Pitch != OldPitch) {
        hr = D3DDevice_PushCmd(NV097_SET_TEXTURE_CONTROL1(Stage),
            MASK(NV097_SET_TEXTURE_CONTROL1_IMAGE_PITCH, Pitch));
        if (FAILED(hr)) return hr;
    }

    DWORD dwLevelCount = IDirect3DBaseTexture8_GetLevelCount(pTexture);
    DWORD dwOldLevelCount = 0;
    if (pOldBase != NULL) {
        dwOldLevelCount = IDirect3DBaseTexture8_GetLevelCount(
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

HRESULT IDirect3DDevice8_SetTexture(LPDIRECT3DDEVICE8 pThis, DWORD Stage, 
                                    LPDIRECT3DBASETEXTURE8 pTexture)
{
    assert(pThis == &g_d3ddev.iface);
    return D3DDevice_SetTexture(Stage, pTexture);
}

HRESULT D3DDevice_InsertCallback(D3DCALLBACKTYPE Type, 
                                 D3DCALLBACK pCallback, 
                                 DWORD Context)
{
    // TODO: avoid pipeline flush if Type == D3DCALLBACKTYPE_READ
    return D3DDevice_FireInterrupt2(PB_USER, (DWORD)pCallback, Context);
}

HRESULT IDirect3DDevice8_InsertCallback(LPDIRECT3DDEVICE8 pThis,
                                        D3DCALLBACKTYPE Type, 
                                        D3DCALLBACK pCallback, 
                                        DWORD Context)
{
    assert(pThis == &g_d3ddev.iface);
    return D3DDevice_InsertCallback(Type, pCallback, Context);
}
// ============================================================================

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
