#include <mod/amlmod.h>
#include <mod/logger.h>
#include <mod/config.h>

#include <stdio.h>
#include "ES3Shader.h"

#define DUMP_SHADERS

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
char blurShaderOwn[SHADER_LEN + 1], gradingShaderOwn[SHADER_LEN + 1], shadowResolveOwn[SHADER_LEN + 1], contrastVertexOwn[SHADER_LEN + 1], contrastFragmentOwn[SHADER_LEN + 1];
char customPixelShader[SHADER_LEN + 1], customVertexShader[SHADER_LEN + 1];
int lastModelId = -1;

// Config


// Game Vars
const char **blurShader, **gradingShader, **shadowResolve, **contrastVertex, **contrastFragment;
CVector *m_VectorToSun;
int *m_CurrentStoredValue;
uint32_t *m_snTimeInMilliseconds, *curShaderStateFlags;
uint8_t *ms_nGameClockSeconds, *ms_nGameClockMinutes;
float *UnderWaterness, *WetRoads;
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
    if(flags == 0x421) return "reqqqqq";
    return NULL;
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

            case 0x1010040A:
                return "building/untextured";

            case 0x10020430:
                return "building/textured_compressedTex";
            case 0x12020430:
                return "building/textured_compressedTex_normal";
            case 0x10220432:
                return "building/textured_compressedTex_light";
            case 0x10222432:
                return "building/textured_compressedTex_light2";

            case 0x1010042A:
                return "building/textured2Colors_light";

            case 0x1013042A:
            case 0x1012042A:
                return "building/textured2Colors_comp_light";

            case 0x10110430:
            case 0x10100430:
                return "building/textured2Colors";

            case 0x1092042A:
                return "building/textured2Colors_xenv";

            case 0x10120434:
            case 0x10120630:
            case 0x10130430:
            case 0x10120430:
            case 0x1011042A:
                return "building/textured2Colors_light";
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

            case 0x1010040A:
                return "building/untextured";

            case 0x10020430:
            case 0x12020430:
            case 0x10220432:
            case 0x10222432:
                return "building/textured";

            case 0x10100430:
            case 0x10120430:
            case 0x10120630:
            case 0x1010042A:
            case 0x1012042A:
            case 0x1092042A:
            case 0x10120434:
            case 0x10110430:
            case 0x10130430:
            case 0x1013042A:
            case 0x1011042A:
                return "building/textured2Colors";

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
            logger->Info(szTmp);
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
                freadfull(*pxlsrc, SHADER_LEN, pFile);
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
                freadfull(*vtxsrc, SHADER_LEN, pFile);
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
    self->uid_fRoadsWetness = _glGetUniformLocation(self->nShaderId, "RoadsWetness");
    self->uid_fFarClipDist = _glGetUniformLocation(self->nShaderId, "FarClipDist");
    self->uid_nEntityModel = _glGetUniformLocation(self->nShaderId, "EntityModel");
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
    if(shader->uid_fRoadsWetness >= 0) _glUniform1fv(shader->uid_fRoadsWetness, 1, WetRoads);
    if(shader->uid_fFarClipDist >= 0 && TheCamera->m_pRwCamera != NULL) _glUniform1fv(shader->uid_fFarClipDist, 1, &TheCamera->m_pRwCamera->farClip);
    if(shader->uid_nEntityModel >= 0) _glUniform1i(shader->uid_nEntityModel, lastModelId);
}
DECL_HOOKv(RenderSkyPolys)
{
    *curShaderStateFlags |= FLAG_CUSTOM_SKY;
    RenderSkyPolys();
    *curShaderStateFlags &= ~FLAG_CUSTOM_SKY;
}
DECL_HOOKv(OnEntityRender, CEntity* self)
{
    if(self->m_nType == ENTITY_TYPE_BUILDING)
    {
        lastModelId = self->m_nModelIndex;
        *curShaderStateFlags |= FLAG_CUSTOM_BUILDING;
        OnEntityRender(self);
        *curShaderStateFlags &= ~FLAG_CUSTOM_BUILDING;
        lastModelId = -1;
        return;
    }
    OnEntityRender(self);
}

// Patch funcs
#ifdef AML32
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
#else

#endif

