"use strict";

var viewer;


document.addEventListener("DOMContentLoaded", function() {
    viewer = new GlViewer('viewer');
    viewer.createFpsMeter();
    
    var prog = createStandardMaterial(viewer);
    
    /*
    const bufferInfo = viewer.createShape({
        position: [
          -1, -1, 0, 
           1, -1, 0, 
          -1,  1, 0, 
           1,  1, 0
        ],
        normal: [
            0,0,1,
            0,0,1,
            0,0,1,
            0,0,1
        ],
        indices: [
          0,1,3, 0,3,2
        ]
    });
    */

    


    var grid = Shapes.createGrid(viewer);
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

    const bufferInfo = createModel(viewer.gl);
    const vertexArrayInfo = twgl.createVertexArrayInfo(viewer.gl, prog.pInfo, bufferInfo);
 
    viewer.startRenderLoop((v)=>{

        const gl = v.gl;

        gl.useProgram(lineProg.pInfo.program);
        twgl.setBuffersAndAttributes(gl, lineProg.pInfo, grid);
        twgl.setUniforms(lineProg.pInfo, { u_worldViewProjection: v.worldViewProjection });
        twgl.drawBufferInfo(gl, grid, gl.LINES);


        gl.useProgram(prog.pInfo.program);
        twgl.setUniforms(prog.pInfo, { 
            u_worldViewProjection: v.worldViewProjection, 
            u_world : v.world,   
            u_viewInverse : v.camera,
            u_worldInverseTranspose : v.worldInverseTranspose,
            u_pan : [0,0,0]
        });


        //twgl.setBuffersAndAttributes(gl, prog.pInfo, vertexArrayInfo);
        //twgl.drawBufferInfo(gl, vertexArrayInfo);    
        /*

        var numInstances = 10;
        twgl.setBuffersAndAttributes(gl, prog.pInfo, vertexArrayInfo);
        gl.drawElementsInstanced(gl.TRIANGLES, 
            vertexArrayInfo.numElements, 
            gl.UNSIGNED_SHORT, 
            0, numInstances);

        */

        /*
        var i,j,k;
        var m = 8;
        for(i = -m; i<= m; i++) 
        for(j = -m; j<= m; j++) 
        for(k = -m; k<= m; k++)
        {
            twgl.setUniforms(prog.pInfo, { u_pan : [2*i,2*j,2*k] });
            twgl.drawBufferInfo(gl, bufferInfo);    
        } 
        */





    });
});




function createStandardMaterial(viewer) {
    var prog = viewer.createProgram(`
        uniform mat4 u_worldViewProjection;
        uniform mat4 u_world;
        uniform mat4 u_viewInverse;
        uniform mat4 u_worldInverseTranspose;
        uniform vec3 u_pan;
        
        attribute vec4 position;
        attribute vec3 normal;
        
        varying vec4 v_position;
        varying vec3 v_normal;
        varying vec3 v_surfaceToLight;
        varying vec3 v_surfaceToView;
        
        void main() {
            vec3 lightWorldPos = vec3(0.0,0.0,50.0);
            vec4 pp = position + vec4(u_pan, 1.0);

            v_position = u_worldViewProjection * pp;
            v_normal = (u_worldInverseTranspose * vec4(normal, 0)).xyz;
            v_surfaceToLight = lightWorldPos - (u_world * pp).xyz;
            v_surfaceToView = (u_viewInverse[3] - (u_world * pp)).xyz;
            gl_Position = v_position;
        }
    `,`
        precision mediump float;

        varying vec4 v_position;
        varying vec3 v_normal;
        varying vec3 v_surfaceToLight;
        varying vec3 v_surfaceToView;

        vec4 lit(float l ,float h, float m) {
            return vec4(1.0, max(l, 0.0), (l > 0.0) ? pow(max(0.0, h), m) : 0.0, 1.0);
        }

        void main() {
            vec3 diffuseColor = vec3(1.0,0.5,0.0);
            vec3 a_normal = normalize(v_normal);
            vec3 surfaceToLight = normalize(v_surfaceToLight);
            vec3 surfaceToView = normalize(v_surfaceToView);
            vec3 halfVector = normalize(surfaceToLight + surfaceToView);
            vec4 litR = lit(dot(a_normal, surfaceToLight),
                            dot(a_normal, halfVector), 120.0);
            vec3 color = diffuseColor * (litR.y + 0.1) +  vec3(0.4,0.4,0.7) * litR.z;
            //float att = 1.0/(1.0 + max(0.0, v_position.z));
            // gl_FragColor = vec4(color*att, 1.0);
            gl_FragColor = vec4(color, 1.0);
        }
        `
    );
    return prog;
}


class MeshBuilder {
    constructor(gl) {
        this.gl = gl;
        this.matrixStack = [twgl.m4.identity()];
        this.arrays = {
            position:[],
            normal:[],
            indices:[]
        };
        this.vCount = 0;
    }

