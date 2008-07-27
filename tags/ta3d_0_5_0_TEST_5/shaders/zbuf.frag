varying vec2 t_coord;
uniform sampler2D tex;

void main()
{
		vec4 col = texture2D(tex, t_coord);
//        gl_FragColor = col; f * n / ((zw / s) * (f-n) - f) [*]
//        gl_FragDepth = col.r+col.g/256+col.b/65536;
        gl_FragDepth = -500.0 / ((col.r+col.g/256+col.b/65536)*499 - 500);
}
