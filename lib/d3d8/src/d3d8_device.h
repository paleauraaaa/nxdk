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
    DWORD Dirty;
} D3DTextureStageState;

#define D3DTSS_ADDRESS_DIRTY   0x00000001
#define D3DTSS_FILTER_DIRTY    0x00000002
#define D3DTSS_CONTROL0_DIRTY  0x00000004

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
#endif // __cplusplus
    D3DRefcount             refcount;
    DWORD                   KickOffSize;
    D3DSurface*             pSurfaces[3];
    UINT                    CurrentSurface;
    UINT                    MaxSurfaces;
    D3DSurface*             pDepthStencilSurface;
    D3DPushBuffer           DefaultPB;
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
    DWORD                   dwNextFence;
    DWORD                   dwLastFenceCompleted;
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

HRESULT D3DDevice_Push1    (DWORD dwData);
HRESULT D3DDevice_PushCmd  (DWORD cmd, DWORD dwData);
HRESULT D3DDevice_PushCmd2 (DWORD cmd, DWORD dwData1, DWORD dwData2);
HRESULT D3DDevice_PushCmd3 (DWORD cmd, DWORD dwData1, 
                            DWORD dwData2, DWORD dwData3);
HRESULT D3DDevice_PushCmd4 (DWORD cmd, DWORD dwData1, DWORD dwData2, 
                                       DWORD dwData3, DWORD dwData4);
HRESULT D3DDevice_PushCmdf (DWORD cmd, float fData);
HRESULT D3DDevice_PushCmd2f(DWORD cmd, float fData1, float fData2);
HRESULT D3DDevice_PushCmd3f(DWORD cmd, float fData1, 
                            float fData2, float fData3);
HRESULT D3DDevice_PushCmd4f(DWORD cmd, float fData1, float fData2, 
                                       float fData3, float fData4);
HRESULT D3DDevice_PushN    (DWORD dwData, SIZE_T n);
HRESULT D3DDevice_PushA    (CONST DWORD* pdwData, SIZE_T n);
HRESULT D3DDevice_InitDeviceState(void);
DWORD   D3DDevice_GetCurrentFence(void); 
DWORD   D3DDevice_GetLastCompletedFence(void);
VOID    D3DDevice_BlockOnFence(DWORD Fence);

extern D3DDevice*            g_pDevice;
extern IDirect3DDeviceVtbl8* g_pDeviceVtbl;

#ifdef __cplusplus
}
#endif // __cplusplus