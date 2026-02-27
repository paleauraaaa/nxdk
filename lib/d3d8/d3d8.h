#ifndef NXDK_HAVE_D3D8
#define NXDK_HAVE_D3D8

#include <windows.h>
#include <winnt.h>

#include "d3d8types.h"

#ifndef DIRECT3D_VERSION
#define DIRECT3D_VERSION 0x0800
#endif // DIRECT3D_VERSION

#ifdef __cplusplus 
#define INHERITS(base) : base
#define IMPLEMENTS(base) : base
#else 
#define INHERITS(base)
#define IMPLEMENTS(base)
#endif // __cplusplus

struct IUnknown;
typedef struct IUnknown IUnknown, *LPUNKNOWN;

typedef struct UnknownVtbl {
    ULONG   (*AddRef)(LPUNKNOWN pThis);
    ULONG   (*Release)(LPUNKNOWN pThis);
} UnknownVtbl, *LPUNKNOWNVTBL;

struct IUnknown {
    LPUNKNOWNVTBL lpVtbl;
#ifdef __cplusplus
    virtual ULONG   AddRef()  = 0;
    virtual ULONG   Release() = 0;
#endif // __cplusplus
};

// ============================================================================
struct IDirect3DResource8;
typedef struct IDirect3DResource8 IDirect3DResource8, *LPDIRECT3DRESOURCE8;

typedef struct IDirect3DResourceVtbl8 {
    /*** IUnknown methods ***/
    ULONG   (*AddRef)(LPDIRECT3DRESOURCE8 pThis);
    ULONG   (*Release)(LPDIRECT3DRESOURCE8 pThis);

    /*** IDirect3DResource8 methods ***/
    D3DRESOURCETYPE (*GetType)(LPDIRECT3DRESOURCE8 pThis);
    VOID            (*Register)(LPDIRECT3DRESOURCE8 pThis, PVOID pBase);
    VOID            (*BlockUntilNotBusy)(LPDIRECT3DRESOURCE8 pThis);
    BOOL            (*IsBusy)(LPDIRECT3DRESOURCE8 pThis);
} IDirect3DResourceVtbl8, *LPDIRECT3DRESOURCEVTBL8;

struct IDirect3DResource8 INHERITS(IUnknown) {
    LPDIRECT3DRESOURCEVTBL8 lpVtbl;
#ifdef __cplusplus
    virtual D3DRESOURCETYPE GetType() = 0;
    virtual VOID            Register(PVOID pBase) = 0;
    virtual VOID            BlockUntilNotBusy() = 0;
    virtual BOOL            IsBusy() = 0;
#endif // __cplusplus
};

ULONG IDirect3DResource8_AddRef(LPDIRECT3DRESOURCE8 pThis);
ULONG IDirect3DResource8_Release(LPDIRECT3DRESOURCE8 pThis);
D3DRESOURCETYPE IDirect3DResource8_GetType(LPDIRECT3DRESOURCE8 pThis);
VOID IDirect3DResource8_Register(LPDIRECT3DRESOURCE8 pThis, PVOID pBase);
VOID IDirect3DResource8_BlockUntilNotBusy(LPDIRECT3DRESOURCE8 pThis);
// ============================================================================

// ============================================================================
struct IDirect3DBaseTexture8;
typedef struct IDirect3DBaseTexture8 IDirect3DBaseTexture8, 
                                     *LPDIRECT3DBASETEXTURE8;

typedef struct IDirect3DBaseTextureVtbl8 {
    /*** IUnknown methods ***/
    ULONG   (*AddRef)(LPDIRECT3DBASETEXTURE8 pThis);
    ULONG   (*Release)(LPDIRECT3DBASETEXTURE8 pThis);

    /*** IDirect3DResource8 methods ***/
    D3DRESOURCETYPE (*GetType)(LPDIRECT3DBASETEXTURE8 pThis);
    VOID            (*Register)(LPDIRECT3DBASETEXTURE8 pThis, PVOID pBase);
    VOID            (*BlockUntilNotBusy)(LPDIRECT3DBASETEXTURE8 pThis);
    BOOL            (*IsBusy)(LPDIRECT3DBASETEXTURE8 pThis);

    /*** IDirect3DBaseTexture8 methods ***/
    DWORD           (*GetLevelCount)(LPDIRECT3DBASETEXTURE8 pThis);
} IDirect3DBaseTextureVtbl8, *LPDIRECT3DBASETEXTUREVTBL8;

struct IDirect3DBaseTexture8 INHERITS(IDirect3DResource8) {
    LPDIRECT3DBASETEXTUREVTBL8 lpVtbl;
#ifdef __cplusplus
    virtual DWORD GetLevelCount() = 0;
#endif // __cplusplus
};

ULONG IDirect3DBaseTexture8_AddRef(LPDIRECT3DBASETEXTURE8 pThis);
ULONG IDirect3DBaseTexture8_Release(LPDIRECT3DBASETEXTURE8 pThis);
D3DRESOURCETYPE IDirect3DBaseTexture8_GetType(LPDIRECT3DBASETEXTURE8 pThis);
VOID IDirect3DBaseTexture8_Register(LPDIRECT3DBASETEXTURE8 pThis, PVOID pBase);
DWORD IDirect3DBaseTexture8_GetLevelCount(LPDIRECT3DBASETEXTURE8 pThis);
// ============================================================================

// ============================================================================
struct IDirect3DTexture8;
typedef struct IDirect3DTexture8 IDirect3DTexture8, 
                                 *LPDIRECT3DTEXTURE8;

