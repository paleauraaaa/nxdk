/*
 * This sample provides a very basic demonstration of 3D rendering on the Xbox,
 * using pbkit. Based on the pbkit demo sources.
 */
#include <assert.h>
#include <hal/video.h>
#include <hal/xbox.h>
#include <math.h>
#include <pbkit/pbkit.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <xboxkrnl/xboxkrnl.h>
#include <hal/debug.h>
#include <windows.h>
#include <d3d8.h>
#include <stdarg.h>

static uint32_t  num_vertices;
static float     m_viewport[4][4];

typedef struct {
    float pos[3];
    float color[3];
} __attribute__((packed)) ColoredVertex;

static const ColoredVertex verts[] = {
    //  X     Y     Z       R     G     B
    {{-1.0, -1.0,  1.0}, { 0.1,  0.1,  0.6}}, /* Background triangle 1 */
    {{-1.0,  1.0,  1.0}, { 0.0,  0.0,  0.0}},
    {{ 1.0,  1.0,  1.0}, { 0.0,  0.0,  0.0}},
    {{-1.0, -1.0,  1.0}, { 0.1,  0.1,  0.6}}, /* Background triangle 2 */
    {{ 1.0,  1.0,  1.0}, { 0.0,  0.0,  0.0}},
    {{ 1.0, -1.0,  1.0}, { 0.1,  0.1,  0.6}},
    {{-1.0, -1.0,  1.0}, { 1.0,  0.0,  0.0}}, /* Foreground triangle */
    {{ 0.0,  1.0,  1.0}, { 0.0,  1.0,  0.0}},
    {{ 1.0, -1.0,  1.0}, { 0.0,  0.0,  1.0}},
};

#define MASK(mask, val) (((val) << (ffs(mask)-1)) & (mask))

static void matrix_viewport(float out[4][4], float x, float y, float width, float height, float z_min, float z_max);

static void debug_printf(const char* fmt, ...) {
    char buf[4096];
    va_list va;
    va_start(va, fmt);
    vsnprintf(buf, sizeof(buf), fmt, va);
    va_end(va);
    debugPrint("%s", fmt);
    OutputDebugStringA(buf);
}

