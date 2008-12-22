varying vec2 t_coord;
uniform float dt;
uniform sampler2D sim;

const vec2 du = vec2(1.0/256.0, 0.0);
const vec2 dv = vec2(0.0, 1.0/256.0);

void main()
{
    vec4 P0 = texture2DLod( sim, t_coord, 0.0 );
    vec4 PU = texture2DLod( sim, t_coord - du, 0.0 );
    vec4 PV = texture2DLod( sim, t_coord - dv, 0.0 );
    P0.y += dt * (P0.x + P0.z - PU.x - PV.z);
    gl_FragColor = P0;
}
