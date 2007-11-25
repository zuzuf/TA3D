varying vec2 t_coord;
varying vec2 dt_coord;
varying vec3 color;

void main()
{
	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
	t_coord = vec2(gl_TextureMatrix[0] * gl_MultiTexCoord0);
	dt_coord = vec2(gl_TextureMatrix[1] * gl_MultiTexCoord1);
	color = gl_Color.rgb;
	gl_FogFragCoord = gl_Position.z;
}
