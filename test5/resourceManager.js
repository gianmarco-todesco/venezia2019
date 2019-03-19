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
    attribute vec4 instancePos;


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


