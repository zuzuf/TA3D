varying vec3 t_coord;
varying vec3 dir;
uniform float t;
uniform vec2 factor;
uniform sampler2D lava;
uniform sampler2D map;

vec3 wave_grad( vec2 d, float p, float type, vec2 tn_coord )
{
    vec2 base_grad = 2.0 * texture2D( map, vec2( dot( d, tn_coord ) + p, type ) ).xy - vec2( 1.0, 1.0 );
    vec3 grad = vec3( d.x * base_grad.x, base_grad.y, d.y * base_grad.x );
    return normalize( grad );
}

void main()
{
    vec2 tn_coord = t_coord.xy / t_coord.z;
    vec3 N =  wave_grad( vec2( 10.1, 11.1 ), 0.1 * t, 0.0, tn_coord )
            + wave_grad( vec2( 9.7, -3.1 ), 0.31 * t, 0.2, tn_coord )
            + wave_grad( vec2( -1.3, -0.3 ), 0.01 * t, 0.4, tn_coord )
            + wave_grad( vec2( -3.0, 1.3 ), 0.1 * t, 0.6, tn_coord )
            + wave_grad( vec2( 65.0, 15.1 ), 0.15 * t, 0.8, tn_coord )
            + wave_grad( vec2( 19.7, -33.1 ), 0.21 * t, 0.3, tn_coord )
            + wave_grad( vec2( 15.3, -10.3 ), 0.41 * t, 0.25, tn_coord )
            + wave_grad( vec2( -27.0, 23.3 ), 0.13 * t, 0.45, tn_coord )
            + wave_grad( vec2( 7.0, 8.1 ), 0.17 * t, 0.67, tn_coord )
            + wave_grad( vec2( 77.0, -130.3 ), 0.13 * t, 0.65, tn_coord )
            + wave_grad( vec2( 97.0, 87.1 ), 0.17 * t, 0.77, tn_coord )
            + wave_grad( vec2( -15.0, -1.3 ), 0.3 * t, 1.0, tn_coord);

    N = normalize(N);

    vec2 lava_coord = (tn_coord - vec2(0.5,0.5)) * factor + vec2(0.5,0.5);
    float lava_c = 0.5 * texture2D(lava,lava_coord).r + 0.5;

    gl_FragColor = vec4(0.5*(N + vec3(1.0,1.0,1.0)), lava_c);
}
