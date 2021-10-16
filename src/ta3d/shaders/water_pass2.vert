varying vec3 dir;

void main()
{
    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
    dir = (gl_ModelViewMatrixInverse[3] - gl_Vertex ).xyz;
}
