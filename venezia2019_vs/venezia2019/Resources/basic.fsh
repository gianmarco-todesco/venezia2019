varying vec3 normal;
varying vec4 pos;


void main() {
  vec4 color = gl_FrontMaterial.diffuse;
  // color.y = 0.5+0.5*sin(uv.x*10);
  vec4 matspec = gl_FrontMaterial.specular;
  float shininess = gl_FrontMaterial.shininess;
  vec4 lightspec = gl_LightSource[0].specular;
  vec4 lpos = gl_LightSource[0].position;
  vec4 s = -normalize(pos-lpos);

  vec3 light = s.xyz;
  float ff = 0.1;
  vec3 n = normalize(normal);
  if(dot(n,s.xyz)<0.0) n = -n;



  vec3 r = -reflect(light, n);
  r = normalize(r);
  vec3 v = -pos.xyz;
  v = normalize(v);

  vec4 diffuse  = color * max(0.0, dot(n, s.xyz)) * gl_LightSource[0].diffuse;
  vec4 specular;
  if (shininess != 0.0) {
    specular = lightspec * matspec * pow(max(0.0,                 dot(r, v)), shininess);
  } else {
    specular = vec4(0.0, 0.0, 0.0, 0.0);
  }

  gl_FragColor = 0.1*gl_FrontMaterial.ambient + diffuse + specular ;
}
