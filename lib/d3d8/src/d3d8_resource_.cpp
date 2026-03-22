// Even though the actual bodies of these member functions are all implemented
// on the C side (D3DResource_*/D3DSurface_*/etc.), and the vtables are 
// manually set up on the C side, we have to define them in C++ too to prevent 
// abstract base class compile errors.
//
// For now, these functions should do nothing besides call the appropriate 
// implementation function. In theory, they should maybe even throw an 
// assertion in debug mode, since they should never actually be called 
// (constructing the vtables in C will overwrite them.) 
//
// The C wrapper functions (IDirect3D*) are defined to allow for 
// ergonomic calls to the vtable functions from C and C++. They should always
// call the vtable's funnction (pThis->*), not the implementation function 
// so that if the vtable function is overriden, that override is honored.
//
// This file's name must have the trailing underscore (or some name that 
// doesn't conflict with d3d8_resource.c) to prevent compilation of one file
// overriding the object file of the other source file (i.e. d3d8_resource.c 
// will get compiled to d3d8_resource.obj, but so would d3d8_resource.cpp).

#include "d3d8_resource.h"

// ============================================================================
D3DAPI ULONG D3DResource::AddRef() {
    return D3DResource_AddRef(this);
}

D3DEXTERN D3DAPI ULONG IDirect3DResource8_AddRef(LPDIRECT3DRESOURCE8 pThis) {
    return pThis->AddRef();
}

D3DAPI ULONG D3DResource::Release() {
    return D3DResource_Release(this);
}

D3DEXTERN D3DAPI ULONG IDirect3DResource8_Release(LPDIRECT3DRESOURCE8 pThis) {
    return pThis->Release();
}

D3DAPI D3DRESOURCETYPE D3DResource::GetType() {
    return D3DResource_GetType(this);
}

D3DEXTERN D3DAPI D3DRESOURCETYPE IDirect3DResource8_GetType(
    LPDIRECT3DRESOURCE8 pThis) 
{
    return pThis->GetType();
}

D3DAPI VOID D3DResource::Register(PVOID pBase) {
    D3DResource_Register(this, pBase);
}

D3DEXTERN D3DAPI VOID IDirect3DResource8_Register(LPDIRECT3DRESOURCE8 pThis, 
                                                  PVOID pBase) 
{
    pThis->Register(pBase);
}

D3DAPI BOOL D3DResource::IsBusy() {
    return D3DResource_IsBusy(this);
}

D3DEXTERN D3DAPI BOOL IDirect3DResource8_IsBusy(LPDIRECT3DRESOURCE8 pThis) {
    return pThis->IsBusy();
}

D3DAPI VOID D3DResource::BlockUntilNotBusy() {
    D3DResource_BlockUntilNotBusy(this);
}

D3DEXTERN D3DAPI VOID IDirect3DResource8_BlockUntilNotBusy(
    LPDIRECT3DRESOURCE8 pThis) 
{
    pThis->BlockUntilNotBusy();
}
// ============================================================================

// ============================================================================
D3DAPI ULONG D3DPushBuffer::AddRef() {
    return D3DPushBuffer_AddRef(this);
}

D3DEXTERN D3DAPI ULONG IDirect3DPushBuffer8_AddRef(
    LPDIRECT3DPUSHBUFFER8 pThis) 
{
    return pThis->AddRef();
}

D3DAPI ULONG D3DPushBuffer::Release() {
    return D3DPushBuffer_Release(this);
}

D3DEXTERN D3DAPI ULONG IDirect3DPushBuffer8_Release(
    LPDIRECT3DPUSHBUFFER8 pThis) 
{
    return pThis->Release();
}

D3DAPI D3DRESOURCETYPE D3DPushBuffer::GetType() {
    return D3DPushBuffer_GetType(this);
}

D3DEXTERN D3DAPI D3DRESOURCETYPE IDirect3DPushBuffer8_GetType(
    LPDIRECT3DPUSHBUFFER8 pThis) 
{
    return pThis->GetType();
}

D3DAPI VOID D3DPushBuffer::Register(PVOID pBase) {
    D3DPushBuffer_Register(this, pBase);
}