typedef struct IDirect3DTextureVtbl8 {
    /*** IUnknown methods ***/
    ULONG   (*AddRef)(LPDIRECT3DTEXTURE8 pThis);
    ULONG   (*Release)(LPDIRECT3DTEXTURE8 pThis);

    /*** IDirect3DResource8 methods ***/
    D3DRESOURCETYPE (*GetType)(LPDIRECT3DTEXTURE8 pThis);
    VOID            (*Register)(LPDIRECT3DTEXTURE8 pThis, PVOID pBase);
    VOID            (*BlockUntilNotBusy)(LPDIRECT3DTEXTURE8 pThis);
    BOOL            (*IsBusy)(LPDIRECT3DTEXTURE8 pThis);

    /*** IDirect3DBaseTexture8 methods ***/
    DWORD           (*GetLevelCount)(LPDIRECT3DTEXTURE8 pThis);

    /*** IDirect3DTexture8 methods ***/
    HRESULT         (*GetLevelDesc)(LPDIRECT3DTEXTURE8 pThis, 
                                   UINT Level, 
                                   D3DSURFACE_DESC* pDesc);
    HRESULT         (*GetSurfaceLevel)(LPDIRECT3DTEXTURE8 pThis, 
                                      UINT Level, 
                                      LPDIRECT3DSURFACE8* ppSurfaceLevel);
    HRESULT         (*LockRect)(LPDIRECT3DTEXTURE8 pThis, 
                                UINT Level, 
                                D3DLOCKED_RECT* pLockedRect, 
                                const RECT* pRect, 
                                DWORD Flags);
    HRESULT         (*UnlockRect)(LPDIRECT3DTEXTURE8 pThis, 
                                  UINT Level);
} IDirect3DTextureVtbl8, *LPDIRECT3DTEXTUREVTBL8;

struct IDirect3DTexture8 INHERITS(IDirect3DBaseTexture8) {
    LPDIRECT3DTEXTUREVTBL8 lpVtbl;
#ifdef __cplusplus
    virtual HRESULT GetLevelDesc(UINT Level, D3DSURFACE_DESC* pDesc) = 0;
    virtual HRESULT GetSurfaceLevel(UINT Level, 
                                    LPDIRECT3DSURFACE8* ppSurfaceLevel) = 0;
    virtual HRESULT LockRect(UINT Level, D3DLOCKED_RECT* pLockedRect, 
                             const RECT* pRect, DWORD Flags) = 0;
    virtual HRESULT UnlockRect(UINT Level) = 0;
#endif // __cplusplus
};

ULONG           IDirect3DTexture8_AddRef(LPDIRECT3DTEXTURE8 pThis);
ULONG           IDirect3DTexture8_Release(LPDIRECT3DTEXTURE8 pThis);
D3DRESOURCETYPE IDirect3DTexture8_GetType(LPDIRECT3DTEXTURE8 pThis);
VOID            IDirect3DTexture8_Register(LPDIRECT3DTEXTURE8 pThis, PVOID pBase);
DWORD           IDirect3DTexture8_GetLevelCount(LPDIRECT3DTEXTURE8 pThis);
HRESULT         IDirect3DTexture8_GetLevelDesc(LPDIRECT3DTEXTURE8 pThis, 
                                               UINT Level, 
                                               D3DSURFACE_DESC* pDesc);
HRESULT         IDirect3DTexture8_GetSurfaceLevel(
    LPDIRECT3DTEXTURE8 pThis, UINT Level, LPDIRECT3DSURFACE8* ppSurfaceLevel);
HRESULT         IDirect3DTexture8_LockRect(LPDIRECT3DTEXTURE8 pThis, 
                                           UINT Level, 
                                           D3DLOCKED_RECT* pLockedRect, 
                                           const RECT* pRect, 
                                           DWORD Flags);
HRESULT         IDirect3DTexture8_UnlockRect(LPDIRECT3DTEXTURE8 pThis, 
                                             UINT Level);  
// ============================================================================

// ============================================================================
struct IDirect3DCubeTexture8;
typedef struct IDirect3DCubeTexture8 IDirect3DCubeTexture8, 
                                     *LPDIRECT3DCUBETEXTURE8;

typedef struct IDirect3DCubeTextureVtbl8 {
    /*** IUnknown methods ***/
    ULONG   (*AddRef)(LPDIRECT3DCUBETEXTURE8 pThis);
    ULONG   (*Release)(LPDIRECT3DCUBETEXTURE8 pThis);

    /*** IDirect3DResource8 methods ***/
    D3DRESOURCETYPE (*GetType)(LPDIRECT3DCUBETEXTURE8 pThis);
    VOID            (*Register)(LPDIRECT3DCUBETEXTURE8 pThis, PVOID pBase);
    VOID            (*BlockUntilNotBusy)(LPDIRECT3DCUBETEXTURE8 pThis);
    BOOL            (*IsBusy)(LPDIRECT3DCUBETEXTURE8 pThis);

    /*** IDirect3DBaseTexture8 methods ***/
    DWORD           (*GetLevelCount)(LPDIRECT3DCUBETEXTURE8 pThis);

    /*** IDirect3DCubeTexture8 methods ***/
    HRESULT         (*GetLevelDesc)(LPDIRECT3DCUBETEXTURE8 pThis, 
                                   UINT Level, 
                                   D3DSURFACE_DESC* pDesc);
    HRESULT         (*LockRect)(LPDIRECT3DCUBETEXTURE8 pThis, 
                                D3DCUBEMAP_FACES FaceType, 
                                UINT Level, 
                                D3DLOCKED_RECT* pLockedRect, 
                                const RECT* pRect, 
                                DWORD Flags);
    HRESULT         (*UnlockRect)(LPDIRECT3DCUBETEXTURE8 pThis, 
                                  D3DCUBEMAP_FACES FaceType, 
                                  UINT Level);
    HRESULT         (*GetCubeMapSurface)(LPDIRECT3DCUBETEXTURE8 pThis,
                                         D3DCUBEMAP_FACES FaceType, 
                                         UINT Level, 
                                         LPDIRECT3DSURFACE8* ppCubeMapSurface);
} IDirect3DCubeTextureVtbl8, *LPDIRECT3DCUBETEXTUREVTBL8;

