varying vec3 normal;
varying vec4 pos;
varying vec2 uv;
uniform sampler2D texture;


void main() {

	vec4 lpos = vec4(30.0,40.0,-20.0,1.0);
	vec3 n = normalize(normal);

	float p = dot(normalize(lpos).xyz, n);
	vec4 color = texture2D(texture, uv) * (0.7 + p*0.3);



	// float att = exp(-130.0*max(0.0, gl_FragCoord.z-0.97));
	float att = exp(-320.0*max(0.0, gl_FragCoord.z-0.99));

	// if(gl_FragCoord.z < 0.95) discard;


	vec3 outcolor = vec3(1.0,1.0,1.0)*(1.0-att) + color.xyz * att; 
	gl_FragColor =  vec4(outcolor, 1.0);

  


}
