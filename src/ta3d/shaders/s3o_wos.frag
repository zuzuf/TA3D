uniform sampler2D       tex0;
uniform sampler2D       tex1;
uniform float           t;
uniform vec4            team;
varying vec3            normal;

void main()
{
    float fog_coef = clamp( (gl_FogFragCoord - gl_Fog.start) * gl_Fog.scale, 0.0, 1.0 );

    float diffuse_coef = clamp( dot( normalize(normal), gl_LightSource[0].position.xyz ), 0.0, 1.0 );
    vec4 light_eq = diffuse_coef + gl_LightModel.ambient;
    vec4 texture_color = texture2D( tex0, gl_TexCoord[0].xy );
    vec4 prop = texture2D( tex1, gl_TexCoord[0].xy );
    texture_color = mix(texture_color, team, texture_color.a);
    texture_color.a = prop.a;

    gl_FragColor = vec4(light_eq.rgb, 1.0) * gl_Color * texture_color + prop.r * vec4(t, t, t, 0.0);
    gl_FragColor.rgb = mix( gl_FragColor.rgb, gl_Fog.color.rgb, fog_coef);
}
