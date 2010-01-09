uniform sampler2DShadow shadowmap0;
uniform sampler2DShadow shadowmap1;
uniform sampler2DShadow shadowmap2;
uniform sampler2DShadow shadowmap3;
uniform sampler2DShadow shadowmap4;
uniform sampler2DShadow shadowmap5;
uniform sampler2DShadow shadowmap6;
uniform sampler2DShadow shadowmap7;
uniform int nb;

void main()
{
  float shaded = 0.0;
  gl_TexCoord[0].z -= 0.0001 * gl_TexCoord[0].w;
  gl_TexCoord[1].z -= 0.0001 * gl_TexCoord[1].w;
  gl_TexCoord[2].z -= 0.0001 * gl_TexCoord[2].w;
  gl_TexCoord[3].z -= 0.0001 * gl_TexCoord[3].w;
  gl_TexCoord[4].z -= 0.0001 * gl_TexCoord[4].w;
  gl_TexCoord[5].z -= 0.0001 * gl_TexCoord[5].w;
  gl_TexCoord[6].z -= 0.0001 * gl_TexCoord[6].w;
  gl_TexCoord[7].z -= 0.0001 * gl_TexCoord[7].w;
  shaded += shadow2D( shadowmap0, gl_TexCoord[0].xyz ).x > 0.0 ? 1.0 : 0.0;
  if (nb >= 2)
    shaded += shadow2D( shadowmap1, gl_TexCoord[1].xyz ).x > 0.0 ? 1.0 : 0.0;
  if (nb >= 3)
    shaded += shadow2D( shadowmap2, gl_TexCoord[2].xyz ).x > 0.0 ? 1.0 : 0.0;
  if (nb >= 4)
    shaded += shadow2D( shadowmap3, gl_TexCoord[3].xyz ).x > 0.0 ? 1.0 : 0.0;
  if (nb >= 5)
    shaded += shadow2D( shadowmap4, gl_TexCoord[4].xyz ).x > 0.0 ? 1.0 : 0.0;
  if (nb >= 6)
    shaded += shadow2D( shadowmap5, gl_TexCoord[5].xyz ).x > 0.0 ? 1.0 : 0.0;
  if (nb >= 7)
    shaded += shadow2D( shadowmap6, gl_TexCoord[6].xyz ).x > 0.0 ? 1.0 : 0.0;
  if (nb >= 8)
    shaded += shadow2D( shadowmap7, gl_TexCoord[7].xyz ).x > 0.0 ? 1.0 : 0.0;
  gl_FragColor = vec4(shaded, shaded, shaded, 0.0);
}