    setNormal(x,y,z) {
        const topMatrix = this.matrixStack[this.matrixStack.length-1];
        this.normal = twgl.m4.transformNormal(topMatrix, twgl.v3.create(x,y,z));
    }
    addVertex(x,y,z) {
        const topMatrix = this.matrixStack[this.matrixStack.length-1];
        const v = twgl.m4.transformPoint(topMatrix, twgl.v3.create(x,y,z));
        this.arrays.position.push(v[0],v[1],v[2]);
        this.arrays.normal.push(this.normal[0],this.normal[1],this.normal[2]);
        const index = this.vCount++;
        return index;
    }
    addVertices(v) {
        for(var i=0; i+2<v.length; i+=3) this.addVertex(v[i],v[i+1],v[i+2]); 
    }

    
    pushMatrix() { 
        this.matrixStack.push(twgl.m4.copy(this.matrixStack[this.matrixStack.length-1]));
    }
    popMatrix() {
        this.matrixStack.pop();
    };

    rotateX(rad) {
        const j = this.matrixStack.length-1
        this.matrixStack[j] = twgl.m4.rotateX(this.matrixStack[j], rad); 
    }
    rotateY(rad) {
        const j = this.matrixStack.length-1
        this.matrixStack[j] = twgl.m4.rotateY(this.matrixStack[j], rad); 
    }
    rotateZ(rad) {
        const j = this.matrixStack.length-1
        this.matrixStack[j] = twgl.m4.rotateZ(this.matrixStack[j], rad); 
    }
    translate(x,y,z) { 
        const j = this.matrixStack.length-1
        this.matrixStack[j] = twgl.m4.translate(this.matrixStack[j], twgl.v3.create(x,y,z));
    }
    multmatrix(m) {
        const j = this.matrixStack.length-1
        this.matrixStack[j] = twgl.m4.multiply(this.matrixStack[j],m);
    }

    addTriangleIndices(a,b,c) { this.arrays.indices.push(a,b,c); }
    addQuadIndices(a,b,c,d) { 
        this.arrays.indices.push(a,b,c); 
        this.arrays.indices.push(a,c,d); 
    }


    addQuads(v) {
        for(var i=0; i+11<v.length; i+=12) {
            const k = this.vCount;
            for(var j=0;j<4;j++) this.addVertex(v[i+j*3], v[i+j*3+1], v[i+j*3+2]);
            this.addQuadIndices(k,k+1,k+2,k+3);
        }
    }
    
    createBuffer() {
        return twgl.createBufferInfoFromArrays(this.gl, this.arrays);
    }
}

function createModel(gl) {
    const mb = new MeshBuilder(gl);

    const a = 0.8, b = 1.0, c = 0.95;

    var i,j ;
    const m4 = twgl.m4;
    const faceMatrices = [
        m4.identity(),
        m4.rotationX(Math.PI),
        m4.rotationX(Math.PI/2),
        m4.rotationX(-Math.PI/2),
        m4.rotationY(Math.PI/2),
        m4.rotationY(-Math.PI/2),
    ];
    for(i=0;i<6;i++) {
        mb.pushMatrix();
        mb.multmatrix(faceMatrices[i]);
        mb.setNormal(0,0,1);
        mb.addQuads([
            -b,-b,-a, -a,-b,-a, -a,-a,-a, -b,-a,-a,
             a,-b,-a,  b,-b,-a,  b,-a,-a,  a,-a,-a,
            -b, a,-a, -a, a,-a, -a, b,-a, -b, b,-a,
             a, a,-a,  b, a,-a,  b, b,-a,  a, b,-a,
        ]);    
        mb.popMatrix();
    }

    const edgeMatrices = [
        m4.identity(),
        m4.rotationX(Math.PI/2),
        m4.rotationY(Math.PI/2),
    ]
    for(i=0;i<3;i++) {
        mb.pushMatrix();
        mb.multmatrix(edgeMatrices[i]);
        for(j=0; j<4; j++) {
            mb.pushMatrix();
            mb.rotateZ(Math.PI*0.5*j);
            mb.setNormal(1,0,0);
            mb.addQuads([
                -c,-b,-b, -c,-c,-b, -c,-c, b, -c,-b, b,
                -c, c,-b, -c, b,-b, -c, b, b, -c, c, b,
            ]);
            mb.popMatrix();
        }
        mb.popMatrix();    
    }

    


/*
    const numInstances = 100000;
    const instanceWorlds = new Float32Array(numInstances * 16);
  const instanceColors = [];
  const r = 100;
  for (let i = 0; i < numInstances; ++i) {
    const mat = new Float32Array(instanceWorlds.buffer, i * 16 * 4, 16);
    m4.translation([rand(-r, r), rand(-r, r), rand(-r, r)], mat);
    m4.rotateZ(mat, rand(0, Math.PI * 2), mat);
    m4.rotateX(mat, rand(0, Math.PI * 2), mat);
    instanceColors.push(rand(1), rand(1), rand(1));
  }
  const arrays = twgl.primitives.createCubeVertices();
  */
    var instanceOffsets = [];
    for(var i=0;i<10;i++) {
        instanceOffsets.push(i*2,0,0);
    }

    /*
    Object.assign(mb.arrays, {
        instanceOffset: {
            numComponents: 3,
            data: instanceOffsets,
            divisor: 1,
        },
    });
    */

    //mb.translate(0,1,0);
    //mb.rotateX(Math.PI/2);
        
    // mb.createBuffer();


    // return twgl.createBufferInfoFromArrays(this.gl, arrays);

    var bufferInfo = twgl.createBufferInfoFromArrays(gl, mb.arrays);
    return bufferInfo;

}