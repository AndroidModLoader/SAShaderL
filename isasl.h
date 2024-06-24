#ifndef _SA_SHADER_LOADER_H
#define _SA_SHADER_LOADER_H

#include <stdint.h>

enum eUniformValueType : uint8_t
{
    UNIFORM_INT = 0,
    UNIFORM_UINT,
    UNIFORM_FLOAT,

    UNIFORM_MAXTYPES
};

class ISASL
{
public:
    // 1 = Mod v1.1
    virtual int GetFeaturesVersion() = 0;

    // returns id for values setters
    // if has pointer, uses it's value
    virtual int RegisterUniform(const char* name, eUniformValueType type, uint8_t valuesArraySize, bool alwaysUpdate = false, void* pointer = NULL);

    virtual void SetUniformInt(int id, int dataNum, int value);
    virtual void SetUniformUInt(int id, int dataNum, uint32_t value);
    virtual void SetUniformFloat(int id, int dataNum, float value);
    virtual void SetUniformPtr(int id, int dataNum, void* ptr);
    virtual void ForceUpdateData(int id);
};

#endif // _SA_SHADER_LOADER_H