varying vec2 t_coord;
varying vec2 dt_coord;
varying vec3 color;
uniform float coef;
uniform sampler2D tex;
uniform sampler2D details;

void main()
{
    float fog_coef = clamp( (gl_FogFragCoord - gl_Fog.start) * gl_Fog.scale, 0.0, 1.0 );
    gl_FragColor = mix( vec4( color, coef ) * texture2D( tex, t_coord ) * texture2D( details, dt_coord ), gl_Fog.color, fog_coef );
}
