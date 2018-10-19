varying highp vec4 vColor;
varying highp vec3 vLighting;

void main(void) {
    gl_FragColor = vec4(vColor.rgb * vLighting, vColor.a);
}