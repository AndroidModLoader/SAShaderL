#include <mod/amlmod.h>
#include <mod/logger.h>
#include <mod/config.h>
#include <dlfcn.h>

#include <stdio.h>
#include "ES2Shader.h"

#include <GTASA_STRUCTS.h>

MYMOD(net.rusjj.sashader, SAShaderLoader, 1.0.0, RusJJ)
NEEDGAME(com.rockstargames.gtasa)

// Savings
#define SHADER_LEN (4096)
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
uint32_t *m_snTimeInMilliseconds;
ES3Shader** fragShaders;

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
const char* FlagsToShaderName(int flags, bool isVertex)
{
    if(isVertex)
    {
        switch(flags)
        {
            case 0x10:
            case 0x200010:
            case 0x4000010:
                return "color";
                
            case 0x30:
            case 0x4000030:
                return "texture2d";
                
            case 0x220030:
                return "compressedTexture2d";
                
            case 0x800030:
                return "vehReflectionTexture2d";
                
            case 0x430:
            case 0x434:
                return "fogTexture";
                
            case 0x800410:
                return "vehReflectionFog";
                
            case 0x130434:
                return "fogTextureDetail";
                
            case 0x800430:
                return "vehReflectionFogTexture";
                
            case 0x2202A:
                return "textureLight";
                
            case 0x80430:
            case 0x90430:
                return "waterFogTexture";
                
            case 0x2042A:
            case 0x2042E:
                return "emissFog";
        }
    }
    else // isFragment
    {
        switch(flags)
        {
            case 0x10:
            case 0x200010:
            case 0x200090:
            case 0x200110:
                return "color";
                
            case 0x4000010:
                return "gammaColor";
                
            case 0x800410:
            case 0x202412:
            case 0x10040A:
            case 0x90040A:
                return "fog";
                
            case 0x30:
            case 0x220030:
            case 0x800030:
            case 0x2202A:
                return "texture2d";
                
            case 0x430:
            case 0x800430:
            case 0x222432:
            case 0xA22432:
            case 0x2042A:
            case 0x12042A:
            case 0x92042A:
            case 0x82042A:
            case 0x12062A:
                return "fogTexture";
                
            case 0x4000030:
                return "gammaTexture";
            
            case 0x220432:
            case 0xA20432:
                return "fogTextureCompressed";
                
            case 0x2024B2:
            case 0x222532:
                return "fogTextureBone";
                
            case 0x12042E:
            case 0x12062E:
            case 0x434:
                return "fogTextureAlpha";
                
            case 0x80430:
                return "waterFogTexture";
                
            case 0x90430:
                return "waterFogDetailTexture";
        }
    }
    return NULL;
}
template <size_t size>
void FlagToName(int flags, char (&out)[size])
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
}

// Game Funcs
int (*_glGetUniformLocation)(int, const char*);
void (*_glUniform1i)(int, int);
void (*_glUniform1fv)(int, int, const float*);
DECL_HOOK(int, RQShaderBuildSource, int flags, char **pxlsrc, char **vtxsrc)
{
    int ret = RQShaderBuildSource(flags, pxlsrc, vtxsrc);
    
    FILE *pFile;
    char szTmp[256], szNameCat[128];
    FlagToName(flags, szNameCat);
    
    const char* fragName = FlagsToShaderName(flags, false);
    if(fragName)
    {
        FRAGMENT_SHADER_GEN_STORAGE(szTmp, fragName);
        pFile = fopen(szTmp, "r");
        if(pFile != NULL)
        {
            freadfull(*pxlsrc, 4095, pFile);
            //fwrite(*pxlsrc, 1, strlen(*pxlsrc), pFile);
            fclose(pFile);
        }
    }
    /*else
    {
        sprintf(szTmp, "%s/shaders/f/F_%s_0x%X.glsl", aml->GetAndroidDataPath(), szNameCat, flags);
        pFile = fopen(szTmp, "w");
        if(pFile != NULL)
        {
            fwrite(*pxlsrc, 1, strlen(*pxlsrc), pFile);
            fclose(pFile);
        }
    }*/
    
    const char* vertName = FlagsToShaderName(flags, true);
    if(vertName)
    {
        VERTEX_SHADER_GEN_STORAGE(szTmp, vertName);
        pFile = fopen(szTmp, "r");
        if(pFile != NULL)
        {
            freadfull(*vtxsrc, 4095, pFile);
            //fwrite(*vtxsrc, 1, strlen(*vtxsrc), pFile);
            fclose(pFile);
        }
    }
    /*else
    {
        sprintf(szTmp, "%s/shaders/v/F_%s_0x%X.glsl", aml->GetAndroidDataPath(), szNameCat, flags);
        pFile = fopen(szTmp, "w");
        if(pFile != NULL)
        {
            fwrite(*vtxsrc, 1, strlen(*vtxsrc), pFile);
            fclose(pFile);
        }
    }*/
    
    return ret;
}
DECL_HOOKv(InitES2Shader, ES3Shader* self)
{
    InitES2Shader(self);
    
    self->uid_fAngle = _glGetUniformLocation(self->nShaderId, "SunVector");
    self->uid_nTime = _glGetUniformLocation(self->nShaderId, "Time");
}
DECL_HOOKv(WeatherUpdate, void* self)
{
    WeatherUpdate(self);
    
    static uint32_t next = 0;
    if(*m_snTimeInMilliseconds > next)
    {
        next = *m_snTimeInMilliseconds + 100;
        
        for(char i = 0; i < 4; ++i)
        {
            ES3Shader* s = fragShaders[i];
            if(s && s->uid_fAngle >= 0)
            {
                _glUniform1fv(s->uid_fAngle, 3, (float*)&m_VectorToSun[*m_CurrentStoredValue]);
            }
        }
    }
}
DECL_HOOKv(TimerUpdate, void* self)
{
    TimerUpdate(self);
    
    static uint32_t next = 0;
    if(*m_snTimeInMilliseconds > next)
    {
        next = *m_snTimeInMilliseconds + 10;
        logger->Info("time?");
        for(char i = 0; i < 4; ++i)
        {
            ES3Shader* s = fragShaders[i];
            if(s && s->uid_nTime >= 0)
            {
                logger->Info("set?");
                _glUniform1i(s->uid_nTime, *m_snTimeInMilliseconds);
            }
        }
    }
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
    HOOKPLT(WeatherUpdate, pGTASA + 0x671DF4);
    HOOKPLT(TimerUpdate, pGTASA + 0x6711E8);
    SET_TO(_glGetUniformLocation, *(void**)(pGTASA + 0x6755EC));
    SET_TO(_glUniform1i, *(void**)(pGTASA + 0x674484));
    SET_TO(_glUniform1fv, *(void**)(pGTASA + 0x672388));
    SET_TO(fragShaders, pGTASA + 0x6B408C);
    SET_TO(m_VectorToSun, aml->GetSym(hGTASA, "_ZN10CTimeCycle13m_VectorToSunE"));
    SET_TO(m_CurrentStoredValue, aml->GetSym(hGTASA, "_ZN10CTimeCycle20m_CurrentStoredValueE"));
    SET_TO(m_snTimeInMilliseconds, aml->GetSym(hGTASA, "_ZN6CTimer22m_snTimeInMillisecondsE"));
}
