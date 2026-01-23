#ifndef NXDK_HAVE_D3D8TYPES
#define NXDK_HAVE_D3D8TYPES 1

#include <windows.h>
#include <winnt.h>

typedef struct _GUID {
    unsigned long  Data1;
    unsigned short Data2;
    unsigned short Data3;
    unsigned char  Data4[8];
} GUID;

typedef GUID IID;
typedef IID *REFIID;
typedef HANDLE HMONITOR;

#define _FACD3D  0x876
#define MAKE_D3DHRESULT( code )  MAKE_HRESULT( 1, _FACD3D, code )

/*
 * Direct3D Errors
 */
#define D3D_OK                              S_OK

#define D3DERR_WRONGTEXTUREFORMAT               MAKE_D3DHRESULT(2072)
#define D3DERR_UNSUPPORTEDCOLOROPERATION        MAKE_D3DHRESULT(2073)
#define D3DERR_UNSUPPORTEDCOLORARG              MAKE_D3DHRESULT(2074)
#define D3DERR_UNSUPPORTEDALPHAOPERATION        MAKE_D3DHRESULT(2075)
#define D3DERR_UNSUPPORTEDALPHAARG              MAKE_D3DHRESULT(2076)
#define D3DERR_TOOMANYOPERATIONS                MAKE_D3DHRESULT(2077)
#define D3DERR_CONFLICTINGTEXTUREFILTER         MAKE_D3DHRESULT(2078)
#define D3DERR_UNSUPPORTEDFACTORVALUE           MAKE_D3DHRESULT(2079)
#define D3DERR_CONFLICTINGRENDERSTATE           MAKE_D3DHRESULT(2081)
#define D3DERR_UNSUPPORTEDTEXTUREFILTER         MAKE_D3DHRESULT(2082)
#define D3DERR_CONFLICTINGTEXTUREPALETTE        MAKE_D3DHRESULT(2086)
#define D3DERR_DRIVERINTERNALERROR              MAKE_D3DHRESULT(2087)
#define D3DERR_TESTINCOMPLETE                   MAKE_D3DHRESULT(2088)
#define D3DERR_BUFFERTOOSMALL                   MAKE_D3DHRESULT(2089)
#define D3DERR_TIMEEXPIRED                      MAKE_D3DHRESULT(2090)

#define D3DERR_NOTFOUND                         MAKE_D3DHRESULT(2150)
#define D3DERR_MOREDATA                         MAKE_D3DHRESULT(2151)
#define D3DERR_DEVICELOST                       MAKE_D3DHRESULT(2152)
#define D3DERR_DEVICENOTRESET                   MAKE_D3DHRESULT(2153)
#define D3DERR_NOTAVAILABLE                     MAKE_D3DHRESULT(2154)
#define D3DERR_OUTOFVIDEOMEMORY                 MAKE_D3DHRESULT(380)
#define D3DERR_INVALIDDEVICE                    MAKE_D3DHRESULT(2155)
#define D3DERR_INVALIDCALL                      MAKE_D3DHRESULT(2156)

#define D3DCREATE_FPU_PRESERVE                  0x00000002L
#define D3DCREATE_MULTITHREADED                 0x00000004L

#define D3DCREATE_PUREDEVICE                    0x00000010L
#define D3DCREATE_SOFTWARE_VERTEXPROCESSING     0x00000020L
#define D3DCREATE_HARDWARE_VERTEXPROCESSING     0x00000040L
#define D3DCREATE_MIXED_VERTEXPROCESSING        0x00000080L

#define MAX_DEVICE_IDENTIFIER_STRING        512
typedef struct _D3DADAPTER_IDENTIFIER8 {
    char            Driver     [MAX_DEVICE_IDENTIFIER_STRING];
    char            Description[MAX_DEVICE_IDENTIFIER_STRING];

    LARGE_INTEGER   DriverVersion;            /* Defined for 32 bit components */

    DWORD           VendorId;
    DWORD           DeviceId;
    DWORD           SubSysId;
    DWORD           Revision;

    GUID            DeviceIdentifier;

    DWORD           WHQLLevel;
} D3DADAPTER_IDENTIFIER8;

#define MAKEFOURCC(ch0, ch1, ch2, ch3)                                \
            ((DWORD)(BYTE)(ch0) | ((DWORD)(BYTE)(ch1) << 8) |         \
            ((DWORD)(BYTE)(ch2) << 16) | ((DWORD)(BYTE)(ch3) << 24 ))

typedef enum _D3DFORMAT {
    D3DFMT_UNKNOWN              = 0xFFFFFFFF,

    /* Swizzled formats */

    D3DFMT_A8R8G8B8             = NV097_SET_TEXTURE_FORMAT_COLOR_SZ_A8R8G8B8,
    D3DFMT_X8R8G8B8             = NV097_SET_TEXTURE_FORMAT_COLOR_SZ_X8R8G8B8,
    D3DFMT_R5G6B5               = NV097_SET_TEXTURE_FORMAT_COLOR_SZ_R5G6B5,
    D3DFMT_R6G5B5               = NV097_SET_TEXTURE_FORMAT_COLOR_SZ_R6G5B5,
    D3DFMT_X1R5G5B5             = NV097_SET_TEXTURE_FORMAT_COLOR_SZ_X1R5G5B5,
    D3DFMT_A1R5G5B5             = NV097_SET_TEXTURE_FORMAT_COLOR_SZ_A1R5G5B5,
    D3DFMT_A4R4G4B4             = NV097_SET_TEXTURE_FORMAT_COLOR_SZ_A4R4G4B4,
    D3DFMT_A8                   = NV097_SET_TEXTURE_FORMAT_COLOR_SZ_A8,
    D3DFMT_A8B8G8R8             = NV097_SET_TEXTURE_FORMAT_COLOR_SZ_A8B8G8R8,
    D3DFMT_B8G8R8A8             = NV097_SET_TEXTURE_FORMAT_COLOR_SZ_B8G8R8A8,
    D3DFMT_R4G4B4A4             = NV097_SET_TEXTURE_FORMAT_COLOR_SZ_R4G4B4A4,
    D3DFMT_R5G5B5A1             = NV097_SET_TEXTURE_FORMAT_COLOR_SZ_R5G5B5A1,
    D3DFMT_R8G8B8A8             = NV097_SET_TEXTURE_FORMAT_COLOR_SZ_R8G8B8A8,
    D3DFMT_R8B8                 = NV097_SET_TEXTURE_FORMAT_COLOR_SZ_R8B8,
    D3DFMT_G8B8                 = NV097_SET_TEXTURE_FORMAT_COLOR_SZ_G8B8,

    D3DFMT_P8                   = NV097_SET_TEXTURE_FORMAT_COLOR_SZ_I8_A8R8G8B8,

    D3DFMT_L8                   = NV097_SET_TEXTURE_FORMAT_COLOR_SZ_Y8,
    D3DFMT_A8L8                 = NV097_SET_TEXTURE_FORMAT_COLOR_SZ_A8Y8,
    D3DFMT_AL8                  = NV097_SET_TEXTURE_FORMAT_COLOR_SZ_AY8,
    D3DFMT_L16                  = NV097_SET_TEXTURE_FORMAT_COLOR_LU_IMAGE_DEPTH_Y16,

    D3DFMT_V8U8                 = NV097_SET_TEXTURE_FORMAT_COLOR_SZ_G8B8,
    D3DFMT_L6V5U5               = NV097_SET_TEXTURE_FORMAT_COLOR_SZ_R6G5B5,
    D3DFMT_X8L8V8U8             = NV097_SET_TEXTURE_FORMAT_COLOR_SZ_X8R8G8B8,
    D3DFMT_Q8W8V8U8             = NV097_SET_TEXTURE_FORMAT_COLOR_SZ_A8B8G8R8,
    D3DFMT_V16U16               = NV097_SET_TEXTURE_FORMAT_COLOR_SZ_G16B16,

    D3DFMT_D16_LOCKABLE         = NV097_SET_TEXTURE_FORMAT_COLOR_SZ_DEPTH_Y16_FIXED,
    D3DFMT_D16                  = NV097_SET_TEXTURE_FORMAT_COLOR_SZ_DEPTH_Y16_FIXED,
    D3DFMT_D24S8                = NV097_SET_TEXTURE_FORMAT_COLOR_SZ_DEPTH_X8_Y24_FIXED,
    D3DFMT_F16                  = NV097_SET_TEXTURE_FORMAT_COLOR_SZ_DEPTH_Y16_FLOAT,
    D3DFMT_F24S8                = NV097_SET_TEXTURE_FORMAT_COLOR_SZ_DEPTH_X8_Y24_FLOAT,

    /* YUV formats */

    D3DFMT_YUY2                 = NV097_SET_TEXTURE_FORMAT_COLOR_LC_IMAGE_CR8YB8CB8YA8,
    D3DFMT_UYVY                 = NV097_SET_TEXTURE_FORMAT_COLOR_LC_IMAGE_YB8CR8YA8CB8,

    /* Compressed formats */

    D3DFMT_DXT1                 = NV097_SET_TEXTURE_FORMAT_COLOR_L_DXT1_A1R5G5B5,
    D3DFMT_DXT2                 = NV097_SET_TEXTURE_FORMAT_COLOR_L_DXT23_A8R8G8B8,
    D3DFMT_DXT3                 = NV097_SET_TEXTURE_FORMAT_COLOR_L_DXT23_A8R8G8B8,
    D3DFMT_DXT4                 = NV097_SET_TEXTURE_FORMAT_COLOR_L_DXT45_A8R8G8B8,
    D3DFMT_DXT5                 = NV097_SET_TEXTURE_FORMAT_COLOR_L_DXT45_A8R8G8B8,

    /* Linear formats */

    D3DFMT_LIN_A1R5G5B5         = NV097_SET_TEXTURE_FORMAT_COLOR_LU_IMAGE_A1R5G5B5,
    D3DFMT_LIN_A4R4G4B4         = NV097_SET_TEXTURE_FORMAT_COLOR_LU_IMAGE_A4R4G4B4,
    D3DFMT_LIN_A8               = NV097_SET_TEXTURE_FORMAT_COLOR_LU_IMAGE_A8,
    D3DFMT_LIN_A8B8G8R8         = NV097_SET_TEXTURE_FORMAT_COLOR_LU_IMAGE_A8B8G8R8,
    D3DFMT_LIN_A8R8G8B8         = NV097_SET_TEXTURE_FORMAT_COLOR_LU_IMAGE_A8R8G8B8,
    D3DFMT_LIN_B8G8R8A8         = NV097_SET_TEXTURE_FORMAT_COLOR_LU_IMAGE_B8G8R8A8,
    D3DFMT_LIN_G8B8             = NV097_SET_TEXTURE_FORMAT_COLOR_LU_IMAGE_G8B8,
    D3DFMT_LIN_R4G4B4A4         = NV097_SET_TEXTURE_FORMAT_COLOR_LU_IMAGE_R4G4B4A4,
    D3DFMT_LIN_R5G5B5A1         = NV097_SET_TEXTURE_FORMAT_COLOR_LU_IMAGE_R5G5B5A1,
    D3DFMT_LIN_R5G6B5           = NV097_SET_TEXTURE_FORMAT_COLOR_LU_IMAGE_R5G6B5,
    D3DFMT_LIN_R6G5B5           = NV097_SET_TEXTURE_FORMAT_COLOR_SZ_R6G5B5,
    D3DFMT_LIN_R8B8             = NV097_SET_TEXTURE_FORMAT_COLOR_LU_IMAGE_R8B8,
    D3DFMT_LIN_R8G8B8A8         = NV097_SET_TEXTURE_FORMAT_COLOR_LU_IMAGE_R8G8B8A8,
    D3DFMT_LIN_X1R5G5B5         = NV097_SET_TEXTURE_FORMAT_COLOR_LU_IMAGE_X1R5G5B5,
    D3DFMT_LIN_X8R8G8B8         = NV097_SET_TEXTURE_FORMAT_COLOR_LU_IMAGE_X8R8G8B8,

    D3DFMT_LIN_A8L8             = NV097_SET_TEXTURE_FORMAT_COLOR_LU_IMAGE_A8Y8,
    D3DFMT_LIN_AL8              = NV097_SET_TEXTURE_FORMAT_COLOR_LU_IMAGE_AY8,
    D3DFMT_LIN_L16              = NV097_SET_TEXTURE_FORMAT_COLOR_LU_IMAGE_Y16,
    D3DFMT_LIN_L8               = NV097_SET_TEXTURE_FORMAT_COLOR_LU_IMAGE_Y8,

    D3DFMT_LIN_V16U16           = NV097_SET_TEXTURE_FORMAT_COLOR_SZ_G16B16,
    D3DFMT_LIN_V8U8             = NV097_SET_TEXTURE_FORMAT_COLOR_LU_IMAGE_G8B8,
    D3DFMT_LIN_L6V5U5           = NV097_SET_TEXTURE_FORMAT_COLOR_SZ_R6G5B5,
    D3DFMT_LIN_X8L8V8U8         = NV097_SET_TEXTURE_FORMAT_COLOR_LU_IMAGE_X8R8G8B8,
    D3DFMT_LIN_Q8W8V8U8         = NV097_SET_TEXTURE_FORMAT_COLOR_LU_IMAGE_A8B8G8R8,

    D3DFMT_LIN_D24S8            = NV097_SET_TEXTURE_FORMAT_COLOR_LU_IMAGE_DEPTH_X8_Y24_FIXED,
    D3DFMT_LIN_F24S8            = NV097_SET_TEXTURE_FORMAT_COLOR_LU_IMAGE_DEPTH_X8_Y24_FLOAT,
    D3DFMT_LIN_D16              = NV097_SET_TEXTURE_FORMAT_COLOR_LU_IMAGE_DEPTH_Y16_FIXED,
    D3DFMT_LIN_F16              = NV097_SET_TEXTURE_FORMAT_COLOR_LU_IMAGE_DEPTH_Y16_FLOAT,

    D3DFMT_VERTEXDATA           = 100,
    D3DFMT_INDEX16              = 101,

    D3DFMT_FORCE_DWORD          =0x7fffffff
} D3DFORMAT;

