varying vec2 t_coord;

void main()
{
    gl_Position = gl_Vertex;
    t_coord = gl_MultiTexCoord0.xy;
}
