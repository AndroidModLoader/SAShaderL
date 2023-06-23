#include <mod/amlmod.h>
#include <mod/logger.h>
#include <mod/config.h>
#include <dlfcn.h>

#include <stdio.h>
#include "ES3Shader.h"

#include <GTASA_STRUCTS.h>

//#define DUMP_SHADERS

MYMOD(net.rusjj.sashader, SAShaderLoader, 1.0, RusJJ)
NEEDGAME(com.rockstargames.gtasa)

// Savings
#define SHADER_LEN (16384)
#define FRAGMENT_SHADER_STORAGE(__var1, __var2) sprintf(__var1, "%s/shaders/fragment/" #__var2 ".glsl", aml->GetAndroidDataPath());
#define VERTEX_SHADER_STORAGE(__var1, __var2) sprintf(__var1, "%s/shaders/vertex/" #__var2 ".glsl", aml->GetAndroidDataPath());
#define FRAGMENT_SHADER_GEN_STORAGE(__var1, __var2) sprintf(__var1, "%s/shaders/fragment/gen/%s.glsl", aml->GetAndroidDataPath(), __var2);
#define VERTEX_SHADER_GEN_STORAGE(__var1, __var2) sprintf(__var1, "%s/shaders/vertex/gen/%s.glsl", aml->GetAndroidDataPath(), __var2);
uintptr_t pGTASA;
void* hGTASA;
char blurShaderOwn[SHADER_LEN], gradingShaderOwn[SHADER_LEN], shadowResolveOwn[SHADER_LEN], contrastVertexOwn[SHADER_LEN], contrastFragmentOwn[SHADER_LEN];

// Config


// Game Vars
const char **blurShader, **gradingShader, **shadowResolve, **contrastVertex, **contrastFragment;
CVector *m_VectorToSun;
int *m_CurrentStoredValue;
uint32_t *m_snTimeInMilliseconds, *curShaderStateFlags;
uint8_t *ms_nGameClockSeconds, *ms_nGameClockMinutes;
float* UnderWaterness;
CCamera* TheCamera;
ES3Shader** fragShaders;
ES3Shader** activeShader;

// Own Funcs
inline void freadfull(char* buf, size_t maxlen, FILE *f)
{
    size_t i = 0;
    --maxlen;
    while(!feof(f) && i<maxlen)
    {
        buf[i] = fgetc(f);
        ++i;
    }
    buf[i-1] = 0;
}
inline const char* FlagsToShaderName(int flags, bool isVertex)
{
    if(isVertex)
    {
        switch(flags)
        {
            case 0x10:
            case 0x200010:
                return "untextured2D";
            case 0x4000010:
                return "gammaColor2D";
            case 0x80430:
                return "water";
            case 0x90430:
                return "waterDetailed";

            // Custom shaders below
            case 0x8000010:
                return "sky";
        }
    }
    else // isFragment
    {
        switch(flags)
        {
            case 0x10:
            case 0x200010:
                return "untextured2D";
            case 0x4000010:
                return "gammaColor2D";
            case 0x80430:
                return "water";
            case 0x90430:
                return "waterDetailed";

            // Custom shaders below
            case 0x8000010:
                return "sky";
        }
    }
    return NULL;
}
template <size_t size>
inline void FlagToName(int flags, char (&out)[size])
{
    out[0] = 0;
    if(flags & FLAG_ALPHA_TEST) strlcat(out, "Atest", size);
    if(flags & FLAG_LIGHTING) strlcat(out, "Light", size);
    if(flags & FLAG_ALPHA_MODULATE) strlcat(out, "Mod", size);
    if(flags & FLAG_COLOR_EMISSIVE) strlcat(out, "Emiss", size);
    if(flags & FLAG_COLOR) strlcat(out, "Color", size);
    if(flags & FLAG_TEX0) strlcat(out, "T0", size);
    if(flags & FLAG_ENVMAP) strlcat(out, "Env", size);
    if(flags & FLAG_BONE3) strlcat(out, "B3", size);
    if(flags & FLAG_BONE4) strlcat(out, "B4", size);
    if(flags & FLAG_CAMERA_BASED_NORMALS) strlcat(out, "Norm", size);
    if(flags & FLAG_FOG) strlcat(out, "Fog", size);
    if(flags & FLAG_TEXBIAS) strlcat(out, "Bias", size);
    if(flags & FLAG_BACKLIGHT) strlcat(out, "Backl", size);
    if(flags & FLAG_LIGHT1) strlcat(out, "Light1", size);
    if(flags & FLAG_LIGHT2) strlcat(out, "Light2", size);
    if(flags & FLAG_LIGHT3) strlcat(out, "Light3", size);
    if(flags & FLAG_DETAILMAP) strlcat(out, "Detail", size);
    if(flags & FLAG_COMPRESSED_TEXCOORD) strlcat(out, "Comp", size);
    if(flags & FLAG_PROJECT_TEXCOORD) strlcat(out, "Proj", size);
    if(flags & FLAG_WATER) strlcat(out, "Water", size);
    if(flags & FLAG_COLOR2) strlcat(out, "Color2", size);
    if(flags & FLAG_SPHERE_XFORM) strlcat(out, "Xform", size);
    if(flags & FLAG_SPHERE_ENVMAP) strlcat(out, "Envmap", size);
    if(flags & FLAG_TEXMATRIX) strlcat(out, "Matrix", size);
    if(flags & FLAG_GAMMA) strlcat(out, "Gamma", size);

    if(flags & FLAG_CUSTOM_SKY) strlcat(out, "Sky", size);
    if(flags & FLAG_CUSTOM_BUILDING) strlcat(out, "Building", size);
}

