

class Camera {
    constructor(canvas) {
        this.canvas = canvas;
        this.fov = 30 * Math.PI / 180;
        this.zNear = 0.1;
        this.zFar = 500;
        this.update();
      }

    update() {
        const m4 = twgl.m4;
        const aspect = this.canvas.clientWidth / this.canvas.clientHeight;
        const projection = m4.perspective(this.fov, aspect, this.zNear, this.zFar);
        this.projection = projection;

        const eye = [0, 0, 10];
        const target = [0, 0, 0];
        const up = [0, 1, 0];
        this.cameraMatrix = twgl.m4.lookAt(eye, target, up);
        
    }

}

function createFpsMeter() {
    var meter = new FPSMeter(null, {
        interval:100,
        smoothing:10,
        show: 'fps',
        decimals: 1,
        maxFps: 60,
        threshold: 100,
        
        position: 'absolute',
        zIndex: 10,
        left: '20px',
        top: '20px',
        theme: 'dark',
        heat: 1,
        graph: 1,
        history: 20
    });
    // this.meter = meter;
    return meter;    
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

        this.resourceManager = new ResourceManager(gl);

        this.uniforms = {
            u_lightWorldPos: [1, 8, 10],
            u_lightColor: [1, 0.8, 0.8, 1],
            u_ambient: [0.2, 0.2, 0.2, 1],
            u_specularFactor: 0.5,   
            u_shininess: 120.0, 
        };

        this.grid = createGrid(gl);
        this.lineMaterial = new Material(this.resourceManager.getShaderProgram("lines"));
    }


    setMaterial(material) {
        if(this.currentProgram != material.program) {
            this.currentProgram = material.program;
            this.gl.useProgram(this.currentProgram.pInfo.program);
        }
        this.currentMaterial = material;
        this.gl.useProgram(this.currentProgram.pInfo.program);
        Object.assign(this.uniforms, material.uniforms);
        twgl.setUniforms(this.currentProgram.pInfo, this.uniforms);
    }

    setBuffers(bufferInfo) {
        this.bufferInfo = bufferInfo;
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

    setUniforms(uniforms) {
        twgl.setUniforms(this.currentProgram.pInfo, uniforms);
    }



    draw() {
        twgl.drawBufferInfo(this.gl, this.bufferInfo, this.gl.TRIANGLES);
    }

    useProgram(program) {
        this.currentProgram = program;
        gl.useProgram(prog.pInfo.program);
        twgl.setBuffersAndAttributes(gl, prog.pInfo, bufferInfo);
        twgl.setUniforms(prog.pInfo, uniforms);
        gl.drawElements(gl.TRIANGLES, bufferInfo.numElements, gl.UNSIGNED_SHORT, 0);
  
    }

    drawGrid(word) {
        this.setMaterial(this.lineMaterial);
        this.setBuffers(this.grid);
        this.setWorldMatrix(word);
        twgl.drawBufferInfo(this.gl, this.grid, this.gl.LINES);
    }

    drawAxes(world) { this.drawGrid(world); }
    
}




