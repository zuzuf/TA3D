varying lowp vec4 vColor;
varying lowp vec2 vTexCoord;
uniform sampler2D uTex;

void main()
{
    gl_FragColor = vColor * texture2D(uTex, vTexCoord);
}
