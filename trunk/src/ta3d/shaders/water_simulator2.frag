varying vec2 t_coord;
uniform float dt;
uniform sampler2D sim;

const vec2 du = vec2(0.00390625, 0.0);
const vec2 dv = vec2(0.0, 0.00390625);

void main()
{
    vec4 P0 = texture2D( sim, t_coord );
    vec4 PU = texture2D( sim, t_coord - du );
    vec4 PV = texture2D( sim, t_coord - dv );
    P0.y += dt * (P0.x + P0.z - PU.x - PV.z);
    gl_FragColor = P0;
}
