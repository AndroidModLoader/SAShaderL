### What am i?

I am a shader loader for GTA:San Andreas! That's why my name is San Andreas Shader Loader (SAShaderL)!

I am creating new uniforms for shaders such as:
```
uniform int ShaderFlags; // A flags for the shaders! Some shaders are using the same code!
uniform vec3 SunVector; // A XYZ vector of the sun!
uniform int Time; // Just a time (in milliseconds)
uniform int GameTimeSeconds; // A clock time of the game in seconds! 01:02 AM is 62 seconds!
uniform float UnderWaterness; // If the camera is underwater, this value is NOT 0! (**Values are between 0.0 and 1.0**)
uniform float RoadsWetness; // Shows how much wet roads are. **Values are 0.0 - 1.0**
uniform float FarClipDist; // Almost all the time it's 800.0, but not with the mods!
uniform int EntityModel; // Model index of the last entity being draw (-1 if no model)
```

In a future i will be able to let mods create their own uniforms!

Also it will allow to add uniforms with game variables through the config file! Easy peasy!

Few things to notice:
- Shader size is limited to 32 kilobytes