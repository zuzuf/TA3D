varying vec2 t_coord;
uniform sampler2D sky;
uniform sampler2D rtex;
uniform sampler2D bump;
uniform sampler2D view;
uniform sampler2D water_color;
uniform sampler2D normal_map;
uniform sampler2D height_map;
uniform sampler2D distort_map;
uniform vec2 coef;
uniform vec2 factor;
uniform float cam_h_factor;
uniform float t;

float f(vec3 v)
{
    return dot(v,v) == 0.0 ? 0.0 : 1.0;
}

void main()
{
	vec4 bump_vec = texture2D( bump, t_coord );
	if (bump_vec.w == 0.0)
		gl_FragColor = vec4(0.0,0.0,0.0,1.0);
	else
	{
        vec3 N = normalize( texture2D(normal_map, 5.0 * bump_vec.xy).xyz );
    	vec2 map_coord = (bump_vec.xy - vec2(0.5,0.5)) * factor + vec2(0.5,0.5);

		vec3 D = 2.0 * texture2D( view, t_coord ).xyz - vec3(1.0,1.0,1.0);
		vec3 R = reflect( D, N );

		float depth = clamp( texture2D( height_map, map_coord ).w * 3.0, 0.0, 1.0);

        vec2 dec = depth * 50.0 * cam_h_factor * (R.xz - D.xz);

        vec2 ts_coord = t_coord * coef;
		vec2 scr_pos0 = clamp( ts_coord - dec, 0.0, 1.0 );
        vec2 scr_pos1 = clamp( ts_coord + dec, 0.0, 1.0 );
        vec2 scr_pos0h = clamp( ts_coord - 3.0 * dec, 0.0, 1.0 );
        vec2 scr_pos1h = clamp( ts_coord + 3.0 * dec, 0.0, 1.0 );
        dec.x = -dec.x;
        vec2 scr_pos2 = clamp( ts_coord - dec, 0.0, 1.0 );
        vec2 scr_pos3 = clamp( ts_coord + dec, 0.0, 1.0 );
        vec2 scr_pos2h = clamp( ts_coord - 3.0 * dec, 0.0, 1.0 );
        vec2 scr_pos3h = clamp( ts_coord + 3.0 * dec, 0.0, 1.0 );

        vec4 p[8] = {texture2D( rtex, scr_pos0 ),
                     texture2D( rtex, scr_pos1 ),
                     texture2D( rtex, scr_pos2 ),
                     texture2D( rtex, scr_pos3 ),
                     texture2D( rtex, scr_pos0h ),
                     texture2D( rtex, scr_pos1h ),
                     texture2D( rtex, scr_pos2h ),
                     texture2D( rtex, scr_pos3h )};
        
		vec4 scr_col = 0.125 * (p[0] + p[1] + p[2] + p[3] + p[4] + p[5] + p[6] + p[7]);
        vec4 fuzzy;
        vec2 q = map_coord - vec2(0.5,0.5);
        q *= q;
        float depthCoef = 1.0 - exp(-depth * 2.0);
        if (max(q.x, q.y) > 0.25)
            gl_FragColor = texture2D(rtex, t_coord * coef);
        else
        {
            fuzzy = texture2D(water_color, t_coord);
            scr_col = mix(scr_col, fuzzy, 0.25 * depthCoef);

            gl_FragColor = mix(scr_col, vec4(1.0,1.0,1.0,1.0), depthCoef);
        }
        float l =  (f(p[4].rgb)
                    + f(p[5].rgb)
                    + f(p[6].rgb)
                    + f(p[7].rgb)
                    + f(p[0].rgb)
                    + f(p[1].rgb)
                    + f(p[2].rgb)
                    + f(p[3].rgb)) * 0.125;
        gl_FragColor *= l;
	}
}
