varying vec2 t_coord;
uniform sampler2D sky;
uniform sampler2D rtex;
uniform sampler2D bump;
uniform sampler2D view;
uniform sampler2D water_color;
uniform vec2 coef;

void main()
{
	vec4 bump_vec = texture2D( bump,t_coord );
	if (bump_vec.w == 0.0)
		gl_FragColor = vec4(0.0,0.0,0.0,1.0);
	else
	{
		vec3 N = 2.0 * bump_vec.xyz - vec3(1.0,1.0,1.0);
		vec3 D = 2.0 * texture2D( view, t_coord ).xyz - vec3(1.0,1.0,1.0);
		vec3 R = reflect( D, N );

		vec2 dec = 0.005 * (R.xz - D.xz);
		vec2 scr_pos0 = clamp( t_coord * coef + dec, 0.0, 1.0 );
		vec2 scr_pos1 = clamp( t_coord * coef - dec, 0.0, 1.0 );
        dec.x = -dec.x;
        vec2 scr_pos2 = clamp( t_coord * coef + dec, 0.0, 1.0 );
        vec2 scr_pos3 = clamp( t_coord * coef - dec, 0.0, 1.0 );

		vec4 lava_col = 0.25 * (texture2D( rtex, scr_pos0 ) + texture2D( rtex, scr_pos1 ) + texture2D( rtex, scr_pos2 ) + texture2D( rtex, scr_pos3 ));
		vec4 water_col = vec4( texture2D( water_color, t_coord ).rgb, 1.0 );

		gl_FragColor = mix(lava_col, water_col, 0.25);
	}
}