#define D3DPRESENTFLAG_LOCKABLE_BACKBUFFER      0x00000001
#define D3DPRESENTFLAG_WIDESCREEN               0x00000010
#define D3DPRESENTFLAG_INTERLACED               0x00000020
#define D3DPRESENTFLAG_PROGRESSIVE              0x00000040
#define D3DPRESENTFLAG_FIELD                    0x00000080
#define D3DPRESENTFLAG_10X11PIXELASPECTRATIO    0x00000100
#define D3DPRESENTFLAG_EMULATE_REFRESH_RATE     0x00000200  

#define D3DPRESENT_RATE_DEFAULT         0x00000000
#define D3DPRESENT_RATE_UNLIMITED       0x00000000

typedef struct _D3DDISPLAYMODE {
    UINT            Width;
    UINT            Height;
    UINT            RefreshRate;
    DWORD           Flags;
    D3DFORMAT       Format;
} D3DDISPLAYMODE;

typedef enum _D3DDEVTYPE {
    D3DDEVTYPE_HAL         = 1,
    D3DDEVTYPE_REF         = 2,
    D3DDEVTYPE_SW          = 3,

    D3DDEVTYPE_FORCE_DWORD  = 0x7fffffff
} D3DDEVTYPE;

typedef enum _D3DRESOURCETYPE {
    D3DRTYPE_NONE                   =  0,
    D3DRTYPE_SURFACE                =  1,
    D3DRTYPE_VOLUME                 =  2,
    D3DRTYPE_TEXTURE                =  3,
    D3DRTYPE_VOLUMETEXTURE          =  4,
    D3DRTYPE_CUBETEXTURE            =  5,
    D3DRTYPE_VERTEXBUFFER           =  6,
    D3DRTYPE_INDEXBUFFER            =  7,
    D3DRTYPE_PUSHBUFFER             =  8,
    D3DRTYPE_PALETTE                =  9,
    D3DRTYPE_FIXUP                  =  10,

    D3DRTYPE_FORCE_DWORD            = 0x7fffffff
} D3DRESOURCETYPE;

/* Usages */
#define D3DUSAGE_RENDERTARGET           (0x00000001L)
#define D3DUSAGE_DEPTHSTENCIL           (0x00000002L)

/* Usages for Vertex/Index buffers */
#define D3DUSAGE_WRITEONLY              (0x00000008L)
#define D3DUSAGE_POINTS                 (0x00000040L)
#define D3DUSAGE_RTPATCHES              (0x00000080L)
#define D3DUSAGE_DYNAMIC                (0x00000200L)

/* Usages for CreateVertexShader */
#define D3DUSAGE_PERSISTENTDIFFUSE      (0x00000400L)   // Xbox extension
#define D3DUSAGE_PERSISTENTSPECULAR     (0x00000800L)   // Xbox extension
#define D3DUSAGE_PERSISTENTBACKDIFFUSE  (0x00001000L)   // Xbox extension
#define D3DUSAGE_PERSISTENTBACKSPECULAR (0x00002000L)   // Xbox extension

/* Usages for CreateTexture/CreateImageSurface */
#define D3DUSAGE_BORDERSOURCE_COLOR     (0x00000000L)   // Xbox extension
#define D3DUSAGE_BORDERSOURCE_TEXTURE   (0x00010000L)   // Xbox extension

/* AntiAliasing buffer types */

typedef DWORD D3DMULTISAMPLE_TYPE;

#define D3DMULTISAMPLE_NONE                                      0x0011

// Number of samples, sample type, and filter:
//
#define D3DMULTISAMPLE_2_SAMPLES_MULTISAMPLE_LINEAR              0x1021
#define D3DMULTISAMPLE_2_SAMPLES_MULTISAMPLE_QUINCUNX            0x1121
#define D3DMULTISAMPLE_2_SAMPLES_SUPERSAMPLE_HORIZONTAL_LINEAR   0x2021
#define D3DMULTISAMPLE_2_SAMPLES_SUPERSAMPLE_VERTICAL_LINEAR     0x2012

#define D3DMULTISAMPLE_4_SAMPLES_MULTISAMPLE_LINEAR              0x1022
#define D3DMULTISAMPLE_4_SAMPLES_MULTISAMPLE_GAUSSIAN            0x1222
#define D3DMULTISAMPLE_4_SAMPLES_SUPERSAMPLE_LINEAR              0x2022
#define D3DMULTISAMPLE_4_SAMPLES_SUPERSAMPLE_GAUSSIAN            0x2222

#define D3DMULTISAMPLE_9_SAMPLES_MULTISAMPLE_GAUSSIAN            0x1233
#define D3DMULTISAMPLE_9_SAMPLES_SUPERSAMPLE_GAUSSIAN            0x2233

// Format of the pre-filter (big) color buffer:
//
#define D3DMULTISAMPLE_PREFILTER_FORMAT_DEFAULT                  0x00000
#define D3DMULTISAMPLE_PREFILTER_FORMAT_X1R5G5B5                 0x10000
#define D3DMULTISAMPLE_PREFILTER_FORMAT_R5G6B5                   0x20000
#define D3DMULTISAMPLE_PREFILTER_FORMAT_X8R8G8B8                 0x30000
#define D3DMULTISAMPLE_PREFILTER_FORMAT_A8R8G8B8                 0x40000

// Defaults:
//
#define D3DMULTISAMPLE_2_SAMPLES D3DMULTISAMPLE_2_SAMPLES_MULTISAMPLE_QUINCUNX
#define D3DMULTISAMPLE_4_SAMPLES D3DMULTISAMPLE_4_SAMPLES_MULTISAMPLE_GAUSSIAN
#define D3DMULTISAMPLE_9_SAMPLES D3DMULTISAMPLE_9_SAMPLES_SUPERSAMPLE_GAUSSIAN

typedef enum _D3DMULTISAMPLEMODE {
    D3DMULTISAMPLEMODE_1X                                   = 0,
    D3DMULTISAMPLEMODE_2X                                   = 1,
    D3DMULTISAMPLEMODE_4X                                   = 2,

    D3DMULTISAMPLEMODE_FORCE_DWORD                          = 0x7fffffff
} D3DMULTISAMPLEMODE;

