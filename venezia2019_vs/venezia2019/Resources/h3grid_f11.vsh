varying vec3 normal;
varying vec3 pos;
varying vec2 uv;
uniform mat4 hMatrix;

varying float uffa;

vec4 toBall(vec4 p) {
  vec4 p2 = p * (1.0/p.w);
  float s2 = min(1.0, p2.x*p2.x + p2.y*p2.y + p2.z*p2.z);
  float k = 10.0 / (1.0 + sqrt(1.0 - s2));        
  return vec4(p2.xyz*k,1.0);
}


void main() {
  
  uv = gl_MultiTexCoord0.xy;

  vec4 inputPos = toBall(hMatrix * gl_Vertex);

  

  gl_Position = gl_ModelViewProjectionMatrix * inputPos;
  

  vec4 inputPos2 = toBall(hMatrix * (gl_Vertex + vec4(0.01*gl_Normal, 0.0)));

  normal = gl_NormalMatrix * normalize((inputPos2 - inputPos).xyz);
  vec4 pos4 = gl_ModelViewMatrix * inputPos;  
  pos = pos4.xyz / pos4.w;

  uffa = (pos4 - gl_ModelViewMatrix * vec4(0.0,0.0,0.0,1.0)).z;
  
  
}

