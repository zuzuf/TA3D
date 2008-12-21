varying vec2 t_coord;

void main()
{
	gl_FragColor = vec4(t_coord.y,t_coord.y,t_coord.y,1.0);
}
