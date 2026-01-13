#ifndef NXDK_HAVE_D3D8
#define NXDK_HAVE_D3D8

#include <windows.h>
#include <winnt.h>

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

typedef struct _GUID {
    unsigned long  Data1;
    unsigned short Data2;
    unsigned short Data3;
    unsigned char  Data4[8];
} GUID;

typedef GUID IID;
typedef IID *REFIID;
typedef HANDLE HMONITOR;

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

#define D3DERR_NOTFOUND                         MAKE_D3DHRESULT(2150)
#define D3DERR_MOREDATA                         MAKE_D3DHRESULT(2151)
#define D3DERR_DEVICELOST                       MAKE_D3DHRESULT(2152)
#define D3DERR_DEVICENOTRESET                   MAKE_D3DHRESULT(2153)
#define D3DERR_NOTAVAILABLE                     MAKE_D3DHRESULT(2154)
#define D3DERR_OUTOFVIDEOMEMORY                 MAKE_D3DHRESULT(380)
#define D3DERR_INVALIDDEVICE                    MAKE_D3DHRESULT(2155)
#define D3DERR_INVALIDCALL                      MAKE_D3DHRESULT(2156)
#define D3DERR_DRIVERINVALIDCALL                MAKE_D3DHRESULT(2157)

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

    D3DFMT_A8R8G8B8             = 0x00000006,
    D3DFMT_X8R8G8B8             = 0x00000007,
    D3DFMT_R5G6B5               = 0x00000005,
    D3DFMT_R6G5B5               = 0x00000027,
    D3DFMT_X1R5G5B5             = 0x00000003,
    D3DFMT_A1R5G5B5             = 0x00000002,
    D3DFMT_A4R4G4B4             = 0x00000004,
    D3DFMT_A8                   = 0x00000019,
    D3DFMT_A8B8G8R8             = 0x0000003A,
    D3DFMT_B8G8R8A8             = 0x0000003B,
    D3DFMT_R4G4B4A4             = 0x00000039,
    D3DFMT_R5G5B5A1             = 0x00000038,
    D3DFMT_R8G8B8A8             = 0x0000003C,
    D3DFMT_R8B8                 = 0x00000029,
    D3DFMT_G8B8                 = 0x00000028,

    D3DFMT_P8                   = 0x0000000B,

    D3DFMT_L8                   = 0x00000000,
    D3DFMT_A8L8                 = 0x0000001A,
    D3DFMT_AL8                  = 0x00000001,
    D3DFMT_L16                  = 0x00000032,

    D3DFMT_V8U8                 = 0x00000028,
    D3DFMT_L6V5U5               = 0x00000027,
    D3DFMT_X8L8V8U8             = 0x00000007,
    D3DFMT_Q8W8V8U8             = 0x0000003A,
    D3DFMT_V16U16               = 0x00000033,

    D3DFMT_D16_LOCKABLE         = 0x0000002C,
    D3DFMT_D16                  = 0x0000002C,
    D3DFMT_D24S8                = 0x0000002A,
    D3DFMT_F16                  = 0x0000002D,
    D3DFMT_F24S8                = 0x0000002B,

    /* YUV formats */

    D3DFMT_YUY2                 = 0x00000024,
    D3DFMT_UYVY                 = 0x00000025,

    /* Compressed formats */

    D3DFMT_DXT1                 = 0x0000000C,
    D3DFMT_DXT2                 = 0x0000000E,
    D3DFMT_DXT3                 = 0x0000000E,
    D3DFMT_DXT4                 = 0x0000000F,
    D3DFMT_DXT5                 = 0x0000000F,

    /* Linear formats */

    D3DFMT_LIN_A1R5G5B5         = 0x00000010,
    D3DFMT_LIN_A4R4G4B4         = 0x0000001D,
    D3DFMT_LIN_A8               = 0x0000001F,
    D3DFMT_LIN_A8B8G8R8         = 0x0000003F,
    D3DFMT_LIN_A8R8G8B8         = 0x00000012,
    D3DFMT_LIN_B8G8R8A8         = 0x00000040,
    D3DFMT_LIN_G8B8             = 0x00000017,
    D3DFMT_LIN_R4G4B4A4         = 0x0000003E,
    D3DFMT_LIN_R5G5B5A1         = 0x0000003D,
    D3DFMT_LIN_R5G6B5           = 0x00000011,
    D3DFMT_LIN_R6G5B5           = 0x00000037,
    D3DFMT_LIN_R8B8             = 0x00000016,
    D3DFMT_LIN_R8G8B8A8         = 0x00000041,
    D3DFMT_LIN_X1R5G5B5         = 0x0000001C,
    D3DFMT_LIN_X8R8G8B8         = 0x0000001E,

    D3DFMT_LIN_A8L8             = 0x00000020,
    D3DFMT_LIN_AL8              = 0x0000001B,
    D3DFMT_LIN_L16              = 0x00000035,
    D3DFMT_LIN_L8               = 0x00000013,

    D3DFMT_LIN_V16U16           = 0x00000036,
    D3DFMT_LIN_V8U8             = 0x00000017,
    D3DFMT_LIN_L6V5U5           = 0x00000037,
    D3DFMT_LIN_X8L8V8U8         = 0x0000001E,
    D3DFMT_LIN_Q8W8V8U8         = 0x00000012,

    D3DFMT_LIN_D24S8            = 0x0000002E,
    D3DFMT_LIN_F24S8            = 0x0000002F,
    D3DFMT_LIN_D16              = 0x00000030,
    D3DFMT_LIN_F16              = 0x00000031,

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

