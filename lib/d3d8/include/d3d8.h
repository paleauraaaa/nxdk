// This file contains the public interfaces to the D3D8 implementation, and is
// the only file that need be included by external code. The headers in src/
// explicitly should not be included.

#pragma once

#include <windows.h>
#include <winnt.h>

#include "d3d8types.h"

#ifndef DIRECT3D_VERSION
#define DIRECT3D_VERSION 0x0800
#endif // DIRECT3D_VERSION

// TODO: set this in build process
#ifndef NXDK_DEBUG
#define NXDK_DEBUG 1
#endif // NXDK_DEBUG

#ifdef __cplusplus
#define INHERITS(base)   : public base
#define IMPLEMENTS(base) : public base
#else
#define INHERITS(base)
#define IMPLEMENTS(base)
#endif // __cplusplus

#define STDMETHODCALLTYPE __stdcall
#define D3DAPI STDMETHODCALLTYPE

#ifdef __cplusplus
#   define D3DEXTERN extern "C"
#else
#   define D3DEXTERN extern
#endif // __cplusplus

#ifdef __cplusplus
#define D3DINTERFACE class __declspec(novtable)
#else
#define D3DINTERFACE struct
#endif // __cplusplus

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

struct IUnknown;
typedef struct IUnknown IUnknown, *LPUNKNOWN;

typedef struct UnknownVtbl {
    ULONG (D3DAPI *AddRef) (LPUNKNOWN pThis);
    ULONG (D3DAPI *Release)(LPUNKNOWN pThis);
} UnknownVtbl, *LPUNKNOWNVTBL;

D3DINTERFACE IUnknown {
#ifndef __cplusplus
    LPUNKNOWNVTBL lpVtbl;
#else
public:
    virtual D3DAPI ULONG AddRef()  = 0;
    virtual D3DAPI ULONG Release() = 0;
#endif // __cplusplus
};

// ============================================================================
struct IDirect3DResource8;
typedef struct IDirect3DResource8 IDirect3DResource8, *LPDIRECT3DRESOURCE8;

#ifndef __cplusplus
typedef struct IDirect3DResourceVtbl8 {
    /*** IUnknown methods ***/
    ULONG (D3DAPI *AddRef) (LPDIRECT3DRESOURCE8 pThis);
    ULONG (D3DAPI *Release)(LPDIRECT3DRESOURCE8 pThis);

    /*** IDirect3DResource8 methods ***/
    D3DRESOURCETYPE (D3DAPI *GetType)(LPDIRECT3DRESOURCE8 pThis);
    VOID (D3DAPI *Register)(LPDIRECT3DRESOURCE8 pThis, PVOID pBase);
    VOID (D3DAPI *BlockUntilNotBusy)(LPDIRECT3DRESOURCE8 pThis);
    BOOL (D3DAPI *IsBusy)(LPDIRECT3DRESOURCE8 pThis);
} IDirect3DResourceVtbl8, *LPDIRECT3DRESOURCEVTBL8;
#endif // __cplusplus

D3DINTERFACE IDirect3DResource8 INHERITS(IUnknown) {
#ifndef __cplusplus
    LPDIRECT3DRESOURCEVTBL8 lpVtbl;
#else
public:
    virtual D3DAPI D3DRESOURCETYPE GetType() = 0;
    virtual D3DAPI VOID            Register(PVOID pBase) = 0;
    virtual D3DAPI VOID            BlockUntilNotBusy() = 0;
    virtual D3DAPI BOOL            IsBusy() = 0;
#endif // __cplusplus
};

D3DAPI ULONG IDirect3DResource8_AddRef(LPDIRECT3DRESOURCE8 pThis);
D3DAPI ULONG IDirect3DResource8_Release(LPDIRECT3DRESOURCE8 pThis);
D3DAPI D3DRESOURCETYPE IDirect3DResource8_GetType(LPDIRECT3DRESOURCE8 pThis);
D3DAPI VOID IDirect3DResource8_Register(LPDIRECT3DRESOURCE8 pThis,
                                        PVOID pBase);
D3DAPI BOOL IDirect3DResource8_IsBusy(LPDIRECT3DRESOURCE8 pThis);
D3DAPI VOID IDirect3DResource8_BlockUntilNotBusy(LPDIRECT3DRESOURCE8 pThis);
// ============================================================================

// ============================================================================
struct IDirect3DBaseTexture8;
typedef struct IDirect3DBaseTexture8 IDirect3DBaseTexture8,
                                     *LPDIRECT3DBASETEXTURE8;

#ifndef __cplusplus
typedef struct IDirect3DBaseTextureVtbl8 {
    /*** IUnknown methods ***/
    ULONG   (D3DAPI *AddRef)(LPDIRECT3DBASETEXTURE8 pThis);
    ULONG   (D3DAPI *Release)(LPDIRECT3DBASETEXTURE8 pThis);

    /*** IDirect3DResource8 methods ***/
    D3DRESOURCETYPE (D3DAPI *GetType)(LPDIRECT3DBASETEXTURE8 pThis);
    VOID (D3DAPI *Register)(LPDIRECT3DBASETEXTURE8 pThis, PVOID pBase);
    VOID (D3DAPI *BlockUntilNotBusy)(LPDIRECT3DBASETEXTURE8 pThis);
    BOOL (D3DAPI *IsBusy)(LPDIRECT3DBASETEXTURE8 pThis);

    /*** IDirect3DBaseTexture8 methods ***/
    DWORD (D3DAPI *GetLevelCount)(LPDIRECT3DBASETEXTURE8 pThis);
} IDirect3DBaseTextureVtbl8, *LPDIRECT3DBASETEXTUREVTBL8;
#endif // __cplusplus

D3DINTERFACE IDirect3DBaseTexture8 INHERITS(IDirect3DResource8) {
#ifndef __cplusplus
    LPDIRECT3DBASETEXTUREVTBL8 lpVtbl;
#else
public:
    virtual D3DAPI DWORD GetLevelCount() = 0;
#endif // __cplusplus
};

D3DAPI ULONG IDirect3DBaseTexture8_AddRef(LPDIRECT3DBASETEXTURE8 pThis);
D3DAPI ULONG IDirect3DBaseTexture8_Release(LPDIRECT3DBASETEXTURE8 pThis);
D3DAPI D3DRESOURCETYPE IDirect3DBaseTexture8_GetType(
    LPDIRECT3DBASETEXTURE8 pThis);
D3DAPI VOID IDirect3DBaseTexture8_Register(LPDIRECT3DBASETEXTURE8 pThis,
                                           PVOID pBase);
D3DAPI BOOL IDirect3DBaseTexture8_IsBusy(LPDIRECT3DBASETEXTURE8 pThis);
D3DAPI VOID IDirect3DBaseTexture8_BlockUntilNotBusy(
    LPDIRECT3DBASETEXTURE8 pThis);
D3DAPI DWORD IDirect3DBaseTexture8_GetLevelCount(LPDIRECT3DBASETEXTURE8 pThis);
// ============================================================================

// ============================================================================
struct IDirect3DTexture8;
typedef struct IDirect3DTexture8 IDirect3DTexture8,
                                 *LPDIRECT3DTEXTURE8;