struct IDirect3DCubeTexture8 INHERITS(IDirect3DBaseTexture8) {
    LPDIRECT3DCUBETEXTUREVTBL8 lpVtbl;
#ifdef __cplusplus
    virtual HRESULT GetLevelDesc(UINT Level, D3DSURFACE_DESC* pDesc) = 0;
    virtual HRESULT LockRect(UINT Level, D3DLOCKED_RECT* pLockedRect, 
                             const RECT* pRect, DWORD Flags) = 0;
    virtual HRESULT UnlockRect(UINT Level) = 0;
    virtual HRESULT GetCubeMapSurface(D3DCUBEMAP_FACES FaceType, UINT Level, 
                                     LPDIRECT3DSURFACE8* ppCubeMapSurface) = 0;
#endif // __cplusplus
};

ULONG           IDirect3DCubeTexture8_AddRef(LPDIRECT3DCUBETEXTURE8 pThis);
ULONG           IDirect3DCubeTexture8_Release(LPDIRECT3DCUBETEXTURE8 pThis);
D3DRESOURCETYPE IDirect3DCubeTexture8_GetType(LPDIRECT3DCUBETEXTURE8 pThis);
VOID            IDirect3DCubeTexture8_Register(LPDIRECT3DCUBETEXTURE8 pThis, 
                                               PVOID pBase);
DWORD  IDirect3DCubeTexture8_GetLevelCount(LPDIRECT3DCUBETEXTURE8 pThis);
HRESULT         IDirect3DCubeTexture8_GetLevelDesc(LPDIRECT3DCUBETEXTURE8 pThis, 
                                               UINT Level, 
                                               D3DSURFACE_DESC* pDesc);
HRESULT         IDirect3DCubeTexture8_LockRect(LPDIRECT3DCUBETEXTURE8 pThis,
                                               D3DCUBEMAP_FACES FaceType, 
                                               UINT Level,  
                                               D3DLOCKED_RECT* pLockedRect, 
                                               CONST RECT* pRect, DWORD Flags);
HRESULT         IDirect3DCubeTexture8_UnlockRect(LPDIRECT3DCUBETEXTURE8 pThis, 
                                                 D3DCUBEMAP_FACES FaceType,
                                                 UINT Level);  
HRESULT IDirect3DCubeTexture8_GetCubeMapSurface(LPDIRECT3DCUBETEXTURE8 pThis,
    D3DCUBEMAP_FACES FaceType, UINT Level, 
    LPDIRECT3DSURFACE8* ppCubeMapSurface);
// ============================================================================

// ============================================================================
typedef struct IDirect3DSurfaceVtbl8 {
    /*** IUnknown methods ***/
    ULONG   (*AddRef)(LPDIRECT3DSURFACE8 pThis);
    ULONG   (*Release)(LPDIRECT3DSURFACE8 pThis);

    /*** IDirect3DResource8 methods ***/
    D3DRESOURCETYPE (*GetType)(LPDIRECT3DSURFACE8 pThis);
    VOID            (*Register)(LPDIRECT3DSURFACE8 pThis, PVOID pBase);
    VOID            (*BlockUntilNotBusy)(LPDIRECT3DSURFACE8 pThis);
    BOOL            (*IsBusy)(LPDIRECT3DSURFACE8 pThis);

    /*** IDirect3DSurface8 methods ***/
    HRESULT         (*GetContainer)(LPDIRECT3DSURFACE8 pThis, 
                                    LPDIRECT3DBASETEXTURE8* ppContainer);
    HRESULT         (*GetDesc) (LPDIRECT3DSURFACE8 pThis, 
                                D3DSURFACE_DESC* pDesc);
    HRESULT         (*LockRect)(LPDIRECT3DSURFACE8 pThis, 
                                D3DLOCKED_RECT* pLockedRect, 
                                const RECT* pRect, DWORD Flags);
    HRESULT         (*UnlockRect)(LPDIRECT3DSURFACE8 pThis);
} IDirect3DSurfaceVtbl8, *LPDIRECT3DSURFACEVTBL8;

struct IDirect3DSurface8 INHERITS(IDirect3DResource8) {
    LPDIRECT3DSURFACEVTBL8 lpVtbl;
#ifdef __cplusplus
    virtual HRESULT GetContainer(LPDIRECT3DBASETEXTURE8* ppContainer) = 0;
    virtual HRESULT GetDesc(D3DSURFACE_DESC* pDesc) = 0;
    virtual HRESULT LockRect(D3DLOCKED_RECT* pLockedRect, 
                             const RECT* pRect, DWORD Flags) = 0;
    virtual HRESULT UnlockRect() = 0;
#endif // __cplusplus
};

ULONG IDirect3DSurface8_AddRef(LPDIRECT3DSURFACE8 pThis);
ULONG IDirect3DSurface8_Release(LPDIRECT3DSURFACE8 pThis);
D3DRESOURCETYPE IDirect3DSurface8_GetType(LPDIRECT3DSURFACE8 pThis);
VOID IDirect3DSurface8_Register(LPDIRECT3DSURFACE8 pThis, PVOID pBase);
HRESULT IDirect3DSurface8_GetContainer(LPDIRECT3DSURFACE8 pThis, 
                                       LPDIRECT3DBASETEXTURE8* ppContainer);