typedef enum _D3DTEXTUREFILTERTYPE {
    D3DTEXF_NONE            = NV097_SET_TEXTURE_FILTER_NONE, 
    D3DTEXF_POINT           = NV097_SET_TEXTURE_FILTER_POINT, 
    D3DTEXF_LINEAR          = NV097_SET_TEXTURE_FILTER_LINEAR, 
    D3DTEXF_ANISOTROPIC     = NV097_SET_TEXTURE_FILTER_ANISOTROPIC, 
    D3DTEXF_QUINCUNX        = NV097_SET_TEXTURE_FILTER_QUINCUNX, 
    D3DTEXF_GAUSSIANCUBIC   = NV097_SET_TEXTURE_FILTER_GAUSSIAN_CUBIC, 

    D3DTEXF_MAX             = D3DTEXF_GAUSSIANCUBIC + 1,
    D3DTEXF_FORCE_DWORD     = 0x7fffffff, // force 32-bit size enum
} D3DTEXTUREFILTERTYPE;

// BIT DEFINES FOR D3DCAPS8 DWORD MEMBERS
//

//
// Caps
//
#define D3DCAPS_READ_SCANLINE           0x00020000L

//
// Caps2
//
#define D3DCAPS2_NO2DDURING3DSCENE      0x00000002L
#define D3DCAPS2_FULLSCREENGAMMA        0x00020000L
#define D3DCAPS2_CANRENDERWINDOWED      0x00080000L
#define D3DCAPS2_CANCALIBRATEGAMMA      0x00100000L
#define D3DCAPS2_RESERVED               0x02000000L

//
// Caps3
//
#define D3DCAPS3_RESERVED               0x8000001fL

//
// PresentationIntervals
//
#define D3DPRESENT_INTERVAL_DEFAULT     0x00000000L
#define D3DPRESENT_INTERVAL_ONE         0x00000001L
#define D3DPRESENT_INTERVAL_TWO         0x00000002L
#define D3DPRESENT_INTERVAL_THREE       0x00000004L
#define D3DPRESENT_INTERVAL_FOUR        0x00000008L
#define D3DPRESENT_INTERVAL_IMMEDIATE   0x80000000L

//
// CursorCaps
//
// Driver supports HW color cursor in at least hi-res modes(height >=400)
#define D3DCURSORCAPS_COLOR             0x00000001L
// Driver supports HW cursor also in low-res modes(height < 400)
#define D3DCURSORCAPS_LOWRES            0x00000002L

/* Device can use execute buffers from system memory */
#define D3DDEVCAPS_EXECUTESYSTEMMEMORY  0x00000010L 
 /* Device can use execute buffers from video memory */
#define D3DDEVCAPS_EXECUTEVIDEOMEMORY   0x00000020L
/* Device can use TL buffers from system memory */
#define D3DDEVCAPS_TLVERTEXSYSTEMMEMORY 0x00000040L 
/* Device can use TL buffers from video memory */
#define D3DDEVCAPS_TLVERTEXVIDEOMEMORY  0x00000080L 
/* Device can texture from system memory */
#define D3DDEVCAPS_TEXTURESYSTEMMEMORY  0x00000100L 
/* Device can texture from device memory */
#define D3DDEVCAPS_TEXTUREVIDEOMEMORY   0x00000200L 
/* Device can draw TLVERTEX primitives */
#define D3DDEVCAPS_DRAWPRIMTLVERTEX     0x00000400L 
/* Device can render without waiting for flip to complete */
#define D3DDEVCAPS_CANRENDERAFTERFLIP   0x00000800L 
/* Device can texture from nonlocal video memory */
#define D3DDEVCAPS_TEXTURENONLOCALVIDMEM 0x00001000L 
/* Device can support DrawPrimitives2 */
#define D3DDEVCAPS_DRAWPRIMITIVES2      0x00002000L 
/* Device is texturing from separate memory pools */
#define D3DDEVCAPS_SEPARATETEXTUREMEMORIES 0x00004000L 
/* Device can support Extended DrawPrimitives2 i.e. DX7 compliant driver*/
#define D3DDEVCAPS_DRAWPRIMITIVES2EX    0x00008000L 
/* Device can support transformation and lighting in hardware
   and DRAWPRIMITIVES2EX must be also */
#define D3DDEVCAPS_HWTRANSFORMANDLIGHT  0x00010000L 
/* Device supports a Tex Blt from system memory to non-local vidmem */
#define D3DDEVCAPS_CANBLTSYSTONONLOCAL  0x00020000L 
/* Device has HW acceleration for rasterization */
#define D3DDEVCAPS_HWRASTERIZATION      0x00080000L 
/* Device supports D3DCREATE_PUREDEVICE */
#define D3DDEVCAPS_PUREDEVICE           0x00100000L 
/* Device supports quintic Beziers and BSplines */
#define D3DDEVCAPS_QUINTICRTPATCHES     0x00200000L 
/* Device supports Rect and Tri patches */
#define D3DDEVCAPS_RTPATCHES            0x00400000L 
/* Indicates that RT Patches may be drawn efficiently using handle 0 */
#define D3DDEVCAPS_RTPATCHHANDLEZERO    0x00800000L 
 /* Device supports N-Patches */
#define D3DDEVCAPS_NPATCHES             0x01000000L

//
// PrimitiveMiscCaps
//
#define D3DPMISCCAPS_MASKZ              0x00000002L
#define D3DPMISCCAPS_LINEPATTERNREP     0x00000004L
#define D3DPMISCCAPS_CULLNONE           0x00000010L
#define D3DPMISCCAPS_CULLCW             0x00000020L
#define D3DPMISCCAPS_CULLCCW            0x00000040L
#define D3DPMISCCAPS_COLORWRITEENABLE   0x00000080L
#define D3DPMISCCAPS_CLIPPLANESCALEDPOINTS 0x00000100L /* Device correctly clips scaled points to clip planes */
#define D3DPMISCCAPS_CLIPTLVERTS        0x00000200L /* device will clip post-transformed vertex primitives */
#define D3DPMISCCAPS_TSSARGTEMP         0x00000400L /* device supports D3DTA_TEMP for temporary register */
#define D3DPMISCCAPS_BLENDOP            0x00000800L /* device supports D3DRS_BLENDOP */

//
// LineCaps
//
#define D3DLINECAPS_TEXTURE             0x00000001L
#define D3DLINECAPS_ZTEST               0x00000002L
#define D3DLINECAPS_BLEND               0x00000004L
#define D3DLINECAPS_ALPHACMP            0x00000008L
#define D3DLINECAPS_FOG                 0x00000010L

//
// RasterCaps
//
#define D3DPRASTERCAPS_DITHER           0x00000001L
#define D3DPRASTERCAPS_PAT              0x00000008L
#define D3DPRASTERCAPS_ZTEST            0x00000010L
#define D3DPRASTERCAPS_FOGVERTEX        0x00000080L
#define D3DPRASTERCAPS_FOGTABLE         0x00000100L
#define D3DPRASTERCAPS_ANTIALIASEDGES   0x00001000L
#define D3DPRASTERCAPS_MIPMAPLODBIAS    0x00002000L
#define D3DPRASTERCAPS_ZBIAS            0x00004000L
#define D3DPRASTERCAPS_ZBUFFERLESSHSR   0x00008000L
#define D3DPRASTERCAPS_FOGRANGE         0x00010000L
#define D3DPRASTERCAPS_ANISOTROPY       0x00020000L
#define D3DPRASTERCAPS_WBUFFER          0x00040000L
#define D3DPRASTERCAPS_WFOG             0x00100000L
#define D3DPRASTERCAPS_ZFOG             0x00200000L
#define D3DPRASTERCAPS_COLORPERSPECTIVE 0x00400000L /* Device iterates colors perspective correct */
#define D3DPRASTERCAPS_STRETCHBLTMULTISAMPLE  0x00800000L

//
// ZCmpCaps, AlphaCmpCaps
//
#define D3DPCMPCAPS_NEVER               0x00000001L
#define D3DPCMPCAPS_LESS                0x00000002L
#define D3DPCMPCAPS_EQUAL               0x00000004L
#define D3DPCMPCAPS_LESSEQUAL           0x00000008L
#define D3DPCMPCAPS_GREATER             0x00000010L
#define D3DPCMPCAPS_NOTEQUAL            0x00000020L
#define D3DPCMPCAPS_GREATEREQUAL        0x00000040L
#define D3DPCMPCAPS_ALWAYS              0x00000080L

//
// SourceBlendCaps, DestBlendCaps
//
#define D3DPBLENDCAPS_ZERO              0x00000001L
#define D3DPBLENDCAPS_ONE               0x00000002L
#define D3DPBLENDCAPS_SRCCOLOR          0x00000004L
#define D3DPBLENDCAPS_INVSRCCOLOR       0x00000008L
#define D3DPBLENDCAPS_SRCALPHA          0x00000010L
#define D3DPBLENDCAPS_INVSRCALPHA       0x00000020L
#define D3DPBLENDCAPS_DESTALPHA         0x00000040L
#define D3DPBLENDCAPS_INVDESTALPHA      0x00000080L
#define D3DPBLENDCAPS_DESTCOLOR         0x00000100L
#define D3DPBLENDCAPS_INVDESTCOLOR      0x00000200L
#define D3DPBLENDCAPS_SRCALPHASAT       0x00000400L
#define D3DPBLENDCAPS_BOTHSRCALPHA      0x00000800L
#define D3DPBLENDCAPS_BOTHINVSRCALPHA   0x00001000L

//
// ShadeCaps
//
#define D3DPSHADECAPS_COLORGOURAUDRGB       0x00000008L
#define D3DPSHADECAPS_SPECULARGOURAUDRGB    0x00000200L
#define D3DPSHADECAPS_ALPHAGOURAUDBLEND     0x00004000L
#define D3DPSHADECAPS_FOGGOURAUD            0x00080000L

