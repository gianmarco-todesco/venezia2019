varying vec3 normal;
varying vec3 pos;

const vec3 lightPos = vec3(1.0,1.0,1.0);
const vec3 ambientColor = vec3(0.3, 0.0, 0.0);
const vec3 diffuseColor = vec3(0.5, 0.0, 0.0);
const vec3 specColor = vec3(1.0, 1.0, 1.0);


void main() {
  vec3 norm = normalize(normal);

  vec3 lightDir = normalize(lightPos - pos);
  vec3 reflectDir = reflect(-lightDir, norm);
  vec3 viewDir = normalize(-pos);

  float lambertian = max(dot(lightDir,normal), 0.0);
  float specular = 0.0;

  if(lambertian > 0.0) {
     float specAngle = max(dot(reflectDir, viewDir), 0.0);
     specular = pow(specAngle, 4.0);
  }
  gl_FragColor = vec4(ambientColor +
                      lambertian*diffuseColor +
                      specular*specColor, 1.0);

    
}