HRESULT IDirect3DSurface8_GetDesc(LPDIRECT3DSURFACE8 pThis, 
                                  D3DSURFACE_DESC* pDesc);
HRESULT IDirect3DSurface8_LockRect(LPDIRECT3DSURFACE8 pThis, 
                                   D3DLOCKED_RECT* pLockedRect, 
                                   const RECT* pRect, DWORD Flags);
HRESULT IDirect3DSurface8_UnlockRect(LPDIRECT3DSURFACE8 pThis);
// ============================================================================

// ============================================================================
struct IDirect3DVertexBuffer8;
typedef struct IDirect3DVertexBuffer8 IDirect3DVertexBuffer8, 
                                      *LPDIRECT3DVERTEXBUFFER8;

typedef struct _D3DVERTEXBUFFER_DESC {
    D3DFORMAT           Format;
    D3DRESOURCETYPE     Type;
} D3DVERTEXBUFFER_DESC;

typedef struct IDirect3DVertexBufferVtbl8 {
    /*** IUnknown methods ***/
    ULONG   (*AddRef)(LPDIRECT3DVERTEXBUFFER8 pThis);
    ULONG   (*Release)(LPDIRECT3DVERTEXBUFFER8 pThis);

    /*** IDirect3DResource8 methods ***/
    D3DRESOURCETYPE (*GetType)(LPDIRECT3DVERTEXBUFFER8 pThis);
    VOID            (*Register)(LPDIRECT3DVERTEXBUFFER8 pThis, PVOID pBase);
    VOID            (*BlockUntilNotBusy)(LPDIRECT3DVERTEXBUFFER8 pThis);
    BOOL            (*IsBusy)(LPDIRECT3DVERTEXBUFFER8 pThis);

    /*** IDirect3DVertexBuffer methods ***/
    HRESULT         (*GetDesc)(LPDIRECT3DVERTEXBUFFER8 pThis, 
                               D3DVERTEXBUFFER_DESC* pDesc);
    HRESULT         (*Lock)(LPDIRECT3DVERTEXBUFFER8 pThis, UINT OffsetToLock,
                            UINT SizeToLock, BYTE** ppbData, DWORD Flags);
    HRESULT         (*Unlock)(LPDIRECT3DVERTEXBUFFER8 pThis);
} IDirect3DVertexBufferVtbl8, *LPDIRECT3DVERTEXBUFFERVTBL8;

struct IDirect3DVertexBuffer8 INHERITS(IDirect3DResource8) {
    LPDIRECT3DVERTEXBUFFERVTBL8 lpVtbl;
#ifdef __cplusplus
    virtual HRESULT GetDesc(D3DVERTEXBUFFER_DESC* pDesc) = 0;
    virtual HRESULT Lock(UINT OffsetToLock, UINT SizeToLock, 
                         BYTE** ppbData, DWORD Flags) = 0;
    virtual HRESULT Unlock() = 0;
#endif // __cplusplus
};

ULONG IDirect3DVertexBuffer8_AddRef(LPDIRECT3DVERTEXBUFFER8 pThis);
ULONG IDirect3DVertexBuffer8_Release(LPDIRECT3DVERTEXBUFFER8 pThis);
D3DRESOURCETYPE IDirect3DVertexBuffer8_GetType(LPDIRECT3DVERTEXBUFFER8 pThis);
VOID IDirect3DVertexBuffer8_Register(LPDIRECT3DVERTEXBUFFER8 pThis, 
                                     PVOID pBase);
HRESULT IDirect3DVertexBuffer8_GetContainer(
    LPDIRECT3DVERTEXBUFFER8 pThis, LPDIRECT3DBASETEXTURE8* ppContainer);
HRESULT IDirect3DVertexBuffer8_GetDesc(LPDIRECT3DVERTEXBUFFER8 pThis, 
                                       D3DVERTEXBUFFER_DESC* pDesc);
HRESULT IDirect3DVertexBuffer8_Lock(LPDIRECT3DVERTEXBUFFER8 pThis, 
                                      UINT OffsetToLock, UINT SizeToLock,
                                      BYTE** ppbData, DWORD Flags);
HRESULT IDirect3DVertexBuffer8_Unlock(LPDIRECT3DVERTEXBUFFER8 pThis);
// ============================================================================

// ============================================================================
struct IDirect3DPushBuffer8;
typedef struct IDirect3DPushBuffer8 IDirect3DPushBuffer8, 
                                    *LPDIRECT3DPUSHBUFFER8;

typedef struct IDirect3DPushBufferVtbl8 {
    /*** IUnknown methods ***/
    ULONG   (*AddRef)(LPDIRECT3DPUSHBUFFER8 pThis);
    ULONG   (*Release)(LPDIRECT3DPUSHBUFFER8 pThis);

    /*** IDirect3DResource8 methods ***/
    D3DRESOURCETYPE (*GetType)(LPDIRECT3DPUSHBUFFER8 pThis);
    VOID            (*Register)(LPDIRECT3DPUSHBUFFER8 pThis, PVOID pBase);
    VOID            (*BlockUntilNotBusy)(LPDIRECT3DPUSHBUFFER8 pThis);
    BOOL            (*IsBusy)(LPDIRECT3DPUSHBUFFER8 pThis);

    /*** IDirect3DPushBuffer8 methods ***/
    HRESULT         (*GetSize)(LPDIRECT3DPUSHBUFFER8 pThis, UINT* pSize);
} IDirect3DPushBufferVtbl8, *LPDIRECT3DPUSHBUFFERVTBL8;

struct IDirect3DPushBuffer8 INHERITS(IDirect3DResource8) {
    LPDIRECT3DPUSHBUFFERVTBL8 lpVtbl;
#ifdef __cplusplus
    virtual DWORD GetSize() = 0;
#endif // __cplusplus
};

