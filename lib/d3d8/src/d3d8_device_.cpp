// Even though the actual bodies of these member functions are all implemented
// on the C side (Direct3DDevice8_*), and the vtables are manually set up on 
// the C side, we have to define them in C++ too to prevent abstract base 
// class compile errors.
//
// For now, these functions should do nothing besides call the appropriate 
// Direct3DDevice8_* function. In theory, they should maybe even throw an 
// assertion in debug mode, since they should never actually be called 
// (constructing the vtables in C will overwrite them.) 
//
// The C wrapper functions (IDirect3DDevice8_*) are defined to allow for 
// ergonomic calls to the vtable functions from C and C++. They should always
// call the vtable's funnction (pThis->*), not the implementation function 
// (Direct3DDevice8_*), so that if the vtable function is overriden, 
// that override is honored.
//
// This file's name must have the trailing underscore (or some name that 
// doesn't conflict with d3d8_device.c) to prevent compilation of one file
// overriding the object file of the other source file (i.e. d3d8_device.c 
// will get compiled to d3d8_device.obj, but so would d3d8_device.cpp).

#include "d3d8_device.h"

D3DAPI ULONG D3DDevice::AddRef() {
    return Direct3DDevice8_AddRef(this);
}

D3DAPI D3DEXTERN ULONG IDirect3DDevice8_AddRef(LPDIRECT3DDEVICE8 pThis) {
    return pThis->AddRef();
}

D3DAPI ULONG D3DDevice::Release() {
    return Direct3DDevice8_Release(this);
}

D3DAPI D3DEXTERN ULONG IDirect3DDevice8_Release(LPDIRECT3DDEVICE8 pThis) {
    return pThis->Release();
}

D3DAPI VOID D3DDevice::BlockUntilVerticalBlank() {
    Direct3DDevice8_BlockUntilVerticalBlank(this);
}

D3DAPI D3DEXTERN VOID IDirect3DDevice8_BlockUntilVerticalBlank(
    LPDIRECT3DDEVICE8 pThis) 
{
    pThis->BlockUntilVerticalBlank();
}

D3DAPI HRESULT D3DDevice::CreateImageSurface(
    UINT Width, UINT Height, D3DFORMAT Format, 
    LPDIRECT3DSURFACE8* ppSurface) 
{
    return Direct3DDevice8_CreateImageSurface(this, Width, Height, 
                                              Format, ppSurface);
}

D3DAPI D3DEXTERN HRESULT IDirect3DDevice8_CreateImageSurface(
    LPDIRECT3DDEVICE8 pThis, UINT Width, UINT Height, D3DFORMAT Format, 
    LPDIRECT3DSURFACE8* ppSurface) 
{
    return pThis->CreateImageSurface(Width, Height, Format, ppSurface);
}

D3DAPI HRESULT D3DDevice::CreateDepthStencilSurface(
    UINT Width, UINT Height, D3DFORMAT Format, 
    D3DMULTISAMPLE_TYPE MultiSampleType,
    LPDIRECT3DSURFACE8* ppSurface) 
{
    return Direct3DDevice8_CreateDepthStencilSurface(this, Width, Height, 
                                                     Format, MultiSampleType, 
                                                     ppSurface);
}

D3DAPI D3DEXTERN HRESULT IDirect3DDevice8_CreateDepthStencilSurface(
    LPDIRECT3DDEVICE8 pThis, UINT Width, UINT Height, D3DFORMAT Format,
    D3DMULTISAMPLE_TYPE MultiSampleType, LPDIRECT3DSURFACE8* ppSurface) 
{
    return pThis->CreateDepthStencilSurface(Width, Height, Format, 
                                            MultiSampleType, ppSurface);
}

D3DAPI HRESULT D3DDevice::CreateVertexBuffer(
    UINT Length, DWORD Usage, DWORD FVF, 
    DWORD Pool, LPDIRECT3DVERTEXBUFFER8* ppVertexBuffer) 
{
    return Direct3DDevice8_CreateVertexBuffer(this, Length, Usage, FVF, 
                                              Pool, ppVertexBuffer);
}

D3DAPI D3DEXTERN HRESULT IDirect3DDevice8_CreateVertexBuffer(
    LPDIRECT3DDEVICE8 pThis, UINT Length, DWORD Usage, DWORD FVF, 
    DWORD Pool, LPDIRECT3DVERTEXBUFFER8* ppVertexBuffer) 
{
    return pThis->CreateVertexBuffer(Length, Usage, FVF, Pool, ppVertexBuffer);
}

D3DAPI HRESULT D3DDevice::CreatePushBuffer(
    UINT Size, BOOL RunUsingCpuCopy, 
    LPDIRECT3DPUSHBUFFER8* ppPushBuffer) 
{
    return Direct3DDevice8_CreatePushBuffer(this, Size, RunUsingCpuCopy, 
                                            ppPushBuffer);
}

