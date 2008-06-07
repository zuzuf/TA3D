void main()
{
	vec3 dir = gl_Color.xyz;
	if (dot(gl_Normal.xyz, dir) > 0.0 )
		gl_Position = gl_ProjectionMatrix * (gl_ModelViewMatrix * gl_Vertex + vec4(dir,0));
	else
		gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
}