//
// TextureCaps
//
#define D3DPTEXTURECAPS_PERSPECTIVE         0x00000001L /* Perspective-correct texturing is supported */
#define D3DPTEXTURECAPS_POW2                0x00000002L /* Power-of-2 texture dimensions are required - applies to non-Cube/Volume textures only. */
#define D3DPTEXTURECAPS_ALPHA               0x00000004L /* Alpha in texture pixels is supported */
#define D3DPTEXTURECAPS_SQUAREONLY          0x00000020L /* Only square textures are supported */
#define D3DPTEXTURECAPS_TEXREPEATNOTSCALEDBYSIZE 0x00000040L /* Texture indices are not scaled by the texture size prior to interpolation */
#define D3DPTEXTURECAPS_ALPHAPALETTE        0x00000080L /* Device can draw alpha from texture palettes */
// Device can use non-POW2 textures if:
//  1) D3DTEXTURE_ADDRESS is set to CLAMP for this texture's stage
//  2) D3DRS_WRAP(N) is zero for this texture's coordinates
//  3) mip mapping is not enabled (use magnification filter only)
#define D3DPTEXTURECAPS_NONPOW2CONDITIONAL  0x00000100L
#define D3DPTEXTURECAPS_PROJECTED           0x00000400L /* Device can do D3DTTFF_PROJECTED */
#define D3DPTEXTURECAPS_CUBEMAP             0x00000800L /* Device can do cubemap textures */
#define D3DPTEXTURECAPS_VOLUMEMAP           0x00002000L /* Device can do volume textures */
#define D3DPTEXTURECAPS_MIPMAP              0x00004000L /* Device can do mipmapped textures */
#define D3DPTEXTURECAPS_MIPVOLUMEMAP        0x00008000L /* Device can do mipmapped volume textures */
#define D3DPTEXTURECAPS_MIPCUBEMAP          0x00010000L /* Device can do mipmapped cube maps */
#define D3DPTEXTURECAPS_CUBEMAP_POW2        0x00020000L /* Device requires that cubemaps be power-of-2 dimension */
#define D3DPTEXTURECAPS_VOLUMEMAP_POW2      0x00040000L /* Device requires that volume maps be power-of-2 dimension */

//
// TextureFilterCaps
//
#define D3DPTFILTERCAPS_MINFPOINT           0x00000100L /* Min Filter */
#define D3DPTFILTERCAPS_MINFLINEAR          0x00000200L
#define D3DPTFILTERCAPS_MINFANISOTROPIC     0x00000400L
#define D3DPTFILTERCAPS_MIPFPOINT           0x00010000L /* Mip Filter */
#define D3DPTFILTERCAPS_MIPFLINEAR          0x00020000L
#define D3DPTFILTERCAPS_MAGFPOINT           0x01000000L /* Mag Filter */
#define D3DPTFILTERCAPS_MAGFLINEAR          0x02000000L
#define D3DPTFILTERCAPS_MAGFANISOTROPIC     0x04000000L
#define D3DPTFILTERCAPS_MAGFAFLATCUBIC      0x08000000L
#define D3DPTFILTERCAPS_MAGFGAUSSIANCUBIC   0x10000000L

//
// TextureAddressCaps
//
#define D3DPTADDRESSCAPS_WRAP           0x00000001L
#define D3DPTADDRESSCAPS_MIRROR         0x00000002L
#define D3DPTADDRESSCAPS_CLAMP          0x00000004L
#define D3DPTADDRESSCAPS_BORDER         0x00000008L
#define D3DPTADDRESSCAPS_INDEPENDENTUV  0x00000010L
#define D3DPTADDRESSCAPS_MIRRORONCE     0x00000020L

//
// StencilCaps
//
#define D3DSTENCILCAPS_KEEP             0x00000001L
#define D3DSTENCILCAPS_ZERO             0x00000002L
#define D3DSTENCILCAPS_REPLACE          0x00000004L
#define D3DSTENCILCAPS_INCRSAT          0x00000008L
#define D3DSTENCILCAPS_DECRSAT          0x00000010L
#define D3DSTENCILCAPS_INVERT           0x00000020L
#define D3DSTENCILCAPS_INCR             0x00000040L
#define D3DSTENCILCAPS_DECR             0x00000080L

//
// TextureOpCaps
//
#define D3DTEXOPCAPS_DISABLE                    0x00000001L
#define D3DTEXOPCAPS_SELECTARG1                 0x00000002L
#define D3DTEXOPCAPS_SELECTARG2                 0x00000004L
#define D3DTEXOPCAPS_MODULATE                   0x00000008L
#define D3DTEXOPCAPS_MODULATE2X                 0x00000010L
#define D3DTEXOPCAPS_MODULATE4X                 0x00000020L
#define D3DTEXOPCAPS_ADD                        0x00000040L
#define D3DTEXOPCAPS_ADDSIGNED                  0x00000080L
#define D3DTEXOPCAPS_ADDSIGNED2X                0x00000100L
#define D3DTEXOPCAPS_SUBTRACT                   0x00000200L
#define D3DTEXOPCAPS_ADDSMOOTH                  0x00000400L
#define D3DTEXOPCAPS_BLENDDIFFUSEALPHA          0x00000800L
#define D3DTEXOPCAPS_BLENDTEXTUREALPHA          0x00001000L
#define D3DTEXOPCAPS_BLENDFACTORALPHA           0x00002000L
#define D3DTEXOPCAPS_BLENDTEXTUREALPHAPM        0x00004000L
#define D3DTEXOPCAPS_BLENDCURRENTALPHA          0x00008000L
#define D3DTEXOPCAPS_PREMODULATE                0x00010000L
#define D3DTEXOPCAPS_MODULATEALPHA_ADDCOLOR     0x00020000L
#define D3DTEXOPCAPS_MODULATECOLOR_ADDALPHA     0x00040000L
#define D3DTEXOPCAPS_MODULATEINVALPHA_ADDCOLOR  0x00080000L
#define D3DTEXOPCAPS_MODULATEINVCOLOR_ADDALPHA  0x00100000L
#define D3DTEXOPCAPS_BUMPENVMAP                 0x00200000L
#define D3DTEXOPCAPS_BUMPENVMAPLUMINANCE        0x00400000L
#define D3DTEXOPCAPS_DOTPRODUCT3                0x00800000L
#define D3DTEXOPCAPS_MULTIPLYADD                0x01000000L
#define D3DTEXOPCAPS_LERP                       0x02000000L

//
// FVFCaps
//
#define D3DFVFCAPS_TEXCOORDCOUNTMASK    0x0000ffffL /* mask for texture coordinate count field */
#define D3DFVFCAPS_DONOTSTRIPELEMENTS   0x00080000L /* Device prefers that vertex elements not be stripped */
#define D3DFVFCAPS_PSIZE                0x00100000L /* Device can receive point size */

//
// VertexProcessingCaps
//
#define D3DVTXPCAPS_TEXGEN              0x00000001L /* device can do texgen */
#define D3DVTXPCAPS_MATERIALSOURCE7     0x00000002L /* device can do DX7-level colormaterialsource ops */
#define D3DVTXPCAPS_DIRECTIONALLIGHTS   0x00000008L /* device can do directional lights */
#define D3DVTXPCAPS_POSITIONALLIGHTS    0x00000010L /* device can do positional lights (includes point and spot) */
#define D3DVTXPCAPS_LOCALVIEWER         0x00000020L /* device can do local viewer */
#define D3DVTXPCAPS_TWEENING            0x00000040L /* device can do vertex tweening */
#define D3DVTXPCAPS_NO_VSDT_UBYTE4      0x00000080L /* device does not support D3DVSDT_UBYTE4 */

typedef struct _D3DCAPS8 {
    /* Device Info */
    D3DDEVTYPE  DeviceType;
    UINT    AdapterOrdinal;

    /* Caps from DX7 Draw */
    DWORD   Caps;
    DWORD   Caps2;
    DWORD   Caps3;
    DWORD   PresentationIntervals;

    /* Cursor Caps */
    DWORD   CursorCaps;

    /* 3D Device Caps */
    DWORD   DevCaps;

    DWORD   PrimitiveMiscCaps;
    DWORD   RasterCaps;
    DWORD   ZCmpCaps;
    DWORD   SrcBlendCaps;
    DWORD   DestBlendCaps;
    DWORD   AlphaCmpCaps;
    DWORD   ShadeCaps;
    DWORD   TextureCaps;
    DWORD   TextureFilterCaps;          // D3DPTFILTERCAPS for IDirect3DTexture8's
    DWORD   CubeTextureFilterCaps;      // D3DPTFILTERCAPS for IDirect3DCubeTexture8's
    DWORD   VolumeTextureFilterCaps;    // D3DPTFILTERCAPS for IDirect3DVolumeTexture8's
    DWORD   TextureAddressCaps;         // D3DPTADDRESSCAPS for IDirect3DTexture8's
    DWORD   VolumeTextureAddressCaps;   // D3DPTADDRESSCAPS for IDirect3DVolumeTexture8's

    DWORD   LineCaps;                   // D3DLINECAPS

    DWORD   MaxTextureWidth, MaxTextureHeight;
    DWORD   MaxVolumeExtent;

    DWORD   MaxTextureRepeat;
    DWORD   MaxTextureAspectRatio;
    DWORD   MaxAnisotropy;
    float   MaxVertexW;

    float   GuardBandLeft;
    float   GuardBandTop;
    float   GuardBandRight;
    float   GuardBandBottom;

    float   ExtentsAdjust;
    DWORD   StencilCaps;

    DWORD   FVFCaps;
    DWORD   TextureOpCaps;
    DWORD   MaxTextureBlendStages;
    DWORD   MaxSimultaneousTextures;

    DWORD   VertexProcessingCaps;
    DWORD   MaxActiveLights;
    DWORD   MaxUserClipPlanes;
    DWORD   MaxVertexBlendMatrices;
    DWORD   MaxVertexBlendMatrixIndex;

    float   MaxPointSize;

    DWORD   MaxPrimitiveCount;          // max number of primitives per DrawPrimitive call
    DWORD   MaxVertexIndex;
    DWORD   MaxStreams;
    DWORD   MaxStreamStride;            // max stride for SetStreamSource

    DWORD   VertexShaderVersion;
    DWORD   MaxVertexShaderConst;       // number of vertex shader constant registers

    DWORD   PixelShaderVersion;
    float   MaxPixelShaderValue;        // max value of pixel shader arithmetic component

} D3DCAPS8;

