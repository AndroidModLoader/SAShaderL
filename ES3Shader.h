#include <stdint.h>

#ifdef AML32
    #include "AArchASMHelper/Thumbv7_ASMHelper.h"
    #include "GTASA_STRUCTS.h"
    using namespace ThumbV7;
#else
    #include "AArchASMHelper/ARMv8_ASMHelper.h"
    #include "GTASA_STRUCTS_210.h"
    using namespace ARMv8;
#endif

#define FLAG_ALPHA_TEST           0x01
#define FLAG_LIGHTING             0x02
#define FLAG_ALPHA_MODULATE       0x04
#define FLAG_COLOR_EMISSIVE       0x08
#define FLAG_COLOR                0x10
#define FLAG_TEX0                 0x20
#define FLAG_ENVMAP               0x40          // normal envmap
#define FLAG_BONE3                0x80
#define FLAG_BONE4                0x100
#define FLAG_CAMERA_BASED_NORMALS 0x200
#define FLAG_FOG                  0x400
#define FLAG_TEXBIAS              0x800
#define FLAG_BACKLIGHT            0x1000
#define FLAG_LIGHT1               0x2000
#define FLAG_LIGHT2               0x4000
#define FLAG_LIGHT3               0x8000
#define FLAG_DETAILMAP            0x10000
#define FLAG_COMPRESSED_TEXCOORD  0x20000
#define FLAG_PROJECT_TEXCOORD     0x40000
#define FLAG_WATER                0x80000
#define FLAG_COLOR2               0x100000
#define FLAG_SPHERE_XFORM         0x800000      // this renders the scene as a sphere map for vehicle reflections
#define FLAG_SPHERE_ENVMAP        0x1000000     // spherical real-time envmap
#define FLAG_TEXMATRIX            0x2000000
#define FLAG_GAMMA                0x4000000

#define FLAG_CUSTOM_SKY           0x8000000
#define FLAG_CUSTOM_BUILDING      0x10000000
#define FLAG_CUSTOM_POSTPROCESS   0x20000000
#define FLAG_CUSTOM2              0x40000000
#define FLAG_CUSTOM1              0x80000000

class ES3Shader : public ES2Shader
{
public:
    int uid_nShaderFlags;
    int uid_fAngle;
    int uid_nTime;
    int uid_nGameTimeSeconds;
    int uid_fUnderWaterness;
    int uid_fRoadsWetness;
    int uid_fFarClipDist;
    int uid_nEntityModel;
};