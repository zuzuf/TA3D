varying vec2 t_coord;

void main()
{
	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
	t_coord = vec2(gl_TextureMatrix[0] * gl_MultiTexCoord0);
}
