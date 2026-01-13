/*
 * This sample provides a very basic demonstration of 3D rendering on the Xbox,
 * using pbkit. Based on the pbkit demo sources.
 */
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

static uint32_t *alloc_vertices;
static uint32_t  num_vertices;

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

static void init_shader(void);

/* Main program function */
int main(void)
{
    uint32_t *p;
    int       i, status;
    int       width, height;
    int       start, last, now;
    int       fps, frames, frames_total;

    LPDIRECT3D8 d3d8 = Direct3DCreate8(D3D_SDK_VERSION);
    if (d3d8 == NULL) {
        debugPrint("Direct3DCreate8 failed\n");
        Sleep(2000);
        return 1;
    }

    D3DDISPLAYMODE mode;
    HRESULT hr = IDirect3D8_GetAdapterDisplayMode(d3d8, D3DADAPTER_DEFAULT, &mode);
    if(FAILED(hr)) {
        debugPrint("IDirect3D8::GetAdapterDisplayMode failed\n");
        Sleep(2000);
        return 1;
    }
    width = mode.Width;
    height = mode.Height;

    D3DPRESENT_PARAMETERS d3dpp;
    memset(&d3dpp, 0, sizeof(d3dpp));
    d3dpp.BackBufferWidth                 = mode.Width;
    d3dpp.BackBufferHeight                = mode.Height;
    d3dpp.BackBufferFormat                = mode.Format;
    d3dpp.BackBufferCount                 = 1;
    d3dpp.MultiSampleType                 = D3DMULTISAMPLE_NONE;
    d3dpp.SwapEffect                      = D3DSWAPEFFECT_DISCARD;
    d3dpp.FullScreen_RefreshRateInHz      = mode.RefreshRate;
    d3dpp.FullScreen_PresentationInterval = D3DPRESENT_INTERVAL_DEFAULT;
    d3dpp.Flags                           = mode.Flags;
    d3dpp.EnableAutoDepthStencil          = TRUE;
    d3dpp.AutoDepthStencilFormat          = D3DFMT_D24S8;

    LPDIRECT3DDEVICE8 d3ddev = NULL;
    hr = IDirect3D8_CreateDevice(d3d8, D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, NULL, D3DCREATE_HARDWARE_VERTEXPROCESSING, &d3dpp, &d3ddev);
    if(FAILED(hr)) {
        debugPrint("IDirect3D8::CreateDevice failed\n");
        Sleep(2000);
        return 1;
    }

    /* Load constant rendering things (shaders, geometry) */
    init_shader();
    num_vertices = sizeof(verts)/sizeof(verts[0]);

    /* Setup to determine frames rendered every second */
    start = now = last = GetTickCount();
    frames_total = frames = fps = 0;

    D3DVIEWPORT8 viewport;
    viewport.X = 0;
    viewport.Y = 0;
    viewport.Width = width;
    viewport.Height = height;
    viewport.MinZ = 0.0f;
    viewport.MaxZ = 65536.0f;
    hr = IDirect3DDevice8_SetViewport(d3ddev, &viewport);
    if(FAILED(hr)) {
        debugPrint("IDirect3DDevice8::SetViewport failed\n");
        Sleep(2000);
        return 1;
    }

    LPDIRECT3DVERTEXBUFFER8 vb = NULL;
    hr = IDirect3DDevice8_CreateVertexBuffer(d3ddev, sizeof(verts), 0, 0, 0, &vb);
    if(FAILED(hr)) {
        debugPrint("IDirect3DDevice8::CreateVertexBuffer failed\n");
        Sleep(2000);
        return 1;
    }

    BYTE* pData = NULL;
    hr = IDirect3DVertexBuffer8_Lock(vb, 0, sizeof(verts), &pData, 0);
    if(FAILED(hr)) {
        debugPrint("IDirect3DVertexBuffer8::Lock failed\n");
        Sleep(2000);
        return 1;
    }
    memcpy(pData, verts, sizeof(verts));
    hr = IDirect3DVertexBuffer8_Unlock(vb);
    if(FAILED(hr)) {
        debugPrint("IDirect3DVertexBuffer8::Unlock failed\n");
        Sleep(2000);
        return 1;
    }

    D3DSTREAM_INPUT input;
    input.VertexBuffer = vb;
    input.Stride       = sizeof(ColoredVertex);
    input.Offset       = 0;

    D3DVERTEXATTRIBUTEFORMAT vaf;
    vaf.Input[0].StreamIndex = 0;
    vaf.Input[0].Offset      = 0;
    vaf.Input[0].Format      = D3DVSDT_FLOAT3;
    vaf.Input[1].StreamIndex = 0;
    vaf.Input[1].Offset      = 3 * sizeof(float);
    vaf.Input[1].Format      = D3DVSDT_FLOAT3;

    hr = IDirect3DDevice8_SetVertexShaderInputDirect(d3ddev, &vaf, 1, &input);
    if(FAILED(hr)) {
        debugPrint("IDirect3DDevice8::SetVertexShaderInputDirect failed\n");
        Sleep(2000);
        return 1;
    }
    while(1) {
        hr = IDirect3DDevice8_BeginScene(d3ddev);
        if(FAILED(hr)) {
            debugPrint("IDirect3DDevice8::BeginScene failed\n");
            Sleep(2000);
            return 1;
        }

        /* Send shader constants
         *
         * WARNING: Changing shader source code may impact constant locations!
         * Check the intermediate file (*.inl) for the expected locations after
         * changing the code.
         */
        p = pb_begin();

        /* Set shader constants cursor at C0 */
        p = pb_push1(p, NV097_SET_TRANSFORM_CONSTANT_LOAD, 96);
        pb_end(p);

        /* Begin drawing triangles */
        hr = IDirect3DDevice8_DrawPrimitive(d3ddev, D3DPT_TRIANGLELIST, 0, num_vertices / 3);
        if(FAILED(hr)) {
            debugPrint("IDirect3DDevice8::DrawPrimitive failed\n");
            Sleep(2000);
            return 1;
        }

        /* Draw some text on the screen */
        pb_print("Triangle Demo\n");
        pb_print("Frames: %d\n", frames_total);
        if (fps > 0) {
            pb_print("FPS: %d", fps);
        }
        pb_draw_text_screen();

        hr = IDirect3DDevice8_EndScene(d3ddev);
        if(FAILED(hr)) {
            debugPrint("IDirect3DDevice8::EndScene failed\n");
            Sleep(2000);
            return 1;
        }
        hr = IDirect3DDevice8_Present(d3ddev, NULL, NULL);
        if(FAILED(hr)) {
            debugPrint("IDirect3DDevice8::Present failed\n");
            Sleep(2000);
            return 1;
        }
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
    pb_show_debug_screen();
    IDirect3DVertexBuffer8_Release(vb);
    IDirect3DDevice8_Release(d3ddev);
    IDirect3D8_Release(d3d8);
    return 0;
}

/* Load the shader we will render with */
static void init_shader(void)
{
    uint32_t *p;
    int       i;

    /* Setup vertex shader */
    uint32_t vs_program[] = {
        #include "vs.inl"
    };

    p = pb_begin();

    /* Set run address of shader */
    p = pb_push1(p, NV097_SET_TRANSFORM_PROGRAM_START, 0);

    /* Set execution mode */
    p = pb_push1(p, NV097_SET_TRANSFORM_EXECUTION_MODE,
                 MASK(NV097_SET_TRANSFORM_EXECUTION_MODE_MODE, NV097_SET_TRANSFORM_EXECUTION_MODE_MODE_PROGRAM)
                 | MASK(NV097_SET_TRANSFORM_EXECUTION_MODE_RANGE_MODE, NV097_SET_TRANSFORM_EXECUTION_MODE_RANGE_MODE_PRIV));

    p = pb_push1(p, NV097_SET_TRANSFORM_PROGRAM_CXT_WRITE_EN, 0);

    pb_end(p);

    /* Set cursor for program upload */
    p = pb_begin();
    p = pb_push1(p, NV097_SET_TRANSFORM_PROGRAM_LOAD, 0);
    pb_end(p);

    /* Copy program instructions (16-bytes each) */
    for (i=0; i<sizeof(vs_program)/16; i++) {
        p = pb_begin();
        pb_push(p++, NV097_SET_TRANSFORM_PROGRAM, 4);
        memcpy(p, &vs_program[i*4], 4*4);
        p+=4;
        pb_end(p);
    }

    /* Setup fragment shader */
    p = pb_begin();
    #include "ps.inl"
    pb_end(p);
}
