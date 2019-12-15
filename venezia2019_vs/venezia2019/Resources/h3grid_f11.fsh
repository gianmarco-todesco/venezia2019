varying vec3 normal;
varying vec3 pos;
varying vec2 uv;
varying float uffa;
uniform float flag;

const vec3 lightPos = vec3(1.0,1.0,1.0);
vec3 ambientColor = vec3(uv.x, uv.y, 0.4);
vec3 diffuseColor = vec3(uv.x, uv.y, 0.4);
//vec3 ambientColor = vec3(0.1, 0.2, 0.4);
//vec3 diffuseColor = vec3(0.1, 0.2, 0.4);
const vec3 specColor = vec3(0.2, 0.2, 0.2);

uniform sampler2D texture;

void main() {

  if(flag>0.0 && pos.x>0.0 && uffa > 0) discard;


  vec3 norm = normalize(normal);

  vec3 lightDir = normalize(lightPos - pos);
  vec3 reflectDir = reflect(-lightDir, norm);
  vec3 viewDir = normalize(-pos);

  float lambertian = max(dot(lightDir,normal), 0.0);
  float specular = 0.0;

  if(lambertian > 0.0) {
     float specAngle = max(dot(reflectDir, viewDir), 0.0);
     specular = pow(specAngle, 14.0);
  }
  
  vec4 color = texture2D(texture, uv);
  
  // float att = exp(-130.0*max(0.0, gl_FragCoord.z-0.97));
  float att = max(0.0, min(1.0, (1.15 - gl_FragCoord.z)/0.1));
  att = 1.0;
  gl_FragColor = vec4((color.rgb +
                      lambertian*color.rgb +
                      specular*specColor)*att + vec3(1.0,1.0,1.0)*(1.0-att), 1.0);

    
}
