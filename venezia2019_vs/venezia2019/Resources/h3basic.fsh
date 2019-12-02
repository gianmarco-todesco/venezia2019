varying vec3 normal;
varying vec3 pos;
varying vec2 uv;
uniform sampler2D u_texture;
uniform vec2 u_texScale;
uniform vec3 u_color;

const vec3 lightPos = vec3(1.0,1.0,1.0);
const vec3 specColor = vec3(0.2, 0.2, 0.2);


void main() {
  vec3 lightDir = normalize(lightPos - pos);
  vec3 viewDir = normalize(-pos);

  vec3 diffuseColor = u_color * texture(u_texture, uv*u_texScale).rgb;
  vec3 ambientColor = diffuseColor * 0.1;

  
  vec3 norm = normalize(normal);
  if(dot(norm, viewDir)<0.0) norm = -norm;

  vec3 reflectDir = reflect(-lightDir, norm);


  float lambertian = max(dot(lightDir,norm), 0.0);
  float specular = 0.0;

  if(lambertian > 0.0) {
     float specAngle = max(dot(reflectDir, norm), 0.0);
     specular = pow(specAngle, 60.0);
  }
   
  gl_FragColor = vec4(ambientColor +
                      lambertian*diffuseColor +
                      specular*specColor, 1.0);

    
}

