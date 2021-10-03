varying vec4 light_coord;
uniform mat4 light_Projection;

void main()
{
    gl_Position = ftransform();
    gl_FrontColor = gl_Color;
    light_coord = light_Projection * (gl_ModelViewMatrix * gl_Vertex);
    gl_TexCoord[0] = gl_TextureMatrix[0] * gl_MultiTexCoord0;
    gl_TexCoord[1] = vec4(0.015625 * gl_Vertex.xz, 0.0, 0.0);
    gl_FogFragCoord = gl_Position.z;
}
