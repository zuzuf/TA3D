varying vec3 t_coord;
varying vec3 dir;

void main()
{
    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
    t_coord = (gl_TextureMatrix[0] * gl_MultiTexCoord0).xyz;
    dir = (gl_ModelViewMatrixInverse[3] - gl_Vertex ).xyz;
}