// Game Funcs
EmuShader* (*emu_CustomShaderCreate)(const char* fragShad, const char* vertShad);
int (*_glGetUniformLocation)(int, const char*);
void (*_glUniform1i)(int, int);
void (*_glUniform1fv)(int, int, const float*);
DECL_HOOK(int, RQShaderBuildSource, int flags, char **pxlsrc, char **vtxsrc)
{
    int ret = RQShaderBuildSource(flags, pxlsrc, vtxsrc);
    FILE *pFile;
    char szTmp[256], szNameCat[128];
    FlagToName(flags, szNameCat);
    
    #ifdef DUMP_SHADERS
        const char* fragName = FlagsToShaderName(flags, false);
        if(fragName)
        {
            FRAGMENT_SHADER_GEN_STORAGE(szTmp, fragName);
            pFile = fopen(szTmp, "w");
            if(pFile != NULL)
            {
                fwrite(*pxlsrc, 1, strlen(*pxlsrc), pFile);
                fclose(pFile);
            }
        }
        else
        {
            sprintf(szTmp, "%s/shaders/f/F_%s_0x%X.glsl", aml->GetAndroidDataPath(), szNameCat, flags);
            pFile = fopen(szTmp, "w");
            if(pFile != NULL)
            {
                fwrite(*pxlsrc, 1, strlen(*pxlsrc), pFile);
                fclose(pFile);
            }
        }
        
        const char* vertName = FlagsToShaderName(flags, true);
        if(vertName)
        {
            VERTEX_SHADER_GEN_STORAGE(szTmp, vertName);
            pFile = fopen(szTmp, "w");
            if(pFile != NULL)
            {
                fwrite(*vtxsrc, 1, strlen(*vtxsrc), pFile);
                fclose(pFile);
            }
        }
        else
        {
            sprintf(szTmp, "%s/shaders/v/F_%s_0x%X.glsl", aml->GetAndroidDataPath(), szNameCat, flags);
            pFile = fopen(szTmp, "w");
            if(pFile != NULL)
            {
                fwrite(*vtxsrc, 1, strlen(*vtxsrc), pFile);
                fclose(pFile);
            }
        }
    #else
        const char* fragName = FlagsToShaderName(flags, false);
        if(fragName)
        {
            FRAGMENT_SHADER_GEN_STORAGE(szTmp, fragName);
            pFile = fopen(szTmp, "r");
            if(pFile != NULL)
            {
                logger->Info("Loading custom fragment shader \"%s\"", fragName);
                freadfull(*pxlsrc, 4095, pFile);
                fclose(pFile);
            }
        }
        
        const char* vertName = FlagsToShaderName(flags, true);
        if(vertName)
        {
            VERTEX_SHADER_GEN_STORAGE(szTmp, vertName);
            pFile = fopen(szTmp, "r");
            if(pFile != NULL)
            {
                logger->Info("Loading custom vertex shader \"%s\"", vertName);
                freadfull(*vtxsrc, 4095, pFile);
                fclose(pFile);
            }
        }
    #endif
    
    return ret;
}
DECL_HOOKv(InitES2Shader, ES3Shader* self)
{
    InitES2Shader(self);
    
    self->uid_nShaderFlags = _glGetUniformLocation(self->nShaderId, "ShaderFlags");
    self->uid_fAngle = _glGetUniformLocation(self->nShaderId, "SunVector");
    self->uid_nTime = _glGetUniformLocation(self->nShaderId, "Time");
    self->uid_nGameTimeSeconds = _glGetUniformLocation(self->nShaderId, "GameTimeSeconds");
    self->uid_fUnderWaterness = _glGetUniformLocation(self->nShaderId, "UnderWaterness");
    self->uid_fFarClipDist = _glGetUniformLocation(self->nShaderId, "FarClipDist");
}
DECL_HOOKv(RQ_Command_rqSelectShader, ES3Shader*** ptr)
{
    ES3Shader* shader = **ptr;
    RQ_Command_rqSelectShader(ptr);

    if(shader->uid_nShaderFlags >= 0) _glUniform1i(shader->uid_nShaderFlags, shader->flags);
    if(shader->uid_fAngle >= 0) _glUniform1fv(shader->uid_fAngle, 3, (float*)&m_VectorToSun[*m_CurrentStoredValue]);
    if(shader->uid_nTime >= 0) _glUniform1i(shader->uid_nTime, *m_snTimeInMilliseconds);
    if(shader->uid_nGameTimeSeconds >= 0) _glUniform1i(shader->uid_nGameTimeSeconds, (int)*ms_nGameClockMinutes * 60 + (int)*ms_nGameClockSeconds);
    if(shader->uid_fUnderWaterness >= 0) _glUniform1fv(shader->uid_fUnderWaterness, 1, UnderWaterness);
    if(shader->uid_fFarClipDist >= 0 && TheCamera->m_pRwCamera != NULL) _glUniform1fv(shader->uid_fFarClipDist, 1, &TheCamera->m_pRwCamera->farClip);
}
DECL_HOOKv(RenderSkyPolys)
{
    *curShaderStateFlags |= FLAG_CUSTOM_SKY;
    RenderSkyPolys();
    *curShaderStateFlags &= ~FLAG_CUSTOM_SKY;
}
DECL_HOOKv(OnEntityRender, CEntity* self)
{
    if(self->m_nType == ENTITY_TYPE_BUILDING) *curShaderStateFlags |= FLAG_CUSTOM_BUILDING;

    OnEntityRender(self);

    if(self->m_nType == ENTITY_TYPE_BUILDING) *curShaderStateFlags &= ~FLAG_CUSTOM_BUILDING;
}