D3DEXTERN D3DAPI VOID IDirect3DPushBuffer8_Register(
    LPDIRECT3DPUSHBUFFER8 pThis, PVOID pBase) 
{
    pThis->Register(pBase);
}

D3DAPI BOOL D3DPushBuffer::IsBusy() {
    return D3DPushBuffer_IsBusy(this);
}

D3DEXTERN D3DAPI BOOL IDirect3DPushBuffer8_IsBusy(
    LPDIRECT3DPUSHBUFFER8 pThis) 
{
    return pThis->IsBusy();
}

D3DAPI VOID D3DPushBuffer::BlockUntilNotBusy() {
    D3DPushBuffer_BlockUntilNotBusy(this);
}

D3DEXTERN D3DAPI VOID IDirect3DPushBuffer8_BlockUntilNotBusy(
    LPDIRECT3DPUSHBUFFER8 pThis) 
{
    pThis->BlockUntilNotBusy();
}

D3DAPI HRESULT D3DPushBuffer::GetSize(UINT* pSize) {
    return D3DPushBuffer_GetSize(this, pSize);
}

D3DEXTERN D3DAPI HRESULT IDirect3DPushBuffer8_GetSize(
    LPDIRECT3DPUSHBUFFER8 pThis, UINT* pSize) 
{
    return pThis->GetSize(pSize);
}

D3DAPI HRESULT D3DPushBuffer::GetData(CONST DWORD** ppData) {
    return D3DPushBuffer_GetData(this, ppData);
}

D3DEXTERN D3DAPI HRESULT IDirect3DPushBuffer8_GetData(
    LPDIRECT3DPUSHBUFFER8 pThis, CONST DWORD** ppData) 
{
    return pThis->GetData(ppData);
}
// ============================================================================

// ============================================================================
D3DAPI ULONG D3DSurface::AddRef() {
    return D3DSurface_AddRef(this);
}

D3DEXTERN D3DAPI ULONG IDirect3DSurface8_AddRef(LPDIRECT3DSURFACE8 pThis) {
    return pThis->AddRef();
}

D3DAPI ULONG D3DSurface::Release() {
    return D3DSurface_Release(this);
}

D3DEXTERN D3DAPI ULONG IDirect3DSurface8_Release(LPDIRECT3DSURFACE8 pThis) {
    return pThis->Release();
}

D3DAPI D3DRESOURCETYPE D3DSurface::GetType() {
    return D3DSurface_GetType(this);
}

D3DEXTERN D3DAPI D3DRESOURCETYPE IDirect3DSurface8_GetType(
    LPDIRECT3DSURFACE8 pThis) 
{
    return pThis->GetType();
}

D3DAPI VOID D3DSurface::Register(PVOID pBase) {
    D3DSurface_Register(this, pBase);
}

D3DEXTERN D3DAPI VOID IDirect3DSurface8_Register(
    LPDIRECT3DSURFACE8 pThis, PVOID pBase) 
{
    pThis->Register(pBase);
}

D3DAPI VOID D3DSurface::BlockUntilNotBusy() {
    D3DSurface_BlockUntilNotBusy(this);
}

D3DEXTERN D3DAPI VOID IDirect3DSurface8_BlockUntilNotBusy(
    LPDIRECT3DSURFACE8 pThis) 
{
    pThis->BlockUntilNotBusy();
}

D3DAPI BOOL D3DSurface::IsBusy() {
    return D3DSurface_IsBusy(this);
}

D3DEXTERN D3DAPI BOOL IDirect3DSurface8_IsBusy(LPDIRECT3DSURFACE8 pThis) {
    return pThis->IsBusy();
}

D3DAPI HRESULT D3DSurface::GetContainer(
    LPDIRECT3DBASETEXTURE8* ppContainer, REFIID riid)
{
    return D3DSurface_GetContainer(this, ppContainer, riid);
}

D3DEXTERN D3DAPI HRESULT IDirect3DSurface8_GetContainer(
    LPDIRECT3DSURFACE8 pThis, 
    LPDIRECT3DBASETEXTURE8* ppContainer,
    REFIID riid)
{
    return pThis->GetContainer(ppContainer, riid);
}

D3DAPI HRESULT D3DSurface::GetDesc(D3DSURFACE_DESC* pDesc)
{
    return D3DSurface_GetDesc(this, pDesc);
}