#ifndef __cplusplus
typedef struct IDirect3DTextureVtbl8 {
    /*** IUnknown methods ***/
    ULONG   (D3DAPI *AddRef)(LPDIRECT3DTEXTURE8 pThis);
    ULONG   (D3DAPI *Release)(LPDIRECT3DTEXTURE8 pThis);

    /*** IDirect3DResource8 methods ***/
    D3DRESOURCETYPE (D3DAPI *GetType)(LPDIRECT3DTEXTURE8 pThis);
    VOID   (D3DAPI *Register)(LPDIRECT3DTEXTURE8 pThis, PVOID pBase);
    VOID   (D3DAPI *BlockUntilNotBusy)(LPDIRECT3DTEXTURE8 pThis);
    BOOL   (D3DAPI *IsBusy)(LPDIRECT3DTEXTURE8 pThis);

    /*** IDirect3DBaseTexture8 methods ***/
    DWORD   (D3DAPI *GetLevelCount)(LPDIRECT3DTEXTURE8 pThis);

    /*** IDirect3DTexture8 methods ***/
    HRESULT (D3DAPI *GetLevelDesc)(LPDIRECT3DTEXTURE8 pThis,
                                   UINT Level,
                                   D3DSURFACE_DESC* pDesc);
    HRESULT (D3DAPI *GetSurfaceLevel)(LPDIRECT3DTEXTURE8 pThis,
                                      UINT Level,
                                      LPDIRECT3DSURFACE8* ppSurfaceLevel);
    HRESULT (D3DAPI *LockRect)(LPDIRECT3DTEXTURE8 pThis,
                               UINT Level,
                               D3DLOCKED_RECT* pLockedRect,
                               const RECT* pRect,
                               DWORD Flags);
    HRESULT (D3DAPI *UnlockRect)(LPDIRECT3DTEXTURE8 pThis,
                                 UINT Level);
} IDirect3DTextureVtbl8, *LPDIRECT3DTEXTUREVTBL8;
#endif // __cplusplus

D3DINTERFACE IDirect3DTexture8 INHERITS(IDirect3DBaseTexture8) {
#ifndef __cplusplus
    LPDIRECT3DTEXTUREVTBL8 lpVtbl;
#else
public:
    virtual D3DAPI HRESULT GetLevelDesc(UINT Level,
                                                   D3DSURFACE_DESC* pDesc) = 0;
    virtual D3DAPI HRESULT GetSurfaceLevel(UINT Level,
                                    LPDIRECT3DSURFACE8* ppSurfaceLevel) = 0;
    virtual D3DAPI HRESULT LockRect(UINT Level,
                                               D3DLOCKED_RECT* pLockedRect,
                                               const RECT* pRect,
                                               DWORD Flags) = 0;
    virtual D3DAPI HRESULT UnlockRect(UINT Level) = 0;
#endif // __cplusplus
};

D3DAPI ULONG   IDirect3DTexture8_AddRef(LPDIRECT3DTEXTURE8 pThis);
D3DAPI ULONG   IDirect3DTexture8_Release(LPDIRECT3DTEXTURE8 pThis);
D3DAPI D3DRESOURCETYPE IDirect3DTexture8_GetType(LPDIRECT3DTEXTURE8 pThis);
D3DAPI VOID    IDirect3DTexture8_Register(LPDIRECT3DTEXTURE8 pThis,
                                          PVOID pBase);
D3DAPI BOOL    IDirect3DTexture8_IsBusy(LPDIRECT3DTEXTURE8 pThis);
D3DAPI VOID    IDirect3DTexture8_BlockUntilNotBusy(LPDIRECT3DTEXTURE8 pThis);
D3DAPI DWORD   IDirect3DTexture8_GetLevelCount(LPDIRECT3DTEXTURE8 pThis);
D3DAPI HRESULT IDirect3DTexture8_GetLevelDesc(LPDIRECT3DTEXTURE8 pThis,
                                              UINT Level,
                                              D3DSURFACE_DESC* pDesc);
D3DAPI HRESULT IDirect3DTexture8_GetSurfaceLevel(
    LPDIRECT3DTEXTURE8 pThis, UINT Level, LPDIRECT3DSURFACE8* ppSurfaceLevel);
D3DAPI HRESULT IDirect3DTexture8_LockRect(LPDIRECT3DTEXTURE8 pThis,
                                          UINT Level,
                                          D3DLOCKED_RECT* pLockedRect,
                                          const RECT* pRect,
                                          DWORD Flags);
D3DAPI HRESULT IDirect3DTexture8_UnlockRect(LPDIRECT3DTEXTURE8 pThis,
                                            UINT Level);
// ============================================================================

// ============================================================================
D3DINTERFACE IDirect3DCubeTexture8;
typedef D3DINTERFACE IDirect3DCubeTexture8 IDirect3DCubeTexture8,
                                           *LPDIRECT3DCUBETEXTURE8;

#ifndef __cplusplus
typedef struct IDirect3DCubeTextureVtbl8 {
    /*** IUnknown methods ***/
    ULONG   (D3DAPI *AddRef)(LPDIRECT3DCUBETEXTURE8 pThis);
    ULONG   (D3DAPI *Release)(LPDIRECT3DCUBETEXTURE8 pThis);

    /*** IDirect3DResource8 methods ***/
    D3DRESOURCETYPE (D3DAPI *GetType)(LPDIRECT3DCUBETEXTURE8 pThis);
    VOID    (D3DAPI *Register)(LPDIRECT3DCUBETEXTURE8 pThis, PVOID pBase);
    VOID    (D3DAPI *BlockUntilNotBusy)(LPDIRECT3DCUBETEXTURE8 pThis);
    BOOL    (D3DAPI *IsBusy)(LPDIRECT3DCUBETEXTURE8 pThis);

    /*** IDirect3DBaseTexture8 methods ***/
    DWORD   (D3DAPI *GetLevelCount)(LPDIRECT3DCUBETEXTURE8 pThis);

    /*** IDirect3DCubeTexture8 methods ***/
    HRESULT (D3DAPI *GetLevelDesc)(LPDIRECT3DCUBETEXTURE8 pThis,
                                   UINT Level,
                                   D3DSURFACE_DESC* pDesc);
    HRESULT (D3DAPI *LockRect)(LPDIRECT3DCUBETEXTURE8 pThis,
                               D3DCUBEMAP_FACES FaceType,
                               UINT Level,
                               D3DLOCKED_RECT* pLockedRect,
                               const RECT* pRect,
                               DWORD Flags);
    HRESULT (D3DAPI *UnlockRect)(LPDIRECT3DCUBETEXTURE8 pThis,
                                 D3DCUBEMAP_FACES FaceType,
                                 UINT Level);
    HRESULT (D3DAPI *GetCubeMapSurface)(LPDIRECT3DCUBETEXTURE8 pThis,
                                        D3DCUBEMAP_FACES FaceType,
                                        UINT Level,
                                        LPDIRECT3DSURFACE8* ppCubeMapSurface);
} IDirect3DCubeTextureVtbl8, *LPDIRECT3DCUBETEXTUREVTBL8;
#endif // __cplusplus

