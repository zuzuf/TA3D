uniform sampler2DShadow shadowMap;
uniform sampler2D       tex0;
varying vec4            light_coord;
varying vec3            normal;

void main()
{
	float fog_coef = clamp( (gl_FogFragCoord - gl_Fog.start) * gl_Fog.scale, 0.0, 1.0 );

    float shaded = shadow2D( shadowMap, light_coord.xyz ).x;
    if (light_coord.x < 0.0 || light_coord.x > 1.0 || light_coord.y < 0.0 || light_coord.y > 1.0)
        shaded = 1.0;

    float diffuse_coef = clamp( dot( normalize(normal), normalize(vec3(gl_LightSource[0].position)) ), 0.0, 1.0 );
    vec4 light_eq = (diffuse_coef * shaded + gl_LightModel.ambient) * gl_Color;
    vec4 texture_color = texture2D( tex0, gl_TexCoord[0].xy );

    gl_FragColor = light_eq * texture_color;
    gl_FragColor.rgb = mix( gl_FragColor.rgb, gl_Fog.color.rgb, fog_coef);
}