D3DEXTERN D3DAPI HRESULT IDirect3DSurface8_GetDesc(
    LPDIRECT3DSURFACE8 pThis, D3DSURFACE_DESC* pDesc)
{
    return pThis->GetDesc(pDesc);
}

D3DAPI HRESULT D3DSurface::LockRect(
    D3DLOCKED_RECT* pLockedRect, CONST RECT* pRect, DWORD Flags)
{
    return D3DSurface_LockRect(this, pLockedRect, pRect, Flags);
}

D3DEXTERN D3DAPI HRESULT IDirect3DSurface8_LockRect(
    LPDIRECT3DSURFACE8 pThis, 
    D3DLOCKED_RECT* pLockedRect, CONST RECT* pRect, DWORD Flags)
{
    return pThis->LockRect(pLockedRect, pRect, Flags);
}

D3DAPI HRESULT D3DSurface::UnlockRect() {
    return D3DSurface_UnlockRect(this);
}

D3DEXTERN D3DAPI HRESULT IDirect3DSurface8_UnlockRect(
    LPDIRECT3DSURFACE8 pThis) 
{
    return pThis->UnlockRect();
}
// ============================================================================

// ============================================================================
D3DAPI ULONG D3DVertexBuffer::AddRef() {
    return D3DVertexBuffer_AddRef(this);
}

D3DEXTERN D3DAPI ULONG IDirect3DVertexBuffer8_AddRef(
    LPDIRECT3DVERTEXBUFFER8 pThis) 
{
    return pThis->AddRef();
}

D3DAPI ULONG D3DVertexBuffer::Release() {
    return D3DVertexBuffer_Release(this);
}

D3DEXTERN D3DAPI ULONG IDirect3DVertexBuffer8_Release(
    LPDIRECT3DVERTEXBUFFER8 pThis) 
{
    return pThis->Release();
}

D3DAPI D3DRESOURCETYPE D3DVertexBuffer::GetType() {
    return D3DVertexBuffer_GetType(this);
}

D3DEXTERN D3DAPI D3DRESOURCETYPE IDirect3DVertexBuffer8_GetType(
    LPDIRECT3DVERTEXBUFFER8 pThis) 
{
    return pThis->GetType();
}

D3DAPI VOID D3DVertexBuffer::Register(PVOID pBase) {
    D3DVertexBuffer_Register(this, pBase);
}

D3DEXTERN D3DAPI VOID IDirect3DVertexBuffer8_Register(
    LPDIRECT3DVERTEXBUFFER8 pThis, PVOID pBase) 
{
    pThis->Register(pBase);
}

D3DAPI BOOL D3DVertexBuffer::IsBusy() {
    return D3DVertexBuffer_IsBusy(this);
}

D3DEXTERN D3DAPI BOOL IDirect3DVertexBuffer8_IsBusy(
    LPDIRECT3DVERTEXBUFFER8 pThis) 
{
    return pThis->IsBusy();
}

D3DAPI VOID D3DVertexBuffer::BlockUntilNotBusy() {
    D3DVertexBuffer_BlockUntilNotBusy(this);
}

D3DEXTERN D3DAPI VOID IDirect3DVertexBuffer8_BlockUntilNotBusy(
    LPDIRECT3DVERTEXBUFFER8 pThis) 
{
    pThis->BlockUntilNotBusy();
}

D3DAPI HRESULT D3DVertexBuffer::Lock(
        UINT OffsetToLock, UINT SizeToLock, 
        BYTE** ppbData, DWORD Flags) 
{
    return D3DVertexBuffer_Lock(
        this, OffsetToLock, SizeToLock, ppbData, Flags);
}

D3DEXTERN D3DAPI HRESULT IDirect3DVertexBuffer8_Lock(
    LPDIRECT3DVERTEXBUFFER8 pThis, UINT OffsetToLock, UINT SizeToLock, 
    BYTE** ppbData, DWORD Flags) 
{
    return pThis->Lock(OffsetToLock, SizeToLock, ppbData, Flags);
}

D3DAPI HRESULT D3DVertexBuffer::Unlock() {
    return D3DVertexBuffer_Unlock(this);
}