D3DINTERFACE IDirect3DCubeTexture8 INHERITS(IDirect3DBaseTexture8) {
#ifndef __cplusplus
    LPDIRECT3DCUBETEXTUREVTBL8 lpVtbl;
#else
public:
    virtual D3DAPI HRESULT GetLevelDesc(
        UINT Level, D3DSURFACE_DESC* pDesc) = 0;
    virtual D3DAPI HRESULT LockRect(
        D3DCUBEMAP_FACES FaceType, UINT Level, D3DLOCKED_RECT* pLockedRect,
        const RECT* pRect, DWORD Flags) = 0;
    virtual D3DAPI HRESULT UnlockRect(
        D3DCUBEMAP_FACES FaceType, UINT Level) = 0;
    virtual D3DAPI HRESULT GetCubeMapSurface(
        D3DCUBEMAP_FACES FaceType, UINT Level,
        LPDIRECT3DSURFACE8* ppCubeMapSurface) = 0;
#endif // __cplusplus
};

D3DAPI ULONG IDirect3DCubeTexture8_AddRef(LPDIRECT3DCUBETEXTURE8 pThis);
D3DAPI ULONG IDirect3DCubeTexture8_Release(LPDIRECT3DCUBETEXTURE8 pThis);
D3DAPI D3DRESOURCETYPE IDirect3DCubeTexture8_GetType(
    LPDIRECT3DCUBETEXTURE8 pThis);
D3DAPI VOID IDirect3DCubeTexture8_Register(LPDIRECT3DCUBETEXTURE8 pThis,
                                           PVOID pBase);
D3DAPI BOOL IDirect3DCubeTexture8_IsBusy(LPDIRECT3DCUBETEXTURE8 pThis);
D3DAPI VOID IDirect3DCubeTexture8_BlockUntilNotBusy(
    LPDIRECT3DCUBETEXTURE8 pThis);
D3DAPI DWORD IDirect3DCubeTexture8_GetLevelCount(LPDIRECT3DCUBETEXTURE8 pThis);
D3DAPI HRESULT IDirect3DCubeTexture8_GetLevelDesc(LPDIRECT3DCUBETEXTURE8 pThis,
                                           UINT Level,
                                           D3DSURFACE_DESC* pDesc);
D3DAPI HRESULT IDirect3DCubeTexture8_LockRect(LPDIRECT3DCUBETEXTURE8 pThis,
                                              D3DCUBEMAP_FACES FaceType,
                                              UINT Level,
                                              D3DLOCKED_RECT* pLockedRect,
                                              CONST RECT* pRect, DWORD Flags);
D3DAPI HRESULT IDirect3DCubeTexture8_UnlockRect(LPDIRECT3DCUBETEXTURE8 pThis,
                                                D3DCUBEMAP_FACES FaceType,
                                                UINT Level);
D3DAPI HRESULT IDirect3DCubeTexture8_GetCubeMapSurface(
    LPDIRECT3DCUBETEXTURE8 pThis,
    D3DCUBEMAP_FACES FaceType, UINT Level,
    LPDIRECT3DSURFACE8* ppCubeMapSurface);
// ============================================================================

// ============================================================================
#ifndef __cplusplus
typedef struct IDirect3DSurfaceVtbl8 {
    /*** IUnknown methods ***/
    ULONG   (D3DAPI *AddRef)(LPDIRECT3DSURFACE8 pThis);
    ULONG   (D3DAPI *Release)(LPDIRECT3DSURFACE8 pThis);

    /*** IDirect3DResource8 methods ***/
    D3DRESOURCETYPE (D3DAPI *GetType)(LPDIRECT3DSURFACE8 pThis);
    VOID   (D3DAPI *Register)(LPDIRECT3DSURFACE8 pThis, PVOID pBase);
    VOID   (D3DAPI *BlockUntilNotBusy)(LPDIRECT3DSURFACE8 pThis);
    BOOL   (D3DAPI *IsBusy)(LPDIRECT3DSURFACE8 pThis);

    /*** IDirect3DSurface8 methods ***/
    HRESULT (D3DAPI *GetContainer)(LPDIRECT3DSURFACE8 pThis,
                                   LPDIRECT3DBASETEXTURE8* ppContainer,
                                   REFIID riid);
    HRESULT (D3DAPI *GetDesc) (LPDIRECT3DSURFACE8 pThis,
                                D3DSURFACE_DESC* pDesc);
    HRESULT (D3DAPI *LockRect)(LPDIRECT3DSURFACE8 pThis,
                               D3DLOCKED_RECT* pLockedRect,
                               const RECT* pRect, DWORD Flags);
    HRESULT (D3DAPI *UnlockRect)(LPDIRECT3DSURFACE8 pThis);
} IDirect3DSurfaceVtbl8, *LPDIRECT3DSURFACEVTBL8;
#endif // __cplusplus

D3DINTERFACE IDirect3DSurface8 INHERITS(IDirect3DResource8) {
#ifndef __cplusplus
    LPDIRECT3DSURFACEVTBL8 lpVtbl;
#else
public:
    virtual D3DAPI HRESULT GetContainer(
        LPDIRECT3DBASETEXTURE8* ppContainer, REFIID riid) = 0;
    virtual D3DAPI HRESULT GetDesc(D3DSURFACE_DESC* pDesc) = 0;
    virtual D3DAPI HRESULT LockRect(D3DLOCKED_RECT* pLockedRect,
                             const RECT* pRect, DWORD Flags) = 0;
    virtual D3DAPI HRESULT UnlockRect() = 0;
#endif // __cplusplus
};

D3DAPI ULONG IDirect3DSurface8_AddRef(LPDIRECT3DSURFACE8 pThis);
D3DAPI ULONG IDirect3DSurface8_Release(LPDIRECT3DSURFACE8 pThis);
D3DAPI D3DRESOURCETYPE IDirect3DSurface8_GetType(LPDIRECT3DSURFACE8 pThis);
D3DAPI VOID IDirect3DSurface8_Register(LPDIRECT3DSURFACE8 pThis, PVOID pBase);
D3DAPI BOOL IDirect3DSurface8_IsBusy(LPDIRECT3DSURFACE8 pThis);
D3DAPI VOID IDirect3DSurface8_BlockUntilNotBusy(LPDIRECT3DSURFACE8 pThis);
D3DAPI HRESULT IDirect3DSurface8_GetContainer(
    LPDIRECT3DSURFACE8 pThis,
    LPDIRECT3DBASETEXTURE8* ppContainer,
    REFIID riid);
D3DAPI HRESULT IDirect3DSurface8_GetDesc(LPDIRECT3DSURFACE8 pThis,
                                  D3DSURFACE_DESC* pDesc);
D3DAPI HRESULT IDirect3DSurface8_LockRect(LPDIRECT3DSURFACE8 pThis,
                                   D3DLOCKED_RECT* pLockedRect,
                                   const RECT* pRect, DWORD Flags);
D3DAPI HRESULT IDirect3DSurface8_UnlockRect(LPDIRECT3DSURFACE8 pThis);
// ============================================================================

// ============================================================================
D3DINTERFACE IDirect3DVertexBuffer8;
typedef D3DINTERFACE IDirect3DVertexBuffer8 IDirect3DVertexBuffer8,
                                            *LPDIRECT3DVERTEXBUFFER8;

typedef struct _D3DVERTEXBUFFER_DESC {
    D3DFORMAT           Format;
    D3DRESOURCETYPE     Type;
} D3DVERTEXBUFFER_DESC;

