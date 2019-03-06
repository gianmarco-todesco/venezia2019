"use strict";


var viewer;


document.addEventListener("DOMContentLoaded", function() {
    viewer = new GlViewer('viewer');
    viewer.createFpsMeter();


    
    var prog = viewer.createProgram(`
        attribute vec4 position;

        void main() {
        gl_Position = position;
        }
        `,`
        precision mediump float;

        uniform vec2 resolution;
        uniform float time;

        void main() {
        vec2 uv = gl_FragCoord.xy / resolution;
        float color = 0.0;
        // lifted from glslsandbox.com
        color += sin( uv.x * cos( time / 3.0 ) * 60.0 ) + cos( uv.y * cos( time / 2.80 ) * 10.0 );
        color += sin( uv.y * sin( time / 2.0 ) * 40.0 ) + cos( uv.x * sin( time / 1.70 ) * 40.0 );
        color += sin( uv.x * sin( time / 1.0 ) * 10.0 ) + sin( uv.y * sin( time / 3.50 ) * 80.0 );
        color *= sin( time / 10.0 ) * 0.5;

        gl_FragColor = vec4( vec3( color * 0.5, sin( color + time / 2.5 ) * 0.75, color ), 1.0 );
        }
        `
    );
        
    const bufferInfo = viewer.createShape({
        position: [0, -1, 0, 1, -1, 0, -1, 1, 0, -1, 1, 0, 1, -1, 0, 1, 1, 0],
    });



    
    var grid = Shapes.createGrid(viewer);
    console.log(grid);
    var lineProg = viewer.createProgram(`
        uniform mat4 u_worldViewProjection;
        attribute vec4 position;
        attribute vec3 color;
        varying vec3 v_color;
        void main() {
            gl_Position = u_worldViewProjection * position;
            v_color = color;
        }
    `,`
        precision mediump float;
        varying vec3 v_color;
        void main() {
            gl_FragColor = vec4(v_color, 1.0);
        }
    `);
    /*
    window.lineProg = lineProg;
    window.grid = grid;
    */


   var stripProg = viewer.createProgram(`
       uniform mat4 u_worldViewProjection;
       attribute vec4 position;
       void main() {
           gl_Position = u_worldViewProjection * position;
           
       }
   `,`
       precision mediump float;
       void main() {
           gl_FragColor = vec4(0.2,0.6,0.3, 1.0);
       }
   `);


    var hProg = viewer.createProgram(`
        uniform mat4 u_worldViewProjection;
        uniform mat4 hMatrix;
        attribute vec4 position;

        vec4 toBall(vec4 p) {  
            vec4 p2 = p * (1.0/p.w);
            float s2 = min(1.0, p2.x*p2.x + p2.y*p2.y + p2.z*p2.z);
            float k = 1.0 / (1.0 + sqrt(1.0 - s2));
            
            return vec4(p2.xyz*k,1.0);
        }

        void main() {
            gl_Position = u_worldViewProjection *  toBall(hMatrix * position);
            
        }
    `,`
        precision mediump float;
        void main() {
            gl_FragColor = vec4(0.2,0.6,0.3, 1.0);
        }
    `);


    const stripes = [];

    const sc = 0.8;
    const dod = regularPolyhedra.dodecahedron;
    dod.edges.forEach(e => {
        const v3 = twgl.v3;
        var p0 = v3.mulScalar(dod.vertices[e[0]], sc);
        var p1 = v3.mulScalar(dod.vertices[e[1]], sc);
        var p2 = v3.mulScalar(p1,0.99);
        var p3 = v3.mulScalar(p0,0.99);
        var strip = Shapes.createStrip(viewer, p0,p1,p2,p3, 100);
        stripes.push(strip);           
    });


    viewer.startRenderLoop((v)=>{


        /*
        const uniforms = {
            time: performance.now() * 0.001,
            resolution: [v.width, v.height],
            u_worldViewProjection: v.worldViewProjection,
        };
        const gl = v.gl;

        gl.useProgram(lineProg.pInfo.program);
        twgl.setBuffersAndAttributes(gl, lineProg.pInfo, grid);
        twgl.setUniforms(lineProg.pInfo, uniforms);
        twgl.drawBufferInfo(gl, grid, gl.LINES);
    */

        const gl = v.gl;

        
        gl.useProgram(lineProg.pInfo.program);
        twgl.setBuffersAndAttributes(gl, lineProg.pInfo, grid);
        twgl.setUniforms(lineProg.pInfo, { u_worldViewProjection: v.worldViewProjection });
        twgl.drawBufferInfo(gl, grid, gl.LINES);

        var prog = hProg;

        gl.useProgram(prog.pInfo.program);
        twgl.setUniforms(prog.pInfo, { 
            u_worldViewProjection: v.worldViewProjection,
            hMatrix: twgl.m4.identity()
        });
        stripes.forEach(strip => {
            twgl.setBuffersAndAttributes(gl, prog.pInfo, strip);
            twgl.drawBufferInfo(gl, strip, gl.LINES);
        });

        //var p = twgl.v3.mulScalar(dod.vertices[0], sc);
        //var hMatrix =  KPoint.reflection(new KPoint(p[0],p[1],p[2]));

        const m4 = twgl.m4;

        var p = twgl.v3.mulScalar(dod.centers[0], sc);
        var hMatrix = m4.rotateZ(twgl.m4.multiply(
            KPoint.reflection(new KPoint(0,0,0)),
            KPoint.reflection(new KPoint(p[0],p[1],p[2]))), Math.PI/5);

        stripes.forEach(strip => {
            twgl.setBuffersAndAttributes(gl, prog.pInfo, strip);
            for(var i=0; i<12; i++) {
                for(var j = 0; j<12; j++) {
                    twgl.setUniforms(prog.pInfo, { 
                        hMatrix : m4.multiply(m4.multiply(dod.rotations[i], hMatrix), m4.multiply(dod.rotations[j], hMatrix))
                    });
                    twgl.drawBufferInfo(gl, strip, gl.LINES);
                }
            }
        });


/*
        for(var i=0; i<12; i++) {

            const parentHMatrix = m4.multiply(dod.rotations[i], hMatrix);

            for(var j = 0; j<12; j++) {
                twgl.setUniforms(prog.pInfo, { 
                    hMatrix : m4.multiply(m4.multiply(dod.rotations[i], hMatrix), m4.multiply(dod.rotations[j], hMatrix))
                });
                stripes.forEach(strip => {
                    twgl.setBuffersAndAttributes(gl, prog.pInfo, strip);
                    twgl.drawBufferInfo(gl, strip, gl.LINES);
                });    
                twgl.setUniforms(prog.pInfo, { 
                    hMatrix : m4.multiply(dod.rotations[i], m4.multiply(hMatrix, hMatrix))
                });
                stripes.forEach(strip => {
                    twgl.setBuffersAndAttributes(gl, prog.pInfo, strip);
                    twgl.drawBufferInfo(gl, strip, gl.LINES);
                });    
    
            }
        }
        */


    });
});