D3DEXTERN D3DAPI HRESULT IDirect3DVertexBuffer8_Unlock(
    LPDIRECT3DVERTEXBUFFER8 pThis) 
{
    return pThis->Unlock();
}

D3DAPI HRESULT D3DVertexBuffer::GetDesc(
        D3DVERTEXBUFFER_DESC* pDesc) 
{
    return D3DVertexBuffer_GetDesc(this, pDesc);
}

D3DEXTERN D3DAPI HRESULT IDirect3DVertexBuffer8_GetDesc(
    LPDIRECT3DVERTEXBUFFER8 pThis, D3DVERTEXBUFFER_DESC* pDesc) 
{
    return pThis->GetDesc(pDesc);
}
// ============================================================================

// ============================================================================
D3DAPI ULONG D3DBaseTexture::AddRef() {
    return D3DBaseTexture_AddRef(this);
}

D3DEXTERN D3DAPI ULONG IDirect3DBaseTexture8_AddRef(
    LPDIRECT3DBASETEXTURE8 pThis) 
{
    return pThis->AddRef();
}

D3DAPI ULONG D3DBaseTexture::Release() {
    return D3DBaseTexture_Release(this);
}

D3DEXTERN D3DAPI ULONG IDirect3DBaseTexture8_Release(
    LPDIRECT3DBASETEXTURE8 pThis) 
{
    return pThis->Release();
}

D3DAPI D3DRESOURCETYPE D3DBaseTexture::GetType() {
    return D3DBaseTexture_GetType(this);
}

D3DEXTERN D3DAPI D3DRESOURCETYPE IDirect3DBaseTexture8_GetType(
    LPDIRECT3DBASETEXTURE8 pThis) 
{
    return pThis->GetType();
}

D3DAPI VOID D3DBaseTexture::Register(PVOID pBase) {
    D3DBaseTexture_Register(this, pBase);
}

D3DEXTERN D3DAPI VOID IDirect3DBaseTexture8_Register(
    LPDIRECT3DBASETEXTURE8 pThis, PVOID pBase) 
{
    pThis->Register(pBase);
}

D3DAPI VOID D3DBaseTexture::BlockUntilNotBusy() {
    D3DBaseTexture_BlockUntilNotBusy(this);
}

D3DEXTERN D3DAPI VOID IDirect3DBaseTexture8_BlockUntilNotBusy(
    LPDIRECT3DBASETEXTURE8 pThis) 
{
    pThis->BlockUntilNotBusy();
}

D3DAPI BOOL D3DBaseTexture::IsBusy() {
    return D3DBaseTexture_IsBusy(this);
}

D3DEXTERN D3DAPI BOOL IDirect3DBaseTexture8_IsBusy(
    LPDIRECT3DBASETEXTURE8 pThis) 
{
    return pThis->IsBusy();
}

D3DAPI DWORD D3DBaseTexture::GetLevelCount() {
    return D3DBaseTexture_GetLevelCount(this);
}

D3DEXTERN D3DAPI DWORD IDirect3DBaseTexture8_GetLevelCount(
    LPDIRECT3DBASETEXTURE8 pThis) 
{
    return pThis->GetLevelCount();
}
// ============================================================================

// ============================================================================
D3DAPI ULONG D3DTexture::AddRef() {
    return D3DTexture_AddRef(this);
}

D3DEXTERN D3DAPI ULONG IDirect3DTexture8_AddRef(LPDIRECT3DTEXTURE8 pThis) {
    return pThis->AddRef();
}

D3DAPI ULONG D3DTexture::Release() {
    return D3DTexture_Release(this);
}

D3DEXTERN D3DAPI ULONG IDirect3DTexture8_Release(LPDIRECT3DTEXTURE8 pThis) {
    return pThis->Release();
}

D3DAPI D3DRESOURCETYPE D3DTexture::GetType() {
    return D3DTexture_GetType(this);
}

D3DEXTERN D3DAPI D3DRESOURCETYPE IDirect3DTexture8_GetType(
    LPDIRECT3DTEXTURE8 pThis) 
{
    return pThis->GetType();
}

D3DAPI VOID D3DTexture::Register(PVOID pBase) {
    D3DTexture_Register(this, pBase);
}

