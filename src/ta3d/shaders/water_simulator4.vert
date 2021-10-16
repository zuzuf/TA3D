varying vec3 t_coord;

void main()
{
    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
    t_coord = gl_MultiTexCoord0.xyz;
}
