#include <stdint.h>

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

enum RQShaderVectorConstantID
{
    
};
enum RQShaderMatrixConstantID
{
    
};

class ES2Shader
{
public:
    //virtual ~ES2Shader() = 0; // Auto
    virtual void Destruct() = 0;
    virtual void SetVectorConstant(RQShaderVectorConstantID, float const*, int) = 0;
    virtual void SetMatrixConstant(RQShaderMatrixConstantID, float const*) = 0;
    virtual void SetBonesConstant(int, float const*) = 0;
    virtual void SetColorAttribute(float const*) = 0;
    virtual void Select() = 0;
    
public:
    char pad1[1000-4]; // minus vtable
    int nShaderId;
    char pad2[4];
};

class ES3Shader : public ES2Shader
{
public:
    int uid_fAngle;
    int uid_nTime;
    int uid_fUnderWaterness;
};

struct EmuShader
{
    ES3Shader *shader;
    bool ownsShader;
    unsigned int programFlags;
    unsigned int pCodeHash;
    unsigned int vCodeHash;
    EmuShader *nextShader;
};