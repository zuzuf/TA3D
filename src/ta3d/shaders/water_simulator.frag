varying vec2 t_coord;
uniform float fluid;
uniform float t;
uniform sampler2D sim;

const vec2 du = vec2(0.00390625, 0.0);
const vec2 dv = vec2(0.0, 0.00390625);

void main()
{
    vec4 P0 = texture2D( sim, t_coord );
    vec4 PU = texture2D( sim, t_coord + du );
    vec4 PV = texture2D( sim, t_coord + dv );
    P0.xz = P0.xz + fluid * vec2(PU.y - P0.y, PV.y - P0.y);
    if(t > 0.5)
        gl_FragColor = vec4( 0.0, P0.w, 0.0, P0.w );
    else
        gl_FragColor = P0;
}
