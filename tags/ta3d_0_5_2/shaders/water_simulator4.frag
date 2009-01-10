varying vec2 t_coord;
uniform vec2 factor;
uniform sampler2D lava;

void main()
{
	vec2 real_coord = (t_coord-vec2(0.5,0.5))*factor+vec2(0.5,0.5);

	float lava_c = 0.5 * texture2D(lava,real_coord).r + 0.5;
	if (real_coord.x < 0.0 || real_coord.y < 0.0 || real_coord.x > 1.0 || real_coord.y > 1.0)
		lava_c = 0.0;

	gl_FragColor = vec4(t_coord, 0.0, lava_c);
}
