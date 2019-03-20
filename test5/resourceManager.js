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

