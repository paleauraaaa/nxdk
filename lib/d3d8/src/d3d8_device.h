// This file contains the internal definition of D3DDevice and its interface.

// Anything contained herein is intended to be private to the outside world,
// but public within the D3D implementation. Some of these functions will be
// "exported" via D3DDevice's vtable, but their declarations or definitions
// need not be exposed to facilitate that.

#pragma once

#include "d3d8.h"
#include "d3d8_private.h"
#include "d3d8_resource.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

// Various render states we need to track in order to ensure they're updated
// as needed.
typedef struct D3DRenderState {
    D3DCULL FaceCull;
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
    // D3DTSS_*_DIRTY
    DWORD Dirty;
} D3DTextureStageState;

#define D3DTSS_ADDRESS_DIRTY   0x00000001
#define D3DTSS_FILTER_DIRTY    0x00000002
#define D3DTSS_CONTROL0_DIRTY  0x00000004

// The graphics device.
//
// An abstraction over the GPU's entire rendering pipeline. This is where most
// of the magic happens, and where the bulk of the implementation lies.
//
// The device is largely "stateless", with most operations immediately
// translating to GPU commands that are submitted without any tracking by D3D
// and without any way to query via the GPU. Some states are tracked to
// optimize for not sending unnecessary commands, but in general applications
// should keep track of any state they need to keep track of.
//
// As implementation details, the device piggybacks off pbkit for
// initialization and a couple other operations (presentation, most notably),
// and the interplay between these two results in some quirks (see
// D3DDevice_SyncPushBuffer and D3DDevice_Present), but by and large it
// operates independently of pbkit.
typedef struct D3DDevice IMPLEMENTS(IDirect3DDevice8) {
#ifndef __cplusplus
    IDirect3DDevice8        iface;
#else
    virtual D3DAPI ULONG AddRef() override;
    virtual D3DAPI ULONG Release() override;
    virtual D3DAPI VOID BlockUntilVerticalBlank() override;
    virtual D3DAPI HRESULT CreateImageSurface(
        UINT Width, UINT Height, D3DFORMAT Format,
        LPDIRECT3DSURFACE8* ppSurface) override;
    virtual D3DAPI HRESULT CreateDepthStencilSurface(
        UINT Width, UINT Height,
        D3DFORMAT Format,
        D3DMULTISAMPLE_TYPE MultiSampleType,
        LPDIRECT3DSURFACE8* ppSurface) override;
    virtual D3DAPI HRESULT CreateVertexBuffer(
        UINT Length, DWORD Usage, DWORD FVF,
        DWORD Pool, LPDIRECT3DVERTEXBUFFER8* ppVertexBuffer) override;
    virtual D3DAPI HRESULT CreateTexture(
        UINT Width,
        UINT Height,
        UINT Levels,
        DWORD Usage,
        D3DFORMAT Format,
        D3DPOOL Pool,
        LPDIRECT3DTEXTURE8* ppTexture) override;
    virtual D3DAPI HRESULT CreateCubeTexture(
        UINT EdgeLength,
        UINT Levels,
        DWORD Usage,
        D3DFORMAT Format,
        D3DPOOL Pool,
        LPDIRECT3DCUBETEXTURE8* ppCubeTexture) override;
    virtual D3DAPI HRESULT CreatePushBuffer(
        UINT Size, BOOL RunUsingCpuCopy,
        LPDIRECT3DPUSHBUFFER8* ppPushBuffer) override;
    virtual D3DAPI HRESULT BeginScene() override;
    virtual D3DAPI HRESULT EndScene() override;
    virtual D3DAPI HRESULT Present(
        CONST RECT* pSourceRect, CONST RECT* pDestRect) override;
    virtual D3DAPI HRESULT DrawPrimitive(
        D3DPRIMITIVETYPE PrimitiveType,
        UINT StartVertex, UINT VertexCount) override;
    virtual D3DAPI HRESULT DrawVertices(
        D3DPRIMITIVETYPE PrimitiveType,
        UINT StartVertex, UINT PrimitiveCount) override;
    virtual D3DAPI HRESULT DrawIndexedVertices(
        D3DPRIMITIVETYPE PrimitiveType,
        UINT VertexCount, CONST WORD *pIndexData) override;
    virtual D3DAPI HRESULT SetViewport(
        CONST D3DVIEWPORT8* pViewport) override;
    virtual D3DAPI HRESULT SetVertexShaderInputDirect(
        D3DVERTEXATTRIBUTEFORMAT *pVAF,
        UINT StreamCount,
        D3DSTREAM_INPUT *pStreamInputs) override;
    virtual D3DAPI HRESULT SetTextureStageState(
        DWORD Stage,
        D3DTEXTURESTAGESTATETYPE Type,
        DWORD Value) override;
    virtual D3DAPI HRESULT SetRenderState(
        D3DRENDERSTATETYPE Type, DWORD Value) override;
    virtual D3DAPI HRESULT LoadVertexShaderProgram(CONST DWORD *pFunction,
                                    DWORD Address) override;
    virtual D3DAPI HRESULT SetPixelShaderProgram(
        CONST D3DPIXELSHADERDEF *pPSDef) override;
    virtual D3DAPI HRESULT Clear(
        DWORD Count, CONST D3DRECT* pRects, DWORD Flags,
        D3DCOLOR Color, float Z, DWORD Stencil) override;
    virtual D3DAPI HRESULT SetScissors(
        DWORD Count, BOOL Exclusive, CONST D3DRECT *pRects) override;
    virtual D3DAPI HRESULT SetTile(
        DWORD Index, CONST D3DTILE* pTile) override;
    virtual D3DAPI HRESULT SetTexture(
        DWORD Stage, LPDIRECT3DBASETEXTURE8 pTexture) override;
    virtual D3DAPI HRESULT InsertCallback(
        D3DCALLBACKTYPE Type, D3DCALLBACK pCallback,  DWORD Context) override;
    virtual D3DAPI HRESULT Reset(
        D3DPRESENT_PARAMETERS* pPresentationParameters) override;
    virtual D3DAPI DWORD InsertFence() override;
    virtual D3DAPI VOID BlockOnFence(DWORD Fence) override;
    virtual D3DAPI BOOL IsFencePending(DWORD Fence) override;
    virtual D3DAPI HRESULT SetVertexShaderConstant(
        INT Register, CONST VOID* pConstantData, DWORD ConstantCount) override;
    virtual D3DAPI HRESULT SetPixelShaderConstant(
        DWORD Register, CONST VOID* pConstantData,
        DWORD ConstantCount) override;
    virtual D3DAPI HRESULT GetPushBuffer(
        LPDIRECT3DPUSHBUFFER8* ppPushBuffer) override;
    virtual D3DAPI VOID KickPushBuffer() override;
    virtual D3DAPI VOID SyncPushBuffer() override;
#endif // __cplusplus
    D3DRefcount             refcount;
    // Transfered from IDirect3D8 on device creation.
    DWORD                   KickOffSize;
    // Rendering surfaces. pSurfaces[2] will be unused if the device was
    // created with a back buffer count < 2.
    D3DSurface*             pSurfaces[3];
    // Index of the surface currently being targeted for rendering.
    UINT                    CurrentSurface;
    // Back buffer count + 1 (for front buffer).
    UINT                    MaxSurfaces;
    D3DSurface*             pDepthStencilSurface;
    // The "default" push buffer that will be used if a different push buffer
    // isn't currently being targeted by a BeginPushBuffer/EndPushBuffer block.
    D3DPushBuffer           DefaultPB;
    // Pointer to the push buffer currently being targeted. Will point to
    // DefaultPB outside of BeginPushBuffer/EndPushBuffer blocks.
    D3DPushBuffer*          pCurrentPB;
    // Tracker for whether we're in a BeginScene/EndScene block. Mostly useful
    // for debugging.
    BOOL                    bInScene;
    // Current viewport. Can be queried by GetViewport, and also used by
    // SetViewport to avoid sending GPU commands if the "new" viewport is
    // identical to the old one.
    D3DVIEWPORT8            Viewport;
    // Retrieved from pb_get_vbl_count. Used to maintain presentation
    // intervals.
    UINT                    LastPresentVBlankCount;
    // Avoids freeing the rendering surfaces if they were supplied in
    // CreateDevice's presentation parameters, rather than being allocated by
    // D3D.
    BOOL                    bUserSuppliedBufferSurfaces;
    BOOL                    bUserSuppliedDepthStencilSurface;

    // GPU state tracking to avoid issuing new commands when unnecessary.
    // TODO: These should probably go in their own struct.
    DWORD                   Control0;
    DWORD                   Stencil;
    BOOL                    ZStencilClearDirty;
    DWORD                   ClearColor;
    BOOL                    ColorClearDirty;

    D3DPRESENT_INTERVAL     PresentInterval;

    // More GPU state tracking.
    D3DRenderState          RenderState;
    D3DTextureStageState    TextureStageState[D3DTSS_MAXSTAGES];
    DWORD                   Wrap[D3DTSS_MAXSTAGES];
    DWORD                   WrapDirty;

    // Currently attached textures. Used by SetTexture, NULL when not attached.
    D3DBaseTexture*         pTexture[D3DTSS_MAXSTAGES];

    // The next fence to be handed out by InsertFence.
    DWORD                   dwNextFence;
    // Last fence processed by the GPU.
    DWORD                   dwLastFenceCompleted;
    // Event set by pb_subprog to alert D3D that a fence has been passed.
    // dwLastFenceCompleted will be updated to said fence. Useful for
    // efficiently awaiting a fence instead of just spinlocking on it.
    HANDLE                  hFenceEvent;
} D3DDevice;

