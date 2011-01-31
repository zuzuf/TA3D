uniform sampler2D       tex0;

const vec2 du = vec2(0.015625, 0.0);
const vec2 dv = vec2(0.0, 0.015625);
const vec4 one = vec4(1.0,1.0,1.0,0.0);

void main()
{
    vec4 v0 = texture2D( tex0, gl_TexCoord[0].xy );
    vec4 v1 = texture2D( tex0, gl_TexCoord[0].xy + du );
    vec4 v2 = texture2D( tex0, gl_TexCoord[0].xy + dv );
    vec4 v = normalize(vec4(dot(v1-v0,one), 0.2, dot(v2-v0,one), 0.0));
    gl_FragColor = gl_Color * v * (gl_Color.a * v0.a);
}