typedef enum _D3DSWAPEFFECT {
    D3DSWAPEFFECT_DISCARD           = 1,
    D3DSWAPEFFECT_FLIP              = 2,
    D3DSWAPEFFECT_COPY              = 3,
    D3DSWAPEFFECT_COPY_VSYNC        = 4,

    D3DSWAPEFFECT_FORCE_DWORD       = 0x7fffffff
} D3DSWAPEFFECT;

struct IDirect3DSurface8;
typedef struct IDirect3DSurface8 IDirect3DSurface8, *LPDIRECT3DSURFACE8;


typedef struct _D3DPRESENT_PARAMETERS_ {
    UINT                BackBufferWidth;
    UINT                BackBufferHeight;
    D3DFORMAT           BackBufferFormat;
    UINT                BackBufferCount;

    D3DMULTISAMPLE_TYPE MultiSampleType;

    D3DSWAPEFFECT       SwapEffect;
    HWND                hDeviceWindow;
    BOOL                Windowed;
    BOOL                EnableAutoDepthStencil;
    D3DFORMAT           AutoDepthStencilFormat;
    DWORD               Flags;

    UINT                FullScreen_RefreshRateInHz;
    UINT                FullScreen_PresentationInterval;

    IDirect3DSurface8  *BufferSurfaces[3];
    IDirect3DSurface8  *DepthStencilSurface;

} D3DPRESENT_PARAMETERS;

#define D3DADAPTER_DEFAULT 0

typedef struct tagRECT {
    LONG left;
    LONG top;
    LONG right;
    LONG bottom;
} RECT, *PRECT, *NPRECT, *LPRECT;

typedef struct _D3DLOCKED_RECT {
    INT                 Pitch;
    void*               pBits;
} D3DLOCKED_RECT;

typedef enum _D3DPRIMITIVETYPE {
    D3DPT_POINTLIST             = NV097_SET_BEGIN_END_OP_POINTS,
    D3DPT_LINELIST              = NV097_SET_BEGIN_END_OP_LINES,
    D3DPT_LINELOOP              = NV097_SET_BEGIN_END_OP_LINE_LOOP,
    D3DPT_LINESTRIP             = NV097_SET_BEGIN_END_OP_LINE_STRIP,
    D3DPT_TRIANGLELIST          = NV097_SET_BEGIN_END_OP_TRIANGLES,
    D3DPT_TRIANGLESTRIP         = NV097_SET_BEGIN_END_OP_TRIANGLE_STRIP,
    D3DPT_TRIANGLEFAN           = NV097_SET_BEGIN_END_OP_TRIANGLE_FAN,
    D3DPT_QUADLIST              = NV097_SET_BEGIN_END_OP_QUADS,
    D3DPT_QUADSTRIP             = NV097_SET_BEGIN_END_OP_QUAD_STRIP,
    D3DPT_POLYGON               = NV097_SET_BEGIN_END_OP_POLYGON,
    D3DPT_FORCE_DWORD           = 0x7fffffff, /* force 32-bit size enum */
} D3DPRIMITIVETYPE;

#define D3D_RENDER_MEMORY_ALIGNMENT 64
#define D3DSURFACE_ALIGNMENT        D3D_RENDER_MEMORY_ALIGNMENT
#define D3DVERTEXBUFFER_ALIGNMENT   1
#define D3DTEXTURE_ALIGNMENT        128
#define D3DPUSHBUFFER_ALIGNMENT     4



typedef struct _D3DPixelShaderDef {
   DWORD    PSAlphaInputs[8];          // Alpha inputs for each stage
   DWORD    PSFinalCombinerInputsABCD; // Final combiner inputs
   DWORD    PSFinalCombinerInputsEFG;  // Final combiner inputs (continued)
   DWORD    PSConstant0[8];            // C0 for each stage
   DWORD    PSConstant1[8];            // C1 for each stage
   DWORD    PSAlphaOutputs[8];         // Alpha output for each stage
   DWORD    PSRGBInputs[8];            // RGB inputs for each stage
   DWORD    PSCompareMode;             // Compare modes for clipplane texture mode /
   DWORD    PSFinalCombinerConstant0;  // C0 in final combiner                     
   DWORD    PSFinalCombinerConstant1;  // C1 in final combiner                     
   DWORD    PSRGBOutputs[8];           // Stage 0 RGB outputs
   DWORD    PSCombinerCount;           // Active combiner count (Stages 0-7)       
   DWORD    PSTextureModes;            // Texture addressing modes                 
   DWORD    PSDotMapping;              // Input mapping for dot product modes      
   DWORD    PSInputTexture;            // Texture source for some texture modes    

   DWORD    PSC0Mapping;               // Mapping of c0 regs to D3D constants
   DWORD    PSC1Mapping;               // Mapping of c1 regs to D3D constants
   DWORD    PSFinalCombinerConstants;  // Final combiner constant mapping
} D3DPIXELSHADERDEF;

