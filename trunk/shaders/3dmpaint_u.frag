varying vec2 t_coord;

void main()
{
	gl_FragColor = vec4(t_coord.x,t_coord.x,t_coord.x,1.0);
}