D3DAPI D3DEXTERN HRESULT IDirect3DDevice8_CreatePushBuffer(
    LPDIRECT3DDEVICE8 pThis, UINT Size, BOOL RunUsingCpuCopy, 
    LPDIRECT3DPUSHBUFFER8* ppPushBuffer) 
{
    return pThis->CreatePushBuffer(Size, RunUsingCpuCopy, ppPushBuffer);
}

D3DAPI HRESULT D3DDevice::CreateTexture(
    UINT Width, UINT Height, UINT Levels, DWORD Usage, D3DFORMAT Format,
    D3DPOOL Pool, LPDIRECT3DTEXTURE8* ppTexture) 
{
    return Direct3DDevice8_CreateTexture(
        this, Width, Height, Levels, Usage, Format, Pool, ppTexture);
}

D3DAPI D3DEXTERN HRESULT IDirect3DDevice8_CreateTexture(
    LPDIRECT3DDEVICE8 pThis, UINT Width, UINT Height, UINT Levels, DWORD Usage,
    D3DFORMAT Format, D3DPOOL Pool, LPDIRECT3DTEXTURE8* ppTexture) 
{
    return pThis->CreateTexture(Width, Height, Levels, Usage, Format, Pool, 
                                ppTexture);
}

D3DAPI HRESULT D3DDevice::CreateCubeTexture(
    UINT EdgeLength, 
    UINT Levels, 
    DWORD Usage, 
    D3DFORMAT Format, 
    D3DPOOL Pool,
    LPDIRECT3DCUBETEXTURE8* ppCubeTexture)
{
    return Direct3DDevice8_CreateCubeTexture(this, EdgeLength, Levels, 
                                             Usage, Format, Pool, 
                                             ppCubeTexture);
}

D3DAPI D3DEXTERN HRESULT IDirect3DDevice8_CreateCubeTexture(
    LPDIRECT3DDEVICE8 pThis, UINT EdgeLength, UINT Levels, DWORD Usage, 
    D3DFORMAT Format, D3DPOOL Pool, LPDIRECT3DCUBETEXTURE8* ppCubeTexture) 
{
    return pThis->CreateCubeTexture(EdgeLength, Levels, Usage, Format, Pool, 
                                    ppCubeTexture);
}

D3DAPI HRESULT D3DDevice::BeginScene() {
    return Direct3DDevice8_BeginScene(this);
}

D3DAPI D3DEXTERN HRESULT IDirect3DDevice8_BeginScene(LPDIRECT3DDEVICE8 pThis) {
    return pThis->BeginScene();
}

D3DAPI HRESULT D3DDevice::EndScene() {
    return Direct3DDevice8_EndScene(this);
}

D3DAPI D3DEXTERN HRESULT IDirect3DDevice8_EndScene(LPDIRECT3DDEVICE8 pThis) {
    return pThis->EndScene();
}

D3DAPI HRESULT D3DDevice::Present(CONST RECT* pSourceRect, 
                                  CONST RECT* pDestRect) 
{
    return Direct3DDevice8_Present(this, pSourceRect, pDestRect);
}

D3DAPI D3DEXTERN HRESULT IDirect3DDevice8_Present(
    LPDIRECT3DDEVICE8 pThis, CONST RECT* pSourceRect, CONST RECT* pDestRect) 
{
    return pThis->Present(pSourceRect, pDestRect);
}

D3DAPI HRESULT D3DDevice::DrawPrimitive(
    D3DPRIMITIVETYPE PrimitiveType, UINT StartVertex, UINT VertexCount) 
{
    return Direct3DDevice8_DrawPrimitive(this,
        PrimitiveType, StartVertex, VertexCount);
}

D3DAPI D3DEXTERN HRESULT IDirect3DDevice8_DrawPrimitive(
    LPDIRECT3DDEVICE8 pThis, D3DPRIMITIVETYPE PrimitiveType, UINT StartVertex, 
    UINT VertexCount) 
{
    return pThis->DrawPrimitive(PrimitiveType, StartVertex, VertexCount);
}

D3DAPI HRESULT D3DDevice::DrawVertices(
    D3DPRIMITIVETYPE PrimitiveType, UINT StartVertex, UINT PrimitiveCount) 
{
    return Direct3DDevice8_DrawVertices(this,
        PrimitiveType, StartVertex, PrimitiveCount);
}

D3DAPI D3DEXTERN HRESULT IDirect3DDevice8_DrawVertices(
    LPDIRECT3DDEVICE8 pThis, D3DPRIMITIVETYPE PrimitiveType, UINT StartVertex, 
    UINT PrimitiveCount) 
{
    return pThis->DrawVertices(PrimitiveType, StartVertex, PrimitiveCount);
}

