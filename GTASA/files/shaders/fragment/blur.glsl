precision mediump float;
uniform sampler2D Diffuse;
varying mediump vec2 Out_Tex0;
varying mediump float Out_Z;
uniform mediump vec4 RedGrade;
uniform mediump vec4 GreenGrade;
uniform mediump vec4 BlueGrade;

void main()
{
  vec4 color = texture2D(Diffuse, Out_Tex0) * 0.25;
  mediump vec2 dist = vec2(0.001, 0.001) * Out_Z;
  color += texture2D(Diffuse, Out_Tex0 + dist) * 0.175;
  color += texture2D(Diffuse, Out_Tex0 - dist) * 0.175;
  color += texture2D(Diffuse, Out_Tex0 + vec2(dist.x, -dist.y)) * 0.2;
  color += texture2D(Diffuse, Out_Tex0 + vec2(-dist.x, dist.y)) * 0.2;
  gl_FragColor.r = dot(color, RedGrade);
  gl_FragColor.g = dot(color, GreenGrade);
  gl_FragColor.b = dot(color, BlueGrade);
}