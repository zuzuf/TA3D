attribute highp vec2 aVertex;
attribute lowp vec4 aColor;
varying lowp vec4 vColor;
uniform highp mat4 uMatrix;

void main()
{
    vColor = aColor;
    gl_Position = uMatrix * vec4(aVertex, 0.0, 1.0);
}
