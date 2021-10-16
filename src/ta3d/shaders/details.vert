varying vec2 t_coord;
varying vec2 dt_coord;
varying vec3 color;
uniform float coef;

void main()
{
    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
    t_coord = vec2(gl_TextureMatrix[0] * gl_MultiTexCoord0);
    dt_coord = 0.015625 * gl_Vertex.xz;
    color = coef * gl_Color.rgb;
    gl_FogFragCoord = gl_Position.z;
}
