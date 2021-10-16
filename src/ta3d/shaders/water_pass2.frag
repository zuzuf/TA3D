varying vec3 dir;

void main()
{
    vec3 D = normalize(dir);
    gl_FragColor = vec4(0.5*(D+vec3(1.0,1.0,1.0)),1.0);
}
