"use strict";

class ShaderProgram {

    constructor(gl, vSrc, fSrc) { 
        this.gl = gl;
        this.vSrc = vSrc;
        this.fSrc = fSrc;
        this.pInfo = this._createProgramInfoFromSources(vSrc, fSrc);        
    }

    _createShaderFromSource(type, src) {
        const gl = this.gl;
        const shader = gl.createShader(type);
        gl.shaderSource(shader, src);
        gl.compileShader(shader);
        const compiled = gl.getShaderParameter(shader, gl.COMPILE_STATUS);
        if (!compiled) {
            // Something went wrong during compilation; get the error
            var lastError = gl.getShaderInfoLog(shader);
            gl.deleteShader(shader);
            throw lastError;
        }
        return shader;
    }    

    _createProgramInfoFromSources = function (vSrc, fSrc) {
        const gl = this.gl;
        const vShader = this._createShaderFromSource(gl.VERTEX_SHADER, vSrc);        
        const fShader = this._createShaderFromSource(gl.FRAGMENT_SHADER, fSrc);
        const program = gl.createProgram();
        gl.attachShader(program, vShader);
        gl.attachShader(program, fShader);
        gl.linkProgram(program);
        if ( !gl.getProgramParameter( program, gl.LINK_STATUS) ) {
            var info = gl.getProgramInfoLog(program);
            throw 'Could not compile WebGL program. \n\n' + info;
        }
        const programInfo = twgl.createProgramInfoFromProgram(gl, program);
        return programInfo;    
    }
}    


class Material {
    constructor(program) {
        this.program = program;
        this.uniforms = {};
    }
}



class ResourceManager {
    constructor(gl) {
        this.gl = gl;
        this.shaderPrograms = {};
        this.materials = {};
        this.geometries = {};
    }

    getShaderProgram(name) {
        let prog = this.shaderPrograms[name];
        if(prog) return prog;
        const src = ResourceStore.shaderPrograms[name];
        if(!src) throw `Undefined shader program : ${name}`;
        prog = this.shaderPrograms[name] = new ShaderProgram(this.gl, src.vs, src.fs);
        return prog;
    }

    createStandardMaterial(r,g,b) {
        const material = new Material(this.getShaderProgram("standard"));
        material.uniforms.u_diffuseColor = [r,g,b,1.0];
        return material;
    }


    getGeometry(name) {
        let geo = this.geometries[name];
        if(!geo) {
            const f = ResourceStore.geometries[name];
            if(!f) throw `Undefined geometry : ${name}`;
            geo = this.geometries[name] = f(this.gl);
        }
        return geo;
    }

    
};

