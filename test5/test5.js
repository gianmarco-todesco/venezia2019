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


class MaterialStore {
    constructor(gl) {
        this.gl = gl;
        this.programs = {};
    }

    _touchProgram(name, src) {
        if(this.programs[name]) return this.programs[name];    
        const prog = this.programs[name] = new ShaderProgram(this.gl, src.vs, src.fs);
        return prog;
    }
    standardProgram() { return this._touchProgram("standard",  MaterialStore.progSources.standard); }

    createStandardMaterial(r,g,b) {
        const material = new Material(this.standardProgram());
        material.uniforms.u_diffuseColor = [r,g,b,1.0];
        return material;
    }

}

MaterialStore.progSources = {};
MaterialStore.progSources.standard = { 
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
        vec4 pposition = position + instancePos;
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
    `};

MaterialStore.progSources.standardWithTexture = { 
    vs : `
    uniform mat4 u_worldViewProjection;
    uniform vec3 u_lightWorldPos;
    uniform mat4 u_world;
    uniform mat4 u_viewInverse;
    uniform mat4 u_worldInverseTranspose;

    attribute vec4 position;
    attribute vec3 normal;
    attribute vec2 texcoord;

    varying vec4 v_position;
    varying vec2 v_texCoord;
    varying vec3 v_normal;
    varying vec3 v_surfaceToLight;
    varying vec3 v_surfaceToView;

    void main() {
        v_texCoord = texcoord;
        v_position = u_worldViewProjection * position;
        v_normal = (u_worldInverseTranspose * vec4(normal, 0)).xyz;
        v_surfaceToLight = u_lightWorldPos - (u_world * position).xyz;
        v_surfaceToView = (u_viewInverse[3] - (u_world * position)).xyz;
        gl_Position = v_position;
    }`, 
    fs : `
    precision mediump float;

    varying vec4 v_position;
    varying vec2 v_texCoord;
    varying vec3 v_normal;
    varying vec3 v_surfaceToLight;
    varying vec3 v_surfaceToView;
    
    uniform vec4 u_lightColor;
    uniform vec4 u_ambient;
    uniform sampler2D u_diffuse;
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
        vec4 diffuseColor = texture2D(u_diffuse, v_texCoord);
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
    `};

class Camera {
    constructor(canvas) {
        this.canvas = canvas;
        this.fov = 30 * Math.PI / 180;
        this.zNear = 0.1;
        this.zFar = 20;
        this.update();
      }

    update() {
        const m4 = twgl.m4;
        const aspect = this.canvas.clientWidth / this.canvas.clientHeight;
        const projection = m4.perspective(this.fov, aspect, this.zNear, this.zFar);
        this.projection = projection;

        const eye = [0, 0, -10];
        const target = [0, 0, 0];
        const up = [0, 1, 0];
        this.cameraMatrix = twgl.m4.lookAt(eye, target, up);
        

    }
/*        uniforms.u_viewInverse = camera;
      uniforms.u_world = world;
      uniforms.u_worldInverseTranspose = m4.transpose(m4.inverse(world));
      uniforms.u_worldViewProjection = m4.multiply(viewProjection, world);

        const eye = [1, 4, -6];
        const target = [0, 0, 0];
        const up = [0, 1, 0];
  
        const camera = m4.lookAt(eye, target, up);
        const view = m4.inverse(camera);
        const viewProjection = m4.multiply(projection, view);
        const world = m4.rotationY(time);
  
    }
    
    }
    */
}

class Material {
    constructor(program) {
        this.program = program;
        this.uniforms = {};
    }

}





class Mesh {

}


class Engine {
    
    constructor(canvasId) {
        const canvas = this.canvas = document.getElementById(canvasId);
        const gl = this.gl = canvas.getContext("webgl");
        twgl.addExtensionsToContext(gl);
        if (!gl.drawArraysInstanced || !gl.createVertexArray) {
            alert("need drawArraysInstanced and createVertexArray"); // eslint-disable-line
            return;
        }
        this.camera = new Camera(this.canvas);
        this.world = twgl.m4.identity();
        this.materialStore = new MaterialStore(gl);
        this.uniforms = {
            u_lightWorldPos: [1, 8, -10],
            u_lightColor: [1, 0.8, 0.8, 1],
            u_ambient: [0, 0, 0, 1],    
        };
    }


    setMaterial(material) {
        if(this.currentProgram != material.program) {
            this.currentProgram = material.program;
            this.gl.useProgram(this.currentProgram.pInfo.program);
        }
        this.currentMaterial = material;
        Object.assign(this.uniforms, material.uniforms);
        twgl.setUniforms(this.currentProgram.pInfo, this.uniforms);
    }

    setBuffers(bufferInfo) {
        twgl.setBuffersAndAttributes(this.gl, this.currentProgram.pInfo, bufferInfo);
    }

    setWorldMatrix(world) {
        const m4 = twgl.m4;
        const view = m4.inverse(this.camera.cameraMatrix);
        const viewProjection = m4.multiply(this.camera.projection, view);
        const uniforms = {
            u_viewInverse : this.camera.cameraMatrix,
            u_world : world,
            u_worldInverseTranspose : m4.transpose(m4.inverse(world)),
            u_worldViewProjection : m4.multiply(viewProjection, world)
        };
        twgl.setUniforms(this.currentProgram.pInfo, uniforms);
    }




    useProgram(program) {
        this.currentProgram = program;
        gl.useProgram(prog.pInfo.program);
        twgl.setBuffersAndAttributes(gl, prog.pInfo, bufferInfo);
        twgl.setUniforms(prog.pInfo, uniforms);
        gl.drawElements(gl.TRIANGLES, bufferInfo.numElements, gl.UNSIGNED_SHORT, 0);
  
    }


}


function initialize() {
    const engine = new Engine("viewer");

    const gl = engine.gl;

    const arrays = {
        position: [1, 1, -1, 1, 1, 1, 1, -1, 1, 1, -1, -1, -1, 1, 1, -1, 1, -1, -1, -1, -1, -1, -1, 1, -1, 1, 1, 1, 1, 1, 1, 1, -1, -1, 1, -1, -1, -1, -1, 1, -1, -1, 1, -1, 1, -1, -1, 1, 1, 1, 1, -1, 1, 1, -1, -1, 1, 1, -1, 1, -1, 1, -1, 1, 1, -1, 1, -1, -1, -1, -1, -1],
        normal:   [1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1],
        texcoord: [1, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1],
        indices:  [0, 1, 2, 0, 2, 3, 4, 5, 6, 4, 6, 7, 8, 9, 10, 8, 10, 11, 12, 13, 14, 12, 14, 15, 16, 17, 18, 16, 18, 19, 20, 21, 22, 20, 22, 23],

        instancePos: {
            numComponents: 4,
            data: [0,0,0,1, 1,1,1,1, 2,2,2,1],
            divisor: 1
        }
      };




    const bufferInfo = twgl.createBufferInfoFromArrays(gl, arrays);
   

    
    const tex = twgl.createTexture(gl, {
        min: gl.NEAREST,
        mag: gl.NEAREST,
        src: [
          255, 255, 255, 255,
          192, 192, 192, 255,
          192, 192, 192, 255,
          255, 255, 255, 255,
        ],
    });

    /*
    const uniforms = {
        u_specular: [1, 1, 1, 1],
        u_shininess: 50,
        u_specularFactor: 1,
        u_diffuse: tex,
    };
    */

    const camera = engine.camera;

    const material1 = engine.materialStore.createStandardMaterial(1,0.5,0.2);
    const material2 = engine.materialStore.createStandardMaterial(0,0.6,0.8);


    
    const vertexArrayInfo = twgl.createVertexArrayInfo(
        gl, 
        material1.program.pInfo, 
        bufferInfo);
    

    function render(time) {
        time *= 0.001;


        twgl.resizeCanvasToDisplaySize(gl.canvas);
        gl.viewport(0, 0, gl.canvas.width, gl.canvas.height);
  
        gl.enable(gl.DEPTH_TEST);
        gl.enable(gl.CULL_FACE);
        gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);
  
        camera.update();

        const m4 = twgl.m4;
        /*
        const projection = camera.projection;
        
        const eye = [1, 4, -10];
        const target = [0, 0, 0];
        const up = [0, 1, 0];
        

        const cameraMatrix = m4.lookAt(eye, target, up);
        const view = m4.inverse(cameraMatrix);
        const viewProjection = m4.multiply(projection, view);

        //--------------
        const uniforms = engine.uniforms;

        uniforms.u_viewInverse = cameraMatrix;
        */


        let world = m4.multiply(m4.translation([-1,0,0]), m4.rotationY(time));
  
        engine.setMaterial(material1);

        engine.setWorldMatrix(world);

        /*
        uniforms.u_world = world;
        uniforms.u_worldInverseTranspose = m4.transpose(m4.inverse(world));
        uniforms.u_worldViewProjection = m4.multiply(viewProjection, world);
        //--------------
 
        */

        // prog.enable(bufferInfo, uniforms);
        engine.setMaterial(material1);

        // engine.setBuffers(bufferInfo);
        twgl.setBuffersAndAttributes(engine.gl, engine.currentProgram.pInfo, vertexArrayInfo);


        twgl.drawBufferInfo(engine.gl, vertexArrayInfo, gl.TRIANGLES, 
            vertexArrayInfo.numelements, 0, 3);

        // gl.drawElements(gl.TRIANGLES, bufferInfo.numElements, gl.UNSIGNED_SHORT, 0);
/* 
        const uniforms2 = {};
        uniforms2.u_world = world;
        uniforms2.u_worldInverseTranspose = m4.transpose(m4.inverse(world));
        uniforms2.u_worldViewProjection = m4.multiply(viewProjection, world);
        */

        
        engine.setMaterial(material2);
        world = m4.multiply(m4.translation([1,0.3,0]), m4.rotationY(time)); 
        engine.setWorldMatrix(world);
        engine.setBuffers(bufferInfo);
        // twgl.setUniforms(engine.currentProgram.pInfo, uniforms2);
        gl.drawElements(gl.TRIANGLES, bufferInfo.numElements, gl.UNSIGNED_SHORT, 0);


        /*
        let pInfo = material1.prog.pInfo;
        gl.useProgram(pInfo.program);

        twgl.setBuffersAndAttributes(gl, pInfo, bufferInfo);

        Object.assign(uniforms, material.uniforms);
        twgl.setUniforms(pInfo, uniforms);

        
        */


  

        /*
        world = m4.multiply(m4.translation([ 1,0,0]), m4.rotationY(time));
  
        uniforms.u_world = world;
        uniforms.u_worldInverseTranspose = m4.transpose(m4.inverse(world));
        uniforms.u_worldViewProjection = m4.multiply(viewProjection, world);
  
        // prog.enable(bufferInfo, uniforms);
        pInfo = material.prog.pInfo;
        gl.useProgram(pInfo.program);

        twgl.setBuffersAndAttributes(gl, pInfo, bufferInfo);

        Object.assign(uniforms, material.uniforms);
        twgl.setUniforms(pInfo, uniforms);

        gl.drawElements(gl.TRIANGLES, bufferInfo.numElements, gl.UNSIGNED_SHORT, 0);
        */






        requestAnimationFrame(render);
      }
      requestAnimationFrame(render);
  
    
}

