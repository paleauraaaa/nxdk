#pragma once

#include "d3d8.h"
#include "d3d8_private.h"
#include "d3d8_resource.h"

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

extern D3DDevice            g_d3ddev;
extern IDirect3DDeviceVtbl8 g_d3ddevVtbl;