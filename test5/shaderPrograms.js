ResourceStore.shaderPrograms["lines"] = {
    vs : `
    uniform mat4 u_worldViewProjection;
    attribute vec4 position;
    attribute vec3 color;
    varying vec3 v_color;
    void main() {
        gl_Position = u_worldViewProjection * position;
        v_color = color;
    }
    `,
    fs:`
    precision mediump float;
    varying vec3 v_color;
    void main() {
        gl_FragColor = vec4(v_color, 1.0);
    }
    `
};

ResourceStore.shaderPrograms["H2"] = {
    vs : `
    uniform mat4 u_worldViewProjection;
    uniform mat4 u_hMatrix;
    attribute vec2 position;
    
    vec4 toBall(vec4 p) {  
        vec4 p2 = p * (1.0/p.w);
        float s2 = min(1.0, p2.x*p2.x + p2.y*p2.y + p2.z*p2.z);
        float k = 1.0 / (1.0 + sqrt(1.0 - s2));        
        return vec4(p2.xyz*k,1.0);
    }

    void main() {
            vec4 p = toBall(u_hMatrix * vec4(position,0.0,1.0));
            gl_Position = u_worldViewProjection *  p;
    }
    `,
    fs:`
    precision mediump float;
    // varying vec4 v_color;
    void main() {
        gl_FragColor = vec4(0.0,0.4,0.8,1.0);
    }
    `
};

ResourceStore.shaderPrograms["2D"] = {
    vs : `
    precision mediump float;
    uniform mat4 u_worldViewProjection;
    attribute vec2 position;
    void main() {
        gl_Position = u_worldViewProjection * vec4(position,0.0,1.0);
    }
    `,
    fs:`
    precision mediump float;
    uniform vec4 u_color;
    void main() {
        gl_FragColor = u_color;
    }
    `
};


ResourceStore.shaderPrograms["standard"] = {
    vs : `
    uniform mat4 u_worldViewProjection;
    uniform vec3 u_lightWorldPos;
    uniform mat4 u_world;
    uniform mat4 u_viewInverse;
    uniform mat4 u_worldInverseTranspose;
    
    attribute vec4 position;
    attribute vec3 normal;

    varying vec4 v_position;
    varying vec3 v_normal;
    varying vec3 v_surfaceToLight;
    varying vec3 v_surfaceToView;

    void main() {
        vec4 pposition = position ;
        v_position = u_worldViewProjection * pposition;
        v_normal = (u_worldInverseTranspose * vec4(normal, 0)).xyz;
        v_surfaceToLight = u_lightWorldPos - (u_world * pposition).xyz;
        v_surfaceToView = (u_viewInverse[3] - (u_world * pposition)).xyz;
        gl_Position = v_position;
    }`, 
    fs : `
    precision mediump float;

    varying vec4 v_position;
    varying vec3 v_normal;
    varying vec3 v_surfaceToLight;
    varying vec3 v_surfaceToView;
    
    uniform vec4 u_lightColor;
    uniform vec4 u_diffuseColor;
    uniform vec4 u_ambient;
    uniform vec4 u_specular;
    uniform float u_shininess;
    uniform float u_specularFactor;
    
    vec4 lit(float l ,float h, float m) {
    return vec4(1.0,
                max(l, 0.0),
                (l > 0.0) ? pow(max(0.0, h), m) : 0.0,
                1.0);
    }
    
    void main() {
        vec4 diffuseColor = u_diffuseColor;
        vec3 a_normal = normalize(v_normal);
        vec3 surfaceToLight = normalize(v_surfaceToLight);
        vec3 surfaceToView = normalize(v_surfaceToView);
        vec3 halfVector = normalize(surfaceToLight + surfaceToView);
        vec4 litR = lit(dot(a_normal, surfaceToLight),
                            dot(a_normal, halfVector), u_shininess);
        vec4 outColor = vec4((
        u_lightColor * (diffuseColor * litR.y + diffuseColor * u_ambient +
                        u_specular * litR.z * u_specularFactor)).rgb,
            diffuseColor.a);
        gl_FragColor = outColor;
    }       
    `
};