#define D3DUSAGE_RENDERTARGET 0x00000001L
#define D3DUSAGE_DEPTHSTENCIL 0x00000002L

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

// ============================================================================
struct IDirect3DResource8;
typedef struct IDirect3DResource8 IDirect3DResource8, *LPDIRECT3DRESOURCE8;

typedef struct Direct3DResourceVtbl8 {
    /*** IUnknown methods ***/
    ULONG   (*AddRef)(LPDIRECT3DRESOURCE8 pThis);
    ULONG   (*Release)(LPDIRECT3DRESOURCE8 pThis);

    /*** IDirect3DResource8 methods ***/
    D3DRESOURCETYPE (*GetType)(LPDIRECT3DRESOURCE8 pThis);
    VOID            (*Register)(LPDIRECT3DRESOURCE8 pThis, PVOID pBase);
    VOID            (*BlockUntilNotBusy)(LPDIRECT3DRESOURCE8 pThis);
    BOOL            (*IsBusy)(LPDIRECT3DRESOURCE8 pThis);
} Direct3DResourceVtbl8, *LPDIRECT3DRESOURCEVTBL8;

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
// ============================================================================

// ============================================================================
struct IDirect3DBaseTexture8;
typedef struct IDirect3DBaseTexture8 IDirect3DBaseTexture8, 
                                     *LPDIRECT3DBASETEXTURE8;

typedef struct Direct3DBaseTextureVtbl8 {
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
} Direct3DBaseTextureVtbl8, *LPDIRECT3DBASETEXTUREVTBL8;

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

typedef struct _D3DSURFACE_DESC {
    D3DFORMAT           Format;
    D3DRESOURCETYPE     Type;
    DWORD               Usage;
    UINT                Size;
    D3DMULTISAMPLE_TYPE MultiSampleType;
    UINT                Width;
    UINT                Height;
} D3DSURFACE_DESC;

// ============================================================================
typedef struct Direct3DSurfaceVtbl8 {
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
} Direct3DSurfaceVtbl8, *LPDIRECT3DSURFACEVTBL8;

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
typedef struct IDirect3DVertexBuffer8 IDirect3DVertexBuffer8, *LPDIRECT3DVERTEXBUFFER8;

typedef struct _D3DVERTEXBUFFER_DESC {
    D3DFORMAT           Format;
    D3DRESOURCETYPE     Type;
} D3DVERTEXBUFFER_DESC;