D3DAPI HRESULT D3DDevice::SetViewport(
    CONST D3DVIEWPORT8* pViewport) 
{
    return Direct3DDevice8_SetViewport(this, pViewport);
}

D3DAPI D3DEXTERN HRESULT IDirect3DDevice8_SetViewport(
    LPDIRECT3DDEVICE8 pThis, CONST D3DVIEWPORT8* pViewport) 
{
    return pThis->SetViewport(pViewport);
}

D3DAPI HRESULT D3DDevice::SetVertexShaderInputDirect(
    D3DVERTEXATTRIBUTEFORMAT *pVAF,
    UINT StreamCount, 
    D3DSTREAM_INPUT *pStreamInputs) 
{
    return Direct3DDevice8_SetVertexShaderInputDirect(this, pVAF, 
                                                      StreamCount, 
                                                      pStreamInputs);
}

D3DAPI D3DEXTERN HRESULT IDirect3DDevice8_SetVertexShaderInputDirect(
    LPDIRECT3DDEVICE8 pThis, D3DVERTEXATTRIBUTEFORMAT *pVAF, UINT StreamCount, 
    D3DSTREAM_INPUT *pStreamInputs) 
{
    return pThis->SetVertexShaderInputDirect(pVAF, StreamCount, pStreamInputs);
}

D3DAPI HRESULT D3DDevice::SetTextureStageState(
    DWORD Stage,
    D3DTEXTURESTAGESTATETYPE Type,
    DWORD Value) 
{
    return Direct3DDevice8_SetTextureStageState(this, Stage, Type, Value);
}

D3DAPI HRESULT IDirect3DDevice8_SetTextureStageState(
    LPDIRECT3DDEVICE8 pThis, DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, 
    DWORD Value) 
{
    return pThis->SetTextureStageState(Stage, Type, Value);
}

D3DAPI HRESULT D3DDevice::SetRenderState(D3DRENDERSTATETYPE Type,
                                                    DWORD Value) 
{
    return Direct3DDevice8_SetRenderState(this, Type, Value);
}

D3DAPI HRESULT IDirect3DDevice8_SetRenderState(
    LPDIRECT3DDEVICE8 pThis, D3DRENDERSTATETYPE Type, DWORD Value) 
{
    return pThis->SetRenderState(Type, Value);
}

D3DAPI HRESULT D3DDevice::LoadVertexShaderProgram(
    CONST DWORD *pFunction, DWORD Address) 
{
    return Direct3DDevice8_LoadVertexShaderProgram(this, pFunction, Address);
}

D3DAPI HRESULT IDirect3DDevice8_LoadVertexShaderProgram(
    LPDIRECT3DDEVICE8 pThis, CONST DWORD *pFunction, DWORD Address) 
{
    return pThis->LoadVertexShaderProgram(pFunction, Address);
}

D3DAPI HRESULT D3DDevice::SetPixelShaderProgram(
    CONST D3DPIXELSHADERDEF *pPSDef) 
{
    return Direct3DDevice8_SetPixelShaderProgram(this, pPSDef);
}

D3DAPI HRESULT IDirect3DDevice8_SetPixelShaderProgram(
    LPDIRECT3DDEVICE8 pThis, CONST D3DPIXELSHADERDEF *pPSDef) 
{
    return pThis->SetPixelShaderProgram(pPSDef);
}

D3DAPI HRESULT D3DDevice::Clear(DWORD Count, CONST D3DRECT* pRects, 
                                             DWORD Flags, D3DCOLOR Color,
                                             float Z, DWORD Stencil) 
{
    return Direct3DDevice8_Clear(this, Count, pRects, 
                                 Flags, Color, Z, Stencil);
}

D3DAPI HRESULT IDirect3DDevice8_Clear(
    LPDIRECT3DDEVICE8 pThis, DWORD Count, CONST D3DRECT* pRects, DWORD Flags, 
    D3DCOLOR Color, float Z, DWORD Stencil) 
{
    return pThis->Clear(Count, pRects, Flags, Color, Z, Stencil);
}

D3DAPI HRESULT D3DDevice::SetScissors(DWORD Count, BOOL Exclusive, 
                                      CONST D3DRECT *pRects) 
{
    return Direct3DDevice8_SetScissors(this, Count, Exclusive, pRects);
}

D3DAPI HRESULT IDirect3DDevice8_SetScissors(
    LPDIRECT3DDEVICE8 pThis, DWORD Count, BOOL Exclusive, CONST D3DRECT *pRects) 
{
    return pThis->SetScissors(Count, Exclusive, pRects);
}

D3DAPI HRESULT D3DDevice::SetTile(DWORD Index, CONST D3DTILE* pTile) 
{
    return Direct3DDevice8_SetTile(this, Index, pTile);
}

