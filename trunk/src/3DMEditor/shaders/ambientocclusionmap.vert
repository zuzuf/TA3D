uniform mat4	cam0;
uniform mat4	cam1;
uniform mat4	cam2;
uniform mat4	cam3;
uniform mat4	cam4;
uniform mat4	cam5;
uniform mat4	cam6;
uniform mat4	cam7;

void main()
{
  gl_Position = vec4(2.0 * gl_MultiTexCoord0.xy - vec2(1.0, 1.0), 0.0, 1.0);
  vec4 vertex = gl_ModelViewMatrix * gl_Vertex;
  gl_TexCoord[0] = cam0 * vertex;
  gl_TexCoord[1] = cam1 * vertex;
  gl_TexCoord[2] = cam2 * vertex;
  gl_TexCoord[3] = cam3 * vertex;
  gl_TexCoord[4] = cam4 * vertex;
  gl_TexCoord[5] = cam5 * vertex;
  gl_TexCoord[6] = cam6 * vertex;
  gl_TexCoord[7] = cam7 * vertex;
}
