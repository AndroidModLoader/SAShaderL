precision mediump float;
uniform sampler2D Diffuse;
varying mediump vec2 Out_Tex0;
uniform mediump vec4 RedGrade;
uniform mediump vec4 GreenGrade;
uniform mediump vec4 BlueGrade;

void main()
{
  vec4 color = texture2D(Diffuse, Out_Tex0);
  gl_FragColor.r = dot(color, RedGrade);
  gl_FragColor.g = dot(color, GreenGrade);
  gl_FragColor.b = dot(color, BlueGrade);
}