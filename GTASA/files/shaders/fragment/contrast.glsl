precision mediump float;
uniform sampler2D Diffuse;
varying mediump vec2 Out_Tex0;
uniform mediump vec3 ContrastMult;
uniform mediump vec3 ContrastAdd;

void main()
{
  gl_FragColor = texture2D(Diffuse, Out_Tex0);
  gl_FragColor.rgb *= ContrastMult;
  gl_FragColor.rgb += ContrastAdd;
}