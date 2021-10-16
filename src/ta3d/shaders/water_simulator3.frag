varying vec2 t_coord;
uniform sampler2D sim;

const vec2 du = vec2(0.00390625, 0.0);
const vec2 dv = vec2(0.0, 0.00390625);

void main()
{
    vec4 P0 = texture2D( sim, t_coord );
    vec4 PU = texture2D( sim, t_coord + du );
    vec4 PV = texture2D( sim, t_coord + dv );
    vec3 normal = normalize( vec3( PU.y - P0.y, 0.1, PV.y - P0.y ) );
    gl_FragColor = vec4(normal, 0.0);
}
