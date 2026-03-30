/*
 * This sample provides a very basic demonstration of 3D rendering on the Xbox,
 * using pbkit. Based on the pbkit demo sources.
 */
#include <hal/video.h>
#include <hal/xbox.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include <pbkit/pbkit.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <windows.h>
#include <xboxkrnl/xboxkrnl.h>
#include <hal/debug.h>
#include <d3d8.h>
#include <stdarg.h>
#include "math3d.h"

static uint32_t  num_indices;

MATRIX m_model, m_view, m_proj, m_mvp;

VECTOR v_obj_rot     = {  0,   0,   0,  1 };
VECTOR v_obj_scale   = {  1,   1,   1,  1 };
VECTOR v_obj_pos     = {  0,   0,   0,  1 };
VECTOR v_cam_loc     = {  0,   0, 165,  1 };
VECTOR v_cam_rot     = {  0,   0,   0,  1 };
VECTOR v_light_color = {  1,   1,   1,  1 };
VECTOR v_light_pos   = {  0, 140,   0,  1 };

float light_ambient  = 0.125f;

#pragma pack(1)
typedef struct Vertex {
    float pos[3];
    float normal[3];
    float texcoord[2];
} Vertex;
#pragma pack()

#include "verts.h"
#include "texture.h"

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
    uint32_t *p;
    int       i, status;
    int       width, height;
    int       start, last, now;
    int       fps, frames, frames_total;
    float     m_viewport[4][4];

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
    /* Load constant rendering things (shaders, geometry) */
    /* Setup vertex shader */
    uint32_t vs_program[] = {
        #include "vs.inl"
        D3DVS_END(),
    };

    debug_printf("Loading vertex shader program.\n");
    IDirect3DDevice8_SyncPushBuffer(pDevice);
    hr = IDirect3DDevice8_LoadVertexShaderProgram(pDevice, (PDWORD)vs_program, 0);
    if(FAILED(hr)) {
        debug_printf("IDirect3DDevice8::LoadVertexShaderProgram failed: %d\n", hr);
        Sleep(INFINITE);
    }

    debug_printf("Selecting vertex shader program.\n");
    hr = D3DDevice_SelectVertexShaderDirect(0);
    if(FAILED(hr)) {
        debug_printf("IDirect3DDevice8::SelectVertexShaderDirect failed: %d\n", hr);
        Sleep(INFINITE);
    }

    /* Setup fragment shader */
    D3DPIXELSHADERDEF PSDef = {
        #include "ps.inl"
    };
    debug_printf("Setting pixel shader program.\n");
    hr = IDirect3DDevice8_SetPixelShaderProgram(pDevice, &PSDef);
    if(FAILED(hr)) {
        debug_printf("IDirect3DDevice8::SetPixelShaderProgram failed: %d\n", hr);
        Sleep(INFINITE);
    }

    debug_printf("Creating texture.\n");
    LPDIRECT3DTEXTURE8 pTexture = NULL;
    hr = IDirect3DDevice8_CreateTexture(
        pDevice, texture_width, texture_height, 1, 0,
        D3DFMT_LIN_A8R8G8B8, D3DPOOL_DEFAULT, &pTexture);
    if(FAILED(hr)) {
        debug_printf("IDirect3DDevice8::CreateTexture failed: %d\n", hr);
        Sleep(INFINITE);
    }

    D3DLOCKED_RECT rect;
    hr = IDirect3DTexture8_LockRect(pTexture, 0, &rect, NULL, 0);
    if(FAILED(hr)) {
        debug_printf("IDirect3DDevice8::LockRect failed: %d\n", hr);
        Sleep(INFINITE);
    }
    memcpy(rect.pBits, texture_rgba, texture_height * texture_pitch);
    // No need to call Unlock
    debug_printf("Texture data copied.\n");

    debug_printf("Setting texture.\n");
    hr = IDirect3DDevice8_SetTexture(pDevice, 0, (LPDIRECT3DBASETEXTURE8)pTexture);
    if(FAILED(hr)) {
        debug_printf("IDirect3DDevice8::SetTexture failed: %d\n", hr);
        Sleep(INFINITE);
        return 1;
    }

    debug_printf("Creating vertex buffer.\n");
    LPDIRECT3DVERTEXBUFFER8 pVB = NULL;
    hr = IDirect3DDevice8_CreateVertexBuffer(pDevice, sizeof(vertices), 0, 0, 0, &pVB);
    if(FAILED(hr)) {
        debug_printf("IDirect3DDevice8::CreateVertexBuffer failed: %d\n", hr);
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
    memcpy(pBytes, vertices, sizeof(vertices));
    debug_printf("Vertex data copied.\n");
    // No need to call Unlock

    D3DSTREAM_INPUT input;
    input.VertexBuffer          = pVB;
    input.Stride                = sizeof(Vertex);
    input.Offset                = 0;

    D3DVERTEXATTRIBUTEFORMAT vaf;
    memset(&vaf, 0, sizeof(vaf));
    vaf.Input[D3DVSDE_POSITION].StreamIndex    = 0;
    vaf.Input[D3DVSDE_POSITION].Offset         = 0;
    vaf.Input[D3DVSDE_POSITION].Format         = D3DVSDT_FLOAT3;
    vaf.Input[D3DVSDE_DIFFUSE] .StreamIndex    = 0;
    vaf.Input[D3DVSDE_DIFFUSE] .Offset         = 3 * sizeof(float);
    vaf.Input[D3DVSDE_DIFFUSE] .Format         = D3DVSDT_FLOAT3;

    debug_printf("Setting vertex shader input.\n");
    hr = IDirect3DDevice8_SetVertexShaderInputDirect(pDevice, &vaf, 1, &input);
    if(FAILED(hr)) {
        debug_printf("IDirect3DDevice8::SetVertexShaderInputDirect failed %d\n", hr);
        Sleep(INFINITE);
        return 1;
    }

    debug_printf("Setting texture addressing modes.\n");
    hr = IDirect3DDevice8_SetTextureStageState(pDevice, 0, D3DTSS_ADDRESSU, D3DTADDRESS_CLAMP);
    if(FAILED(hr)) {
        debug_printf("IDirect3DDevice8::SetTextureStageState failed %d\n", hr);
        Sleep(INFINITE);
        return 1;
    }

    hr = IDirect3DDevice8_SetTextureStageState(pDevice, 0, D3DTSS_ADDRESSV, D3DTADDRESS_CLAMP);
    if(FAILED(hr)) {
        debug_printf("IDirect3DDevice8::SetTextureStageState failed %d\n", hr);
        Sleep(INFINITE);
        return 1;
    }

    hr = IDirect3DDevice8_SetTextureStageState(pDevice, 0, D3DTSS_ADDRESSW, D3DTADDRESS_CLAMP);
    if(FAILED(hr)) {
        debug_printf("IDirect3DDevice8::SetTextureStageState failed %d\n", hr);
        Sleep(INFINITE);
        return 1;
    }

    debug_printf("Setting texture filters.\n");
    hr = IDirect3DDevice8_SetTextureStageState(pDevice, 0, D3DTSS_MINFILTER, D3DTEXF_GAUSSIANCUBIC);
    if(FAILED(hr)) {
        debug_printf("IDirect3DDevice8::SetTextureStageState failed %d\n", hr);
        Sleep(INFINITE);
        return 1;
    }

    hr = IDirect3DDevice8_SetTextureStageState(pDevice, 0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
    if(FAILED(hr)) {
        debug_printf("IDirect3DDevice8::SetTextureStageState failed %d\n", hr);
        Sleep(INFINITE);
        return 1;
    }

    /* Send shader constants
     *
     * WARNING: Changing shader source code may impact constant locations!
     * Check the intermediate file (*.inl) for the expected locations after
     * changing the code.
     */
    debug_printf("Setting vertex shader constants.\n");
    /* Send shader constants 0 2 64 1 */
    float constants_0[4] = { 0, 2, 64, 1 };
    hr = IDirect3DDevice8_SetVertexShaderConstant(pDevice, 16, constants_0, 1);
    if(FAILED(hr)) {
        debug_printf("IDirect3DDevice8::SetVertexShaderConstant failed %d\n", hr);
        Sleep(INFINITE);
        return 1;
    }

    /* Setup to determine frames rendered every second */
    start = now = last = GetTickCount();
    frames_total = frames = fps = 0;

    /* Create view matrix (our camera is static) */
    matrix_unit(m_view);
    create_world_view(m_view, v_cam_loc, v_cam_rot);

    /* Create projection matrix */
    matrix_unit(m_proj);
    create_view_screen(m_proj, (float)width/(float)height, -1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 10000.0f);

    /* Create viewport matrix, combine with projection */
    matrix_viewport(m_viewport, 0, 0, width, height, 0, 65536.0f);
    matrix_multiply(m_proj, m_proj, (float*)m_viewport);

    IDirect3DDevice8_KickPushBuffer(pDevice);
    while(1) {
        /* Clear depth & stencil buffers */
        pb_erase_depth_stencil_buffer(0, 0, width, height);
        IDirect3DDevice8_SyncPushBuffer(pDevice);
        /* Tilt and rotate the object a bit */
        v_obj_rot[0] = (float)((now-start))/1000.0f * M_PI * -0.25f;

        /* Create local->world matrix given our updated object */
        matrix_unit(m_model);
        matrix_rotate(m_model, m_model, v_obj_rot);
        matrix_scale(m_model, m_model, v_obj_scale);
        matrix_translate(m_model, m_model, v_obj_pos);

        hr = IDirect3DDevice8_SetVertexShaderConstant(pDevice, 0, m_model, 4);
        if(FAILED(hr)) {
            debug_printf("IDirect3DDevice8::SetVertexShaderConstant failed %d\n", hr);
            Sleep(INFINITE);
            return 1;
        }

        hr = IDirect3DDevice8_SetVertexShaderConstant(pDevice, 4, m_view, 4);
        if(FAILED(hr)) {
            debug_printf("IDirect3DDevice8::SetVertexShaderConstant failed %d\n", hr);
            Sleep(INFINITE);
            return 1;
        }

        hr = IDirect3DDevice8_SetVertexShaderConstant(pDevice, 8, m_proj, 4);
        if(FAILED(hr)) {
            debug_printf("IDirect3DDevice8::SetVertexShaderConstant failed %d\n", hr);
            Sleep(INFINITE);
            return 1;
        }

        hr = IDirect3DDevice8_SetVertexShaderConstant(pDevice, 12, v_cam_loc, 1);
        if(FAILED(hr)) {
            debug_printf("IDirect3DDevice8::SetVertexShaderConstant failed %d\n", hr);
            Sleep(INFINITE);
            return 1;
        }

        hr = IDirect3DDevice8_SetVertexShaderConstant(pDevice, 13, v_light_pos, 1);
        if(FAILED(hr)) {
            debug_printf("IDirect3DDevice8::SetVertexShaderConstant failed %d\n", hr);
            Sleep(INFINITE);
            return 1;
        }

        hr = IDirect3DDevice8_SetVertexShaderConstant(pDevice, 14, v_light_color, 1);
        if(FAILED(hr)) {
            debug_printf("IDirect3DDevice8::SetVertexShaderConstant failed %d\n", hr);
            Sleep(INFINITE);
            return 1;
        }

        VECTOR v_light_ambient;
        v_light_ambient[0] = light_ambient;
        hr = IDirect3DDevice8_SetVertexShaderConstant(pDevice, 15, v_light_ambient, 1);
        if(FAILED(hr)) {
            debug_printf("IDirect3DDevice8::SetVertexShaderConstant failed %d\n", hr);
            Sleep(INFINITE);
            return 1;
        }

        hr = IDirect3DDevice8_DrawIndexedVertices(pDevice, D3DPT_TRIANGLELIST,
                                                  num_indices, (CONST WORD*)indices);
        if(FAILED(hr)) {
            debug_printf("IDirect3DDevice8::Present failed %d\n", hr);
            Sleep(INFINITE);
            return 1;
        }

        hr = IDirect3DDevice8_Present(pDevice, NULL, NULL);
        if(FAILED(hr)) {
            debug_printf("IDirect3DDevice8::Present failed %d\n", hr);
            Sleep(INFINITE);
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

        debug_printf("Frame %d: Presented (FPS=%d).\n", frames_total, fps);
    }

    /* Unreachable cleanup code */
    IDirect3DDevice8_SetTexture(pDevice, 0, NULL);
    IDirect3DTexture8_Release(pTexture);
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
