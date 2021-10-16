uniform sampler2DShadow shadowMap;
uniform sampler2D       tex0;
varying vec4            light_coord;
varying vec3            normal;

void main()
{
    float fog_coef = clamp( (gl_FogFragCoord - gl_Fog.start) * gl_Fog.scale, 0.0, 1.0 );

    float shaded = shadow2D( shadowMap, light_coord.xyz ).x;
    vec2 test = abs(light_coord.xy - vec2(0.5, 0.5));
    if (test.x > 0.5 || test.y > 0.5)
        shaded = 1.0;

    float diffuse_coef = clamp( dot( normalize(normal), gl_LightSource[0].position.xyz ), 0.0, 1.0 );
    vec4 light_eq = diffuse_coef * shaded + gl_LightModel.ambient;
    vec4 texture_color = texture2D( tex0, gl_TexCoord[0].xy );

    gl_FragColor = vec4(light_eq.rgb, 1.0) * gl_Color * texture_color;
    gl_FragColor.rgb = mix( gl_FragColor.rgb, gl_Fog.color.rgb, fog_coef);
}