ULONG IDirect3DPushBuffer8_AddRef(LPDIRECT3DPUSHBUFFER8 pThis);
ULONG IDirect3DPushBuffer8_Release(LPDIRECT3DPUSHBUFFER8 pThis);
D3DRESOURCETYPE IDirect3DPushBuffer8_GetType(LPDIRECT3DPUSHBUFFER8 pThis);
VOID IDirect3DPushBuffer8_Register(LPDIRECT3DPUSHBUFFER8 pThis, PVOID pBase);
VOID IDirect3DPushBuffer8_BlockUntilNotBusy(LPDIRECT3DPUSHBUFFER8 pThis);
BOOL IDirect3DPushBuffer8_IsBusy(LPDIRECT3DPUSHBUFFER8 pThis);
HRESULT IDirect3DPushBuffer8_GetSize(LPDIRECT3DPUSHBUFFER8 pThis, UINT* pSize);
// ============================================================================

// ============================================================================
struct IDirect3DDevice8;
typedef struct IDirect3DDevice8 IDirect3DDevice8, *LPDIRECT3DDEVICE8;

typedef struct IDirect3DDeviceVtbl8 {
    /*** IUnknown methods ***/
    ULONG   (*AddRef)(LPDIRECT3DDEVICE8 pThis);
    ULONG   (*Release)(LPDIRECT3DDEVICE8 pThis);

    /*** IDirect3DDevice8 methods ***/
    VOID    (*BlockUntilVerticalBlank)(LPDIRECT3DDEVICE8 pThis);
    HRESULT (*CreateImageSurface)(LPDIRECT3DDEVICE8 pThis, UINT Width, 
                                  UINT Height, D3DFORMAT Format, 
                                  LPDIRECT3DSURFACE8* ppSurface);
    HRESULT (*CreateDepthStencilSurface)(LPDIRECT3DDEVICE8 pThis, UINT Width, 
                                         UINT Height, D3DFORMAT Format, 
                                         D3DMULTISAMPLE_TYPE MultiSampleType,
                                         LPDIRECT3DSURFACE8* ppSurface);
    HRESULT (*CreateVertexBuffer)(
        LPDIRECT3DDEVICE8 pThis, UINT Length, DWORD Usage, DWORD FVF, 
        D3DPOOL Pool, LPDIRECT3DVERTEXBUFFER8* ppVertexBuffer);
     HRESULT (*CreatePushBuffer)(
        LPDIRECT3DDEVICE8 pThis, UINT Size, 
        BOOL RunUsingCpuCopy, LPDIRECT3DPUSHBUFFER8* ppPushBuffer);
    HRESULT (*CreateTexture)(LPDIRECT3DDEVICE8 pThis,
                             UINT Width,
                             UINT Height,
                             UINT Levels,
                             DWORD Usage,
                             D3DFORMAT Format,
                             D3DPOOL Pool,
                             LPDIRECT3DTEXTURE8* ppTexture);
    HRESULT (*CreateCubeTexture)(LPDIRECT3DDEVICE8 pThis, 
                                 UINT EdgeLength, 
                                 UINT Levels, 
                                 DWORD Usage, 
                                 D3DFORMAT Format, 
                                 D3DPOOL Pool,
                                 LPDIRECT3DCUBETEXTURE8* ppCubeTexture);
    HRESULT (*BeginScene)(LPDIRECT3DDEVICE8 pThis);
    HRESULT (*EndScene)(LPDIRECT3DDEVICE8 pThis);
    HRESULT (*Present)(LPDIRECT3DDEVICE8 pThis, 
                       CONST RECT* pSourceRect, 
                       CONST RECT* pDestRect);
    HRESULT (*DrawPrimitive)(LPDIRECT3DDEVICE8 pThis, 
                             D3DPRIMITIVETYPE PrimitiveType, 
                             UINT StartVertex, UINT PrimitiveCount);
    HRESULT (*DrawVertices)(LPDIRECT3DDEVICE8 pThis, 
                            D3DPRIMITIVETYPE PrimitiveType, 
                            UINT StartVertex, UINT VertexCount);
    HRESULT (*SetViewport)(LPDIRECT3DDEVICE8 pThis, 
                           CONST D3DVIEWPORT8* pViewport);
    HRESULT (*SetVertexShaderInputDirect)(
                           LPDIRECT3DDEVICE8 pThis,
                           D3DVERTEXATTRIBUTEFORMAT *pVAF,
                           UINT StreamCount, 
                           D3DSTREAM_INPUT *pStreamInputs);
    HRESULT (*SetTextureStageState)(LPDIRECT3DDEVICE8 pThis,
                                 DWORD Stage,
                                 D3DTEXTURESTAGESTATETYPE Type,
                                 DWORD Value);
    HRESULT (*SetRenderState)(LPDIRECT3DDEVICE8 pThis, 
                              D3DRENDERSTATETYPE Type, DWORD VALUE);
    HRESULT (*LoadVertexShaderProgram)(
        LPDIRECT3DDEVICE8 pThis, CONST DWORD *pFunction, DWORD Address);
    HRESULT (*SetPixelShaderProgram)(LPDIRECT3DDEVICE8 pThis, 
                                     CONST D3DPIXELSHADERDEF *pPSDef);
    HRESULT (*Clear)(LPDIRECT3DDEVICE8 pThis, DWORD Count, 
                     CONST D3DRECT* pRects, DWORD Flags, D3DCOLOR Color, 
                     float Z, DWORD Stencil);
    HRESULT (*SetScissors)(LPDIRECT3DDEVICE8 pThis, DWORD Count, 
                           BOOL Exclusive, CONST D3DRECT *pRects);
    HRESULT (*SetTile)(LPDIRECT3DDEVICE8 pThis, 
                       DWORD Index, CONST D3DTILE* pTile);
    HRESULT (*SetTexture)(LPDIRECT3DDEVICE8 pThis, DWORD Stage, 
                          LPDIRECT3DBASETEXTURE8 pTexture);
} IDirect3DDeviceVtbl8, *LPDIRECT3DDEVICEVTBL8;