#ifndef __cplusplus
typedef struct IDirect3DVertexBufferVtbl8 {
    /*** IUnknown methods ***/
    ULONG   (D3DAPI *AddRef)(LPDIRECT3DVERTEXBUFFER8 pThis);
    ULONG   (D3DAPI *Release)(LPDIRECT3DVERTEXBUFFER8 pThis);

    /*** IDirect3DResource8 methods ***/
    D3DRESOURCETYPE (D3DAPI *GetType)(LPDIRECT3DVERTEXBUFFER8 pThis);
    VOID    (D3DAPI *Register)(LPDIRECT3DVERTEXBUFFER8 pThis, PVOID pBase);
    VOID    (D3DAPI *BlockUntilNotBusy)(LPDIRECT3DVERTEXBUFFER8 pThis);
    BOOL    (D3DAPI *IsBusy)(LPDIRECT3DVERTEXBUFFER8 pThis);

    /*** IDirect3DVertexBuffer methods ***/
    HRESULT (D3DAPI *GetDesc)(LPDIRECT3DVERTEXBUFFER8 pThis,
                              D3DVERTEXBUFFER_DESC* pDesc);
    HRESULT (D3DAPI *Lock)(LPDIRECT3DVERTEXBUFFER8 pThis, UINT OffsetToLock,
                           UINT SizeToLock, BYTE** ppbData, DWORD Flags);
    HRESULT (D3DAPI *Unlock)(LPDIRECT3DVERTEXBUFFER8 pThis);
} IDirect3DVertexBufferVtbl8, *LPDIRECT3DVERTEXBUFFERVTBL8;
#endif // __cplusplus

D3DINTERFACE IDirect3DVertexBuffer8 INHERITS(IDirect3DResource8) {
#ifndef __cplusplus
    LPDIRECT3DVERTEXBUFFERVTBL8 lpVtbl;
#else
public:
    virtual D3DAPI HRESULT GetDesc(D3DVERTEXBUFFER_DESC* pDesc) = 0;
    virtual D3DAPI HRESULT Lock(UINT OffsetToLock, UINT SizeToLock,
                         BYTE** ppbData, DWORD Flags) = 0;
    virtual D3DAPI HRESULT Unlock() = 0;
#endif // __cplusplus
};

D3DAPI ULONG IDirect3DVertexBuffer8_AddRef(LPDIRECT3DVERTEXBUFFER8 pThis);
D3DAPI ULONG IDirect3DVertexBuffer8_Release(LPDIRECT3DVERTEXBUFFER8 pThis);
D3DAPI D3DRESOURCETYPE IDirect3DVertexBuffer8_GetType(
    LPDIRECT3DVERTEXBUFFER8 pThis);
D3DAPI VOID IDirect3DVertexBuffer8_Register(LPDIRECT3DVERTEXBUFFER8 pThis,
                                     PVOID pBase);
D3DAPI BOOL IDirect3DVertexBuffer8_IsBusy(LPDIRECT3DVERTEXBUFFER8 pThis);
D3DAPI VOID IDirect3DVertexBuffer8_BlockUntilNotBusy(
    LPDIRECT3DVERTEXBUFFER8 pThis);
D3DAPI HRESULT IDirect3DVertexBuffer8_GetContainer(
    LPDIRECT3DVERTEXBUFFER8 pThis,
    LPDIRECT3DBASETEXTURE8* ppContainer,
    REFIID riid);
D3DAPI HRESULT IDirect3DVertexBuffer8_GetDesc(LPDIRECT3DVERTEXBUFFER8 pThis,
                                              D3DVERTEXBUFFER_DESC* pDesc);
D3DAPI HRESULT IDirect3DVertexBuffer8_Lock(LPDIRECT3DVERTEXBUFFER8 pThis,
                                           UINT OffsetToLock, UINT SizeToLock,
                                           BYTE** ppbData, DWORD Flags);
D3DAPI HRESULT IDirect3DVertexBuffer8_Unlock(LPDIRECT3DVERTEXBUFFER8 pThis);
// ============================================================================

// ============================================================================
D3DINTERFACE IDirect3DPushBuffer8;
typedef D3DINTERFACE IDirect3DPushBuffer8 IDirect3DPushBuffer8,
                                    *LPDIRECT3DPUSHBUFFER8;

#ifndef __cplusplus
typedef struct IDirect3DPushBufferVtbl8 {
    /*** IUnknown methods ***/
    ULONG   (D3DAPI *AddRef)(LPDIRECT3DPUSHBUFFER8 pThis);
    ULONG   (D3DAPI *Release)(LPDIRECT3DPUSHBUFFER8 pThis);

    /*** IDirect3DResource8 methods ***/
    D3DRESOURCETYPE (D3DAPI *GetType)(LPDIRECT3DPUSHBUFFER8 pThis);
    VOID    (D3DAPI *Register)(LPDIRECT3DPUSHBUFFER8 pThis, PVOID pBase);
    VOID    (D3DAPI *BlockUntilNotBusy)(LPDIRECT3DPUSHBUFFER8 pThis);
    BOOL    (D3DAPI *IsBusy)(LPDIRECT3DPUSHBUFFER8 pThis);

    /*** IDirect3DPushBuffer8 methods ***/
    HRESULT (D3DAPI *GetSize)(LPDIRECT3DPUSHBUFFER8 pThis, UINT*  pSize);
    HRESULT (D3DAPI *GetData)(LPDIRECT3DPUSHBUFFER8 pThis,
                              CONST DWORD** ppData);
} IDirect3DPushBufferVtbl8, *LPDIRECT3DPUSHBUFFERVTBL8;
#endif // __cplusplus

D3DINTERFACE IDirect3DPushBuffer8 INHERITS(IDirect3DResource8) {
#ifndef __cplusplus
    LPDIRECT3DPUSHBUFFERVTBL8 lpVtbl;
#else
public:
    virtual D3DAPI HRESULT GetSize(UINT *pSize)  = 0;
    virtual D3DAPI HRESULT GetData(CONST DWORD** ppData) = 0;
#endif // __cplusplus
};

D3DAPI ULONG IDirect3DPushBuffer8_AddRef(LPDIRECT3DPUSHBUFFER8 pThis);
D3DAPI ULONG IDirect3DPushBuffer8_Release(LPDIRECT3DPUSHBUFFER8 pThis);
D3DAPI D3DRESOURCETYPE IDirect3DPushBuffer8_GetType(
    LPDIRECT3DPUSHBUFFER8 pThis);
D3DAPI VOID IDirect3DPushBuffer8_Register(LPDIRECT3DPUSHBUFFER8 pThis,
                                          PVOID pBase);
D3DAPI VOID IDirect3DPushBuffer8_BlockUntilNotBusy(
    LPDIRECT3DPUSHBUFFER8 pThis);
D3DAPI BOOL IDirect3DPushBuffer8_IsBusy(LPDIRECT3DPUSHBUFFER8 pThis);

// For the default push buffer: size (in DWORDs) of the CURRENT BATCH of
// commands yet to be sent to the GPU. Only guaranteed to be valid until the
// next KickPushBuffer or until the next command is pushed.
//
// For user-recorded push buffers: size (in DWORDs) of all commands pushed
// to the push buffer. Valid until a new command is pushed.
D3DAPI HRESULT IDirect3DPushBuffer8_GetSize(LPDIRECT3DPUSHBUFFER8 pThis,
                                            UINT* pSize);

// For the default push buffer: pointer to the CURRENT BATCH of commands
// awaiting submission to the GPU. Only guaranteed to be valid until the
// next KickPushBuffer or until the next command is pushed.
//
// For user-recorded push buffers: pointer to the beginning of the push buffer.
// Valid for the object's lifetime.
D3DAPI HRESULT IDirect3DPushBuffer8_GetData(LPDIRECT3DPUSHBUFFER8 pThis,
                                            CONST DWORD** ppData);