D3DAPI ULONG Direct3DDevice8_AddRef(LPDIRECT3DDEVICE8 pThis);
D3DAPI ULONG Direct3DDevice8_Release(LPDIRECT3DDEVICE8 pThis);
D3DAPI VOID  Direct3DDevice8_BlockUntilVerticalBlank(LPDIRECT3DDEVICE8 pThis);
D3DAPI HRESULT Direct3DDevice8_CreateImageSurface(
    LPDIRECT3DDEVICE8 pThis, UINT Width, UINT Height,
    D3DFORMAT Format, LPDIRECT3DSURFACE8* ppSurface);
D3DAPI HRESULT Direct3DDevice8_CreateDepthStencilSurface(
    LPDIRECT3DDEVICE8 pThis, UINT Width, UINT Height, D3DFORMAT Format,
    D3DMULTISAMPLE_TYPE MultiSample, IDirect3DSurface8** ppSurface);
D3DAPI HRESULT Direct3DDevice8_CreateVertexBuffer(
    LPDIRECT3DDEVICE8 pThis, UINT Length, DWORD Usage, DWORD FVF,
    DWORD Pool, LPDIRECT3DVERTEXBUFFER8* ppVertexBuffer);
D3DAPI HRESULT Direct3DDevice8_CreatePushBuffer(
    LPDIRECT3DDEVICE8 pThis, UINT Size,
    BOOL RunUsingCpuCopy, LPDIRECT3DPUSHBUFFER8* ppPushBuffer);
