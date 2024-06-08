precision highp float;
attribute vec3 Position;
attribute vec2 TexCoord0;
varying mediump vec2 Out_Tex0;
varying mediump float Out_Z;

void main()
{
  gl_Position = vec4(Position.xy, 0.0, 1.0);
  Out_Z = Position.z;
  Out_Tex0 = TexCoord0;
}