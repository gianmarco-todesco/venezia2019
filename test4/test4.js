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
    `,`
        precision mediump float;
        varying vec3 v_normal;
        void main() {
            vec3 normal = normalize(v_normal);
            float v =abs(normal.z);

            gl_FragColor = vec4(vec3(0.2,0.6,0.3) * v, 1.0);
        }
    `);


    const stripes = [];
    const v3 = twgl.v3;

    const sc = computeRadius();
    const dod = regularPolyhedra.dodecahedron;
    const pts = dod.vertices.map(v=>v3.mulScalar(v, sc));
    const centers = dod.centers.map(v=>v3.mulScalar(v, sc));


    const m4 = twgl.m4;

    var p = centers[0];
    var hMatrix = m4.rotateZ(twgl.m4.multiply(
        KPoint.reflection(new KPoint(0,0,0)),
        KPoint.reflection(new KPoint(p[0],p[1],p[2]))), Math.PI/5);

    const nodes = fooBar(dod, hMatrix);

    dod.edges.forEach(e => {
        var p0 = pts[e[0]];
        var p1 = pts[e[1]];

        var pa = centers[e[2]];
        var pb = centers[e[3]];

        var p2 = v3.lerp(p0,pa,0.1);
        var p3 = v3.lerp(p1,pa,0.1);
        var p4 = v3.lerp(p0,pb,0.1);
        var p5 = v3.lerp(p1,pb,0.1);
        
        
        //var p2 = v3.mulScalar(p1,0.99);
        //var p3 = v3.mulScalar(p0,0.99);
        //var strip = Shapes.createStrip(viewer, p0,p1,p2,p3, 100);
        var strip = createHStrip(viewer.gl, p2,p3,p5,p4, 30);
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

        // draw axes
        gl.useProgram(lineProg.pInfo.program);
        twgl.setBuffersAndAttributes(gl, lineProg.pInfo, grid);
        twgl.setUniforms(lineProg.pInfo, { 
            u_worldViewProjection: v.worldViewProjection 
        });
        twgl.drawBufferInfo(gl, grid, gl.LINES);


        // draw stripes
        var prog = hProg;
        gl.useProgram(prog.pInfo.program);
        twgl.setUniforms(prog.pInfo, { 
            u_worldViewProjection: v.worldViewProjection,
            u_worldInverseTranspose  : v.worldInverseTranspose,
            u_world : v.world,            
            hMatrix: twgl.m4.identity()
        });


        var nodes2 = [];
        const zero = twgl.v3.create(0,0,0);
        nodes.forEach(node => {
            var mat = m4.multiply(v.worldViewProjection, node.mat);
            var p = m4.transformPoint(mat, zero);
            if(p[2]>0) nodes2.push(node);
        });
        

        stripes.forEach(strip => {
            twgl.setBuffersAndAttributes(gl, prog.pInfo, strip);
            nodes2.forEach(node => {
                twgl.setUniforms(prog.pInfo, { 
                    hMatrix : node.mat
                });
                twgl.drawBufferInfo(gl, strip, gl.TRIANGLE_STRIP);
            })
        });


        /*
        var prog = hProg;
        gl.useProgram(prog.pInfo.program);
        twgl.setUniforms(prog.pInfo, { 
            u_worldViewProjection: v.worldViewProjection,
            u_worldInverseTranspose  : v.worldInverseTranspose,
            u_world : v.world,
            
            hMatrix: twgl.m4.identity()
        });
        stripes.forEach(strip => {
            twgl.setBuffersAndAttributes(gl, prog.pInfo, strip);
            twgl.drawBufferInfo(gl, strip, gl.TRIANGLE_STRIP);
        });
        */


        //var p = twgl.v3.mulScalar(dod.vertices[0], sc);
        //var hMatrix =  KPoint.reflection(new KPoint(p[0],p[1],p[2]));

        /*

        stripes.forEach(strip => {
            twgl.setBuffersAndAttributes(gl, prog.pInfo, strip);

            
            for(var i=0; i<12; i++) {
                twgl.setUniforms(prog.pInfo, { 
                    hMatrix : m4.multiply(dod.rotations[i], hMatrix)
                });
                twgl.drawBufferInfo(gl, strip, gl.TRIANGLE_STRIP);
            }
        });
    
        stripes.forEach(strip => {
            twgl.setBuffersAndAttributes(gl, prog.pInfo, strip);

            
            for(var i=0; i<12; i++) {
                for(var j = 0; j<12; j++) {
                    twgl.setUniforms(prog.pInfo, { 
                        hMatrix : m4.multiply(m4.multiply(dod.rotations[i], hMatrix), m4.multiply(dod.rotations[j], hMatrix))
                    });
                    twgl.drawBufferInfo(gl, strip, gl.TRIANGLE_STRIP);
                }
            }
        });

        */

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

function binarySearch(arr, acc, v) {
    if(arr.length==0) return 0;
    var a=0, b=arr.length-1;
    if(v<=acc(arr)) return 0; 
    else if(v>=acc(arr[b])) return b+1;
    while(b-a>1) {
        var c = (a+b)/2 | 0;
        if(acc(arr[c])<=v) a=c;
        else b=c;
    }
    // arr[a]<=v<arr[b]
    return b;
}

function shrinkingFactor(mat) {
    const v3 = twgl.v3;
    const m4 = twgl.m4;
    const r = 0.1;
    var d = 0;
    d += v3.distance(
        m4.transformPoint(mat, v3.create(r,0,0)),
        m4.transformPoint(mat, v3.create(-r,0,0)));
    d += v3.distance(
        m4.transformPoint(mat, v3.create(0,r,0)),
        m4.transformPoint(mat, v3.create(0,-r,0)));
    d += v3.distance(
        m4.transformPoint(mat, v3.create(0,0,r)),
        m4.transformPoint(mat, v3.create(0,0,-r)));
    return d;
}

function fooBar(dod, hMatrix) {
    const startTime = performance.now();
    const v3 = twgl.v3;
    const m4 = twgl.m4;
    
    const mats = dod.rotations.map(r=>m4.multiply(r, hMatrix));
    const zero = v3.create(0,0,0);
    
    const nodes = [
        {
            r:0.0, 
            center: zero,
            mat: m4.identity(), 
            id: 0, 
            links: []
        }
    ];

    var todo = [nodes[0]];
    var nextTodo = [];
    const rEps = 0.001;

    const rMax = 0.999999;
    var actualRMax = 0;

    var distances = window.distances = [];

    var count = 0;
    while(todo.length>0 && ++count<=5) {
        todo.forEach((parentNode, parentNodeIndex) => {
            mats.forEach((nextMat, nextMatIndex) => {
                const mat = m4.multiply(parentNode.mat, nextMat);
                const p = m4.transformPoint(mat, zero);
                const r = v3.length(p);
                if(r>rMax) {
                    window.wish = r;
                    return;                    
                } 
                if(shrinkingFactor(mat)<0.005) return;
                var j0 = binarySearch(nodes, node=>node.r, r);
                var j = j0;
                var found = null;
                var minDist = 0;
                while(j-1>=0 && r-nodes[j-1].r<rEps) j--;
                while(j<nodes.length && nodes[j].r-r<rEps) {
                    const d = v3.distance(nodes[j].center, p);
                    if(found == null || d<minDist) { found = nodes[j]; minDist = d; }
                    j++;
                }
                distances.push(minDist);
                // console.log("dist = ", minDist);
                if(minDist>1.0e-4) found = null;
                if(found) {
    
                } else {
                    const node = {
                        r,
                        center : p,
                        mat,
                        id : nodes.length,
                        links : []
                    };
                    if(r>actualRMax) actualRMax = r;
                    nodes.splice(j0, 0, node);
                    nextTodo.push(node);
                }
            });
        });
        todo = nextTodo;    
    }
    

    console.log(nodes.length);
    console.log("rmax = ", actualRMax);
    window.nodes = nodes;

    console.log("time=", performance.now() - startTime);
    return nodes;
}

function createHStrip(gl, p0,p1,p2,p3, m) {
    const v3 = twgl.v3;
    var arrays = {
      position: [],
      position_du: { numComponents:3, data: []},
      position_dv: { numComponents:3, data: []},
            
    };
    const du = v3.mulScalar(v3.normalize(v3.subtract(p1,p0)),0.01);
    const dv = v3.mulScalar(v3.normalize(v3.subtract(p3,p0)),0.01);
    
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
        var p = v3.create();
        v3.add(p01, du, p); arrays.position_du.data.push(...p);
        v3.add(p01, dv, p); arrays.position_dv.data.push(...p);
        v3.add(p32, du, p); arrays.position_du.data.push(...p);
        v3.add(p32, dv, p); arrays.position_dv.data.push(...p);        
    }
    return twgl.createBufferInfoFromArrays(gl, arrays);

}


function computeRadius() {
    const v3 = twgl.v3;

    var m = 4; // voglio 4 dodecaedri attorno ad ogni spigolo
    const dod = regularPolyhedra.dodecahedron;
    const phi = Math.acos(v3.dot(
        v3.normalize(dod.centers[0]),
        v3.normalize(dod.centers[1])));
    // phi è il supplementare dell'angolo diedro del dod
    
    const theta = Math.acos(v3.dot(
        v3.normalize(dod.vertices[0]),
        v3.normalize(dod.vertices[1])));
    // theta = angolo sotteso da un lato
    
    const d1 = Math.acosh(1.0/(Math.tan(phi*0.5)*Math.tan(Math.PI/m)));
    // distanza (iperbolica) fra il centro e il punto medio di uno spigolo
    // uso : cosh(hyp) = cot(A)*cot(B)
    // console.log(d1);

    const d2 = Math.atanh(Math.tanh(d1)/Math.cos(theta/2));
    // distanza (iperbolica) fra il centro e un vertice
    // uso: cos(A) = tanh(b)/tanh(c)
    // console.log(d2);

    
    const tmp = Math.exp(2*d2);
    const d3 = (tmp-1)/(tmp+1);
    // distanza (euclidea) dal centro di un punto nel modello di Klein
    // che abbia d2 come distanza iperbolica.
    // uso la formula per la dist. hyp. del modello di klein: 
    // d=1/2log((aq*pb)/(ap*qb)), con d=d2, ap=pb=1, pq=d3, qb=1-d3
    // console.log("d3=",d3);

    return d3;
}