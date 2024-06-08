precision mediump float;
uniform sampler2D Diffuse;
varying mediump vec2 Out_Tex0;
uniform mediump vec4 RedGrade;

void main()
{
  vec4 color = texture2D(Diffuse, Out_Tex0);
  gl_FragColor = vec4(0, 0, 0, (1.0 - color.r) * RedGrade.a);
}