D3DAPI HRESULT IDirect3DDevice8_SetTile(
    LPDIRECT3DDEVICE8 pThis, DWORD Index, CONST D3DTILE* pTile) 
{
    return pThis->SetTile(Index, pTile);
}

D3DAPI HRESULT D3DDevice::SetTexture(
    DWORD Stage, LPDIRECT3DBASETEXTURE8 pTexture) 
{
    return Direct3DDevice8_SetTexture(this, Stage, pTexture);
}

D3DAPI HRESULT IDirect3DDevice8_SetTexture(
    LPDIRECT3DDEVICE8 pThis, DWORD Stage, LPDIRECT3DBASETEXTURE8 pTexture) 
{
    return pThis->SetTexture(Stage, pTexture);
}

D3DAPI HRESULT D3DDevice::InsertCallback(D3DCALLBACKTYPE Type, 
                                         D3DCALLBACK pCallback, 
                                         DWORD Context) 
{
    return Direct3DDevice8_InsertCallback(this, Type, pCallback, Context);
}

D3DAPI HRESULT IDirect3DDevice8_InsertCallback(
    LPDIRECT3DDEVICE8 pThis, D3DCALLBACKTYPE Type, D3DCALLBACK pCallback, 
    DWORD Context) 
{
    return pThis->InsertCallback(Type, pCallback, Context);
}

D3DAPI HRESULT D3DDevice::Reset(
    D3DPRESENT_PARAMETERS* pPresentationParameters) 
{
    return Direct3DDevice8_Reset(this, pPresentationParameters);
}

D3DAPI HRESULT IDirect3DDevice8_Reset(
    LPDIRECT3DDEVICE8 pThis, D3DPRESENT_PARAMETERS* pPresentationParameters) 
{
    return pThis->Reset(pPresentationParameters);
}

D3DAPI DWORD D3DDevice::InsertFence() {
    return Direct3DDevice8_InsertFence(this);
}

D3DAPI DWORD IDirect3DDevice8_InsertFence(LPDIRECT3DDEVICE8 pThis) {
    return pThis->InsertFence();
}

D3DAPI VOID D3DDevice::BlockOnFence(DWORD Fence) {
    Direct3DDevice8_BlockOnFence(this, Fence);
}

D3DAPI VOID IDirect3DDevice8_BlockOnFence(LPDIRECT3DDEVICE8 pThis, 
                                          DWORD Fence) 
{
    pThis->BlockOnFence(Fence);
}

D3DAPI BOOL D3DDevice::IsFencePending(DWORD Fence) {
    return Direct3DDevice8_IsFencePending(this, Fence);
}

D3DAPI BOOL IDirect3DDevice8_IsFencePending(LPDIRECT3DDEVICE8 pThis, 
                                            DWORD Fence) 
{
    return pThis->IsFencePending(Fence);
}

D3DAPI HRESULT D3DDevice::SetVertexShaderConstant(
    INT Register, CONST VOID* pConstantData, DWORD ConstantCount) 
{
    return Direct3DDevice8_SetVertexShaderConstant(this, Register, 
                                                   pConstantData, 
                                                   ConstantCount);
}

D3DAPI HRESULT IDirect3DDevice8_SetVertexShaderConstant(
    LPDIRECT3DDEVICE8 pThis, INT Register, CONST VOID* pConstantData, 
    DWORD ConstantCount) 
{
    return pThis->SetVertexShaderConstant(Register, pConstantData, 
                                          ConstantCount);
}

D3DAPI HRESULT D3DDevice::SetPixelShaderConstant(
    DWORD Register, CONST VOID* pConstantData, DWORD ConstantCount) 
{
    return Direct3DDevice8_SetPixelShaderConstant(this, Register, 
                                                  pConstantData, 
                                                  ConstantCount);
}

D3DAPI HRESULT IDirect3DDevice8_SetPixelShaderConstant(
    LPDIRECT3DDEVICE8 pThis, DWORD Register, CONST VOID* pConstantData, 
    DWORD ConstantCount) 
{
    return pThis->SetPixelShaderConstant(Register, pConstantData, 
                                         ConstantCount);
}

D3DAPI HRESULT D3DDevice::GetPushBuffer(LPDIRECT3DPUSHBUFFER8* ppPushBuffer) {
    return Direct3DDevice8_GetPushBuffer(this, ppPushBuffer);
}

D3DAPI HRESULT IDirect3DDevice8_GetPushBuffer(
    LPDIRECT3DDEVICE8 pThis, LPDIRECT3DPUSHBUFFER8* ppPushBuffer)
{
    return pThis->GetPushBuffer(ppPushBuffer);
}

D3DAPI VOID D3DDevice::KickPushBuffer() {
    return Direct3DDevice8_KickPushBuffer(this);
}

D3DAPI VOID IDirect3DDevice8_KickPushBuffer(LPDIRECT3DDEVICE8 pThis) {
    pThis->KickPushBuffer();
}

