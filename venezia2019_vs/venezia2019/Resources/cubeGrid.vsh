varying vec3 normal;
varying vec4 pos;
varying vec4 rawpos;
uniform vec3 u_offset;
varying vec2 uv;

void main() {
	uv = gl_MultiTexCoord0.xy;
	vec4 inPos4 = gl_Vertex + vec4(u_offset,0.0);
    normal = gl_NormalMatrix * gl_Normal;
    gl_Position = gl_ModelViewProjectionMatrix * inPos4;
    pos = gl_ModelViewMatrix * inPos4;
    rawpos = gl_Vertex;
}