/* Main program function */
int main(void)
{
    int       start, last, now;
    int       fps, frames, frames_total;

    LPDIRECT3D8 pD3D = Direct3DCreate8(D3D_SDK_VERSION);
    if (pD3D == NULL) {
        debug_printf("Direct3DCreate8 Error\n");
        Sleep(2000);
        return 1;
    }

    debug_printf("D3D8 successfully created.\n");

    D3DDISPLAYMODE mode;
    mode.Width          = 640;
    mode.Height         = 480;
    mode.RefreshRate    = 60;
    mode.Format         = D3DFMT_LIN_A8R8G8B8;
    mode.Flags          = 0;
    HRESULT hr = D3D_OK;
    for (int i = 0; i < IDirect3D8_GetAdapterModeCount(pD3D, D3DADAPTER_DEFAULT); i++) {
        D3DDISPLAYMODE found_mode;
        hr = IDirect3D8_EnumAdapterModes(pD3D, D3DADAPTER_DEFAULT, i, &found_mode);
        if (FAILED(hr)) {
            debug_printf("IDirect3D8::EnumAdapterModes failed: %d\n", hr);
            Sleep(2000);
            return 2;
        }
        if (found_mode.Width == mode.Width &&
            found_mode.Height == mode.Height &&
            found_mode.RefreshRate == mode.RefreshRate &&
            found_mode.Format == mode.Format)
        {
            memcpy(&mode, &found_mode, sizeof(mode));
            break;
        }
    }

    if (mode.Flags == 0) {
        debug_printf("Failed to find suitable adapter mode.\n");
        Sleep(2000);
        return 2;
    }

    D3DPRESENT_PARAMETERS d3dpp;
    memset(&d3dpp, 0, sizeof(d3dpp));
    d3dpp.BackBufferWidth                 = mode.Width;
    d3dpp.BackBufferHeight                = mode.Height;
    d3dpp.BackBufferFormat                = mode.Format;
    d3dpp.BackBufferCount                 = 2;
    d3dpp.MultiSampleType                 = D3DMULTISAMPLE_NONE;
    d3dpp.SwapEffect                      = D3DSWAPEFFECT_DISCARD;
    d3dpp.FullScreen_RefreshRateInHz      = mode.RefreshRate;
    d3dpp.FullScreen_PresentationInterval = D3DPRESENT_INTERVAL_ONE;
    d3dpp.Flags                           = mode.Flags;
    d3dpp.EnableAutoDepthStencil          = TRUE;
    d3dpp.AutoDepthStencilFormat          = D3DFMT_D24S8;

    debug_printf("Creating device.\n");
    LPDIRECT3DDEVICE8 pDevice = NULL;
    hr = IDirect3D8_CreateDevice(pD3D, D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL,
                                 NULL,
                                 D3DCREATE_HARDWARE_VERTEXPROCESSING |
                                 D3DCREATE_PUREDEVICE,
                                 &d3dpp, &pDevice);
    if(FAILED(hr)) {
        debug_printf("IDirect3D8::CreateDevice failed: %d\n", hr);
        pb_show_debug_screen();
        Sleep(INFINITE);
        return 1;
    }
    debug_printf("Device creation successful!\n");

    /* Setup vertex shader */
    uint32_t vs_program[] = {
        #include "vs.inl"
        D3DVS_END(),
    };

    debug_printf("Loading vertex shader program.\n");
    IDirect3DDevice8_SyncPushBuffer(pDevice);
    hr = IDirect3DDevice8_LoadVertexShaderProgram(pDevice, vs_program, 0);
    if(FAILED(hr)) {
        debug_printf("IDirect3D8::LoadVertexShaderProgram failed: %d\n", hr);
        Sleep(INFINITE);
    }

    /* Setup fragment shader */
    D3DPIXELSHADERDEF PSDef = {
        #include "ps.inl"
    };
    hr = IDirect3DDevice8_SetPixelShaderProgram(pDevice, &PSDef);
    if(FAILED(hr)) {
        debug_printf("IDirect3D8::SetPixelShaderProgram failed: %d\n", hr);
        Sleep(INFINITE);
    }

    debug_printf("Creating vertex buffer.\n");
    LPDIRECT3DVERTEXBUFFER8 pVB = NULL;
    hr = IDirect3DDevice8_CreateVertexBuffer(pDevice, sizeof(verts), 0, 0, 0, &pVB);
    if(FAILED(hr)) {
        debug_printf("IDirect3D8::CreateVertexBuffer failed: %d\n", hr);
        Sleep(INFINITE);
        return 1;
    }

    BYTE* pBytes = NULL;
    hr = IDirect3DVertexBuffer8_Lock(pVB, 0, 0, &pBytes, 0);
    if(FAILED(hr)) {
        debug_printf("IDirect3DVertexBuffer8::Lock failed: %d\n", hr);
        Sleep(INFINITE);
        return 1;
    }
    memcpy(pBytes, verts, sizeof(verts));
    debug_printf("Vertex data copied.\n");
    // No need to call Unlock

    D3DSTREAM_INPUT input;
    input.VertexBuffer          = pVB;
    input.Stride                = sizeof(ColoredVertex);
    input.Offset                = 0;

    D3DVERTEXATTRIBUTEFORMAT vaf;
    memset(&vaf, 0, sizeof(vaf));
    vaf.Input[0].StreamIndex    = 0;
    vaf.Input[0].Offset         = 0;
    vaf.Input[0].Format         = D3DVSDT_FLOAT3;
    vaf.Input[3].StreamIndex    = 0;
    vaf.Input[3].Offset         = 3 * sizeof(float);
    vaf.Input[3].Format         = D3DVSDT_FLOAT3;

    num_vertices = sizeof(verts)/sizeof(verts[0]);
    matrix_viewport(m_viewport, 0, 0, mode.Width, mode.Height, 0, 65536.0f);

    /* Setup to determine frames rendered every second */
    start = now = last = GetTickCount();
    frames_total = frames = fps = 0;
    IDirect3DDevice8_KickPushBuffer(pDevice);

    LPDIRECT3DPUSHBUFFER8 pPB = NULL;
    IDirect3DDevice8_GetPushBuffer(pDevice, &pPB);

    debug_printf("Entering frame loop...\n");
    while(1) {
        IDirect3DDevice8_BeginScene(pDevice);

        /* Clear depth & stencil buffers */
        // TODO: fix IDirect3DDevice8_Clear.
        // IDirect3DDevice8_Clear(pDevice, 0, NULL, D3DCLEAR_ZSTENCIL, 0, 1.0f, 0);
        pb_erase_depth_stencil_buffer(0, 0, mode.Width, mode.Height);
        IDirect3DDevice8_SyncPushBuffer(pDevice);

        /* Send shader constants
         *
         * WARNING: Changing shader source code may impact constant locations!
         * Check the intermediate file (*.inl) for the expected locations after
         * changing the code.
         */

        hr = IDirect3DDevice8_SetVertexShaderConstant(pDevice, 0, m_viewport, 4);
        if(FAILED(hr)) {
            debug_printf("IDirect3DDevice8::SetVertexShaderConstant failed %d\n", hr);
            Sleep(INFINITE);
            return 1;
        }

        hr = IDirect3DDevice8_SetVertexShaderInputDirect(pDevice, &vaf, 1, &input);
        if(FAILED(hr)) {
            debug_printf("IDirect3DDevice8::SetVertexShaderInputDirect failed %d\n", hr);
            Sleep(INFINITE);
            return 1;
        }

        hr = IDirect3DDevice8_DrawVertices(pDevice, D3DPT_TRIANGLELIST, 0, num_vertices);
        if(FAILED(hr)) {
            debug_printf("IDirect3DDevice8::DrawVertices failed %d\n", hr);
            Sleep(INFINITE);
            return 1;
        }

        hr = IDirect3DDevice8_EndScene(pDevice);
        if(FAILED(hr)) {
            debug_printf("IDirect3DDevice8::EndScene failed %d\n", hr);
            Sleep(INFINITE);
            return 1;
        }

        IDirect3DDevice8_Present(pDevice, NULL, NULL);

        frames++;
        frames_total++;

        /* Latch FPS counter every second */
        now = GetTickCount();
        if ((now-last) > 1000) {
            fps = frames;
            frames = 0;
            last = now;
        }
    }

    /* Unreachable cleanup code */
    IDirect3DVertexBuffer8_Release(pVB);
    IDirect3DDevice8_Release(pDevice);
    IDirect3D8_Release(pD3D);
    pb_show_debug_screen();
    return 0;
}

/* Construct a viewport transformation matrix */
static void matrix_viewport(float out[4][4], float x, float y, float width, float height, float z_min, float z_max)
{
    memset(out, 0, 4*4*sizeof(float));
    out[0][0] = width/2.0f;
    out[1][1] = height/-2.0f;
    out[2][2] = z_max - z_min;
    out[3][3] = 1.0f;
    out[3][0] = x + width/2.0f;
    out[3][1] = y + height/2.0f;
    out[3][2] = z_min;
}