#ifdef NXDK_DEBUG
HRESULT D3DPushBuffer_DebugDump(
    LPDIRECT3DPUSHBUFFER8 pThis, DWORD* pdwData, SIZE_T n);
#endif // NXDK_DEBUG
HRESULT D3DPushBuffer_Verify(LPDIRECT3DPUSHBUFFER8 pThis, PDWORD pdwPos);
DWORD D3DPushBuffer_BytesRemaining(LPDIRECT3DPUSHBUFFER8 pThis);
// ============================================================================

// ============================================================================
D3DINTERFACE IDirect3DDevice8;
typedef D3DINTERFACE IDirect3DDevice8 IDirect3DDevice8, *LPDIRECT3DDEVICE8;

#ifndef __cplusplus
typedef struct IDirect3DDeviceVtbl8 {
    /*** IUnknown methods ***/
    ULONG   (D3DAPI *AddRef)(LPDIRECT3DDEVICE8 pThis);
    ULONG   (D3DAPI *Release)(LPDIRECT3DDEVICE8 pThis);

    /*** IDirect3DDevice8 methods ***/
    VOID    (D3DAPI *BlockUntilVerticalBlank)(LPDIRECT3DDEVICE8 pThis);
    HRESULT (D3DAPI *CreateImageSurface)(LPDIRECT3DDEVICE8 pThis, UINT Width,
                                         UINT Height, D3DFORMAT Format,
                                         LPDIRECT3DSURFACE8* ppSurface);
    HRESULT (D3DAPI *CreateDepthStencilSurface)(
        LPDIRECT3DDEVICE8 pThis, UINT Width, UINT Height, D3DFORMAT Format,
        D3DMULTISAMPLE_TYPE MultiSampleType, LPDIRECT3DSURFACE8* ppSurface);
    HRESULT (D3DAPI *CreateVertexBuffer)(
        LPDIRECT3DDEVICE8 pThis, UINT Length, DWORD Usage, DWORD FVF,
        DWORD Pool, LPDIRECT3DVERTEXBUFFER8* ppVertexBuffer);
    HRESULT (D3DAPI *CreatePushBuffer)(
        LPDIRECT3DDEVICE8 pThis, UINT Size,
        BOOL RunUsingCpuCopy, LPDIRECT3DPUSHBUFFER8* ppPushBuffer);
    HRESULT (D3DAPI *CreateTexture)(LPDIRECT3DDEVICE8 pThis,
                                    UINT Width,
                                    UINT Height,
                                    UINT Levels,
                                    DWORD Usage,
                                    D3DFORMAT Format,
                                    D3DPOOL Pool,
                                    LPDIRECT3DTEXTURE8* ppTexture);
    HRESULT (D3DAPI *CreateCubeTexture)(LPDIRECT3DDEVICE8 pThis,
                                        UINT EdgeLength,
                                        UINT Levels,
                                        DWORD Usage,
                                        D3DFORMAT Format,
                                        D3DPOOL Pool,
                                        LPDIRECT3DCUBETEXTURE8* ppCubeTexture);
    HRESULT (D3DAPI *BeginScene)(LPDIRECT3DDEVICE8 pThis);
    HRESULT (D3DAPI *EndScene)(LPDIRECT3DDEVICE8 pThis);
    HRESULT (D3DAPI *Present)(LPDIRECT3DDEVICE8 pThis,
                              CONST RECT* pSourceRect,
                              CONST RECT* pDestRect);
    HRESULT (D3DAPI *DrawPrimitive)(LPDIRECT3DDEVICE8 pThis,
                                    D3DPRIMITIVETYPE PrimitiveType,
                                    UINT StartVertex, UINT PrimitiveCount);
    HRESULT (D3DAPI *DrawVertices)(LPDIRECT3DDEVICE8 pThis,
                                   D3DPRIMITIVETYPE PrimitiveType,
                                   UINT StartVertex, UINT VertexCount);
    HRESULT (D3DAPI *SetViewport)(LPDIRECT3DDEVICE8 pThis,
                                  CONST D3DVIEWPORT8* pViewport);
    HRESULT (D3DAPI *SetVertexShaderInputDirect)(
        LPDIRECT3DDEVICE8 pThis, D3DVERTEXATTRIBUTEFORMAT *pVAF,
        UINT StreamCount, D3DSTREAM_INPUT *pStreamInputs);
    HRESULT (D3DAPI *SetTextureStageState)(LPDIRECT3DDEVICE8 pThis,
                                           DWORD Stage,
                                           D3DTEXTURESTAGESTATETYPE Type,
                                           DWORD Value);
    HRESULT (D3DAPI *SetRenderState)(LPDIRECT3DDEVICE8 pThis,
                                     D3DRENDERSTATETYPE Type, DWORD VALUE);
    HRESULT (D3DAPI *LoadVertexShaderProgram)(
        LPDIRECT3DDEVICE8 pThis, CONST DWORD *pFunction, DWORD Address);
    HRESULT (D3DAPI *SetPixelShaderProgram)(LPDIRECT3DDEVICE8 pThis,
                                            CONST D3DPIXELSHADERDEF *pPSDef);
    HRESULT (D3DAPI *Clear)(LPDIRECT3DDEVICE8 pThis, DWORD Count,
                            CONST D3DRECT* pRects, DWORD Flags, D3DCOLOR Color,
                            float Z, DWORD Stencil);
    HRESULT (D3DAPI *SetScissors)(LPDIRECT3DDEVICE8 pThis, DWORD Count,
                                  BOOL Exclusive, CONST D3DRECT *pRects);
    HRESULT (D3DAPI *SetTile)(LPDIRECT3DDEVICE8 pThis,
                              DWORD Index, CONST D3DTILE* pTile);
    HRESULT (D3DAPI *SetTexture)(LPDIRECT3DDEVICE8 pThis, DWORD Stage,
                                 LPDIRECT3DBASETEXTURE8 pTexture);
    HRESULT (D3DAPI *InsertCallback)(LPDIRECT3DDEVICE8 pThis,
                                     D3DCALLBACKTYPE Type,
                                     D3DCALLBACK pCallback,
                                     DWORD Context);
    HRESULT (D3DAPI *Reset)(LPDIRECT3DDEVICE8 pThis,
                            D3DPRESENT_PARAMETERS* pPresentationParameters);
    DWORD (D3DAPI *InsertFence)(LPDIRECT3DDEVICE8 pThis);
    VOID (D3DAPI *BlockOnFence)(LPDIRECT3DDEVICE8 pThis, DWORD Fence);
    BOOL (D3DAPI *IsFencePending)(LPDIRECT3DDEVICE8 pThis, DWORD Fence);
    HRESULT (D3DAPI *SetVertexShaderConstant)(
        LPDIRECT3DDEVICE8 pThis, INT Register, CONST VOID* pConstantData,
        DWORD ConstantCount);
    HRESULT (D3DAPI *SetPixelShaderConstant)(
        LPDIRECT3DDEVICE8 pThis, DWORD Register, CONST VOID* pConstantData,
        DWORD ConstantCount);
    HRESULT (D3DAPI *GetPushBuffer)(
        LPDIRECT3DDEVICE8 pThis, LPDIRECT3DPUSHBUFFER8* ppPushBuffer);
    VOID (D3DAPI *KickPushBuffer)(LPDIRECT3DDEVICE8 pThis);
    VOID (D3DAPI *SyncPushBuffer)(LPDIRECT3DDEVICE8 pThis);
} IDirect3DDeviceVtbl8, *LPDIRECT3DDEVICEVTBL8;
#endif // __cplusplus