// Patch funcs
uintptr_t BuildShader_BackTo;
__attribute__((optnone)) __attribute__((naked)) void BuildShader_inject(void)
{
    asm volatile(
        "MOV R5, R0\n"
        "MOV R8, R2\n");
    asm volatile(
        "MOV R0, %0\n"
    :: "r" (sizeof(ES3Shader)));
    asm volatile(
        "PUSH {R0}\n");
    asm volatile(
        "MOV R12, %0\n"
        "POP {R0}\n"
        "BX R12\n"
    :: "r" (BuildShader_BackTo));
}

// int main!
extern "C" void OnModLoad()
{
    logger->SetTag("SA ShaderLoader");
    pGTASA = aml->GetLib("libGTASA.so");
    hGTASA = dlopen("libGTASA.so", RTLD_LAZY);
    
    SET_TO(blurShader, aml->GetSym(hGTASA, "blurPShader"));
    SET_TO(gradingShader, aml->GetSym(hGTASA, "gradingPShader"));
    SET_TO(shadowResolve, aml->GetSym(hGTASA, "shadowResolvePShader"));
    SET_TO(contrastVertex, aml->GetSym(hGTASA, "contrastVShader"));
    SET_TO(contrastFragment, aml->GetSym(hGTASA, "contrastPShader"));
    
    // Other shaders (unstable as hell!)
    HOOK(RQShaderBuildSource, aml->GetSym(hGTASA, "_ZN8RQShader11BuildSourceEjPPKcS2_"));
    
    FILE *pFile;
    char szTmp[256];
    
    FRAGMENT_SHADER_STORAGE(szTmp, blur);
    if((pFile = fopen(szTmp, "r"))!=NULL)
    {
        freadfull(blurShaderOwn, SHADER_LEN, pFile);
        *blurShader = blurShaderOwn;
        fclose(pFile);
    }
    
    FRAGMENT_SHADER_STORAGE(szTmp, grading);
    if((pFile = fopen(szTmp, "r"))!=NULL)
    {
        freadfull(gradingShaderOwn, SHADER_LEN, pFile);
        *gradingShader = gradingShaderOwn;
        fclose(pFile);
    }
    
    FRAGMENT_SHADER_STORAGE(szTmp, shadowResolve);
    if((pFile = fopen(szTmp, "r"))!=NULL)
    {
        freadfull(shadowResolveOwn, SHADER_LEN, pFile);
        *shadowResolve = shadowResolveOwn;
        fclose(pFile);
    }
    
    VERTEX_SHADER_STORAGE(szTmp, contrast);
    if((pFile = fopen(szTmp, "r"))!=NULL)
    {
        freadfull(contrastVertexOwn, SHADER_LEN, pFile);
        *contrastVertex = contrastVertexOwn;
        fclose(pFile);
    }
    
    FRAGMENT_SHADER_STORAGE(szTmp, contrast);
    if((pFile = fopen(szTmp, "r"))!=NULL)
    {
        freadfull(contrastFragmentOwn, SHADER_LEN, pFile);
        *contrastFragment = contrastFragmentOwn;
        fclose(pFile);
    }
    
    BuildShader_BackTo = pGTASA + 0x1CD838 + 0x1;
    aml->Redirect(pGTASA + 0x1CD830 + 0x1, (uintptr_t)BuildShader_inject);

    HOOKPLT(InitES2Shader, pGTASA + 0x671BDC);
    HOOKPLT(RQ_Command_rqSelectShader, pGTASA + 0x67632C);//aml->GetSym(hGTASA, "_Z25RQ_Command_rqSelectShaderRPc"));
    HOOK(RenderSkyPolys, aml->GetSym(hGTASA, "_ZN7CClouds14RenderSkyPolysEv"));
    HOOK(OnEntityRender, aml->GetSym(hGTASA, "_ZN7CEntity6RenderEv"));

    SET_TO(_glGetUniformLocation, *(void**)(pGTASA + 0x6755EC));
    SET_TO(_glUniform1i, *(void**)(pGTASA + 0x674484));
    SET_TO(_glUniform1fv, *(void**)(pGTASA + 0x672388));
    SET_TO(emu_CustomShaderCreate, aml->GetSym(hGTASA, "_Z22emu_CustomShaderCreatePKcS0_"));

    SET_TO(fragShaders, pGTASA + 0x6B408C);
    SET_TO(activeShader, aml->GetSym(hGTASA, "_ZN9ES2Shader12activeShaderE"));
    SET_TO(m_VectorToSun, aml->GetSym(hGTASA, "_ZN10CTimeCycle13m_VectorToSunE"));
    SET_TO(m_CurrentStoredValue, aml->GetSym(hGTASA, "_ZN10CTimeCycle20m_CurrentStoredValueE"));
    SET_TO(m_snTimeInMilliseconds, aml->GetSym(hGTASA, "_ZN6CTimer22m_snTimeInMillisecondsE"));
    SET_TO(curShaderStateFlags, aml->GetSym(hGTASA, "curShaderStateFlags"));
    SET_TO(ms_nGameClockMinutes, aml->GetSym(hGTASA, "_ZN6CClock20ms_nGameClockMinutesE"));
    SET_TO(ms_nGameClockSeconds, aml->GetSym(hGTASA, "_ZN6CClock20ms_nGameClockSecondsE"));
    SET_TO(UnderWaterness, aml->GetSym(hGTASA, "_ZN8CWeather14UnderWaternessE"));
    SET_TO(TheCamera, aml->GetSym(hGTASA, "TheCamera"));
}