// int main!
extern "C" void OnModLoad()
{
    logger->SetTag("SA ShaderLoader");
    pGTASA = aml->GetLib("libGTASA.so");
    hGTASA = aml->GetLibHandle("libGTASA.so");
    
    SET_TO(blurShader, aml->GetSym(hGTASA, "blurPShader"));
    SET_TO(gradingShader, aml->GetSym(hGTASA, "gradingPShader"));
    SET_TO(shadowResolve, aml->GetSym(hGTASA, "shadowResolvePShader"));
    SET_TO(contrastVertex, aml->GetSym(hGTASA, "contrastVShader"));
    SET_TO(contrastFragment, aml->GetSym(hGTASA, "contrastPShader"));
    
    // Other shaders (unstable as hell!)
    HOOKPLT(RQShaderBuildSource, pGTASA + BYBIT(0x6720F8, 0x8439A0));
    
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
    
  #ifdef AML32
    BuildShader_BackTo = pGTASA + 0x1CD838 + 0x1;
    aml->Redirect(pGTASA + 0x1CD830 + 0x1, (uintptr_t)BuildShader_inject);
  #else
    aml->Write32(pGTASA + 0x262AB0, MOVBits::Create(sizeof(ES3Shader), 0, false));
  #endif

  #ifdef AML32
    HOOKPLT(InitES2Shader, pGTASA + 0x671BDC);
    HOOKPLT(RQ_Command_rqSelectShader, pGTASA + 0x67632C);//aml->GetSym(hGTASA, "_Z25RQ_Command_rqSelectShaderRPc"));
    HOOKPLT(RenderSkyPolys, pGTASA + 0x670A7C);
    HOOK(OnEntityRender, aml->GetSym(hGTASA, "_ZN7CEntity6RenderEv"));
  #else
    HOOKBL(InitES2Shader, pGTASA + 0x26213C);
    HOOKPLT(RQ_Command_rqSelectShader, pGTASA + 0x84A6B0);//aml->GetSym(hGTASA, "_Z25RQ_Command_rqSelectShaderRPc"));
    HOOKPLT(RenderSkyPolys, pGTASA + 0x8414C8);
    HOOK(OnEntityRender, aml->GetSym(hGTASA, "_ZN7CEntity6RenderEv"));
  #endif

    SET_TO(_glGetUniformLocation, *(void**)(pGTASA + BYBIT(0x6755EC, 0x8403B0)));
    SET_TO(_glUniform1i, *(void**)(pGTASA + BYBIT(0x674484, 0x846858)));
    SET_TO(_glUniform1fv, *(void**)(pGTASA + BYBIT(0x672388, 0x845048)));
    SET_TO(emu_CustomShaderCreate, aml->GetSym(hGTASA, "_Z22emu_CustomShaderCreatePKcS0_"));

    SET_TO(fragShaders, pGTASA + BYBIT(0x6B408C, 0x891058));
    SET_TO(activeShader, aml->GetSym(hGTASA, "_ZN9ES2Shader12activeShaderE"));
    SET_TO(m_VectorToSun, aml->GetSym(hGTASA, "_ZN10CTimeCycle13m_VectorToSunE"));
    SET_TO(m_CurrentStoredValue, aml->GetSym(hGTASA, "_ZN10CTimeCycle20m_CurrentStoredValueE"));
    SET_TO(m_snTimeInMilliseconds, aml->GetSym(hGTASA, "_ZN6CTimer22m_snTimeInMillisecondsE"));
    SET_TO(curShaderStateFlags, aml->GetSym(hGTASA, "curShaderStateFlags"));
    SET_TO(ms_nGameClockMinutes, aml->GetSym(hGTASA, "_ZN6CClock20ms_nGameClockMinutesE"));
    SET_TO(ms_nGameClockSeconds, aml->GetSym(hGTASA, "_ZN6CClock20ms_nGameClockSecondsE"));
    SET_TO(UnderWaterness, aml->GetSym(hGTASA, "_ZN8CWeather14UnderWaternessE"));
    SET_TO(WetRoads, aml->GetSym(hGTASA, "_ZN8CWeather8WetRoadsE"));
    SET_TO(TheCamera, aml->GetSym(hGTASA, "TheCamera"));
    
  #ifdef AML32
    aml->WriteAddr(pGTASA + 0x1CF73C, (uintptr_t)&customVertexShader - pGTASA - 0x1CEA48);
    aml->WriteAddr(pGTASA + 0x1CF7AC, (uintptr_t)&customVertexShader - pGTASA - 0x1CEAD0);
    aml->WriteAddr(pGTASA + 0x1CF7C0, (uintptr_t)&customVertexShader - pGTASA - 0x1CEB44);
    aml->WriteAddr(pGTASA + 0x1CF7CC, (uintptr_t)&customVertexShader - pGTASA - 0x1CEB8C);
    aml->WriteAddr(pGTASA + 0x1CF7D4, (uintptr_t)&customVertexShader - pGTASA - 0x1CEBB0);
    aml->WriteAddr(pGTASA + 0x1CF7E0, (uintptr_t)&customVertexShader - pGTASA - 0x1CEBF0);
    aml->WriteAddr(pGTASA + 0x1CF7EC, (uintptr_t)&customVertexShader - pGTASA - 0x1CEC2A);
    aml->WriteAddr(pGTASA + 0x1CF80C, (uintptr_t)&customVertexShader - pGTASA - 0x1CEC86);
    aml->WriteAddr(pGTASA + 0x1CF814, (uintptr_t)&customVertexShader - pGTASA - 0x1CEC6C);
    aml->WriteAddr(pGTASA + 0x1CF81C, (uintptr_t)&customVertexShader - pGTASA - 0x1CECA6);
    aml->WriteAddr(pGTASA + 0x1CF824, (uintptr_t)&customVertexShader - pGTASA - 0x1CECCC);
    aml->WriteAddr(pGTASA + 0x1CF838, (uintptr_t)&customVertexShader - pGTASA - 0x1CED30);
    aml->WriteAddr(pGTASA + 0x1CF840, (uintptr_t)&customVertexShader - pGTASA - 0x1CED58);
    aml->WriteAddr(pGTASA + 0x1CF848, (uintptr_t)&customVertexShader - pGTASA - 0x1CEDA2);
    aml->WriteAddr(pGTASA + 0x1CF850, (uintptr_t)&customVertexShader - pGTASA - 0x1CED88);
    aml->WriteAddr(pGTASA + 0x1CF858, (uintptr_t)&customVertexShader - pGTASA - 0x1CEDC2);
    aml->WriteAddr(pGTASA + 0x1CF860, (uintptr_t)&customVertexShader - pGTASA - 0x1CEDEE);
    aml->WriteAddr(pGTASA + 0x1CF868, (uintptr_t)&customVertexShader - pGTASA - 0x1CEE14);
    aml->WriteAddr(pGTASA + 0x1CF874, (uintptr_t)&customVertexShader - pGTASA - 0x1CEE56);
    aml->WriteAddr(pGTASA + 0x1CF888, (uintptr_t)&customVertexShader - pGTASA - 0x1CEECC);
    aml->WriteAddr(pGTASA + 0x1CF894, (uintptr_t)&customVertexShader - pGTASA - 0x1CEF0E);
    aml->WriteAddr(pGTASA + 0x1CF89C, (uintptr_t)&customVertexShader - pGTASA - 0x1CEF46);
    aml->WriteAddr(pGTASA + 0x1CF8A4, (uintptr_t)&customVertexShader - pGTASA - 0x1CEF64);
    aml->WriteAddr(pGTASA + 0x1CF8AC, (uintptr_t)&customVertexShader - pGTASA - 0x1CF146);
    aml->WriteAddr(pGTASA + 0x1CF8B4, (uintptr_t)&customVertexShader - pGTASA - 0x1CEF8C);
    aml->WriteAddr(pGTASA + 0x1CF8E8, (uintptr_t)&customVertexShader - pGTASA - 0x1CF0D2);
    aml->WriteAddr(pGTASA + 0x1CF8F8, (uintptr_t)&customVertexShader - pGTASA - 0x1CF126);
    aml->WriteAddr(pGTASA + 0x1CF900, (uintptr_t)&customVertexShader - pGTASA - 0x1CF198);
    aml->WriteAddr(pGTASA + 0x1CF914, (uintptr_t)&customVertexShader - pGTASA - 0x1CF170);
    aml->WriteAddr(pGTASA + 0x1CF920, (uintptr_t)&customVertexShader - pGTASA - 0x1CF23C);
    aml->WriteAddr(pGTASA + 0x1CF928, (uintptr_t)&customVertexShader - pGTASA - 0x1CF21C);
    aml->WriteAddr(pGTASA + 0x1CF930, (uintptr_t)&customVertexShader - pGTASA - 0x1CF274);
    aml->WriteAddr(pGTASA + 0x1CF938, (uintptr_t)&customVertexShader - pGTASA - 0x1CF25A);
    aml->WriteAddr(pGTASA + 0x1CF944, (uintptr_t)&customVertexShader - pGTASA - 0x1CF2A4);
    aml->WriteAddr(pGTASA + 0x1CF958, (uintptr_t)&customVertexShader - pGTASA - 0x1CF314);
    aml->WriteAddr(pGTASA + 0x1CF960, (uintptr_t)&customVertexShader - pGTASA - 0x1CF2F8);
    aml->WriteAddr(pGTASA + 0x1CF968, (uintptr_t)&customVertexShader - pGTASA - 0x1CF33A);
    aml->WriteAddr(pGTASA + 0x1CF974, (uintptr_t)&customVertexShader - pGTASA - 0x1CF392);
    aml->WriteAddr(pGTASA + 0x1CF97C, (uintptr_t)&customVertexShader - pGTASA - 0x1CF378);
    aml->WriteAddr(pGTASA + 0x1CF988, (uintptr_t)&customVertexShader - pGTASA - 0x1CF3B6);
    aml->WriteAddr(pGTASA + 0x1CF994, (uintptr_t)&customVertexShader - pGTASA - 0x1CF3FA);
    aml->WriteAddr(pGTASA + 0x1CF99C, (uintptr_t)&customVertexShader - pGTASA - 0x1CF458);
    aml->WriteAddr(pGTASA + 0x1CF9A4, (uintptr_t)&customVertexShader - pGTASA - 0x1CF43E);
    aml->WriteAddr(pGTASA + 0x1CF9AC, (uintptr_t)&customVertexShader - pGTASA - 0x1CF41C);
    aml->WriteAddr(pGTASA + 0x1CF9B4, (uintptr_t)&customVertexShader - pGTASA - 0x1CF516);
    aml->WriteAddr(pGTASA + 0x1CF9BC, (uintptr_t)&customVertexShader - pGTASA - 0x1CF6F6);
    aml->WriteAddr(pGTASA + 0x1CF9C4, (uintptr_t)&customVertexShader - pGTASA - 0x1CF71A);
    aml->WriteAddr(pGTASA + 0x1CF9CC, (uintptr_t)&customVertexShader - pGTASA - 0x1CF492);
    aml->WriteAddr(pGTASA + 0x1CF9D4, (uintptr_t)&customVertexShader - pGTASA - 0x1CF4E6);
    aml->WriteAddr(pGTASA + 0x1CF9DC, (uintptr_t)&customVertexShader - pGTASA - 0x1CF534);
    aml->WriteAddr(pGTASA + 0x1CF9E4, (uintptr_t)&customVertexShader - pGTASA - 0x1CF4C4);
    aml->WriteAddr(pGTASA + 0x1CF9EC, (uintptr_t)&customVertexShader - pGTASA - 0x1CF554);
    aml->WriteAddr(pGTASA + 0x1CF9F8, (uintptr_t)&customVertexShader - pGTASA - 0x1CF5EA);
    aml->WriteAddr(pGTASA + 0x1CFA14, (uintptr_t)&customVertexShader - pGTASA - 0x1CF5AC);
    aml->WriteAddr(pGTASA + 0x1CFA20, (uintptr_t)&customVertexShader - pGTASA - 0x1CF674);
    aml->WriteAddr(pGTASA + 0x1CFA30, (uintptr_t)&customVertexShader - pGTASA - 0x1CF6BE);
    aml->WriteAddr(pGTASA + 0x1CFA78, (uintptr_t)&customVertexShader - pGTASA - 0x1CFA50);

    aml->WriteAddr(pGTASA + 0x1CE834, (uintptr_t)&customPixelShader - pGTASA - 0x1CE192);
    aml->WriteAddr(pGTASA + 0x1CE854, (uintptr_t)&customPixelShader - pGTASA - 0x1CE1B6);
    aml->WriteAddr(pGTASA + 0x1CE878, (uintptr_t)&customPixelShader - pGTASA - 0x1CE1FC);
    aml->WriteAddr(pGTASA + 0x1CE884, (uintptr_t)&customPixelShader - pGTASA - 0x1CE288);
    aml->WriteAddr(pGTASA + 0x1CE88C, (uintptr_t)&customPixelShader - pGTASA - 0x1CE238);
    aml->WriteAddr(pGTASA + 0x1CE890, (uintptr_t)&customPixelShader - pGTASA - 0x1CE256);
    aml->WriteAddr(pGTASA + 0x1CE8B4, (uintptr_t)&customPixelShader - pGTASA - 0x1CE2B0);
    aml->WriteAddr(pGTASA + 0x1CE8D8, (uintptr_t)&customPixelShader - pGTASA - 0x1CE2EE);
    aml->WriteAddr(pGTASA + 0x1CE8E0, (uintptr_t)&customPixelShader - pGTASA - 0x1CE322);
    aml->WriteAddr(pGTASA + 0x1CE8E8, (uintptr_t)&customPixelShader - pGTASA - 0x1CE346);
    aml->WriteAddr(pGTASA + 0x1CE8F0, (uintptr_t)&customPixelShader - pGTASA - 0x1CE36A);
    aml->WriteAddr(pGTASA + 0x1CE900, (uintptr_t)&customPixelShader - pGTASA - 0x1CE3BE);
    aml->WriteAddr(pGTASA + 0x1CE910, (uintptr_t)&customPixelShader - pGTASA - 0x1CE446);
    aml->WriteAddr(pGTASA + 0x1CE918, (uintptr_t)&customPixelShader - pGTASA - 0x1CE47A);
    aml->WriteAddr(pGTASA + 0x1CE928, (uintptr_t)&customPixelShader - pGTASA - 0x1CE460);
    aml->WriteAddr(pGTASA + 0x1CE934, (uintptr_t)&customPixelShader - pGTASA - 0x1CE426);
    aml->WriteAddr(pGTASA + 0x1CE93C, (uintptr_t)&customPixelShader - pGTASA - 0x1CE4B8);
    aml->WriteAddr(pGTASA + 0x1CE944, (uintptr_t)&customPixelShader - pGTASA - 0x1CE4D8);
    aml->WriteAddr(pGTASA + 0x1CE94C, (uintptr_t)&customPixelShader - pGTASA - 0x1CE582);
    aml->WriteAddr(pGTASA + 0x1CE954, (uintptr_t)&customPixelShader - pGTASA - 0x1CE530);
    aml->WriteAddr(pGTASA + 0x1CE960, (uintptr_t)&customPixelShader - pGTASA - 0x1CE500);
    aml->WriteAddr(pGTASA + 0x1CE968, (uintptr_t)&customPixelShader - pGTASA - 0x1CE568);
    aml->WriteAddr(pGTASA + 0x1CE970, (uintptr_t)&customPixelShader - pGTASA - 0x1CE5A4);
    aml->WriteAddr(pGTASA + 0x1CE978, (uintptr_t)&customPixelShader - pGTASA - 0x1CE5C8);
    aml->WriteAddr(pGTASA + 0x1CE994, (uintptr_t)&customPixelShader - pGTASA - 0x1CE65E);
    aml->WriteAddr(pGTASA + 0x1CE9A0, (uintptr_t)&customPixelShader - pGTASA - 0x1CE686);
    aml->WriteAddr(pGTASA + 0x1CE9A8, (uintptr_t)&customPixelShader - pGTASA - 0x1CE6A4);
    aml->WriteAddr(pGTASA + 0x1CE9B0, (uintptr_t)&customPixelShader - pGTASA - 0x1CE6BE);
    aml->WriteAddr(pGTASA + 0x1CE9B8, (uintptr_t)&customPixelShader - pGTASA - 0x1CE6DC);
    aml->WriteAddr(pGTASA + 0x1CE9C0, (uintptr_t)&customPixelShader - pGTASA - 0x1CE750);
    aml->WriteAddr(pGTASA + 0x1CE9C8, (uintptr_t)&customPixelShader - pGTASA - 0x1CE78A);
    aml->WriteAddr(pGTASA + 0x1CE9D4, (uintptr_t)&customPixelShader - pGTASA - 0x1CE708);
    aml->WriteAddr(pGTASA + 0x1CE9DC, (uintptr_t)&customPixelShader - pGTASA - 0x1CE73A);
    aml->WriteAddr(pGTASA + 0x1CE9E4, (uintptr_t)&customPixelShader - pGTASA - 0x1CE768);
    aml->WriteAddr(pGTASA + 0x1CE9F0, (uintptr_t)&customPixelShader - pGTASA - 0x1CE724);
    aml->WriteAddr(pGTASA + 0x1CE9F8, (uintptr_t)&customPixelShader - pGTASA - 0x1CE7BE);
    aml->WriteAddr(pGTASA + 0x1CEA00, (uintptr_t)&customPixelShader - pGTASA - 0x1CE7DE);
    aml->WriteAddr(pGTASA + 0x1CEA08, (uintptr_t)&customPixelShader - pGTASA - 0x1CE7F6);
    aml->WriteAddr(pGTASA + 0x1CFA7C, (uintptr_t)&customPixelShader - pGTASA - 0x1CFA54);
  #else
    // 4kb limit is still there, yet.
  #endif
}