ResourceStore.shaderPrograms["instanced"] = {
    vs : `
    uniform mat4 u_worldViewProjection;
    uniform vec3 u_lightWorldPos;
    uniform mat4 u_world;
    uniform mat4 u_viewInverse;
    uniform mat4 u_worldInverseTranspose;
    attribute vec3 instancePos;


    attribute vec4 position;
    attribute vec3 normal;

    varying vec4 v_position;
    varying vec3 v_normal;
    varying vec3 v_surfaceToLight;
    varying vec3 v_surfaceToView;

    void main() {
        vec4 pposition = position + vec4(instancePos,0.0);
        v_position = u_worldViewProjection * pposition;
        v_normal = (u_worldInverseTranspose * vec4(normal, 0)).xyz;
        v_surfaceToLight = u_lightWorldPos - (u_world * pposition).xyz;
        v_surfaceToView = (u_viewInverse[3] - (u_world * pposition)).xyz;
        gl_Position = v_position;
    }`, 
    fs : `
    precision mediump float;

    varying vec4 v_position;
    varying vec3 v_normal;
    varying vec3 v_surfaceToLight;
    varying vec3 v_surfaceToView;
    
    uniform vec4 u_lightColor;
    uniform vec4 u_diffuseColor;
    uniform vec4 u_ambient;
    uniform vec4 u_specular;
    uniform float u_shininess;
    uniform float u_specularFactor;
    
    vec4 lit(float l ,float h, float m) {
    return vec4(1.0,
                max(l, 0.0),
                (l > 0.0) ? pow(max(0.0, h), m) : 0.0,
                1.0);
    }
    
    void main() {
        vec4 diffuseColor = u_diffuseColor;
        vec3 a_normal = normalize(v_normal);
        vec3 surfaceToLight = normalize(v_surfaceToLight);
        vec3 surfaceToView = normalize(v_surfaceToView);
        vec3 halfVector = normalize(surfaceToLight + surfaceToView);
        vec4 litR = lit(dot(a_normal, surfaceToLight),
                            dot(a_normal, halfVector), u_shininess);
        vec4 outColor = vec4((
        u_lightColor * (diffuseColor * litR.y + diffuseColor * u_ambient +
                        u_specular * litR.z * u_specularFactor)).rgb,
            diffuseColor.a);
        // near < v_position.z < 20.0 
        // if(v_position.y>0.0 && v_position.z>20.0) discard;

        float att = exp(-0.2*max(0.0, v_position.z));
        gl_FragColor = vec4(outColor.rgb * att, 1.0);
    }       
    `
};

ResourceStore.shaderPrograms["hyperbolic"] = {
    vs : `
        uniform mat4 u_worldViewProjection;
        uniform mat4 u_worldInverseTranspose;
        uniform mat4 u_world;
        
        uniform mat4 hMatrix;
        attribute vec4 position;
        attribute vec4 position_du;
        attribute vec4 position_dv;
        attribute mat4 cellMatrix;

        varying vec4 v_position;
        varying vec3 v_normal;
        varying vec3 v_surfaceToLight;
        varying vec3 v_surfaceToView;


        vec4 toBall(vec4 p) {  
            vec4 p2 = p * (1.0/p.w);
            float s2 = min(1.0, p2.x*p2.x + p2.y*p2.y + p2.z*p2.z);
            float k = 1.0 / (1.0 + sqrt(1.0 - s2));
            
            return vec4(p2.xyz*k,1.0);
        }

        void main() {
            mat4 mat = hMatrix * cellMatrix;
            vec4 p = toBall(mat * position);
            vec4 p_du = toBall(mat * position_du);
            vec4 p_dv = toBall(mat * position_dv);
            vec3 normal = normalize(cross((p_du-p).xyz, (p_dv-p).xyz));
            v_normal = (u_worldInverseTranspose * vec4(normal, 0)).xyz;
            v_position = u_worldViewProjection *  p;
            gl_Position = u_worldViewProjection *  p;
            
        }
    `,
    fs: `
        precision mediump float;
        varying vec4 v_position;        
        varying vec3 v_normal;
        void main() {
            vec3 normal = normalize(v_normal);
            float v = abs(normal.z);
            // if(v_position.z>1.7) discard;
            float att = 1.0; // exp(-12.0*max(0.0, v_position.z-1.3));
            gl_FragColor = vec4(vec3(0.2,0.6,0.3) * v * att, 1.0);
            // gl_FragColor = vec4(1.0,1.0,0.0,1.0);

        }
    `
};