D3DEXTERN D3DAPI VOID IDirect3DTexture8_Register(
    LPDIRECT3DTEXTURE8 pThis, PVOID pBase) 
{
    pThis->Register(pBase);
}

D3DAPI VOID D3DTexture::BlockUntilNotBusy() {
    D3DTexture_BlockUntilNotBusy(this);
}

D3DEXTERN D3DAPI VOID IDirect3DTexture8_BlockUntilNotBusy(
    LPDIRECT3DTEXTURE8 pThis) 
{
    pThis->BlockUntilNotBusy();
}

D3DAPI BOOL D3DTexture::IsBusy() {
    return D3DTexture_IsBusy(this);
}

D3DEXTERN D3DAPI BOOL IDirect3DTexture8_IsBusy(LPDIRECT3DTEXTURE8 pThis) {
    return pThis->IsBusy();
}

D3DAPI DWORD D3DTexture::GetLevelCount() {
    return D3DTexture_GetLevelCount(this);
}

D3DEXTERN D3DAPI DWORD IDirect3DTexture8_GetLevelCount(
    LPDIRECT3DTEXTURE8 pThis) 
{
    return pThis->GetLevelCount();
}

D3DAPI HRESULT D3DTexture::GetLevelDesc(
        UINT Level, D3DSURFACE_DESC* pDesc) {
    return D3DTexture_GetLevelDesc(this, Level, pDesc);
}

D3DEXTERN D3DAPI HRESULT IDirect3DTexture8_GetLevelDesc(
    LPDIRECT3DTEXTURE8 pThis, UINT Level, D3DSURFACE_DESC* pDesc) 
{
    return pThis->GetLevelDesc(Level, pDesc);
}

D3DAPI HRESULT D3DTexture::GetSurfaceLevel(
        UINT Level, LPDIRECT3DSURFACE8* ppSurfaceLevel) {
    return D3DTexture_GetSurfaceLevel(this, Level, ppSurfaceLevel);
}

D3DEXTERN D3DAPI HRESULT IDirect3DTexture8_GetSurfaceLevel(
    LPDIRECT3DTEXTURE8 pThis, UINT Level, LPDIRECT3DSURFACE8* ppSurfaceLevel) 
{
    return pThis->GetSurfaceLevel(Level, ppSurfaceLevel);
}

D3DAPI HRESULT D3DTexture::LockRect(
        UINT Level, D3DLOCKED_RECT* pLockedRect, 
        CONST RECT* pRect, DWORD Flags) {
    return D3DTexture_LockRect(this, Level, pLockedRect, pRect, Flags);
}

D3DEXTERN D3DAPI HRESULT IDirect3DTexture8_LockRect(
    LPDIRECT3DTEXTURE8 pThis, UINT Level, D3DLOCKED_RECT* pLockedRect, 
    CONST RECT* pRect, DWORD Flags) 
{
    return pThis->LockRect(Level, pLockedRect, pRect, Flags);
}

D3DAPI HRESULT D3DTexture::UnlockRect(UINT Level) {
    return D3DTexture_UnlockRect(this, Level);
}

D3DEXTERN D3DAPI HRESULT IDirect3DTexture8_UnlockRect(
    LPDIRECT3DTEXTURE8 pThis, UINT Level) 
{
    return pThis->UnlockRect(Level);
}
// ============================================================================

// ============================================================================
D3DAPI ULONG D3DCubeTexture::AddRef() {
    return D3DCubeTexture_AddRef(this);
}

D3DEXTERN D3DAPI ULONG IDirect3DCubeTexture8_AddRef(
    LPDIRECT3DCUBETEXTURE8 pThis) 
{
    return pThis->AddRef();
}

D3DAPI ULONG D3DCubeTexture::Release() {
    return D3DCubeTexture_Release(this);
}

D3DEXTERN D3DAPI ULONG IDirect3DCubeTexture8_Release(
    LPDIRECT3DCUBETEXTURE8 pThis) 
{
    return pThis->Release();
}

D3DAPI D3DRESOURCETYPE D3DCubeTexture::GetType() {
    return D3DCubeTexture_GetType(this);
}

D3DEXTERN D3DAPI D3DRESOURCETYPE IDirect3DCubeTexture8_GetType(
    LPDIRECT3DCUBETEXTURE8 pThis) 
{
    return pThis->GetType();
}

