// This file contains the internal definition of Direct3D and its interface,
// as well as macros intended to be used throughout the D3D implementation, and
// utility functions not strictly related any specific D3D object.
//
// Anything contained herein is intended to be private to the outside world,
// but public within the D3D implementation. Some of these functions will be
// "exported" via Direct3D's vtable, but their declarations or definitions 
// need not be exposed to facilitate that.
//
// The general design pattern of this implementation is that all IDirect3D*8
// interfaces are implemented by a D3D* object (e.g. IDirect3DDevice8 is 
// implemented by D3DDevice, etc.), with C-friendly (e.g. the IDirect3D8_* 
// functions defined below) and C++-friendly (e.g. IDirect3D8's C++ member 
// functions) wrappers that call into the object's vtable, where pointers
// to the actual implementation functions are placed on object creation.
//
// Direct3D and D3DDevice are unique in that they act as singleton objects 
// since it's never valid for more than one of each of those to exist at a 
// time, and as such their implementations are a bit messier.

#pragma once

#include "d3d8.h"

#include <stdlib.h>

#define MASK(mask, val) (((val) << (__builtin_ffs(mask)-1)) & (mask))

#define D3D_FALLTHROUGH() [[fallthrough]]
#define D3D_CALL(...) ({      \
    HRESULT hr = __VA_ARGS__; \
    if(FAILED(hr)) return hr; \
    hr;})                     \

#define memcpyp(dest, src) ({                                                 \
    _Static_assert(sizeof(*(src)) == sizeof(*dest),                           \
                   "memcpyp: mismatch in operand size");                      \
    memcpy((&*(dest)), (&*(src)), sizeof((*(dest)))); })

#define memsetp(p, c)     ({ memset((&*(p)), (c), sizeof((*(p)))); })
#define memzro (p, c, n)  ({ memset((p),     (c), (n)); })
#define memzrop(p) memsetp(p, 0)

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

int D3D_FormatBPP(D3DFORMAT fmt);
int D3D_FormatBytesPerPixel(D3DFORMAT fmt);

HRESULT Direct3D_ResetDevice(D3DPRESENT_PARAMETERS* pPresentationParameters);

typedef struct Direct3D IMPLEMENTS(IDirect3D8) {
#ifndef __cplusplus
    IDirect3D8      iface;
#else
    // IUnknown methods
    virtual D3DAPI ULONG AddRef() override;
    virtual D3DAPI ULONG Release() override;
    // IDirect3D8 methods
    virtual D3DAPI UINT GetAdapterCount() override;
    virtual D3DAPI HRESULT GetAdapterIdentifier(
        UINT Adapter, DWORD Flags, 
        D3DADAPTER_IDENTIFIER8* pIdentifier) override;
    virtual D3DAPI UINT GetAdapterModeCount(UINT Adapter) override;
    virtual D3DAPI HRESULT EnumAdapterModes(
        UINT Adapter, UINT Mode, D3DDISPLAYMODE* pMode) override;
    virtual D3DAPI HRESULT GetAdapterDisplayMode(
        UINT Adapter, D3DDISPLAYMODE* pMode) override;
    virtual D3DAPI HRESULT CheckDeviceType(
        UINT Adapter, D3DDEVTYPE CheckType, D3DFORMAT DisplayFormat,
        D3DFORMAT BackBufferFormat, BOOL Windowed) override;
    virtual D3DAPI HRESULT CheckDeviceFormat(
        UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT AdapterFormat, 
        DWORD Usage, D3DRESOURCETYPE RType, D3DFORMAT CheckFormat) override;
    virtual D3DAPI HRESULT CheckDeviceMultiSampleType(
        UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT SurfaceFormat,
        BOOL Windowed, D3DMULTISAMPLE_TYPE MultiSampleType) override;
    virtual D3DAPI HRESULT CheckDepthStencilMatch(
        UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT AdapterFormat,
        D3DFORMAT RenderTargetFormat, D3DFORMAT DepthStencilFormat) override;
    virtual D3DAPI HRESULT GetDeviceCaps(
        UINT Adapter, D3DDEVTYPE DeviceType, D3DCAPS8* pCaps) override;
    virtual D3DAPI HMONITOR GetAdapterMonitor(
        UINT Adapter) override;
    virtual D3DAPI HRESULT SetPushBufferSize(
        DWORD PushBufferSize, DWORD KickOffSize) override;
    virtual D3DAPI HRESULT CreateDevice(
        UINT Adapter, D3DDEVTYPE DeviceType, HWND hFocusWindow, 
        DWORD BehaviorFlags, D3DPRESENT_PARAMETERS* pPresentationParameters,
        LPDIRECT3DDEVICE8* ppReturnedDeviceInterface) override;
#endif // __cplusplus
    DWORD           KickOffSize;
    DWORD           PushBufferSize;
    D3DDISPLAYMODE  DisplayModes[51];
    UINT            DisplayModeCount;
    UINT            CurrentDisplayMode;
} Direct3D;

D3DAPI ULONG    Direct3D8_AddRef(LPDIRECT3D8 pThis);
D3DAPI ULONG    Direct3D8_Release(LPDIRECT3D8 pThis);
D3DAPI UINT     Direct3D8_GetAdapterCount(LPDIRECT3D8 pThis);
D3DAPI HMONITOR Direct3D8_GetAdapterMonitor(LPDIRECT3D8 pThis, UINT Adapter);
D3DAPI HRESULT  Direct3D8_CheckDeviceType(LPDIRECT3D8 pThis, UINT Adapter, 
                                    D3DDEVTYPE CheckType, 
                                    D3DFORMAT DisplayFormat,
                                    D3DFORMAT BackBufferFormat, BOOL Windowed);
D3DAPI HRESULT Direct3D8_CheckDepthStencilMatch(LPDIRECT3D8 pThis, 
                                                 UINT Adapter,
                                                 D3DDEVTYPE DeviceType, 
                                                 D3DFORMAT AdapterFormat, 
                                                 D3DFORMAT RenderTargetFormat, 
                                                 D3DFORMAT DepthStencilFormat);
D3DAPI UINT  Direct3D8_GetAdapterModeCount(LPDIRECT3D8 pThis, UINT Adapter);
D3DAPI HRESULT  Direct3D8_EnumAdapterModes(LPDIRECT3D8 pThis, UINT Adapter, 
                                            UINT Mode, D3DDISPLAYMODE* pMode);
D3DAPI HRESULT  Direct3D8_GetAdapterDisplayMode(LPDIRECT3D8 pThis, 
                                                 UINT Adapter, 
                                                 D3DDISPLAYMODE* pMode);                       
D3DAPI HRESULT  Direct3D8_SetPushBufferSize(LPDIRECT3D8 pThis, 
                                             DWORD PushBufferSize, 
                                             DWORD KickOffSize);   
D3DAPI HRESULT Direct3D8_CreateDevice(
    LPDIRECT3D8 pThis, UINT Adapter, D3DDEVTYPE DeviceType, 
    HWND hFocusWindow, DWORD BehaviorFlags, 
    D3DPRESENT_PARAMETERS* pPresentationParameters,
    LPDIRECT3DDEVICE8* ppReturnedDevice);   
D3DAPI HRESULT Direct3D8_GetDeviceCaps(LPDIRECT3D8 pThis, 
                                        UINT Adapter, 
                                        D3DDEVTYPE DeviceType, 
                                        D3DCAPS8* pCaps);

#ifdef __cplusplus
}
#endif // __cplusplus