struct IDirect3DDevice8 INHERITS(IUnknown) {
    LPDIRECT3DDEVICEVTBL8 lpVtbl;
#ifdef __cplusplus
    virtual HRESULT CreateImageSurface(UINT Width, UINT Height,
                                       D3DFORMAT Format, 
                                       LPDIRECT3DSURFACE8* ppSurface) = 0;
    virtual HRESULT CreateDepthStencilSurface(
        UINT Width, UINT Height, D3DFORMAT Format, 
        D3DMULTISAMPLE_TYPE MultiSampleType, 
        LPDIRECT3DSURFACE8* ppSurface) = 0;
    virtual HRESULT CreateVertexBuffer(
        UINT Length, DWORD Usage, DWORD FVF, 
        D3DPOOL Pool, LPDIRECT3DVERTEXBUFFER8* ppVertexBuffer) = 0;
    virtual HRESULT BeginScene() = 0;
    virtual HRESULT EndScene() = 0;
    virtual HRESULT Present(CONST RECT* pSourceRect, 
                            CONST RECT* pDestRect) = 0;
    virtual HRESULT DrawVertices(D3DPRIMITIVETYPE PrimitiveType, 
                                 UINT StartVertex, UINT VertexCount) = 0;
    virtual HRESULT DrawPrimitive(D3DPRIMITIVETYPE PrimitiveType, 
                                  UINT StartVertex, UINT PrimitiveCount) = 0;
    virtual HRESULT SetViewport(CONST D3DVIEWPORT8* pViewport) = 0;
    virtual HRESULT SetVertexShaderInputDirect(
        D3DVERTEXATTRIBUTEFORMAT *pVAF,
        UINT StreamCount, 
        D3DSTREAM_INPUT *pStreamInputs) = 0;
    virtual HRESULT CreateTexture(
        UINT Width,
        UINT Height,
        UINT Levels,
        DWORD Usage,
        D3DFORMAT Format,
        D3DPOOL Pool,
        LPDIRECT3DTEXTURE8* ppTexture) = 0;
    virtual HRESULT CreateCubeTexture(
        UINT EdgeLength, 
        UINT Levels, 
        DWORD Usage, 
        D3DFORMAT Format, 
        D3DPOOL Pool,
        LPDIRECT3DCUBETEXTURE8* ppCubeTexture) = 0;
    virtual HRESULT SetTextureStageState(
        DWORD Stage,
        D3DTEXTURESTAGESTATETYPE Type,
        DWORD Value) = 0;
    virtual HRESULT SetRenderState(D3DRENDERSTATETYPE Type, DWORD Value) = 0;
    virtual HRESULT SetPixelShaderProgram(CONST D3DPIXELSHADERDEF *pPSDef) = 0;
    virtual HRESULT LoadVertexShaderProgram(
        CONST DWORD *pFunction, DWORD Address) = 0;
    virtual HRESULT SetPixelShaderProgram(
        CONST D3DPIXELSHADERDEF *pPSDef) = 0;
    virtual HRESULT Clear(DWORD Count, CONST D3DRECT* pRects, DWORD Flags, 
                          D3DCOLOR Color, float Z, DWORD Stencil) = 0;
    virtual HRESULT SetScissors(DWORD Count, BOOL Exclusive, 
                                CONST D3DRECT *pRects) = 0;
    virtual HRESULT SetTile(DWORD Index, CONST D3DTILE* pTile) = 0;
    virtual HRESULT SetTexture(DWORD Stage, 
                               LPDIRECT3DBASETEXTURE8 pTexture) = 0;
#endif // __cplusplus
};

ULONG IDirect3DDevice8_AddRef(LPDIRECT3DDEVICE8 pThis);
ULONG IDirect3DDevice8_Release(LPDIRECT3DDEVICE8 pThis);
HRESULT IDirect3DDevice8_CreateImageSurface(LPDIRECT3DDEVICE8 pThis, 
                                            UINT Width, UINT Height, 
                                            D3DFORMAT Format, 
                                            LPDIRECT3DSURFACE8* ppSurface);
HRESULT IDirect3DDevice8_CreateDepthStencilSurface(
    LPDIRECT3DDEVICE8 pThis, UINT Width, UINT Height, D3DFORMAT Format,
    D3DMULTISAMPLE_TYPE MultiSample, IDirect3DSurface8** ppSurface);
HRESULT IDirect3DDevice8_CreateVertexBuffer(
    LPDIRECT3DDEVICE8 pThis, UINT Length, DWORD Usage, DWORD FVF, 
    D3DPOOL Pool, LPDIRECT3DVERTEXBUFFER8* ppVertexBuffer);
HRESULT IDirect3DDevice8_CreatePushBuffer(
    LPDIRECT3DDEVICE8 pThis, UINT Size, 
    BOOL RunUsingCpuCopy, LPDIRECT3DPUSHBUFFER8* ppPushBuffer);
HRESULT IDirect3DDevice8_BeginScene(LPDIRECT3DDEVICE8 pThis);
HRESULT IDirect3DDevice8_EndScene(LPDIRECT3DDEVICE8 pThis);
HRESULT IDirect3DDevice8_Present(LPDIRECT3DDEVICE8 pThis, 
                                 CONST RECT* pSourceRect, 
                                 CONST RECT* pDestRect);