D3DAPI VOID D3DCubeTexture::Register(PVOID pBase) {
    D3DCubeTexture_Register(this, pBase);
}

D3DEXTERN D3DAPI VOID IDirect3DCubeTexture8_Register(
    LPDIRECT3DCUBETEXTURE8 pThis, PVOID pBase) 
{
    pThis->Register(pBase);
}

D3DAPI VOID D3DCubeTexture::BlockUntilNotBusy() {
    D3DCubeTexture_BlockUntilNotBusy(this);
}

D3DEXTERN D3DAPI VOID IDirect3DCubeTexture8_BlockUntilNotBusy(
    LPDIRECT3DCUBETEXTURE8 pThis) 
{
    pThis->BlockUntilNotBusy();
}

D3DAPI BOOL D3DCubeTexture::IsBusy() {
    return D3DCubeTexture_IsBusy(this);
}

D3DEXTERN D3DAPI BOOL IDirect3DCubeTexture8_IsBusy(
    LPDIRECT3DCUBETEXTURE8 pThis) 
{
    return pThis->IsBusy();
}

D3DAPI DWORD D3DCubeTexture::GetLevelCount() {
    return D3DCubeTexture_GetLevelCount(this);
}

D3DEXTERN D3DAPI DWORD IDirect3DCubeTexture8_GetLevelCount(
    LPDIRECT3DCUBETEXTURE8 pThis) 
{
    return pThis->GetLevelCount();
}

D3DAPI HRESULT D3DCubeTexture::GetLevelDesc(
        UINT Level, D3DSURFACE_DESC* pDesc) {
    return D3DCubeTexture_GetLevelDesc(this, Level, pDesc);
}

D3DEXTERN D3DAPI HRESULT IDirect3DCubeTexture8_GetLevelDesc(
    LPDIRECT3DCUBETEXTURE8 pThis, UINT Level, D3DSURFACE_DESC* pDesc) 
{
    return pThis->GetLevelDesc(Level, pDesc);
}

D3DAPI HRESULT D3DCubeTexture::LockRect(
        D3DCUBEMAP_FACES FaceType, 
        UINT Level, D3DLOCKED_RECT* pLockedRect, 
        CONST RECT* pRect, DWORD Flags) 
{
    return D3DCubeTexture_LockRect(
        this, FaceType, Level, pLockedRect, pRect, Flags);
}

D3DEXTERN D3DAPI HRESULT IDirect3DCubeTexture8_LockRect(
    LPDIRECT3DCUBETEXTURE8 pThis, D3DCUBEMAP_FACES FaceType, UINT Level, 
    D3DLOCKED_RECT* pLockedRect, CONST RECT* pRect, DWORD Flags) 
{
    return pThis->LockRect(FaceType, Level, pLockedRect, pRect, Flags);
}

D3DAPI HRESULT D3DCubeTexture::UnlockRect(
    D3DCUBEMAP_FACES FaceType, UINT Level) 
{
    return D3DCubeTexture_UnlockRect(this, FaceType, Level);
}

D3DEXTERN D3DAPI HRESULT IDirect3DCubeTexture8_UnlockRect(
    LPDIRECT3DCUBETEXTURE8 pThis, D3DCUBEMAP_FACES FaceType, UINT Level) 
{
    return pThis->UnlockRect(FaceType, Level);
}

D3DAPI HRESULT D3DCubeTexture::GetCubeMapSurface(
    D3DCUBEMAP_FACES FaceType, UINT Level, 
    LPDIRECT3DSURFACE8* ppCubeMapSurface) 
{
    return D3DCubeTexture_GetCubeMapSurface(
        this, FaceType, Level, ppCubeMapSurface);
}

D3DEXTERN D3DAPI HRESULT IDirect3DCubeTexture8_GetCubeMapSurface(
    LPDIRECT3DCUBETEXTURE8 pThis, D3DCUBEMAP_FACES FaceType, UINT Level, 
    LPDIRECT3DSURFACE8* ppCubeMapSurface) 
{
    return pThis->GetCubeMapSurface(FaceType, Level, ppCubeMapSurface);
}
// ============================================================================