typedef enum _D3DRENDERSTATETYPE {

    // Simple render states that are processed by D3D immediately:

    D3DRS_PS_MIN                        = 0,

    // The following pixel-shader renderstates are all Xbox extensions:

    D3DRS_PSALPHAINPUTS0                = 0,   // Pixel shader, Stage 0 alpha inputs
    D3DRS_PSALPHAINPUTS1                = 1,   // Pixel shader, Stage 1 alpha inputs
    D3DRS_PSALPHAINPUTS2                = 2,   // Pixel shader, Stage 2 alpha inputs
    D3DRS_PSALPHAINPUTS3                = 3,   // Pixel shader, Stage 3 alpha inputs
    D3DRS_PSALPHAINPUTS4                = 4,   // Pixel shader, Stage 4 alpha inputs
    D3DRS_PSALPHAINPUTS5                = 5,   // Pixel shader, Stage 5 alpha inputs
    D3DRS_PSALPHAINPUTS6                = 6,   // Pixel shader, Stage 6 alpha inputs
    D3DRS_PSALPHAINPUTS7                = 7,   // Pixel shader, Stage 7 alpha inputs
    D3DRS_PSFINALCOMBINERINPUTSABCD     = 8,   // Pixel shader, Final combiner inputs ABCD
    D3DRS_PSFINALCOMBINERINPUTSEFG      = 9,   // Pixel shader, Final combiner inputs EFG
    D3DRS_PSCONSTANT0_0                 = 10,  // Pixel shader, C0 in stage 0
    D3DRS_PSCONSTANT0_1                 = 11,  // Pixel shader, C0 in stage 1
    D3DRS_PSCONSTANT0_2                 = 12,  // Pixel shader, C0 in stage 2
    D3DRS_PSCONSTANT0_3                 = 13,  // Pixel shader, C0 in stage 3
    D3DRS_PSCONSTANT0_4                 = 14,  // Pixel shader, C0 in stage 4
    D3DRS_PSCONSTANT0_5                 = 15,  // Pixel shader, C0 in stage 5
    D3DRS_PSCONSTANT0_6                 = 16,  // Pixel shader, C0 in stage 6
    D3DRS_PSCONSTANT0_7                 = 17,  // Pixel shader, C0 in stage 7
    D3DRS_PSCONSTANT1_0                 = 18,  // Pixel shader, C1 in stage 0
    D3DRS_PSCONSTANT1_1                 = 19,  // Pixel shader, C1 in stage 1
    D3DRS_PSCONSTANT1_2                 = 20,  // Pixel shader, C1 in stage 2
    D3DRS_PSCONSTANT1_3                 = 21,  // Pixel shader, C1 in stage 3
    D3DRS_PSCONSTANT1_4                 = 22,  // Pixel shader, C1 in stage 4
    D3DRS_PSCONSTANT1_5                 = 23,  // Pixel shader, C1 in stage 5
    D3DRS_PSCONSTANT1_6                 = 24,  // Pixel shader, C1 in stage 6
    D3DRS_PSCONSTANT1_7                 = 25,  // Pixel shader, C1 in stage 7
    D3DRS_PSALPHAOUTPUTS0               = 26,  // Pixel shader, Stage 0 alpha outputs
    D3DRS_PSALPHAOUTPUTS1               = 27,  // Pixel shader, Stage 1 alpha outputs
    D3DRS_PSALPHAOUTPUTS2               = 28,  // Pixel shader, Stage 2 alpha outputs
    D3DRS_PSALPHAOUTPUTS3               = 29,  // Pixel shader, Stage 3 alpha outputs
    D3DRS_PSALPHAOUTPUTS4               = 30,  // Pixel shader, Stage 4 alpha outputs
    D3DRS_PSALPHAOUTPUTS5               = 31,  // Pixel shader, Stage 5 alpha outputs
    D3DRS_PSALPHAOUTPUTS6               = 32,  // Pixel shader, Stage 6 alpha outputs
    D3DRS_PSALPHAOUTPUTS7               = 33,  // Pixel shader, Stage 7 alpha outputs
    D3DRS_PSRGBINPUTS0                  = 34,  // Pixel shader, Stage 0 RGB inputs
    D3DRS_PSRGBINPUTS1                  = 35,  // Pixel shader, Stage 1 RGB inputs
    D3DRS_PSRGBINPUTS2                  = 36,  // Pixel shader, Stage 2 RGB inputs
    D3DRS_PSRGBINPUTS3                  = 37,  // Pixel shader, Stage 3 RGB inputs
    D3DRS_PSRGBINPUTS4                  = 38,  // Pixel shader, Stage 4 RGB inputs
    D3DRS_PSRGBINPUTS5                  = 39,  // Pixel shader, Stage 5 RGB inputs
    D3DRS_PSRGBINPUTS6                  = 40,  // Pixel shader, Stage 6 RGB inputs
    D3DRS_PSRGBINPUTS7                  = 41,  // Pixel shader, Stage 7 RGB inputs
    D3DRS_PSCOMPAREMODE                 = 42,  // Pixel shader, Compare modes for clipplane texture mode
    D3DRS_PSFINALCOMBINERCONSTANT0      = 43,  // Pixel shader, C0 in final combiner
    D3DRS_PSFINALCOMBINERCONSTANT1      = 44,  // Pixel shader, C1 in final combiner
    D3DRS_PSRGBOUTPUTS0                 = 45,  // Pixel shader, Stage 0 RGB outputs
    D3DRS_PSRGBOUTPUTS1                 = 46,  // Pixel shader, Stage 1 RGB outputs
    D3DRS_PSRGBOUTPUTS2                 = 47,  // Pixel shader, Stage 2 RGB outputs
    D3DRS_PSRGBOUTPUTS3                 = 48,  // Pixel shader, Stage 3 RGB outputs
    D3DRS_PSRGBOUTPUTS4                 = 49,  // Pixel shader, Stage 4 RGB outputs
    D3DRS_PSRGBOUTPUTS5                 = 50,  // Pixel shader, Stage 5 RGB outputs
    D3DRS_PSRGBOUTPUTS6                 = 51,  // Pixel shader, Stage 6 RGB outputs
    D3DRS_PSRGBOUTPUTS7                 = 52,  // Pixel shader, Stage 7 RGB outputs
    D3DRS_PSCOMBINERCOUNT               = 53,  // Pixel shader, Active combiner count (Stages 0-7)
                                               // Pixel shader, Reserved
    D3DRS_PSDOTMAPPING                  = 55,  // Pixel shader, Input mapping for dot product modes
    D3DRS_PSINPUTTEXTURE                = 56,  // Pixel shader, Texture source for some texture modes

    D3DRS_PS_MAX                        = 57,

    D3DRS_ZFUNC                         = 57,  // D3DCMPFUNC
    D3DRS_ALPHAFUNC                     = 58,  // D3DCMPFUNC
    D3DRS_ALPHABLENDENABLE              = 59,  // TRUE to enable alpha blending
    D3DRS_ALPHATESTENABLE               = 60,  // TRUE to enable alpha tests
    D3DRS_ALPHAREF                      = 61,  // BYTE
    D3DRS_SRCBLEND                      = 62,  // D3DBLEND
    D3DRS_DESTBLEND                     = 63,  // D3DBLEND
    D3DRS_ZWRITEENABLE                  = 64,  // TRUE to enable Z writes
    D3DRS_DITHERENABLE                  = 65,  // TRUE to enable dithering
    D3DRS_SHADEMODE                     = 66,  // D3DSHADEMODE
    D3DRS_COLORWRITEENABLE              = 67,  // D3DCOLORWRITEENABLE_ALPHA, etc. per-channel write enable
    D3DRS_STENCILZFAIL                  = 68,  // D3DSTENCILOP to do if stencil test passes and Z test fails
    D3DRS_STENCILPASS                   = 69,  // D3DSTENCILOP to do if both stencil and Z tests pass
    D3DRS_STENCILFUNC                   = 70,  // D3DCMPFUNC
    D3DRS_STENCILREF                    = 71,  // BYTE reference value used in stencil test
    D3DRS_STENCILMASK                   = 72,  // BYTE mask value used in stencil test
    D3DRS_STENCILWRITEMASK              = 73,  // BYTE write mask applied to values written to stencil buffer
    D3DRS_BLENDOP                       = 74,  // D3DBLENDOP setting
    D3DRS_BLENDCOLOR                    = 75,  // D3DCOLOR for D3DBLEND_CONSTANTCOLOR, etc. (Xbox extension)
    D3DRS_SWATHWIDTH                    = 76,  // D3DSWATHWIDTH (Xbox extension)
    D3DRS_POLYGONOFFSETZSLOPESCALE      = 77,  // float Z factor for shadow maps (Xbox extension)
    D3DRS_POLYGONOFFSETZOFFSET          = 78,  // float bias for polygon offset (Xbox extension)
    D3DRS_POINTOFFSETENABLE             = 79,  // TRUE to enable polygon offset for points (Xbox extension)
    D3DRS_WIREFRAMEOFFSETENABLE         = 80,  // TRUE to enable polygon offset for lines (Xbox extension)
    D3DRS_SOLIDOFFSETENABLE             = 81,  // TRUE to enable polygon offset for fills (Xbox extension)
    D3DRS_DEPTHCLIPCONTROL              = 82,  // D3DDCC_CULLPRIMITIVE, etc. (Xbox extension)
    D3DRS_STIPPLEENABLE                 = 83,  // TRUE to enable stipple for polygons (Xbox extension)
    D3DRS_SIMPLE_UNUSED8                = 84,  // Reserved
    D3DRS_SIMPLE_UNUSED7                = 85,  // Reserved
    D3DRS_SIMPLE_UNUSED6                = 86,  // Reserved
    D3DRS_SIMPLE_UNUSED5                = 87,  // Reserved
    D3DRS_SIMPLE_UNUSED4                = 88,  // Reserved
    D3DRS_SIMPLE_UNUSED3                = 89,  // Reserved
    D3DRS_SIMPLE_UNUSED2                = 90,  // Reserved
    D3DRS_SIMPLE_UNUSED1                = 91,  // Reserved

    D3DRS_SIMPLE_MAX                    = 92,

    // State whose handling is deferred until the next Draw[Indexed]Vertices
    // call because of interdependencies on other states:

    D3DRS_FOGENABLE                     = 92,  // TRUE to enable fog blending
    D3DRS_FOGTABLEMODE                  = 93,  // D3DFOGMODE
    D3DRS_FOGSTART                      = 94,  // float fog start (for both vertex and pixel fog)
    D3DRS_FOGEND                        = 95,  // float fog end
    D3DRS_FOGDENSITY                    = 96,  // float fog density
    D3DRS_RANGEFOGENABLE                = 97,  // TRUE to enable range-based fog
    D3DRS_WRAP0                         = 98,  // D3DWRAPCOORD_0, etc. for 1st texture coord.
    D3DRS_WRAP1                         = 99,  // D3DWRAPCOORD_0, etc. for 2nd texture coord.
    D3DRS_WRAP2                         = 100, // D3DWRAPCOORD_0, etc. for 3rd texture coord.
    D3DRS_WRAP3                         = 101, // D3DWRAPCOORD_0, etc. for 4th texture coord.
    D3DRS_LIGHTING                      = 102, // TRUE to enable lighting
    D3DRS_SPECULARENABLE                = 103, // TRUE to enable specular
    D3DRS_LOCALVIEWER                   = 104, // TRUE to enable camera-relative specular highlights
    D3DRS_COLORVERTEX                   = 105, // TRUE to enable per-vertex color
    D3DRS_BACKSPECULARMATERIALSOURCE    = 106, // D3DMATERIALCOLORSOURCE (Xbox extension)
    D3DRS_BACKDIFFUSEMATERIALSOURCE     = 107, // D3DMATERIALCOLORSOURCE (Xbox extension)
    D3DRS_BACKAMBIENTMATERIALSOURCE     = 108, // D3DMATERIALCOLORSOURCE (Xbox extension)
    D3DRS_BACKEMISSIVEMATERIALSOURCE    = 109, // D3DMATERIALCOLORSOURCE (Xbox extension)
    D3DRS_SPECULARMATERIALSOURCE        = 110, // D3DMATERIALCOLORSOURCE
    D3DRS_DIFFUSEMATERIALSOURCE         = 111, // D3DMATERIALCOLORSOURCE
    D3DRS_AMBIENTMATERIALSOURCE         = 112, // D3DMATERIALCOLORSOURCE
    D3DRS_EMISSIVEMATERIALSOURCE        = 113, // D3DMATERIALCOLORSOURCE
    D3DRS_BACKAMBIENT                   = 114, // D3DCOLOR (Xbox extension)
    D3DRS_AMBIENT                       = 115, // D3DCOLOR
    D3DRS_POINTSIZE                     = 116, // float point size
    D3DRS_POINTSIZE_MIN                 = 117, // float point size min threshold
    D3DRS_POINTSPRITEENABLE             = 118, // TRUE to enable point sprites
    D3DRS_POINTSCALEENABLE              = 119, // TRUE to enable point size scaling
    D3DRS_POINTSCALE_A                  = 120, // float point attenuation A value
    D3DRS_POINTSCALE_B                  = 121, // float point attenuation B value
    D3DRS_POINTSCALE_C                  = 122, // float point attenuation C value
    D3DRS_POINTSIZE_MAX                 = 123, // float point size max threshold
    D3DRS_PATCHEDGESTYLE                = 124, // D3DPATCHEDGESTYLE
    D3DRS_PATCHSEGMENTS                 = 125, // DWORD number of segments per edge when drawing patches
    D3DRS_SWAPFILTER                    = 126, // D3DTEXF_LINEAR etc. filter to use for Swap (Xbox extension)
    D3DRS_PRESENTATIONINTERVAL          = 127, // D3DPRESENT_INTERVAL_ONE, etc. (Xbox extension)
    D3DRS_DEFERRED_UNUSED8              = 128, // Reserved
    D3DRS_DEFERRED_UNUSED7              = 129, // Reserved
    D3DRS_DEFERRED_UNUSED6              = 130, // Reserved
    D3DRS_DEFERRED_UNUSED5              = 131, // Reserved
    D3DRS_DEFERRED_UNUSED4              = 132, // Reserved
    D3DRS_DEFERRED_UNUSED3              = 133, // Reserved
    D3DRS_DEFERRED_UNUSED2              = 134, // Reserved
    D3DRS_DEFERRED_UNUSED1              = 135, // Reserved

    D3DRS_DEFERRED_MAX                  = 136,

    // Complex state that has immediate processing:

    D3DRS_PSTEXTUREMODES                = 136, // Pixel shader, Texture addressing modes (Xbox extension)
    D3DRS_VERTEXBLEND                   = 137, // D3DVERTEXBLENDFLAGS
    D3DRS_FOGCOLOR                      = 138, // D3DCOLOR
    D3DRS_FILLMODE                      = 139, // D3DFILLMODE
    D3DRS_BACKFILLMODE                  = 140, // D3DFILLMODE (Xbox extension)
    D3DRS_TWOSIDEDLIGHTING              = 141, // TRUE to enable two-sided lighting (Xbox extension)
    D3DRS_NORMALIZENORMALS              = 142, // TRUE to enable automatic normalization
    D3DRS_ZENABLE                       = 143, // D3DZBUFFERTYPE (or TRUE/FALSE for legacy)
    D3DRS_STENCILENABLE                 = 144, // TRUE to enable stenciling
    D3DRS_STENCILFAIL                   = 145, // D3DSTENCILOP to do if stencil test fails
    D3DRS_FRONTFACE                     = 146, // D3DFRONT (Xbox extension)
    D3DRS_CULLMODE                      = 147, // D3DCULL
    D3DRS_TEXTUREFACTOR                 = 148, // D3DCOLOR used for multi-texture blend
    D3DRS_ZBIAS                         = 149, // LONG Z bias
    D3DRS_LOGICOP                       = 150, // D3DLOGICOP (Xbox extension)
    D3DRS_EDGEANTIALIAS                 = 151, // TRUE to enable edge antialiasing (Xbox extension)
    D3DRS_MULTISAMPLEANTIALIAS          = 152, // TRUE to enable multisample antialiasing
    D3DRS_MULTISAMPLEMASK               = 153, // DWORD per-pixel and per-sample enable/disable
    D3DRS_MULTISAMPLEMODE               = 154, // D3DMULTISAMPLEMODE for the backbuffer (Xbox extension)
    D3DRS_MULTISAMPLERENDERTARGETMODE   = 155, // D3DMULTISAMPLEMODE for non-backbuffer render targets (Xbox extension)
    D3DRS_SHADOWFUNC                    = 156, // D3DCMPFUNC (Xbox extension)
    D3DRS_LINEWIDTH                     = 157, // float (Xbox extension)
    D3DRS_SAMPLEALPHA                   = 158, // D3DSAMPLEALPHA_TOCOVERAGE, etc. (Xbox extension)
    D3DRS_DXT1NOISEENABLE               = 159, // TRUE to enable DXT1 decompression noise (Xbox extension)
    D3DRS_YUVENABLE                     = 160, // TRUE to enable use of D3DFMT_YUY2 and D3DFMT_UYVY texture formats (Xbox extension)
    D3DRS_OCCLUSIONCULLENABLE           = 161, // TRUE to enable Z occlusion culling (Xbox extension)
    D3DRS_STENCILCULLENABLE             = 162, // TRUE to enable stencil culling (Xbox extension)
    D3DRS_ROPZCMPALWAYSREAD             = 163, // TRUE to always read target packet when Z enabled (Xbox extension)
    D3DRS_ROPZREAD                      = 164, // TRUE to always read Z (Xbox extension)
    D3DRS_DONOTCULLUNCOMPRESSED         = 165, // TRUE to never attempt occlusion culling (stencil or Z) on uncompressed packets (Xbox extension)

    D3DRS_MAX                           = 166, // Total number of renderstates

    D3DRS_FORCE_DWORD                   = 0x7fffffff, /* force 32-bit size enum */
} D3DRENDERSTATETYPE;