D3DINTERFACE IDirect3DDevice8 INHERITS(IUnknown) {
#ifndef __cplusplus
    LPDIRECT3DDEVICEVTBL8 lpVtbl;
#else
public:
    virtual D3DAPI VOID BlockUntilVerticalBlank() = 0;
    virtual D3DAPI HRESULT CreateImageSurface(
        UINT Width, UINT Height, D3DFORMAT Format,
        LPDIRECT3DSURFACE8* ppSurface) = 0;
    virtual D3DAPI HRESULT CreateDepthStencilSurface(
        UINT Width, UINT Height, D3DFORMAT Format,
        D3DMULTISAMPLE_TYPE MultiSampleType,
        LPDIRECT3DSURFACE8* ppSurface) = 0;
    virtual D3DAPI HRESULT CreateVertexBuffer(
        UINT Length, DWORD Usage, DWORD FVF,
        DWORD Pool, LPDIRECT3DVERTEXBUFFER8* ppVertexBuffer) = 0;
    virtual D3DAPI HRESULT CreatePushBuffer(
        UINT Size, BOOL RunUsingCpuCopy,
        LPDIRECT3DPUSHBUFFER8* ppPushBuffer) = 0;
    virtual D3DAPI HRESULT CreateTexture(
        UINT Width,
        UINT Height,
        UINT Levels,
        DWORD Usage,
        D3DFORMAT Format,
        D3DPOOL Pool,
        LPDIRECT3DTEXTURE8* ppTexture) = 0;
    virtual D3DAPI HRESULT CreateCubeTexture(
        UINT EdgeLength,
        UINT Levels,
        DWORD Usage,
        D3DFORMAT Format,
        D3DPOOL Pool,
        LPDIRECT3DCUBETEXTURE8* ppCubeTexture) = 0;
    virtual D3DAPI HRESULT BeginScene() = 0;
    virtual D3DAPI HRESULT EndScene() = 0;
    virtual D3DAPI HRESULT Present(CONST RECT* pSourceRect,
                                   CONST RECT* pDestRect) = 0;
    virtual D3DAPI HRESULT DrawPrimitive(
        D3DPRIMITIVETYPE PrimitiveType,
        UINT StartVertex,
        UINT PrimitiveCount) = 0;
    virtual D3DAPI HRESULT DrawVertices(
        D3DPRIMITIVETYPE PrimitiveType,
        UINT StartVertex,
        UINT VertexCount) = 0;
    virtual D3DAPI HRESULT SetViewport(
        CONST D3DVIEWPORT8* pViewport) = 0;
    virtual D3DAPI HRESULT SetVertexShaderInputDirect(
        D3DVERTEXATTRIBUTEFORMAT *pVAF,
        UINT StreamCount,
        D3DSTREAM_INPUT *pStreamInputs) = 0;
    virtual D3DAPI HRESULT SetTextureStageState(
        DWORD Stage,
        D3DTEXTURESTAGESTATETYPE Type,
        DWORD Value) = 0;
    virtual D3DAPI HRESULT SetRenderState(
        D3DRENDERSTATETYPE Type, DWORD Value) = 0;
    virtual D3DAPI HRESULT LoadVertexShaderProgram(
        CONST DWORD *pFunction, DWORD Address) = 0;
    virtual D3DAPI HRESULT SetPixelShaderProgram(
        CONST D3DPIXELSHADERDEF *pPSDef) = 0;
    virtual D3DAPI HRESULT Clear(
        DWORD Count, CONST D3DRECT* pRects, DWORD Flags,
        D3DCOLOR Color, float Z, DWORD Stencil) = 0;
    virtual D3DAPI HRESULT SetScissors(
        DWORD Count, BOOL Exclusive, CONST D3DRECT *pRects) = 0;
    virtual D3DAPI HRESULT SetTile(
        DWORD Index, CONST D3DTILE* pTile) = 0;
    virtual D3DAPI HRESULT SetTexture(DWORD Stage,
                               LPDIRECT3DBASETEXTURE8 pTexture) = 0;
    virtual D3DAPI HRESULT InsertCallback(D3DCALLBACKTYPE Type,
                                          D3DCALLBACK pCallback,
                                          DWORD Context) = 0;
    virtual D3DAPI HRESULT Reset(
        D3DPRESENT_PARAMETERS* pPresentationParameters) = 0;
    virtual D3DAPI DWORD InsertFence() = 0;
    virtual D3DAPI VOID BlockOnFence(DWORD Fence) = 0;
    virtual D3DAPI BOOL IsFencePending(DWORD Fence) = 0;
    virtual D3DAPI HRESULT SetVertexShaderConstant(
        INT Register, CONST VOID* pConstantData, DWORD ConstantCount) = 0;
    virtual D3DAPI HRESULT SetPixelShaderConstant(
        DWORD Register, CONST VOID* pConstantData, DWORD ConstantCount) = 0;
    virtual D3DAPI HRESULT GetPushBuffer(
        LPDIRECT3DPUSHBUFFER8* ppPushBuffer) = 0;
    virtual D3DAPI VOID KickPushBuffer() = 0;
    virtual D3DAPI VOID SyncPushBuffer() = 0;
#endif // __cplusplus
};

D3DAPI ULONG IDirect3DDevice8_AddRef(LPDIRECT3DDEVICE8 pThis);
D3DAPI ULONG IDirect3DDevice8_Release(LPDIRECT3DDEVICE8 pThis);
D3DAPI VOID  IDirect3DDevice8_BlockUntilVerticalBlank(LPDIRECT3DDEVICE8 pThis);
D3DAPI HRESULT IDirect3DDevice8_CreateImageSurface(
    LPDIRECT3DDEVICE8 pThis, UINT Width, UINT Height,
    D3DFORMAT Format, LPDIRECT3DSURFACE8* ppSurface);
D3DAPI HRESULT IDirect3DDevice8_CreateDepthStencilSurface(
    LPDIRECT3DDEVICE8 pThis, UINT Width, UINT Height, D3DFORMAT Format,
    D3DMULTISAMPLE_TYPE MultiSample, IDirect3DSurface8** ppSurface);
D3DAPI HRESULT IDirect3DDevice8_CreateVertexBuffer(
    LPDIRECT3DDEVICE8 pThis, UINT Length, DWORD Usage, DWORD FVF,
    DWORD Pool, LPDIRECT3DVERTEXBUFFER8* ppVertexBuffer);
D3DAPI HRESULT IDirect3DDevice8_CreatePushBuffer(
    LPDIRECT3DDEVICE8 pThis, UINT Size,
    BOOL RunUsingCpuCopy, LPDIRECT3DPUSHBUFFER8* ppPushBuffer);
D3DAPI HRESULT IDirect3DDevice8_BeginScene(LPDIRECT3DDEVICE8 pThis);
D3DAPI HRESULT IDirect3DDevice8_EndScene(LPDIRECT3DDEVICE8 pThis);
D3DAPI HRESULT IDirect3DDevice8_Present(LPDIRECT3DDEVICE8 pThis,
                                        CONST RECT* pSourceRect,
                                        CONST RECT* pDestRect);
