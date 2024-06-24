#include <stdint.h>
#include <vector>
#include "isasl.h"

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

#define CUSTOM_UNIFORMS           128

struct CustomStaticUniform
{
    static int registeredUniforms;

    struct Data
    {
        union
        {
            int i[4];
            uint32_t u[4];
            float f[4];
        };
        union
        {
            int* iptr;
            uint32_t* uptr;
            float* fptr;
        };
    } data, prevdata;
    const char* name;
    int id;
    eUniformValueType type;
    bool alwaysUpdate; // no checks
    uint8_t count; // 1-4

    inline void SetInt(int dataNum, int value)
    {
        prevdata.i[dataNum] = data.i[dataNum];
        data.i[dataNum] = value;
    }
    inline void SetUInt(int dataNum, uint32_t value)
    {
        prevdata.u[dataNum] = data.u[dataNum];
        data.u[dataNum] = value;
    }
    inline void SetFloat(int dataNum, float value)
    {
        prevdata.f[dataNum] = data.f[dataNum];
        data.f[dataNum] = value;
    }
    inline void SetPtr(int dataNum, void* ptr)
    {
        prevdata.iptr = data.iptr;
        data.iptr = (int*)ptr;
    }
    inline bool IsChanged()
    {
        for(uint8_t i = 0; i < count; ++i) { if(prevdata.i[i] != data.i[i]) return true; }
        return data.iptr != prevdata.iptr;
    }
};

struct CustomUniform
{
    int uniformId;
    bool needToApply;
};

class ES3Shader : public ES2Shader
{
public:
    CustomUniform uniforms[CUSTOM_UNIFORMS];

    //int uid_nShaderFlags;
    //int uid_fAngle;
    //int uid_nTime;
    //int uid_nGameTimeSeconds;
    //int uid_fUnderWaterness;
    //int uid_fRoadsWetness;
    //int uid_fFarClipDist;
    //int uid_nEntityModel;
};
extern std::vector<ES3Shader*> g_AllShaders;
extern CustomStaticUniform staticUniforms[CUSTOM_UNIFORMS];