enum PS_CHANNEL {
    PS_CHANNEL_RGB=   0x00, // used as RGB source
    PS_CHANNEL_BLUE=  0x00, // used as ALPHA source
    PS_CHANNEL_ALPHA= 0x10, // used as RGB or ALPHA source
};

enum PS_INPUTMAPPING {
    PS_INPUTMAPPING_UNSIGNED_IDENTITY= 0x00L, // max(0,x)         OK for final combiner
    PS_INPUTMAPPING_UNSIGNED_INVERT=   0x20L, // 1 - max(0,x)     OK for final combiner
    PS_INPUTMAPPING_EXPAND_NORMAL=     0x40L, // 2*max(0,x) - 1   invalid for final combiner
    PS_INPUTMAPPING_EXPAND_NEGATE=     0x60L, // 1 - 2*max(0,x)   invalid for final combiner
    PS_INPUTMAPPING_HALFBIAS_NORMAL=   0x80L, // max(0,x) - 1/2   invalid for final combiner
    PS_INPUTMAPPING_HALFBIAS_NEGATE=   0xa0L, // 1/2 - max(0,x)   invalid for final combiner
    PS_INPUTMAPPING_SIGNED_IDENTITY=   0xc0L, // x                invalid for final combiner
    PS_INPUTMAPPING_SIGNED_NEGATE=     0xe0L, // -x               invalid for final combiner
};

enum PS_REGISTER {
    PS_REGISTER_ZERO=              0x00L, // r
    PS_REGISTER_DISCARD=           0x00L, // w
    PS_REGISTER_C0=                0x01L, // r
    PS_REGISTER_C1=                0x02L, // r
    PS_REGISTER_FOG=               0x03L, // r
    PS_REGISTER_V0=                0x04L, // r/w
    PS_REGISTER_V1=                0x05L, // r/w
    PS_REGISTER_T0=                0x08L, // r/w
    PS_REGISTER_T1=                0x09L, // r/w
    PS_REGISTER_T2=                0x0aL, // r/w
    PS_REGISTER_T3=                0x0bL, // r/w
    PS_REGISTER_R0=                0x0cL, // r/w
    PS_REGISTER_R1=                0x0dL, // r/w
    PS_REGISTER_V1R0_SUM=          0x0eL, // r
    PS_REGISTER_EF_PROD=           0x0fL, // r

    PS_REGISTER_ONE=               PS_REGISTER_ZERO | PS_INPUTMAPPING_UNSIGNED_INVERT, // OK for final combiner
    PS_REGISTER_NEGATIVE_ONE=      PS_REGISTER_ZERO | PS_INPUTMAPPING_EXPAND_NORMAL,   // invalid for final combiner
    PS_REGISTER_ONE_HALF=          PS_REGISTER_ZERO | PS_INPUTMAPPING_HALFBIAS_NEGATE, // invalid for final combiner
    PS_REGISTER_NEGATIVE_ONE_HALF= PS_REGISTER_ZERO | PS_INPUTMAPPING_HALFBIAS_NORMAL, // invalid for final combiner
};

typedef struct _D3DVIEWPORT8 {
    DWORD       X;
    DWORD       Y;
    DWORD       Width;
    DWORD       Height;
    float       MinZ;
    float       MaxZ;
} D3DVIEWPORT8;

typedef struct {
    DWORD StreamIndex;
    DWORD Offset;
    DWORD Format;
    BYTE TessType;
    BYTE TessSource;
} D3DVERTEXSHADERINPUT;

typedef struct {
    D3DVERTEXSHADERINPUT Input[16];
} D3DVERTEXATTRIBUTEFORMAT;

struct IDirect3DVertexBuffer8;
typedef struct IDirect3DVertexBuffer8 IDirect3DVertexBuffer8, *LPDIRECT3DVERTEXBUFFER8;
typedef struct _D3DSTREAM_INPUT { 
    LPDIRECT3DVERTEXBUFFER8 VertexBuffer;
    UINT Stride;
    UINT Offset;
} D3DSTREAM_INPUT;

typedef struct _D3DSURFACE_DESC {
    D3DFORMAT           Format;
    D3DRESOURCETYPE     Type;
    DWORD               Usage;
    UINT                Size;
    D3DMULTISAMPLE_TYPE MultiSampleType;
    UINT                Width;
    UINT                Height;
} D3DSURFACE_DESC;

#define D3DVSDT_FLOAT1      ((1 << 4) | NV097_SET_VERTEX_DATA_ARRAY_FORMAT_TYPE_F) // 1D float expanded to (value, 0., 0., 1.)
#define D3DVSDT_FLOAT2      ((2 << 4) | NV097_SET_VERTEX_DATA_ARRAY_FORMAT_TYPE_F) // 2D float expanded to (value, value, 0., 1.)
#define D3DVSDT_FLOAT3      ((3 << 4) | NV097_SET_VERTEX_DATA_ARRAY_FORMAT_TYPE_F) // 3D float expanded to (value, value, value, 1.)
#define D3DVSDT_FLOAT4      ((4 << 4) | NV097_SET_VERTEX_DATA_ARRAY_FORMAT_TYPE_F) // 4D float
#define D3DVSDT_D3DCOLOR    ((4 << 4) | NV097_SET_VERTEX_DATA_ARRAY_FORMAT_TYPE_UB_D3D) // 4D packed unsigned bytes mapped to 0. to 1. range
                                    // Input is in D3DCOLOR format (ARGB) expanded to (R, G, B, A)
#define D3DVSDT_SHORT1      ((1 << 4) | NV097_SET_VERTEX_DATA_ARRAY_FORMAT_TYPE_S32K) // 1D signed short expanded to (value, 0., 0., 1.)
#define D3DVSDT_SHORT2      ((2 << 4) | NV097_SET_VERTEX_DATA_ARRAY_FORMAT_TYPE_S32K) // 2D signed short expanded to (value, value, 0., 1.)
#define D3DVSDT_SHORT3      ((3 << 4) | NV097_SET_VERTEX_DATA_ARRAY_FORMAT_TYPE_S32K) // 3D signed short expanded to (value, value, value, 1.)
#define D3DVSDT_SHORT4      ((4 << 4) | NV097_SET_VERTEX_DATA_ARRAY_FORMAT_TYPE_S32K) // 4D signed short

#define D3DVSDT_NORMSHORT1  ((1 << 4) | NV097_SET_VERTEX_DATA_ARRAY_FORMAT_TYPE_S1) // 1D signed, normalized short expanded to (value, 0, 0., 1.)
                                    // (signed, normalized short maps from -1.0 to 1.0)
#define D3DVSDT_NORMSHORT2  ((2 << 4) | NV097_SET_VERTEX_DATA_ARRAY_FORMAT_TYPE_S1) // 2D signed, normalized short expanded to (value, value, 0., 1.)
#define D3DVSDT_NORMSHORT3  ((3 << 4) | NV097_SET_VERTEX_DATA_ARRAY_FORMAT_TYPE_S1) // 3D signed, normalized short expanded to (value, value, value, 1.)
#define D3DVSDT_NORMSHORT4  ((4 << 4) | NV097_SET_VERTEX_DATA_ARRAY_FORMAT_TYPE_S1) // 4D signed, normalized short expanded to (value, value, value, value)
#define D3DVSDT_NORMPACKED3 ((1 << 4) | NV097_SET_VERTEX_DATA_ARRAY_FORMAT_TYPE_CMP)    // 3 signed, normalized components packed in 32-bits.  (11,11,10).
                                    // Each component ranges from -1.0 to 1.0.
                                    // Expanded to (value, value, value, 1.)

                                    // Signed shorts map to the range [-32768, 32767]