HRESULT IDirect3DDevice8_DrawPrimitive(LPDIRECT3DDEVICE8 pThis, 
                                       D3DPRIMITIVETYPE PrimitiveType, 
                                       UINT StartVertex, UINT PrimitiveCount);
HRESULT IDirect3DDevice8_DrawVertices(LPDIRECT3DDEVICE8 pThis, 
                                      D3DPRIMITIVETYPE PrimitiveType, 
                                      UINT StartVertex, UINT VertexCount);
HRESULT IDirect3DDevice8_SetViewport(LPDIRECT3DDEVICE8 pThis, 
                                     CONST D3DVIEWPORT8* pViewport);
HRESULT IDirect3DDevice8_SetVertexShaderInputDirect(
    LPDIRECT3DDEVICE8 pThis,
    D3DVERTEXATTRIBUTEFORMAT *pVAF,
    UINT StreamCount, 
    D3DSTREAM_INPUT *pStreamInputs);
HRESULT IDirect3DDevice8_CreateTexture(LPDIRECT3DDEVICE8 pThis,
                                        UINT Width,
                                        UINT Height,
                                        UINT Levels,
                                        DWORD Usage,
                                        D3DFORMAT Format,
                                        D3DPOOL Pool,
                                        LPDIRECT3DTEXTURE8* ppTexture);
HRESULT IDirect3DDevice8_CreateCubeTexture(
    LPDIRECT3DDEVICE8 pThis, UINT EdgeLength, UINT Levels, DWORD Usage, 
    D3DFORMAT Format, D3DPOOL Pool, LPDIRECT3DCUBETEXTURE8* ppCubeTexture);
HRESULT IDirect3DDevice8_SetTextureStageState(
    LPDIRECT3DDEVICE8 pThis, DWORD Stage, 
    D3DTEXTURESTAGESTATETYPE Type, DWORD Value);
HRESULT IDirect3DDevice8_SetRenderState(
    LPDIRECT3DDEVICE8 pThis, D3DRENDERSTATETYPE Type, DWORD Value);
HRESULT IDirect3DDevice8_LoadVertexShaderProgram(
    LPDIRECT3DDEVICE8 pThis, CONST DWORD *pFunction, DWORD Address);
HRESULT IDirect3DDevice8_SetPixelShaderProgram(
    LPDIRECT3DDEVICE8 pThis, CONST D3DPIXELSHADERDEF *pPSDef);
HRESULT IDirect3DDevice8_Clear(LPDIRECT3DDEVICE8 pThis, DWORD Count, 
                               CONST D3DRECT* pRects, DWORD Flags,
                               D3DCOLOR Color, float Z, DWORD Stencil);
HRESULT IDirect3DDevice8_SetScissors(LPDIRECT3DDEVICE8 pThis, DWORD Count, 
                                     BOOL Exclusive, CONST D3DRECT *pRects);
HRESULT D3DDevice_SetRenderState(D3DRENDERSTATETYPE Type, DWORD Value);
HRESULT IDirect3DDevice8_SetTile(LPDIRECT3DDEVICE8 pThis, DWORD Index, 
                                 CONST D3DTILE* pTile);
HRESULT IDirect3DDevice8_SetTexture(LPDIRECT3DDEVICE8 pThis, DWORD Stage, 
                                    LPDIRECT3DBASETEXTURE8 pTexture);

struct IDirect3D8;
typedef struct IDirect3D8 IDirect3D8, *LPDIRECT3D8;

typedef struct IDirect3DVtbl8 {
    /*** IUnknown methods ***/
    ULONG   (*AddRef)(LPDIRECT3D8 pThis);
    ULONG   (*Release)(LPDIRECT3D8 pThis);

    /*** IDirect3D8 methods ***/
    UINT     (*GetAdapterCount)(LPDIRECT3D8 pThis);
    HRESULT  (*GetAdapterIdentifier)(LPDIRECT3D8 pThis, UINT Adapter, DWORD Flags, 
                                     D3DADAPTER_IDENTIFIER8* pIdentifier);
    UINT     (*GetAdapterModeCount)(LPDIRECT3D8 pThis, UINT Adapter);
    HRESULT  (*EnumAdapterModes)(LPDIRECT3D8 pThis, UINT Adapter, UINT Mode, 
                                 D3DDISPLAYMODE* pMode);
    HRESULT  (*GetAdapterDisplayMode)(LPDIRECT3D8 pThis, UINT Adapter, 
                                      D3DDISPLAYMODE* pMode);
    HRESULT  (*CheckDeviceType)(LPDIRECT3D8 pThis, UINT Adapter, 
                                D3DDEVTYPE CheckType, D3DFORMAT DisplayFormat, 
                                D3DFORMAT BackBufferFormat, BOOL Windowed);
    HRESULT  (*CheckDeviceFormat)(LPDIRECT3D8 pThis, UINT Adapter, 
                                  D3DDEVTYPE DeviceType, 
                                  D3DFORMAT AdapterFormat, DWORD Usage, 
                                  D3DRESOURCETYPE RType, 
                                  D3DFORMAT CheckFormat);
    HRESULT  (*CheckDeviceMultiSampleType)(LPDIRECT3D8 pThis, 
        UINT Adapter, D3DDEVTYPE DeviceType, 
        D3DFORMAT SurfaceFormat, BOOL Windowed, 
        D3DMULTISAMPLE_TYPE MultiSampleType);
    HRESULT  (*CheckDepthStencilMatch)(LPDIRECT3D8 pThis, UINT Adapter, 
                                       D3DDEVTYPE DeviceType, 
                                       D3DFORMAT AdapterFormat, 
                                       D3DFORMAT RenderTargetFormat, 
                                       D3DFORMAT DepthStencilFormat);
    HRESULT  (*GetDeviceCaps)(LPDIRECT3D8 pThis, UINT Adapter,
                              D3DDEVTYPE DeviceType, D3DCAPS8* pCaps);
    HMONITOR (*GetAdapterMonitor)(LPDIRECT3D8 pThis, UINT Adapter);
    HRESULT  (*SetPushBufferSize)(LPDIRECT3D8 pThis, DWORD PushBufferSize, 
                                  DWORD KickOffSize);
    HRESULT  (*CreateDevice)(LPDIRECT3D8 pThis, UINT Adapter, 
                             D3DDEVTYPE DeviceType, HWND hFocusWindow, 
                             DWORD BehaviorFlags, 
                             D3DPRESENT_PARAMETERS* pPresentationParameters, 
                             LPDIRECT3DDEVICE8* ppReturnedDeviceInterface);
} IDirect3DVtbl8, *LPDIRECT3DVTBL8;

