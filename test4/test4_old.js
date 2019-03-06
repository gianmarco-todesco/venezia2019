"use strict";



twgl.setDefaults({attribPrefix: "a_"});


function createMeter() {

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
    return meter;    
}

function step(t, a,b) { return t<a?0.0:t>b?1.0:(t-a)/(b-a); }
function smooth(t) { return t*t*(3-2*t); }
function smoothstep(t, a,b) { return smooth(step(t,a,b)); }

var canvas;
var gl;
var grid;
var m4 = twgl.m4;
var v3 = twgl.v3;
var gl;

var camera;
var view,viewProjection,distance;
var uniforms;
var running = true;
var phi, theta;


document.addEventListener("DOMContentLoaded", function() {
    canvas = document.getElementById('viewer');
    gl = twgl.getWebGLContext(canvas, {preserveDrawingBuffer   : true});
    grid = makeGrid(gl);


    var buttonDown = false;
    var theta0 = 0.2;
    var theta1 = Math.PI - theta0;
    theta = 0.7;
    phi = 3.71;

    gl.canvas.addEventListener('mousedown', e => { onMouseDown(e); }, false);
    gl.canvas.addEventListener('mousewheel', function(e) {
    });


        /*
    gl.canvas.addEventListener('mousewheel', function(e) {
        e.stopPropagation();
        e.preventDefault();
        _this.distance = Math.max(5, _this.distance - e.wheelDelta *0.01);
    }, false);
*/



    camera = m4.identity();
    view = m4.identity();
    viewProjection = m4.identity();
    distance = 6.5;

    uniforms = {
        u_lightWorldPos: [1, 8, 10],
        u_lightColor: [1, 1, 1, 1],
        u_diffuseMult: [0.5,0.3,0.8,1],
        u_specular: [1, 1, 1, 1],
        u_shininess: 120,
        u_specularFactor: 1,
        u_viewInverse: camera,
        u_world: m4.identity(),
        u_worldInverseTranspose: m4.identity(),
        u_worldViewProjection: m4.identity(),
        u_time: 0,
        u_cc: [0,0,0,0,0,0,0,0,0,0]
    };

/*
var quad = this.makeQuad(this.gl);
var bgProg = new BackgroundShaderProgram(gl);
this.bg = {
    prog: bgProg,
    type: gl.TRIANGLES,
    shape : quad,
};
this.objects = [obj];

});
*/

    render();
});



function drawScene() {
    twgl.resizeCanvasToDisplaySize(gl.canvas);

    gl.viewport(0, 0, gl.canvas.width, gl.canvas.height);
    gl.enable(gl.DEPTH_TEST);
    // gl.enable(gl.CULL_FACE);
    gl.clearColor(.1,0,.1,1);
    gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);

    var eye = v3.copy([0, 0, distance]);
    var target = v3.copy([0, 0, 0]);
    var up = [0, 1, 0];
    var tex = twgl.createTexture(gl, {
      min: gl.NEAREST,
      mag: gl.NEAREST,
      src: [
        255, 255, 255, 255,
        192, 192, 192, 255,
        192, 192, 192, 255,
        255, 255, 255, 255,
      ],
    });

    // set viewProjection
    var fovy = 30 * Math.PI / 180;
    var projection = m4.perspective(
        fovy,
        gl.canvas.clientWidth / gl.canvas.clientHeight,
        0.5, 200);
    m4.lookAt(eye, target, up, camera);
    m4.inverse(camera, view);
    m4.multiply(projection, view, viewProjection);


    var uni = uniforms;

    var world = uni.u_world;
    m4.identity(world);
    //m4.translate(world, obj.translation, world);
    m4.rotateX(world, -theta, world);
    m4.rotateZ(world, phi, world);

    m4.transpose(
       m4.inverse(world, uni.u_worldInverseTranspose), uni.u_worldInverseTranspose);
    m4.multiply(viewProjection, uni.u_world, uni.u_worldViewProjection);

    
    /*
    if(this.objects) {
        var me = this;
        this.objects.forEach(function(obj) {
            var pi = obj.prog.pInfo;
            gl.useProgram(pi.program);
            twgl.setBuffersAndAttributes(gl, pi, obj.shape);
            me.uniforms.u_time = time;
            twgl.setUniforms(pi, me.uniforms);
            twgl.drawBufferInfo(gl, obj.shape, obj.type);            
        });
    }
    */

    /*

    var pi = this.programManager.linesProgram;
    gl.useProgram(pi.program);
    twgl.setBuffersAndAttributes(gl, pi, this.grid);
    twgl.setUniforms(pi, this.uniforms);
    twgl.drawBufferInfo(gl, this.grid, gl.LINES);

    if(this.bg) {
        var bg = this.bg;
        var pi = bg.prog.pInfo;
        gl.useProgram(pi.program);
        var uniforms = {
            u_worldViewProjection: m4.identity(),
        };
        var m = uniforms.u_worldViewProjection;
        m4.translate(m,uff,m);
        twgl.setBuffersAndAttributes(gl, pi, bg.shape);
        twgl.setUniforms(pi, uniforms);
        twgl.drawBufferInfo(gl, bg.shape, bg.type);            
    }
    */

}

function render() {
    if(window.meter) meter.tickStart();
    if(gl.NO_ERROR != gl.getError()) throw "uff";
    drawScene();
    if(gl.NO_ERROR != gl.getError()) throw "uff";
    if(window.meter) meter.tick();
    if(running) requestAnimationFrame(render);
}

 

function makeGrid() {
    var m = 9;
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