D3DAPI HRESULT Direct3DDevice8_BeginScene(LPDIRECT3DDEVICE8 pThis);
D3DAPI HRESULT Direct3DDevice8_EndScene(LPDIRECT3DDEVICE8 pThis);
D3DAPI HRESULT Direct3DDevice8_Present(LPDIRECT3DDEVICE8 pThis,
                                       CONST RECT* pSourceRect,
                                       CONST RECT* pDestRect);
D3DAPI HRESULT Direct3DDevice8_DrawPrimitive(LPDIRECT3DDEVICE8 pThis,
                                             D3DPRIMITIVETYPE PrimitiveType,
                                             UINT StartVertex,
                                             UINT PrimitiveCount);
D3DAPI HRESULT Direct3DDevice8_DrawVertices(LPDIRECT3DDEVICE8 pThis,
                                            D3DPRIMITIVETYPE PrimitiveType,
                                            UINT StartVertex,
                                            UINT VertexCount);
D3DAPI HRESULT Direct3DDevice8_DrawIndexedVertices(
    LPDIRECT3DDEVICE8 pThis, D3DPRIMITIVETYPE PrimitiveType,
    UINT VertexCount, CONST WORD *pIndexData);
D3DAPI HRESULT Direct3DDevice8_SetViewport(LPDIRECT3DDEVICE8 pThis,
                                           CONST D3DVIEWPORT8* pViewport);
