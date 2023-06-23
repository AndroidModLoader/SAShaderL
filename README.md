What am i?

I am a shader loader for GTA:San Andreas! That's why my name is SA Shader Loader!

I am creating new uniforms for shaders such as:
- uniform int ShaderFlags; // A flags for the shaders! Some shaders are using the same code!
- uniform vec3 SunVector; // A XYZ vector of the sun!
- uniform int Time; // Just a time
- uniform int GameTimeSeconds; // A clock time of the game in seconds! 01:02 AM is 62 seconds!
- uniform float UnderWaterness; // If the camera is underwater, this value is NOT 0!
- uniform float FarClipDist; // Almost all the time it's 800.0, but not with the mod!

Few things to notice:
- Shader size is limited to 4 kilobytes, except "blur", "grading", "shadowResolve", "contrast". They are 16 kilobytes maximum.