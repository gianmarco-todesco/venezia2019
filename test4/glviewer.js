"use strict";


class GlViewer {
    constructor(canvasId) {
        this.canvas = document.getElementById('viewer');
        const gl = this.gl = this.canvas.getContext("webgl");
        const m4 = twgl.m4;
        this.distance = 10.0;
        this.camera = m4.identity();
        this.view = m4.identity();
        this.viewProjection = m4.identity();
        this.worldViewProjection = m4.identity();
        this.world = m4.identity();
        this.worldInverseTranspose = m4.identity();
        this.theta = 0.7;
        this.phi = 0.0;

        this.mouseHandler = new MouseHandler(this);
        gl.enable(gl.DEPTH_TEST);
        gl.clearColor(.1,0,.1,1);
    }

    createFpsMeter() {
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
        this.meter = meter;
        return meter;    
    }

    createProgram(vSrc, fSrc) {
        return new ShaderProgram(this.gl, vSrc, fSrc);
    }

    createShape(arrays) {
        return twgl.createBufferInfoFromArrays(this.gl, arrays);
        
    }

    setCamera() {
        const v3 = twgl.v3;
        const m4 = twgl.m4;        
        const eye = v3.copy([0, 0, this.distance]);
        const target = v3.copy([0, 0, 0]);
        const up = [0, 1, 0];
        const fovy = 30 * Math.PI / 180;
        const projection = m4.perspective(
            fovy,
            this.width / this.height,
            0.5, 200);
        m4.lookAt(eye, target, up, this.camera);
        m4.inverse(this.camera, this.view);
        m4.multiply(projection, this.view, this.viewProjection);
        
        const world = m4.identity();
        m4.rotateX(world, -this.theta, world);
        m4.rotateZ(world, this.phi, world);
        this.world = world;
    
        //m4.transpose(
        //   m4.inverse(world, this.worldInverseTranspose), uni.u_worldInverseTranspose);

        m4.multiply(this.viewProjection, this.world, this.worldViewProjection);    
    }



    

    startRenderLoop(renderFunction) {
        if(this.running) throw "already rendering";
        this.running = true;
        const me = this;
        const gl = this.gl;
        function render() {
            if(!me.running) return;
            if(me.meter) me.meter.tickStart();
            if(gl.NO_ERROR != gl.getError()) throw "OpenGL errors!";
            twgl.resizeCanvasToDisplaySize(gl.canvas);
            me.width = gl.canvas.width;
            me.height = gl.canvas.height;
            
            gl.viewport(0, 0, me.width, me.height);
            // gl.enable(gl.CULL_FACE);
            gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);        
            me.setCamera();

            renderFunction(me);

            if(gl.NO_ERROR != gl.getError()) throw "OpenGL errors!";
            if(me.meter) me.meter.tick();
            requestAnimationFrame(render);
        }
        requestAnimationFrame(render);
    }

    stopRenderLoop() {
        this.running = false;
    }    
}





function ShaderProgram(gl, vSrc, fSrc) { 
    this.gl = gl;
    this.vSrc = vSrc;
    this.fSrc = fSrc;
    this.pInfo = this._createProgramInfoFromSources(vSrc, fSrc);        
}


ShaderProgram.prototype._createShaderFromSource = function (type, src) {
    var gl = this.gl;
    var shader = gl.createShader(type);
    gl.shaderSource(shader, src);
    gl.compileShader(shader);
    var compiled = gl.getShaderParameter(shader, gl.COMPILE_STATUS);
    if (!compiled) {
      // Something went wrong during compilation; get the error
      var lastError = gl.getShaderInfoLog(shader);
      gl.deleteShader(shader);
      throw lastError;
    }
    return shader;
}    

ShaderProgram.prototype._createProgramInfoFromSources = function (vSrc, fSrc) {
    var gl = this.gl;
    var vShader = this._createShaderFromSource(gl.VERTEX_SHADER, vSrc);        
    var fShader = this._createShaderFromSource(gl.FRAGMENT_SHADER, fSrc);
    var program = gl.createProgram();
    gl.attachShader(program, vShader);
    gl.attachShader(program, fShader);
    gl.linkProgram(program);
    if ( !gl.getProgramParameter( program, gl.LINK_STATUS) ) {
        var info = gl.getProgramInfoLog(program);
        throw 'Could not compile WebGL program. \n\n' + info;
    }
    var programInfo = twgl.createProgramInfoFromProgram(gl, program);
    return programInfo;    
}    



const Shapes = {};