typedef struct Direct3DVertexBufferVtbl8 {
    /*** IUnknown methods ***/
    ULONG   (*AddRef)(LPDIRECT3DVERTEXBUFFER8 pThis);
    ULONG   (*Release)(LPDIRECT3DVERTEXBUFFER8 pThis);

    /*** IDirect3DResource8 methods ***/
    D3DRESOURCETYPE (*GetType)(LPDIRECT3DVERTEXBUFFER8 pThis);
    VOID            (*Register)(LPDIRECT3DVERTEXBUFFER8 pThis, PVOID pBase);
    VOID            (*BlockUntilNotBusy)(LPDIRECT3DVERTEXBUFFER8 pThis);
    BOOL            (*IsBusy)(LPDIRECT3DVERTEXBUFFER8 pThis);

    /*** IDirect3DVertexBuffer methods ***/
    HRESULT         (*GetDesc)(LPDIRECT3DVERTEXBUFFER8 pThis, D3DVERTEXBUFFER_DESC* pDesc);
    HRESULT         (*Lock)(LPDIRECT3DVERTEXBUFFER8 pThis, UINT OffsetToLock, 
                            UINT SizeToLock, BYTE** ppbData, DWORD Flags);
    HRESULT         (*Unlock)(LPDIRECT3DVERTEXBUFFER8 pThis);
} Direct3DVertexBufferVtbl8, *LPDIRECT3DVERTEXBUFFERVTBL8;

struct IDirect3DVertexBuffer8 INHERITS(IDirect3DResource8) {
    LPDIRECT3DVERTEXBUFFERVTBL8 lpVtbl;
#ifdef __cplusplus
    virtual D3DRESOURCETYPE GetType() = 0;
    virtual VOID            Register(PVOID pBase) = 0;
    virtual VOID            BlockUntilNotBusy() = 0;
    virtual BOOL            IsBusy() = 0;
#endif // __cplusplus
};

ULONG IDirect3DVertexBuffer8_AddRef(LPDIRECT3DVERTEXBUFFER8 pThis);
ULONG IDirect3DVertexBuffer8_Release(LPDIRECT3DVERTEXBUFFER8 pThis);
D3DRESOURCETYPE IDirect3DVertexBuffer8_GetType(LPDIRECT3DVERTEXBUFFER8 pThis);
VOID IDirect3DVertexBuffer8_Register(LPDIRECT3DVERTEXBUFFER8 pThis, PVOID pBase);
HRESULT IDirect3DVertexBuffer8_GetContainer(LPDIRECT3DVERTEXBUFFER8 pThis, 
                                            LPDIRECT3DBASETEXTURE8* ppContainer);
HRESULT IDirect3DVertexBuffer8_GetDesc(LPDIRECT3DVERTEXBUFFER8 pThis, 
                                       D3DVERTEXBUFFER_DESC* pDesc);
HRESULT IDirect3DVertexBuffer8_Lock(LPDIRECT3DVERTEXBUFFER8 pThis, 
                                      UINT OffsetToLock, UINT SizeToLock,
                                      BYTE** ppbData, DWORD Flags);
HRESULT IDirect3DVertexBuffer8_Unlock(LPDIRECT3DVERTEXBUFFER8 pThis);
// ============================================================================

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

typedef struct _D3DSTREAM_INPUT { 
    LPDIRECT3DVERTEXBUFFER8 VertexBuffer;
    UINT Stride;
    UINT Offset;
} D3DSTREAM_INPUT;

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

typedef DWORD D3DPOOL;

#define D3DPOOL_DEFAULT             0
#define D3DPOOL_MANAGED             1
#define D3DPOOL_SYSTEMMEM           2

struct IDirect3DDevice8;
typedef struct IDirect3DDevice8 IDirect3DDevice8, *LPDIRECT3DDEVICE8;

