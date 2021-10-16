varying vec2 t_coord;
uniform sampler2D sky;
uniform sampler2D rtex;
uniform sampler2D bump;
uniform sampler2D view;
uniform sampler2D water_color;
uniform vec2 coef;

void main()
{
    vec4 bump_vec = texture2D(bump, t_coord);
    if (bump_vec.w == 0.0)
        gl_FragColor = vec4(0.0,0.0,0.0,1.0);
    else
    {
        vec3 N = 2.0 * bump_vec.xyz - vec3(1.0,1.0,1.0);
        vec3 D = 2.0 * texture2D( view, t_coord ).xyz - vec3(1.0,1.0,1.0);
        vec3 R = reflect( D, N );

        vec2 dec = 0.005 * (R.xz - D.xz);
        vec2 scr_pos_r = clamp( t_coord + dec, 0.0, 1.0 );

        vec4 ref = texture2D( sky, scr_pos_r );
        float trans = clamp( D.y, 0.0, 1.0);

        vec2 scr_pos = clamp( t_coord * coef - dec, 0.0, 1.0 );

        vec4 scr_col = texture2D( rtex, scr_pos );
        if (dot(scr_col.rgb, scr_col.rgb) == 0.0)
            gl_FragColor = vec4(0.0);
        else
        {
            vec4 lava_col = vec4(1.0,0.2,0.2,1.0) * scr_col;
            vec3 water_col = texture2D( water_color, t_coord ).rgb;
            vec4 procedural_texture = clamp( vec4( N.y * water_col, 0.0) + pow( clamp( dot( R, vec3( 0.57735026918962576451, -0.57735026918962576451, -0.57735026918962576451 ) ), 0.0, 1.0 ), 20.0 ), 0.0, 1.0 );

            scr_col = mix( scr_col, procedural_texture, procedural_texture );

            gl_FragColor = mix( vec4(mix(ref,scr_col,trans).rgb,0.8), lava_col, bump_vec.w * 2.0 - 1.0);
        }
    }
}
