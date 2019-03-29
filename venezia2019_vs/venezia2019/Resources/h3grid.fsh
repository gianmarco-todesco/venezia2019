varying vec3 normal;
varying vec3 pos;

const vec3 lightPos = vec3(1.0,1.0,1.0);
const vec3 ambientColor = vec3(0.1, 0.3, 0.4);
const vec3 diffuseColor = vec3(0.1, 0.3, 0.4);
const vec3 specColor = vec3(0.2, 0.2, 0.2);


void main() {
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
  
  
  
  // float att = exp(-130.0*max(0.0, gl_FragCoord.z-0.97));
  float att = max(0.0, min(1.0, (0.95 - gl_FragCoord.z)/0.1));
  gl_FragColor = vec4((ambientColor +
                      lambertian*diffuseColor +
                      specular*specColor)*att, 1.0);

    
}