D3DAPI HRESULT IDirect3DDevice8_DrawPrimitive(LPDIRECT3DDEVICE8 pThis,
                                              D3DPRIMITIVETYPE PrimitiveType,
                                              UINT StartVertex,
                                              UINT PrimitiveCount);
D3DAPI HRESULT IDirect3DDevice8_DrawVertices(LPDIRECT3DDEVICE8 pThis,
                                             D3DPRIMITIVETYPE PrimitiveType,
                                             UINT StartVertex,
                                             UINT VertexCount);
D3DAPI HRESULT IDirect3DDevice8_SetViewport(LPDIRECT3DDEVICE8 pThis,
                                            CONST D3DVIEWPORT8* pViewport);
D3DAPI HRESULT IDirect3DDevice8_SetVertexShaderInputDirect(
    LPDIRECT3DDEVICE8 pThis,
    D3DVERTEXATTRIBUTEFORMAT *pVAF,
    UINT StreamCount,
    D3DSTREAM_INPUT *pStreamInputs);
D3DAPI HRESULT IDirect3DDevice8_CreateTexture(LPDIRECT3DDEVICE8 pThis,
                                        UINT Width,
                                        UINT Height,
                                        UINT Levels,
                                        DWORD Usage,
                                        D3DFORMAT Format,
                                        D3DPOOL Pool,
                                        LPDIRECT3DTEXTURE8* ppTexture);
D3DAPI HRESULT IDirect3DDevice8_CreateCubeTexture(
    LPDIRECT3DDEVICE8 pThis, UINT EdgeLength, UINT Levels, DWORD Usage,
    D3DFORMAT Format, D3DPOOL Pool, LPDIRECT3DCUBETEXTURE8* ppCubeTexture);
D3DAPI HRESULT IDirect3DDevice8_SetTextureStageState(
    LPDIRECT3DDEVICE8 pThis, DWORD Stage,
    D3DTEXTURESTAGESTATETYPE Type, DWORD Value);
D3DAPI HRESULT IDirect3DDevice8_SetRenderState(
    LPDIRECT3DDEVICE8 pThis, D3DRENDERSTATETYPE Type, DWORD Value);
D3DAPI HRESULT IDirect3DDevice8_LoadVertexShaderProgram(
    LPDIRECT3DDEVICE8 pThis, CONST DWORD *pFunction, DWORD Address);
D3DAPI HRESULT IDirect3DDevice8_SetPixelShaderProgram(
    LPDIRECT3DDEVICE8 pThis, CONST D3DPIXELSHADERDEF *pPSDef);
D3DAPI HRESULT IDirect3DDevice8_Clear(LPDIRECT3DDEVICE8 pThis, DWORD Count,
                                      CONST D3DRECT* pRects, DWORD Flags,
                                      D3DCOLOR Color, float Z, DWORD Stencil);
D3DAPI HRESULT IDirect3DDevice8_SetScissors(LPDIRECT3DDEVICE8 pThis,
                                            DWORD Count, BOOL Exclusive,
                                            CONST D3DRECT *pRects);
D3DAPI HRESULT D3DDevice_SetRenderState(D3DRENDERSTATETYPE Type, DWORD Value);
D3DAPI HRESULT IDirect3DDevice8_SetTile(LPDIRECT3DDEVICE8 pThis, DWORD Index,
                                 CONST D3DTILE* pTile);
D3DAPI HRESULT IDirect3DDevice8_SetTexture(LPDIRECT3DDEVICE8 pThis,
                                           DWORD Stage,
                                           LPDIRECT3DBASETEXTURE8 pTexture);
D3DAPI HRESULT IDirect3DDevice8_InsertCallback(LPDIRECT3DDEVICE8 pThis,
                                               D3DCALLBACKTYPE Type,
                                               D3DCALLBACK pCallback,
                                               DWORD Context);
D3DAPI HRESULT IDirect3DDevice8_Reset(
    LPDIRECT3DDEVICE8 pThis,
    D3DPRESENT_PARAMETERS* pPresentationParameters);

D3DAPI DWORD IDirect3DDevice8_InsertFence(LPDIRECT3DDEVICE8 pThis);
D3DAPI VOID  IDirect3DDevice8_BlockOnFence(LPDIRECT3DDEVICE8 pThis,
                                           DWORD Fence);
D3DAPI BOOL  IDirect3DDevice8_IsFencePending(LPDIRECT3DDEVICE8 pThis,
                                             DWORD Fence);
D3DAPI HRESULT IDirect3DDevice8_SetVertexShaderConstant(
    LPDIRECT3DDEVICE8 pThis,
    INT Register,
    CONST void* pConstantData, DWORD ConstantCount);

D3DAPI HRESULT IDirect3DDevice8_SetPixelShaderConstant(
    LPDIRECT3DDEVICE8 pThis,
    DWORD Register,
    CONST void* pConstantData, DWORD ConstantCount);

D3DAPI HRESULT IDirect3DDevice8_GetPushBuffer(
    LPDIRECT3DDEVICE8 pThis, LPDIRECT3DPUSHBUFFER8* ppPushBuffer);
D3DAPI VOID IDirect3DDevice8_KickPushBuffer(LPDIRECT3DDEVICE8 pThis);
// This function should be called immediately after any pbkit functions that
// manipulate push buffer state (pb_end, pb_reset, etc., and any functions
// that call them transitively.) DO NOT FORGET TO CALL THIS, or bad things
// will happen.
D3DAPI VOID IDirect3DDevice8_SyncPushBuffer(LPDIRECT3DDEVICE8 pThis);

HRESULT D3DDevice_ResetPushBufferToHead(void);

struct IDirect3D8;
typedef struct IDirect3D8 IDirect3D8, *LPDIRECT3D8;

