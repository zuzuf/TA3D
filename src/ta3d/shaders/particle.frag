#version 120
uniform sampler2D uTex;
uniform lowp vec4 uColor;

void main(void)
{
    gl_FragColor = texture2D(uTex, gl_PointCoord) * uColor;
}
