attribute highp vec3 aVertex;
uniform highp mat4 uProjectionMatrix;
uniform highp mat4 uModelViewMatrix;
uniform highp vec3 uPointDistanceAttenuation;
uniform highp float uPointSize;

void main(void)
{
    highp vec4 vPos = uModelViewMatrix * vec4(aVertex, 1.0);
    gl_Position = uProjectionMatrix * vPos;
    highp float Z = vPos.z / vPos.w;
    gl_PointSize = uPointSize / sqrt(dot(uPointDistanceAttenuation, vec3(1.0, Z, Z * Z)));
}
