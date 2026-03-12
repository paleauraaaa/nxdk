// Even though the actual bodies of these member functions are all implemented
// on the C side (Direct3D8_*), and the vtables are manually set up on 
// the C side, we have to define them in C++ too to prevent abstract base 
// class compile errors.
//
// For now, these functions should do nothing besides call the appropriate 
// Direct3D8_* function. In theory, they should maybe even throw an 
// assertion in debug mode, since they should never actually be called 
// (constructing the vtables in C will overwrite them.) 
//
// The C wrapper functions (IDirect3D8_*) are defined to allow for 
// ergonomic calls to the vtable functions from C and C++. They should always
// call the vtable's funnction (pThis->*), not the implementation function 
// (Direct3D8_*), so that if the vtable function is overriden, that override 
// is honored.
//
// This file's name must have the trailing underscore (or some name that 
// doesn't conflict with d3d8.c) to prevent compilation of one file
// overriding the object file of the other source file (i.e. d3d8.c 
// will get compiled to d3d8.obj, but so would d3d8.cpp).

#include "d3d8_private.h"

D3DAPI ULONG Direct3D::AddRef() {
    return Direct3D8_AddRef(this);
}

D3DAPI ULONG IDirect3D8_AddRef(LPDIRECT3D8 pThis) {
    return pThis->AddRef();
}

D3DAPI ULONG Direct3D::Release() {
    return Direct3D8_Release(this);
}

D3DAPI ULONG IDirect3D8_Release(LPDIRECT3D8 pThis) {
    return pThis->Release();
}

D3DAPI UINT Direct3D::GetAdapterCount() { 
    return Direct3D8_GetAdapterCount(this); 
}

D3DAPI UINT IDirect3D8_GetAdapterCount(LPDIRECT3D8 pThis) {
    return pThis->GetAdapterCount();
}

D3DAPI UINT Direct3D::GetAdapterModeCount(UINT Adapter) { 
    return Direct3D8_GetAdapterModeCount(this, Adapter); 
}

D3DAPI UINT IDirect3D8_GetAdapterModeCount(LPDIRECT3D8 pThis, UINT Adapter) {
    return pThis->GetAdapterModeCount(Adapter);
}

D3DAPI HRESULT Direct3D::EnumAdapterModes(UINT Adapter, UINT Mode,
                                                     D3DDISPLAYMODE* pMode)
{ 
    return Direct3D8_EnumAdapterModes(this, Adapter, Mode, pMode);
}

D3DAPI HRESULT IDirect3D8_EnumAdapterModes(LPDIRECT3D8 pThis, UINT Adapter, 
                                           UINT Mode, D3DDISPLAYMODE* pMode) 
{
    return pThis->EnumAdapterModes(Adapter, Mode, pMode);
}

D3DAPI HRESULT Direct3D::GetAdapterDisplayMode(
    UINT Adapter, D3DDISPLAYMODE* pMode)
{
    return Direct3D8_GetAdapterDisplayMode(this, Adapter, pMode);
}

D3DAPI HRESULT IDirect3D8_GetAdapterDisplayMode(LPDIRECT3D8 pThis, UINT Adapter, 
                                         D3DDISPLAYMODE* pMode)
{
    return pThis->GetAdapterDisplayMode(Adapter, pMode);
}

D3DAPI HRESULT Direct3D::CheckDeviceType(UINT Adapter, 
                                                    D3DDEVTYPE CheckType,
                                                    D3DFORMAT DisplayFormat,
                                                    D3DFORMAT BackBufferFormat,
                                                    BOOL Windowed)
{ 
   return Direct3D8_CheckDeviceType(this, Adapter, CheckType, DisplayFormat,
                                   BackBufferFormat, Windowed); 
}

D3DAPI HRESULT IDirect3D8_CheckDeviceType(LPDIRECT3D8 pThis, UINT Adapter, 
                                   D3DDEVTYPE CheckType, 
                                   D3DFORMAT DisplayFormat,
                                   D3DFORMAT BackBufferFormat, BOOL Windowed)
{
    return pThis->CheckDeviceType(Adapter, CheckType, DisplayFormat,
                                     BackBufferFormat, Windowed);
}

D3DAPI HRESULT Direct3D::CheckDepthStencilMatch(
    UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT AdapterFormat, 
    D3DFORMAT RenderTargetFormat, D3DFORMAT DepthStencilFormat) 
{
    return Direct3D8_CheckDepthStencilMatch(this, Adapter, DeviceType, 
                                           AdapterFormat,
                                           RenderTargetFormat, 
                                           DepthStencilFormat); 
}

D3DAPI HRESULT IDirect3D8_CheckDepthStencilMatch(LPDIRECT3D8 pThis, UINT Adapter, 
                                          D3DDEVTYPE DeviceType, 
                                          D3DFORMAT AdapterFormat, 
                                          D3DFORMAT RenderTargetFormat, 
                                          D3DFORMAT DepthStencilFormat)
{
    return pThis->CheckDepthStencilMatch(Adapter, DeviceType, AdapterFormat,
                                            RenderTargetFormat, DepthStencilFormat);
}

D3DAPI HRESULT Direct3D::GetDeviceCaps(UINT Adapter, 
                                              D3DDEVTYPE DeviceType, 
                                              D3DCAPS8* pCaps) 
{ 
    return Direct3D8_GetDeviceCaps(this, Adapter, DeviceType, pCaps);
}

D3DAPI HRESULT IDirect3D8_GetDeviceCaps(LPDIRECT3D8 pThis, UINT Adapter, 
                                        D3DDEVTYPE DeviceType, 
                                        D3DCAPS8* pCaps)
{
    return pThis->GetDeviceCaps(Adapter, DeviceType, pCaps);
}

D3DAPI HMONITOR Direct3D::GetAdapterMonitor(UINT Adapter) {
    return Direct3D8_GetAdapterMonitor(this, Adapter);
}

D3DAPI HMONITOR IDirect3D8_GetAdapterMonitor(LPDIRECT3D8 pThis, UINT Adapter) {
    return pThis->GetAdapterMonitor(Adapter);
}

D3DAPI HRESULT Direct3D::CreateDevice(
    UINT Adapter, D3DDEVTYPE DeviceType, HWND hFocusWindow, 
    DWORD BehaviorFlags, D3DPRESENT_PARAMETERS* pPresentationParameters, 
    LPDIRECT3DDEVICE8* ppReturnedDeviceInterface) 
{ 
    return Direct3D8_CreateDevice(this, Adapter, DeviceType,
                                 hFocusWindow, BehaviorFlags, 
                                 pPresentationParameters,
                                 ppReturnedDeviceInterface); 
}

D3DAPI HRESULT IDirect3D8_CreateDevice(
    LPDIRECT3D8 pThis, UINT Adapter, D3DDEVTYPE DeviceType, 
    HWND hFocusWindow, DWORD BehaviorFlags, 
    D3DPRESENT_PARAMETERS* pPresentationParameters,
    LPDIRECT3DDEVICE8* ppReturnedDeviceInterface) 
{
    return pThis->CreateDevice(Adapter, DeviceType, hFocusWindow,
                               BehaviorFlags, pPresentationParameters,
                               ppReturnedDeviceInterface);
}
