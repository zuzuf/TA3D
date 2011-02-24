varying vec3 normal;

void main()
{
    gl_ClipVertex = gl_ModelViewMatrix * gl_Vertex;
    gl_Position = ftransform();
    gl_FrontColor = gl_Color;
    gl_TexCoord[0] = gl_TextureMatrix[0] * gl_MultiTexCoord0;
    gl_FogFragCoord = gl_Position.z;
    normal = gl_NormalMatrix * gl_Normal;
}