const ResourceStore = {
    shaderPrograms : {},
    geometries : {},
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
            vec4 p = toBall(hMatrix * position);
            vec4 p_du = toBall(hMatrix * position_du);
            vec4 p_dv = toBall(hMatrix * position_dv);
            vec3 normal = normalize(cross((p_du-p).xyz, (p_dv-p).xyz));
            v_normal = (u_worldInverseTranspose * vec4(normal, 0)).xyz;
            gl_Position = u_worldViewProjection *  p;
            
        }
    `,
    fs: `
        precision mediump float;
        varying vec3 v_normal;
        void main() {
            vec3 normal = normalize(v_normal);
            float v =abs(normal.z);

            gl_FragColor = vec4(vec3(0.2,0.6,0.3) * v, 1.0);
        }
    `
};


ResourceStore.geometries["three-cubes"] = function(gl) {
    const arrays = GeometryBuilder.createCubeArray(gl, 0.5);
    arrays.instancePos = {
        numComponents: 4,
        data: [0,0,0,1, 1,1,1,1, 2,2,2,1],
        divisor: 1
    };
    return twgl.createBufferInfoFromArrays(gl, arrays);
};

const PolygonHeight = 0.01;
const PolygonEdge = 1;

ResourceStore.geometries["sphere"] = function(gl) {
    return twgl.primitives.createSphereBufferInfo(gl, 1, 20, 20);
}


ResourceStore.geometries["triangle"] = function(gl) {
    const r = PolygonEdge*0.5/Math.sin(Math.PI/3);
    const arrays = GeometryBuilder.createThickFaceArray(gl, 3, r, PolygonHeight);
    return twgl.createBufferInfoFromArrays(gl, arrays);
};

ResourceStore.geometries["square"] = function(gl) {
    const r = PolygonEdge/Math.sqrt(2);
    const arrays = GeometryBuilder.createThickFaceArray(gl, 4, r, PolygonHeight);
    return twgl.createBufferInfoFromArrays(gl, arrays);
};


ResourceStore.geometries["pentagon"] = function(gl) {
    const r = PolygonEdge*0.5/Math.sin(Math.PI/5);
    const arrays = GeometryBuilder.createThickFaceArray(gl, 5, r, PolygonHeight);
    return twgl.createBufferInfoFromArrays(gl, arrays);
};

ResourceStore.geometries["cube-grid-cell"] = function(gl) {
    const m4 = twgl.m4;
    const gb = new GeometryBuilder(gl);

    const a = 0.8, b = 1.0, c = 0.95;

    // cubi per i vertici
    var i,j ;
    const faceMatrices = [
        m4.identity(),
        m4.rotationX(Math.PI),
        m4.rotationX(Math.PI/2),
        m4.rotationX(-Math.PI/2),
        m4.rotationY(Math.PI/2),
        m4.rotationY(-Math.PI/2),
    ];
    for(i=0;i<6;i++) {
        gb.pushMatrix();
        gb.multmatrix(faceMatrices[i]);
        gb.setNormal(0,0,1);
        gb.addQuads([
            -b,-b,-a, -a,-b,-a, -a,-a,-a, -b,-a,-a,
             a,-b,-a,  b,-b,-a,  b,-a,-a,  a,-a,-a,
            -b, a,-a, -a, a,-a, -a, b,-a, -b, b,-a,
             a, a,-a,  b, a,-a,  b, b,-a,  a, b,-a,
        ]);    
        gb.popMatrix();
    }

    // parallelepipedi per gli spigoli
    const edgeMatrices = [
        m4.identity(),
        m4.rotationX(Math.PI/2),
        m4.rotationY(Math.PI/2),
    ]
    for(i=0;i<3;i++) {
        gb.pushMatrix();
        gb.multmatrix(edgeMatrices[i]);
        for(j=0; j<4; j++) {
            gb.pushMatrix();
            gb.rotateZ(Math.PI*0.5*j);
            gb.setNormal(1,0,0);
            gb.addQuads([
                -c,-b,-b, -c,-c,-b, -c,-c, b, -c,-b, b,
                -c, c,-b, -c, b,-b, -c, b, b, -c, c, b,
            ]);
            gb.popMatrix();
        }
        gb.popMatrix();    
    }

    const offsets = [];
    var m = 7;
    for(let dx = -m; dx<=m; dx++)
    for(let dy = -m; dy<=m; dy++)
    for(let dz = -m; dz<=m; dz++) {
        offsets.push(dx*2,dy*2,dz*2);
        // offsets.push(0,0,0);
    }

    let arrays = gb.arrays;

    arrays.instancePos = {
        numComponents: 3,
        data: offsets,
        divisor: 1
    };

    
    return twgl.createBufferInfoFromArrays(gl, arrays);
};



ResourceStore.geometries["534-grid-cell"] = function(gl) {
    const m4 = twgl.m4;
    const v3 = twgl.v3;

    const gb = new GeometryBuilder2(gl);

    const sc = computeGrid534Radius();
    const dod = regularPolyhedra.dodecahedron;
    const pts = dod.vertices.map(v=>v3.mulScalar(v, sc));
    const centers = dod.centers.map(v=>v3.mulScalar(v, sc));

    var p = centers[0];
    var hMatrix = m4.rotateZ(twgl.m4.multiply(
        KPoint.reflection(new KPoint(0,0,0)),
        KPoint.reflection(new KPoint(p[0],p[1],p[2]))), Math.PI/5);

    // const nodes = fooBar(dod, hMatrix);

    dod.edges.forEach(e => {
        var p0 = pts[e[0]];
        var p1 = pts[e[1]];

        var pa = centers[e[2]];
        var pb = centers[e[3]];

        var p2 = v3.lerp(p0,pa,0.1);
        var p3 = v3.lerp(p1,pa,0.1);
        var p4 = v3.lerp(p0,pb,0.1);
        var p5 = v3.lerp(p1,pb,0.1);
        
        gb.addStrip(p2,p3,p5,p4, 30);
    });
    const result = gb.createBuffer();
    result.extraData = {
        hMatrix
    };
    return result;
};

