uniform sampler2DShadow shadowMap;
uniform sampler2D       tex;
uniform sampler2D       details;
uniform float           coef;
varying vec4            light_coord;

void main()
{
    float fog_coef = clamp( (gl_FogFragCoord - gl_Fog.start) * gl_Fog.scale, 0.0, 1.0 );

    float shaded = shadow2D( shadowMap, light_coord.xyz ).x;
    vec2 test = abs(light_coord.xy - vec2(0.5, 0.5));
    if (test.x > 0.5 || test.y > 0.5)
        shaded = 1.0;

    vec4 light_eq = mix(0.2, 1.0, shaded) * coef * gl_Color;
    vec4 texture_color = texture2D( tex, gl_TexCoord[0].xy ) * texture2D( details, gl_TexCoord[1].xy );

    gl_FragColor = mix( light_eq * texture_color, gl_Fog.color, fog_coef);
}