typedef struct Direct3DDeviceVtbl8 {
    /*** IUnknown methods ***/
    ULONG   (*AddRef)(LPDIRECT3DDEVICE8 pThis);
    ULONG   (*Release)(LPDIRECT3DDEVICE8 pThis);

    /*** IDirect3DDevice8 methods ***/
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
    HRESULT (*BeginScene)(LPDIRECT3DDEVICE8 pThis);
    HRESULT (*EndScene)(LPDIRECT3DDEVICE8 pThis);
    HRESULT (*Present)(LPDIRECT3DDEVICE8 pThis, 
                       CONST RECT* pSourceRect, 
                       CONST RECT* pDestRect);
    HRESULT (*DrawPrimitive)(LPDIRECT3DDEVICE8 pThis, 
                             D3DPRIMITIVETYPE PrimitiveType, 
                             UINT StartVertex, UINT PrimitiveCount);
    HRESULT (*SetViewport)(LPDIRECT3DDEVICE8 pThis, 
                           CONST D3DVIEWPORT8* pViewport);
    HRESULT (*SetVertexShaderInputDirect)(
                           LPDIRECT3DDEVICE8 pThis,
                           D3DVERTEXATTRIBUTEFORMAT *pVAF,
                           UINT StreamCount, 
                           D3DSTREAM_INPUT *pStreamInputs);
} Direct3DDeviceVtbl8, *LPDIRECT3DDEVICEVTBL8;

struct IDirect3DDevice8 {
    LPDIRECT3DDEVICEVTBL8 lpVtbl;
#ifdef __cplusplus
    virtual ULONG   AddRef()  = 0;
    virtual ULONG   Release() = 0;

    virtual HRESULT CreateImageSurface(UINT Width, UINT Height,
                                       D3DFORMAT Format, 
                                       LPDIRECT3DSURFACE8* ppSurface) = 0;
#endif // __cplusplus
};

ULONG IDirect3DDevice8_AddRef(LPDIRECT3DDEVICE8 pThis);
ULONG IDirect3DDevice8_Release(LPDIRECT3DDEVICE8 pThis);
HRESULT IDirect3DDevice8_CreateImageSurface(LPDIRECT3DDEVICE8 pThis, UINT Width, 
                                            UINT Height, D3DFORMAT Format, 
                                            LPDIRECT3DSURFACE8* ppSurface);
HRESULT IDirect3DDevice8_CreateDepthStencilSurface(
    LPDIRECT3DDEVICE8 pThis, UINT Width, UINT Height, D3DFORMAT Format,
    D3DMULTISAMPLE_TYPE MultiSample, IDirect3DSurface8** ppSurface);
HRESULT IDirect3DDevice8_CreateVertexBuffer(
    LPDIRECT3DDEVICE8 pThis, UINT Length, DWORD Usage, DWORD FVF, 
    D3DPOOL Pool, LPDIRECT3DVERTEXBUFFER8* ppVertexBuffer);
HRESULT IDirect3DDevice8_BeginScene(LPDIRECT3DDEVICE8 pThis);
HRESULT IDirect3DDevice8_EndScene(LPDIRECT3DDEVICE8 pThis);
HRESULT IDirect3DDevice8_Present(LPDIRECT3DDEVICE8 pThis, 
                                 CONST RECT* pSourceRect, 
                                 CONST RECT* pDestRect);
HRESULT IDirect3DDevice8_DrawPrimitive(LPDIRECT3DDEVICE8 pThis, 
                                       D3DPRIMITIVETYPE PrimitiveType, 
                                       UINT StartVertex, UINT PrimitiveCount);
HRESULT IDirect3DDevice8_SetViewport(LPDIRECT3DDEVICE8 pThis, 
                                     CONST D3DVIEWPORT8* pViewport);
HRESULT IDirect3DDevice8_SetVertexShaderInputDirect(
    LPDIRECT3DDEVICE8 pThis,
    D3DVERTEXATTRIBUTEFORMAT *pVAF,
    UINT StreamCount, 
    D3DSTREAM_INPUT *pStreamInputs);

struct IDirect3D8;
typedef struct IDirect3D8 IDirect3D8, *LPDIRECT3D8;

typedef struct Direct3D8Vtbl8 {
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
} Direct3D8Vtbl8, DIRECT3DVTBL8, *LPDIRECT3DVTBL8;

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