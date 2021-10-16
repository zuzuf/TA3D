varying vec2 t_coord;
uniform vec2 coef;
uniform sampler2D sky;
uniform sampler2D rtex;
uniform sampler2D bump;
uniform sampler2D view;

void main()
{
    vec4 bump_vec = texture2D(bump, t_coord);
    float lava = bump_vec.w;
    if (lava == 0.0)
        gl_FragColor = vec4(0.0,0.0,0.0,1.0);
    else
    {
        vec3 N = 2.0 * bump_vec.xyz - vec3(1.0,1.0,1.0);
        vec3 D = 2.0 * texture2D( view, t_coord ).xyz - vec3(1.0,1.0,1.0);
        vec3 R = reflect( D, N );

        vec2 scr_pos_r = clamp( t_coord + 0.005 * R.xz, 0.0, 1.0 );

        vec4 ref = texture2D( sky, scr_pos_r );
        float trans = clamp( D.y, 0.0, 1.0);

        vec2 scr_pos = clamp( t_coord * coef - 0.005 * R.xz, 0.0, 1.0 );

        vec4 scr_col = texture2D( rtex, scr_pos );
        if (dot(scr_col.rgb,scr_col.rgb) == 0.0)
            gl_FragColor = vec4(0.0);
        else
        {
            gl_FragColor = vec4(mix(ref,scr_col,trans).rgb,0.8);

            vec4 lava_col = vec4(1.0,0.2,0.2,1.0)*texture2D(rtex,scr_pos);
            gl_FragColor = mix(gl_FragColor,lava_col,lava * 2.0 - 1.0);
        }
    }
}
