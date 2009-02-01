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

		vec2 dec = 0.3 * (R.xz - D.xz);
		vec2 scr_pos_r = clamp( t_coord + dec, 0.0, 1.0 );

		float depth = clamp( texture2DLod( height_map, map_coord, 0.0 ).w * 3.0, 0.0, 1.0);

		vec4 ref = texture2D( sky, scr_pos_r );
		float trans = clamp( clamp( dot(D,N), 0.0, 1.0) + 1.0 - clamp( 10.0 * depth, 0.0, 1.0 ), 0.0, 1.0);
		
		vec2 scr_pos = clamp( t_coord * coef - (depth * 100.0 * cam_h_factor) * dec, 0.0, 1.0 );

        vec3 light = vec3( 0.57735026918962576451, -0.57735026918962576451, -0.57735026918962576451 );
		vec4 scr_col = texture2D( rtex, scr_pos );
		vec4 lava_col = vec4(1.0,0.2,0.2,1.0) * scr_col;
		vec3 water_col = 2.0 * texture2DLod( water_color, t_coord, 0.0 ).rgb;
		vec4 procedural_texture = clamp( clamp( -dot(N, light) + light.y, -1.0, 1.0)  * vec4( 3.0, 3.0, 5.0, 1.0 ) + vec4( pow( clamp( dot( R, light ), 0.0, 1.0 ), 20.0 ) ), -1.0, 1.0 );

        procedural_texture -= vec4(0.1 * depth);
        float coast_factor = clamp( 1.0 - 8.0 * depth, 0.0, 1.0 );
        float offset = coast_factor * 3.14 - cos(t) * 1.57 - 2.0;
        float f = cos( offset );
        float f2 = cos( offset - 0.2 );
        f = (f <= f2) ? pow(clamp(f,0.0,1.0),10.0) : f;
        f *= (0.5 * cos(t+0.2) + 0.5);
        procedural_texture += vec4( coast_factor * coast_factor * pow(f * 0.5 + 0.5, 10.0) );

		gl_FragColor = mix(ref,scr_col,trans) + procedural_texture;

		gl_FragColor = mix(gl_FragColor,lava_col,bump_vec.w * 2.0 - 1.0);
	}
}