struct IDirect3D8 INHERITS(IUnknown) {
    LPDIRECT3DVTBL8 lpVtbl;
#ifdef __cplusplus
    virtual UINT    GetAdapterCount() = 0;
    virtual HRESULT GetAdapterIdentifier(
        UINT Adapter, DWORD Flags, D3DADAPTER_IDENTIFIER8* pIdentifier) = 0;
    virtual UINT GetAdapterModeCount(UINT Adapter) = 0;
    virtual HRESULT EnumAdapterModes(UINT Adapter, UINT Mode, 
                                     D3DDISPLAYMODE* pMode) = 0;
    virtual HRESULT GetAdapterDisplayMode(UINT Adapter, 
                                          D3DDISPLAYMODE* pMode) = 0;
    virtual HRESULT CheckDeviceType(UINT Adapter, D3DDEVTYPE CheckType, 
                                    D3DFORMAT DisplayFormat, 
                                    D3DFORMAT BackBufferFormat, 
                                    BOOL Windowed) = 0;
    virtual HRESULT CheckDeviceFormat(UINT Adapter, D3DDEVTYPE DeviceType, 
                                      D3DFORMAT AdapterFormat, DWORD Usage, 
                                      D3DRESOURCETYPE RType, 
                                      D3DFORMAT CheckFormat) = 0;
    virtual HRESULT CheckDeviceMultiSampleType(
        UINT Adapter, D3DDEVTYPE DeviceType, 
        D3DFORMAT SurfaceFormat, BOOL Windowed, 
        D3DMULTISAMPLE_TYPE MultiSampleType) = 0;
    virtual HRESULT CheckDepthStencilMatch(UINT Adapter, D3DDEVTYPE DeviceType,
                                           D3DFORMAT AdapterFormat, 
                                           D3DFORMAT RenderTargetFormat, 
                                           D3DFORMAT DepthStencilFormat) = 0;
    virtual HRESULT GetDeviceCaps(UINT Adapter, D3DDEVTYPE DeviceType, 
                                  D3DCAPS8* pCaps) = 0;
    virtual HMONITOR GetAdapterMonitor(UINT Adapter) = 0;
    virtual HRESULT  SetPushBufferSize(DWORD PushBufferSize, DWORD KickOffSize) = 0;
    virtual HRESULT CreateDevice(
        UINT Adapter, D3DDEVTYPE DeviceType, HWND hFocusWindow, 
        DWORD BehaviorFlags, D3DPRESENT_PARAMETERS* pPresentationParameters, 
        LPDIRECT3DDEVICE8* ppReturnedDeviceInterface) = 0;
#endif // __cplusplus
};

ULONG    IDirect3D8_AddRef(LPDIRECT3D8 pThis);
ULONG    IDirect3D8_Release(LPDIRECT3D8 pThis);
UINT     IDirect3D8_GetAdapterCount(LPDIRECT3D8 pThis);
HMONITOR IDirect3D8_GetAdapterMonitor(LPDIRECT3D8 pThis, UINT Adapter);
HRESULT  IDirect3D8_CheckDeviceType(LPDIRECT3D8 pThis, UINT Adapter, 
                                    D3DDEVTYPE CheckType, 
                                    D3DFORMAT DisplayFormat,
                                    D3DFORMAT BackBufferFormat, BOOL Windowed);
UINT     IDirect3D8_GetAdapterModeCount(LPDIRECT3D8 pThis, UINT Adapter);
HRESULT  IDirect3D8_EnumAdapterModes(LPDIRECT3D8 pThis, UINT Adapter, 
                                     UINT Mode, D3DDISPLAYMODE* pMode);
HRESULT  IDirect3D8_GetAdapterDisplayMode(LPDIRECT3D8 pThis, UINT Adapter, 
                                         D3DDISPLAYMODE* pMode);                       
HRESULT  IDirect3D8_SetPushBufferSize(LPDIRECT3D8 pThis, 
                                      DWORD PushBufferSize, 
                                      DWORD KickOffSize);   
HRESULT IDirect3D8_CreateDevice(
    LPDIRECT3D8 pThis, UINT Adapter, D3DDEVTYPE DeviceType, 
    HWND hFocusWindow, DWORD BehaviorFlags, 
    D3DPRESENT_PARAMETERS* pPresentationParameters,
    LPDIRECT3DDEVICE8* ppReturnedDevice);    

#define D3D_SDK_VERSION 120
PVOID D3D_AllocContiguousMemory(DWORD Size, DWORD Alignment);
void  D3D_FreeContiguousMemory(PVOID Base);
LPDIRECT3D8 Direct3DCreate8(UINT SKDVersion);

#endif // NXDK_HAVE_D3D8