#ifndef __cplusplus
typedef struct IDirect3DVtbl8 {
    /*** IUnknown methods ***/
    ULONG   (D3DAPI *AddRef)(LPDIRECT3D8 pThis);
    ULONG   (D3DAPI *Release)(LPDIRECT3D8 pThis);

    /*** IDirect3D8 methods ***/
    UINT     (D3DAPI *GetAdapterCount)(LPDIRECT3D8 pThis);
    HRESULT  (D3DAPI *GetAdapterIdentifier)(
        LPDIRECT3D8 pThis, UINT Adapter, DWORD Flags,
        D3DADAPTER_IDENTIFIER8* pIdentifier);
    UINT     (D3DAPI *GetAdapterModeCount)(LPDIRECT3D8 pThis, UINT Adapter);
    HRESULT  (D3DAPI *EnumAdapterModes)(LPDIRECT3D8 pThis, UINT Adapter,
                                        UINT Mode, D3DDISPLAYMODE* pMode);
    HRESULT  (D3DAPI *GetAdapterDisplayMode)(LPDIRECT3D8 pThis, UINT Adapter,
                                             D3DDISPLAYMODE* pMode);
    HRESULT  (D3DAPI *CheckDeviceType)(LPDIRECT3D8 pThis, UINT Adapter,
                                       D3DDEVTYPE CheckType,
                                       D3DFORMAT DisplayFormat,
                                       D3DFORMAT BackBufferFormat,
                                       BOOL Windowed);
    HRESULT  (D3DAPI *CheckDeviceFormat)(LPDIRECT3D8 pThis, UINT Adapter,
                                         D3DDEVTYPE DeviceType,
                                         D3DFORMAT AdapterFormat, DWORD Usage,
                                         D3DRESOURCETYPE RType,
                                         D3DFORMAT CheckFormat);
    HRESULT  (D3DAPI *CheckDeviceMultiSampleType)(
        LPDIRECT3D8 pThis, UINT Adapter, D3DDEVTYPE DeviceType,
        D3DFORMAT SurfaceFormat, BOOL Windowed,
        D3DMULTISAMPLE_TYPE MultiSampleType);
    HRESULT  (D3DAPI *CheckDepthStencilMatch)(LPDIRECT3D8 pThis,
                                              UINT Adapter,
                                              D3DDEVTYPE DeviceType,
                                              D3DFORMAT AdapterFormat,
                                              D3DFORMAT RenderTargetFormat,
                                              D3DFORMAT DepthStencilFormat);
    HRESULT  (D3DAPI *GetDeviceCaps)(LPDIRECT3D8 pThis, UINT Adapter,
                                     D3DDEVTYPE DeviceType, D3DCAPS8* pCaps);
    HMONITOR (D3DAPI *GetAdapterMonitor)(LPDIRECT3D8 pThis, UINT Adapter);
    HRESULT  (D3DAPI *SetPushBufferSize)(LPDIRECT3D8 pThis,
                                         DWORD PushBufferSize,
                                         DWORD KickOffSize);
    HRESULT  (D3DAPI *CreateDevice)(
        LPDIRECT3D8 pThis, UINT Adapter, D3DDEVTYPE DeviceType,
        HWND hFocusWindow, DWORD BehaviorFlags,
        D3DPRESENT_PARAMETERS* pPresentationParameters,
        LPDIRECT3DDEVICE8* ppReturnedDeviceInterface);
} IDirect3DVtbl8, *LPDIRECT3DVTBL8;
#endif // __cplusplus

D3DINTERFACE IDirect3D8 INHERITS(IUnknown) {
#ifndef __cplusplus
    LPDIRECT3DVTBL8 lpVtbl;
#else
public:
    virtual D3DAPI UINT    GetAdapterCount() = 0;
    virtual D3DAPI HRESULT GetAdapterIdentifier(
        UINT Adapter, DWORD Flags, D3DADAPTER_IDENTIFIER8* pIdentifier) = 0;
    virtual D3DAPI UINT GetAdapterModeCount(UINT Adapter) = 0;
    virtual D3DAPI HRESULT EnumAdapterModes(UINT Adapter, UINT Mode,
                                     D3DDISPLAYMODE* pMode) = 0;
    virtual D3DAPI HRESULT GetAdapterDisplayMode(UINT Adapter,
                                          D3DDISPLAYMODE* pMode) = 0;
    virtual D3DAPI HRESULT CheckDeviceType(
        UINT Adapter, D3DDEVTYPE CheckType, D3DFORMAT DisplayFormat,
        D3DFORMAT BackBufferFormat, BOOL Windowed) = 0;
    virtual D3DAPI HRESULT CheckDeviceFormat(
        UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT AdapterFormat,
        DWORD Usage, D3DRESOURCETYPE RType, D3DFORMAT CheckFormat) = 0;
    virtual D3DAPI HRESULT CheckDeviceMultiSampleType(
        UINT Adapter, D3DDEVTYPE DeviceType,
        D3DFORMAT SurfaceFormat, BOOL Windowed,
        D3DMULTISAMPLE_TYPE MultiSampleType) = 0;
    virtual D3DAPI HRESULT CheckDepthStencilMatch(
        UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT AdapterFormat,
        D3DFORMAT RenderTargetFormat, D3DFORMAT DepthStencilFormat) = 0;
    virtual D3DAPI HRESULT GetDeviceCaps(
        UINT Adapter, D3DDEVTYPE DeviceType, D3DCAPS8* pCaps) = 0;
    virtual D3DAPI HMONITOR GetAdapterMonitor(UINT Adapter) = 0;
    virtual D3DAPI HRESULT  SetPushBufferSize(
        DWORD PushBufferSize, DWORD KickOffSize) = 0;
    virtual D3DAPI HRESULT  CreateDevice(
        UINT Adapter, D3DDEVTYPE DeviceType, HWND hFocusWindow,
        DWORD BehaviorFlags, D3DPRESENT_PARAMETERS* pPresentationParameters,
        LPDIRECT3DDEVICE8* ppReturnedDeviceInterface) = 0;
#endif // __cplusplus
};

D3DAPI ULONG    IDirect3D8_AddRef(LPDIRECT3D8 pThis);
D3DAPI ULONG    IDirect3D8_Release(LPDIRECT3D8 pThis);
D3DAPI UINT     IDirect3D8_GetAdapterCount(LPDIRECT3D8 pThis);
D3DAPI HMONITOR IDirect3D8_GetAdapterMonitor(LPDIRECT3D8 pThis, UINT Adapter);
D3DAPI HRESULT  IDirect3D8_CheckDeviceType(LPDIRECT3D8 pThis, UINT Adapter,
                                    D3DDEVTYPE CheckType,
                                    D3DFORMAT DisplayFormat,
                                    D3DFORMAT BackBufferFormat, BOOL Windowed);
D3DAPI HRESULT IDirect3D8_CheckDepthStencilMatch(LPDIRECT3D8 pThis,
                                                 UINT Adapter,
                                                 D3DDEVTYPE DeviceType,
                                                 D3DFORMAT AdapterFormat,
                                                 D3DFORMAT RenderTargetFormat,
                                                 D3DFORMAT DepthStencilFormat);
D3DAPI UINT  IDirect3D8_GetAdapterModeCount(LPDIRECT3D8 pThis, UINT Adapter);
D3DAPI HRESULT  IDirect3D8_EnumAdapterModes(LPDIRECT3D8 pThis, UINT Adapter,
                                            UINT Mode, D3DDISPLAYMODE* pMode);
D3DAPI HRESULT  IDirect3D8_GetAdapterDisplayMode(LPDIRECT3D8 pThis,
                                                 UINT Adapter,
                                                 D3DDISPLAYMODE* pMode);
D3DAPI HRESULT  IDirect3D8_SetPushBufferSize(LPDIRECT3D8 pThis,
                                             DWORD PushBufferSize,
                                             DWORD KickOffSize);
D3DAPI HRESULT IDirect3D8_CreateDevice(
    LPDIRECT3D8 pThis, UINT Adapter, D3DDEVTYPE DeviceType,
    HWND hFocusWindow, DWORD BehaviorFlags,
    D3DPRESENT_PARAMETERS* pPresentationParameters,
    LPDIRECT3DDEVICE8* ppReturnedDevice);
D3DAPI HRESULT IDirect3D8_GetDeviceCaps(LPDIRECT3D8 pThis,
                                        UINT Adapter,
                                        D3DDEVTYPE DeviceType,
                                        D3DCAPS8* pCaps);

PVOID D3D_AllocContiguousMemory(DWORD Size, DWORD Alignment);
VOID  D3D_FreeContiguousMemory (PVOID Base);
#define D3D_SDK_VERSION 120
D3DEXTERN D3DAPI LPDIRECT3D8 Direct3DCreate8(UINT SKDVersion);

#ifdef __cplusplus
}
#endif // __cplusplus