D3DAPI HRESULT Direct3DDevice8_SetVertexShaderInputDirect(
    LPDIRECT3DDEVICE8 pThis,
    D3DVERTEXATTRIBUTEFORMAT *pVAF,
    UINT StreamCount,
    D3DSTREAM_INPUT *pStreamInputs);
D3DAPI HRESULT Direct3DDevice8_CreateTexture(LPDIRECT3DDEVICE8 pThis,
                                             UINT Width,
                                             UINT Height,
                                             UINT Levels,
                                             DWORD Usage,
                                             D3DFORMAT Format,
                                             D3DPOOL Pool,
                                             LPDIRECT3DTEXTURE8* ppTexture);
D3DAPI HRESULT Direct3DDevice8_CreateCubeTexture(
    LPDIRECT3DDEVICE8 pThis, UINT EdgeLength, UINT Levels, DWORD Usage,
    D3DFORMAT Format, D3DPOOL Pool, LPDIRECT3DCUBETEXTURE8* ppCubeTexture);
D3DAPI HRESULT Direct3DDevice8_SetTextureStageState(
    LPDIRECT3DDEVICE8 pThis, DWORD Stage,
    D3DTEXTURESTAGESTATETYPE Type, DWORD Value);
D3DAPI HRESULT Direct3DDevice8_SetRenderState(
    LPDIRECT3DDEVICE8 pThis, D3DRENDERSTATETYPE Type, DWORD Value);
D3DAPI HRESULT Direct3DDevice8_LoadVertexShaderProgram(
    LPDIRECT3DDEVICE8 pThis, CONST DWORD *pFunction, DWORD Address);
D3DAPI HRESULT Direct3DDevice8_SetPixelShaderProgram(
    LPDIRECT3DDEVICE8 pThis, CONST D3DPIXELSHADERDEF *pPSDef);
D3DAPI HRESULT Direct3DDevice8_Clear(LPDIRECT3DDEVICE8 pThis, DWORD Count,
                                     CONST D3DRECT* pRects, DWORD Flags,
                                     D3DCOLOR Color, float Z, DWORD Stencil);
D3DAPI HRESULT Direct3DDevice8_SetScissors(LPDIRECT3DDEVICE8 pThis,
                                            DWORD Count, BOOL Exclusive,
                                            CONST D3DRECT *pRects);
D3DAPI HRESULT D3DDevice_SetRenderState(D3DRENDERSTATETYPE Type, DWORD Value);
D3DAPI HRESULT Direct3DDevice8_SetTile(LPDIRECT3DDEVICE8 pThis, DWORD Index,
                                       CONST D3DTILE* pTile);
D3DAPI HRESULT Direct3DDevice8_SetTexture(LPDIRECT3DDEVICE8 pThis,
                                          DWORD Stage,
                                          LPDIRECT3DBASETEXTURE8 pTexture);
D3DAPI HRESULT Direct3DDevice8_InsertCallback(LPDIRECT3DDEVICE8 pThis,
                                              D3DCALLBACKTYPE Type,
                                              D3DCALLBACK pCallback,
                                              DWORD Context);