Shapes.createGridOld = function(v) {
    const gl = v.gl;
    // var m = 9;
    var arrays = {
      position: [], // twgl.primitives.createAugmentedTypedArray(3, m*2),
      color: [], // twgl.primitives.createAugmentedTypedArray(3, m*2),
    };

    var addLine = function (x0,y0,z0, x1,y1,z1, r,g,b) {
        arrays.position.push(x0,y0,z0, x1,y1,z1);
        arrays.color.push(r,g,b,1.0,r,g,b,1.0);
    }

    var r = 2.0;
    
    addLine(-r,0,0, r,0,0, 1,0,0);
    addLine(0,-r,0, 0,r,0, 0,1,0);
    addLine(0,0,-r, 0,0,r, 0,0,1);

    var d = 0.1;
    addLine(r,0,0, r-d, d,0, 1,0,0);
    addLine(r,0,0, r-d,-d,0, 1,0,0);

    addLine(0,r,0,  d,r-d,0, 0,1,0);
    addLine(0,r,0, -d,r-d,0, 0,1,0);

    addLine(0,0,r, 0, d,r-d, 0,0,1);
    addLine(0,0,r, 0,-d,r-d, 0,0,1);

    r = 1.8;
    var n = 21;
    for(var i = 0; i<n; i++) {   
        var v = (i%5)==0 ? 0.7 : 0.9;
        var t = i/(n-1);
        var q = r*(-1+2*t);
        addLine(-r,q,0, r,q,0, v,v,v);
        addLine(q,-r,0, q,r,0, v,v,v);        
    }

    var bufferInfo = twgl.createBufferInfoFromArrays(gl, arrays);
    return bufferInfo;    
}

Shapes.createGrid = function(v) {
    const gl = v.gl;
    // var m = 9;
    var arrays = {
      position: [], // twgl.primitives.createAugmentedTypedArray(3, m*2),
      color: [], // twgl.primitives.createAugmentedTypedArray(3, m*2),
    };

    var addLine = function (x0,y0,z0, x1,y1,z1, r,g,b) {
        arrays.position.push(x0,y0,z0, x1,y1,z1);
        arrays.color.push(r,g,b,1.0,r,g,b,1.0);
    }

    var r = 2.0;
    
    addLine(-r,0,0, r,0,0, 1,0,0);
    addLine(0,-r,0, 0,r,0, 0,1,0);
    addLine(0,0,-r, 0,0,r, 0,0,1);

    var d = 0.1;
    addLine(r,0,0, r-d, d,0, 1,0,0);
    addLine(r,0,0, r-d,-d,0, 1,0,0);

    addLine(0,r,0,  d,r-d,0, 0,1,0);
    addLine(0,r,0, -d,r-d,0, 0,1,0);

    addLine(0,0,r, 0, d,r-d, 0,0,1);
    addLine(0,0,r, 0,-d,r-d, 0,0,1);


    r = 1.0;
    var m = 100;
    const color = [0.2,0.6,0.8];
    for(var i = 0; i<m; i++) {
        var phi0 = Math.PI*2*i/m;
        var phi1 = Math.PI*2*(i+1)/m;
        var cs0 = Math.cos(phi0);
        var sn0 = Math.sin(phi0);
        var cs1 = Math.cos(phi1);
        var sn1 = Math.sin(phi1);

        addLine(cs0,sn0,0,cs1,sn1,0, color[0],color[1],color[2]);
        addLine(cs0,0,sn0,cs1,0,sn1, color[0],color[1],color[2]);
        addLine(0,cs0,sn0,0,cs1,sn1, color[0],color[1],color[2]);
    }

    var bufferInfo = twgl.createBufferInfoFromArrays(gl, arrays);
    return bufferInfo;    
}



Shapes.createStrip = function(v,p0,p1,p2,p3,m) {
    const gl = v.gl;
    const v3 = twgl.v3;
    var arrays = {
      position: []      
    };
    var i;
    const p01 = v3.create();
    const p32 = v3.create();
    
    for(i=0; i<m; i++) {
        const t = i/(m-1);
        v3.lerp(p0,p1,t,p01);
        v3.lerp(p3,p2,t,p32);
        arrays.position.push(
            p01[0], p01[1], p01[2],
            p32[0], p32[1], p32[2]
        );
    }
    return twgl.createBufferInfoFromArrays(gl, arrays);
}

class MouseHandler {

    constructor(viewer) {
        this.viewer = viewer;
        const canvas = this.viewer.canvas;
        const me = this;
        canvas.addEventListener('mousedown', e=>{me.onMouseDown(e);}, false);
        this.pressed = false;

        canvas.addEventListener('mousewheel', function(e) {
            e.stopPropagation();
            e.preventDefault();
            me.viewer.distance = Math.max(2, me.viewer.distance - e.wheelDelta *0.005);
        }, false);

    }

    getMousePos(e) {
        return {x:e.offsetX, y:e.offsetY};
    }
    onMouseDown(e) {
        this.lastPos = this.getMousePos(e);
        this.pressed = true;
        const me = this;
        const onMouseDrag = e => me.onMouseDrag(e);
        const onMouseUp = e => {
            document.removeEventListener('mousemove', onMouseDrag, false);
            document.removeEventListener('mouseup', onMouseUp, false);  
            me.pressed = false;
            me.onMouseUp(e);
        }
        document.addEventListener('mousemove', onMouseDrag, false);
        document.addEventListener('mouseup', onMouseUp, false);
    }
    onMouseDrag(e) {
        const pos = this.getMousePos(e);
        const dx = pos.x - this.lastPos.x;
        const dy = -(pos.y - this.lastPos.y);
        this.viewer.phi += dx *0.01;
        this.viewer.theta += dy *0.01;        
        this.lastPos.x = pos.x;
        this.lastPos.y = pos.y;
    }
    onMouseUp(e) {

    }


}