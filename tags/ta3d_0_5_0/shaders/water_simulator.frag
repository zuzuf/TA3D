varying vec2 t_coord;
uniform float fluid;
uniform float t;
uniform sampler2D sim;

const vec2 du = vec2(1.0/256.0, 0.0);
const vec2 dv = vec2(0.0, 1.0/256.0);

void main()
{
    vec4 P0 = texture2DLod( sim, t_coord, 0.0 );
    vec4 PU = texture2DLod( sim, t_coord + du, 0.0 );
    vec4 PV = texture2DLod( sim, t_coord + dv, 0.0 );
    P0.xz = P0.xz + fluid * vec2(PU.y - P0.y, PV.y - P0.y);
    if(t > 0.5)
        gl_FragColor = vec4( 0.0, P0.w, 0.0, P0.w );
    else
        gl_FragColor = P0;
}