#define D3DVSDT_PBYTE1      ((1 << 4) | NV097_SET_VERTEX_DATA_ARRAY_FORMAT_TYPE_UB_OGL) // 1D packed byte expanded to (value, 0., 0., 1.)
                                    // Packed bytes map to the range [0, 1]
#define D3DVSDT_PBYTE2      ((1 << 4) | NV097_SET_VERTEX_DATA_ARRAY_FORMAT_TYPE_UB_OGL) // 2D packed byte expanded to (value, value, 0., 1.)
#define D3DVSDT_PBYTE3      ((1 << 4) | NV097_SET_VERTEX_DATA_ARRAY_FORMAT_TYPE_UB_OGL) // 3D packed byte expanded to (value, value, value, 1.)
#define D3DVSDT_PBYTE4      ((1 << 4) | NV097_SET_VERTEX_DATA_ARRAY_FORMAT_TYPE_UB_OGL) // 4D packed byte expanded to (value, value, value, value)
#define D3DVSDT_FLOAT2H     0x72    // 2D homogeneous float expanded to (value, value,0., value.)
                                    // Useful for projective texture coordinates.
#define D3DVSDT_NONE        0x02    // No stream data

// pixel/vertex shader end token
#define D3DPS_END()  0x0000FFFF
#define D3DVS_END()  0x0000FFFF

typedef DWORD D3DPOOL;

#define D3DPOOL_DEFAULT             0
#define D3DPOOL_MANAGED             1
#define D3DPOOL_SYSTEMMEM           2

typedef enum _D3DTEXTURESTAGESTATETYPE {
    D3DTSS_ADDRESSU              =  0,  
    D3DTSS_ADDRESSV              =  1,  
    D3DTSS_ADDRESSW              =  2,  
    D3DTSS_MAGFILTER             =  3,  
    D3DTSS_MINFILTER             =  4,  
    D3DTSS_MIPFILTER             =  5,  
    D3DTSS_MIPMAPLODBIAS         =  6,  
    D3DTSS_MAXMIPLEVEL           =  7,  
    D3DTSS_MAXANISOTROPY         =  8,  
    D3DTSS_COLORKEYOP            =  9,  
    D3DTSS_COLORSIGN             = 10,  
    D3DTSS_ALPHAKILL             = 11,  

    D3DTSS_DEFERRED_TEXTURE_STATE_MAX = 12,

    D3DTSS_COLOROP               = 12,  
    D3DTSS_COLORARG0             = 13,  
    D3DTSS_COLORARG1             = 14,  
    D3DTSS_COLORARG2             = 15,  
    D3DTSS_ALPHAOP               = 16,  
    D3DTSS_ALPHAARG0             = 17,  
    D3DTSS_ALPHAARG1             = 18,  
    D3DTSS_ALPHAARG2             = 19,  
    D3DTSS_RESULTARG             = 20,  
    D3DTSS_TEXTURETRANSFORMFLAGS = 21,  

    D3DTSS_DEFERRED_MAX          = 22,

    D3DTSS_BUMPENVMAT00          = 22,  
    D3DTSS_BUMPENVMAT01          = 23,  
    D3DTSS_BUMPENVMAT11          = 24,  
    D3DTSS_BUMPENVMAT10          = 25,  
    D3DTSS_BUMPENVLSCALE         = 26,  
    D3DTSS_BUMPENVLOFFSET        = 27,  
    D3DTSS_TEXCOORDINDEX         = 28,  
    D3DTSS_BORDERCOLOR           = 29,  
    D3DTSS_COLORKEYCOLOR         = 30,
                                 
    D3DTSS_MAX                   = 32,
    D3DTSS_FORCE_DWORD           = 0x7fffffff,
} D3DTEXTURESTAGESTATETYPE;

#define PS_COMBINERINPUTS(a,b,c,d) ((DWORD)(((a)<<24)|((b)<<16)|((c)<<8)|(d)))
#define PS_COMBINEROUTPUTS(ab,cd,mux_sum,flags) ((DWORD)(((flags)<<12)|((mux_sum)<<8)|((ab)<<4)|(cd)))

enum PS_COMBINEROUTPUT
{
    PS_COMBINEROUTPUT_IDENTITY=            0x00L, // y = x
    PS_COMBINEROUTPUT_BIAS=                0x08L, // y = x - 0.5
    PS_COMBINEROUTPUT_SHIFTLEFT_1=         0x10L, // y = x*2
    PS_COMBINEROUTPUT_SHIFTLEFT_1_BIAS=    0x18L, // y = (x - 0.5)*2
    PS_COMBINEROUTPUT_SHIFTLEFT_2=         0x20L, // y = x*4
    PS_COMBINEROUTPUT_SHIFTRIGHT_1=        0x30L, // y = x/2

    PS_COMBINEROUTPUT_AB_BLUE_TO_ALPHA=    0x80L, // RGB only

    PS_COMBINEROUTPUT_CD_BLUE_TO_ALPHA=    0x40L, // RGB only

    PS_COMBINEROUTPUT_AB_MULTIPLY=         0x00L,
    PS_COMBINEROUTPUT_AB_DOT_PRODUCT=      0x02L, // RGB only

    PS_COMBINEROUTPUT_CD_MULTIPLY=         0x00L,
    PS_COMBINEROUTPUT_CD_DOT_PRODUCT=      0x01L, // RGB only

    PS_COMBINEROUTPUT_AB_CD_SUM=           0x00L, // 3rd output is AB+CD
    PS_COMBINEROUTPUT_AB_CD_MUX=           0x04L, // 3rd output is MUX(AB,CD) based on R0.a
};

#define PS_COMBINERCOUNT(count, flags) ((DWORD)(((flags)<<8)|(count)))
// count is 1-8, flags contains one or more values from PS_COMBINERCOUNTFLAGS

enum PS_COMBINERCOUNTFLAGS {
    PS_COMBINERCOUNT_MUX_LSB=     0x0000L, // mux on r0.a lsb
    PS_COMBINERCOUNT_MUX_MSB=     0x0001L, // mux on r0.a msb

    PS_COMBINERCOUNT_SAME_C0=     0x0000L, // c0 same in each stage
    PS_COMBINERCOUNT_UNIQUE_C0=   0x0010L, // c0 unique in each stage

    PS_COMBINERCOUNT_SAME_C1=     0x0000L, // c1 same in each stage
    PS_COMBINERCOUNT_UNIQUE_C1=   0x0100L  // c1 unique in each stage
};

#define PS_INPUTTEXTURE(t0,t1,t2,t3) ((DWORD)(((t3)<<20)|((t2)<<16)))

#define PS_TEXTUREMODES(t0,t1,t2,t3) ((DWORD)(((t3)<<15)|((t2)<<10)|((t1)<<5)|(t0)))
enum PS_TEXTUREMODES {                                 // valid in stage 0 1 2 3
    PS_TEXTUREMODES_NONE=                 0x00L, // * * * *
    PS_TEXTUREMODES_PROJECT2D=            0x01L, // * * * *
    PS_TEXTUREMODES_PROJECT3D=            0x02L, // * * * *
    PS_TEXTUREMODES_CUBEMAP=              0x03L, // * * * *
    PS_TEXTUREMODES_PASSTHRU=             0x04L, // * * * *
    PS_TEXTUREMODES_CLIPPLANE=            0x05L, // * * * *
    PS_TEXTUREMODES_BUMPENVMAP=           0x06L, // - * * *
    PS_TEXTUREMODES_BUMPENVMAP_LUM=       0x07L, // - * * *
    PS_TEXTUREMODES_BRDF=                 0x08L, // - - * *
    PS_TEXTUREMODES_DOT_ST=               0x09L, // - - * *
    PS_TEXTUREMODES_DOT_ZW=               0x0aL, // - - * *
    PS_TEXTUREMODES_DOT_RFLCT_DIFF=       0x0bL, // - - * -
    PS_TEXTUREMODES_DOT_RFLCT_SPEC=       0x0cL, // - - - *
    PS_TEXTUREMODES_DOT_STR_3D=           0x0dL, // - - - *
    PS_TEXTUREMODES_DOT_STR_CUBE=         0x0eL, // - - - *
    PS_TEXTUREMODES_DPNDNT_AR=            0x0fL, // - * * *
    PS_TEXTUREMODES_DPNDNT_GB=            0x10L, // - * * *
    PS_TEXTUREMODES_DOTPRODUCT=           0x11L, // - * * -
    PS_TEXTUREMODES_DOT_RFLCT_SPEC_CONST= 0x12L, // - - - *
    // 0x13-0x1f reserved
};

#define D3DCLEAR_TARGET            0x000000f0l  /* Clear target surface */
#define D3DCLEAR_ZBUFFER           0x00000001l  /* Clear target z buffer */
#define D3DCLEAR_STENCIL           0x00000002l  /* Clear stencil planes */

// The following are Xbox extensions
#define D3DCLEAR_TARGET_R          0x00000010l  /* Clear target surface R component */
#define D3DCLEAR_TARGET_G          0x00000020l  /* Clear target surface G component */
#define D3DCLEAR_TARGET_B          0x00000040l  /* Clear target surface B component */
#define D3DCLEAR_TARGET_A          0x00000080l  /* Clear target surface A component */

#define D3DZ_MAX_D16         65535.0
#define D3DZ_MAX_D24S8       16777215.0
#define D3DZ_MAX_F16         511.9375

typedef DWORD D3DCOLOR;
#define D3DCOLOR_ARGB(a,r,g,b) \
    ((D3DCOLOR)((((a)&0xff)<<24)|(((r)&0xff)<<16)|(((g)&0xff)<<8)|((b)&0xff)))
#define D3DCOLOR_RGBA(r,g,b,a) D3DCOLOR_ARGB(a,r,g,b)
#define D3DCOLOR_XRGB(r,g,b)   D3DCOLOR_ARGB(0xff,r,g,b)

typedef struct _D3DRECT {
    LONG x1;
    LONG y1;
    LONG x2;
    LONG y2;
} D3DRECT;

#endif // NXDK_HAVE_D3D8TYPES