varying vec2 t_coord;
uniform sampler2D sim;

const vec2 du = vec2(1.0/256.0, 0.0);
const vec2 dv = vec2(0.0, 1.0/256.0);

void main()
{
    vec4 P0 = texture2DLod( sim, t_coord, 0.0 );
    vec4 PU = texture2DLod( sim, t_coord + du, 0.0 );
    vec4 PV = texture2DLod( sim, t_coord + dv, 0.0 );
	vec3 normal = normalize( vec3( PU.y - P0.y, 0.1, PV.y - P0.y ) );
    gl_FragColor = vec4(0.5 * normal + vec3(0.5, 0.5, 0.5), 0.0);
}
