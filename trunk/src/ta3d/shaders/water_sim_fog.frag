varying vec2 t_coord;
uniform sampler2D sky;
uniform sampler2D rtex;
uniform sampler2D bump;
uniform sampler2D view;
uniform sampler2D water_color;
uniform sampler2D normal_map;
uniform sampler2D height_map;
uniform vec2 coef;
uniform vec2 factor;
uniform float cam_h_factor;
uniform float t;

void main()
{
	vec4 bump_vec = texture2D( bump,t_coord );
	if (bump_vec.w == 0.0)
		gl_FragColor = vec4(0.0,0.0,0.0,1.0);
	else
	{
    	vec3 N = texture2DLod(normal_map, 5.0 * bump_vec.xy, 0.0).xyz;
    	vec2 map_coord = (bump_vec.xy - vec2(0.5,0.5)) * factor + vec2(0.5,0.5);

		vec3 D = 2.0 * texture2D( view, t_coord ).xyz - vec3(1.0,1.0,1.0);
		vec3 R = reflect( D, N );

		float depth = clamp( texture2DLod( height_map, map_coord, 0.0 ).w * 3.0, 0.0, 1.0);

        vec2 dec = depth * 50.0 * cam_h_factor * (R.xz - D.xz);

		vec2 scr_pos0 = clamp( t_coord * coef - dec, 0.0, 1.0 );
        vec2 scr_pos1 = clamp( t_coord * coef + dec, 0.0, 1.0 );
        dec.x = -dec.x;
        vec2 scr_pos2 = clamp( t_coord * coef - dec, 0.0, 1.0 );
        vec2 scr_pos3 = clamp( t_coord * coef + dec, 0.0, 1.0 );

		vec4 scr_col = 0.25 * (texture2D( rtex, scr_pos0 ) + texture2D( rtex, scr_pos1 ) + texture2D( rtex, scr_pos2 ) + texture2D( rtex, scr_pos3 ));
        vec4 fuzzy = texture2D(water_color, t_coord);
        float depthCoef = 1.0 - exp(-depth * 2.0);
        scr_col = mix(scr_col, fuzzy, 0.25 * depthCoef);

        gl_FragColor = mix(scr_col, vec4(1.0,1.0,1.0,1.0), depthCoef);
	}
}
