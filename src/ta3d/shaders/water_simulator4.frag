varying vec3 t_coord;
uniform vec2 factor;
uniform sampler2D lava;

void main()
{
    vec2 tn = t_coord.xy / t_coord.z;
    vec2 real_coord = (tn - vec2(0.5,0.5))*factor+vec2(0.5,0.5);

    float lava_c = 0.5 * texture2D(lava,real_coord).r + 0.5;

    gl_FragColor = vec4(tn, 0.0, lava_c);
}
