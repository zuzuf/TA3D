attribute highp vec2 aVertex;
attribute highp vec2 aTexCoord;
attribute lowp vec4 aColor;
varying lowp vec4 vColor;
varying lowp vec2 vTexCoord;
uniform highp mat4 uMatrix;

void main()
{
    vColor = aColor;
    vTexCoord = aTexCoord;
    gl_Position = uMatrix * vec4(aVertex, 0.0, 1.0);
}