D3DAPI HRESULT Direct3DDevice8_Reset(
    LPDIRECT3DDEVICE8 pThis,
    D3DPRESENT_PARAMETERS* pPresentationParameters);
D3DAPI DWORD Direct3DDevice8_InsertFence(LPDIRECT3DDEVICE8 pThis);
D3DAPI VOID  Direct3DDevice8_BlockOnFence(LPDIRECT3DDEVICE8 pThis,
                                          DWORD Fence);
D3DAPI BOOL  Direct3DDevice8_IsFencePending(LPDIRECT3DDEVICE8 pThis,
                                            DWORD Fence);
D3DAPI HRESULT Direct3DDevice8_SetVertexShaderConstant(
    LPDIRECT3DDEVICE8 pThis, INT Register,
    CONST VOID* pConstantData, DWORD ConstantCount);
D3DAPI HRESULT Direct3DDevice8_SetPixelShaderConstant(
    LPDIRECT3DDEVICE8 pThis, DWORD Register, CONST VOID* pConstantData,
    DWORD ConstantCount);
D3DAPI HRESULT Direct3DDevice8_GetPushBuffer(
    LPDIRECT3DDEVICE8 pThis, LPDIRECT3DPUSHBUFFER8* ppPushBuffer);
D3DAPI VOID Direct3DDevice8_KickPushBuffer(LPDIRECT3DDEVICE8 pThis);
D3DAPI VOID Direct3DDevice8_SyncPushBuffer(LPDIRECT3DDEVICE8 pThis);

HRESULT D3DDevice_BeginPushBuffer(D3DPushBuffer* pPushBuffer);
VOID    D3DDevice_KickPushBuffer(void);
VOID    D3DDevice_SyncPushBuffer(void);
HRESULT D3DDevice_Push1    (DWORD dwData);
HRESULT D3DDevice_PushCmd  (DWORD cmd, DWORD dwData);
HRESULT D3DDevice_PushCmd2 (DWORD cmd, DWORD dwData1,
                            DWORD dwData2, BOOL bIncrement);
HRESULT D3DDevice_PushCmd3 (DWORD cmd, DWORD dwData1,
                            DWORD dwData2, DWORD dwData3, BOOL bIncrement);
HRESULT D3DDevice_PushCmd4 (DWORD cmd, DWORD dwData1, DWORD dwData2,
                            DWORD dwData3, DWORD dwData4, BOOL bIncrement);
HRESULT D3DDevice_PushCmdf (DWORD cmd, float fData);
HRESULT D3DDevice_PushCmd2f(DWORD cmd, float fData1,
                            float fData2, BOOL bIncrement);
HRESULT D3DDevice_PushCmd3f(DWORD cmd, float fData1,
                            float fData2, float fData3, BOOL bIncrement);
HRESULT D3DDevice_PushCmd4f(DWORD cmd, float fData1, float fData2,
                            float fData3, float fData4, BOOL bIncrement);
HRESULT D3DDevice_PushN    (DWORD dwData, SIZE_T n);
HRESULT D3DDevice_PushA    (CONST DWORD* pdwData, SIZE_T n);
HRESULT D3DDevice_InitDeviceState(void);
DWORD   D3DDevice_GetCurrentFence(void);
DWORD   D3DDevice_GetLastCompletedFence(void);
VOID    D3DDevice_BlockOnFence(DWORD Fence);
HRESULT D3DDevice_SetViewport(CONST D3DVIEWPORT8* pViewport);
HRESULT D3DDevice_SelectVertexShaderDirect(DWORD Address);
HRESULT D3DDevice_SetTexture(DWORD Stage, D3DBaseTexture* pTexture);

extern D3DDevice*            g_pDevice;
#ifndef __cplusplus
extern IDirect3DDeviceVtbl8* g_pDeviceVtbl;
#endif

#ifdef __cplusplus
}
#endif // __cplusplus
