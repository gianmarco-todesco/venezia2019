

SlideManager.slides["slide1"] = {
    initialize : function(engine) {
        this.material = engine.resourceManager.createStandardMaterial(0.8,0.2,0.1);
    },
    draw : function(engine) {
        const m4 = twgl.m4;
        engine.setMaterial(this.material);
        const ph = regularPolyhedra.tetrahedron;
        const r = PolygonEdge * ph.inRadius / ph.edgeLength;

        /*
        engine.setWorldMatrix(m4.identity()); 
        const sphereBufferInfo = engine.resourceManager.getGeometry("sphere");
        engine.setBuffers(sphereBufferInfo);
        
        twgl.drawBufferInfo(
            engine.gl, 
            sphereBufferInfo, 
            engine.gl.TRIANGLES, 
            sphereBufferInfo.numElements);

        engine.setWorldMatrix(m4.translation([0,2,0])); 
        
        twgl.drawBufferInfo(
            engine.gl, 
            sphereBufferInfo, 
            engine.gl.TRIANGLES, 
            sphereBufferInfo.numElements);
        */

        const bufferInfo = engine.resourceManager.getGeometry("triangle");
        engine.setBuffers(bufferInfo);        
        
        ph.rotations.forEach(rot => {
            let world = m4.translation([0,0,r]);
            world = m4.multiply(rot, world);
            world = m4.multiply(m4.rotationY(performance.now()*0.001), world);
            engine.setWorldMatrix(world);    
            engine.gl.drawElements(
                engine.gl.TRIANGLES, 
                bufferInfo.numElements, 
                engine.gl.UNSIGNED_SHORT, 
                0);
            /*
                engine.setBuffers(this.vertexArrayInfo);
                twgl.drawBufferInfo(engine.gl, this.vertexArrayInfo, engine.gl.TRIANGLES, 
                this.vertexArrayInfo.numelements, 0, 3);
                // twgl.drawBufferInfo(engine.gl, bufferInfo2, engine.gl.TRIANGLES);
            */
        });
    },
};

SlideManager.slides["slide2"] = {
    initialize : function(engine) {
        this.material = engine.resourceManager.createStandardMaterial(0.2,0.8,0.6);
    },
    draw : function(engine) {
        const m4 = twgl.m4;
        engine.setMaterial(this.material);
        const bufferInfo = engine.resourceManager.getGeometry("square");
        engine.setBuffers(bufferInfo);        
        const ph = regularPolyhedra.cube;
        const r = PolygonEdge * ph.inRadius  / ph.edgeLength;
        ph.rotations.forEach(rot => {
            let world = m4.translation([0,0,r]);
            world = m4.multiply(rot, world);
            world = m4.multiply(m4.rotationY(performance.now()*0.001), world);
            engine.setWorldMatrix(world);    
            engine.gl.drawElements(
                engine.gl.TRIANGLES, 
                bufferInfo.numElements, 
                engine.gl.UNSIGNED_SHORT, 
                0);
        });
    },
};

SlideManager.slides["slide3"] = {
    initialize : function(engine) {
        this.material = engine.resourceManager.createStandardMaterial(0.2,0.6,0.8);
    },
    draw : function(engine) {
        const m4 = twgl.m4;
        engine.setMaterial(this.material);
        const bufferInfo = engine.resourceManager.getGeometry("triangle");
        engine.setBuffers(bufferInfo);        
        const ph = regularPolyhedra.octahedron;
        const r = PolygonEdge * ph.inRadius / ph.edgeLength;
        ph.rotations.forEach(rot => {
            let world = m4.translation([0,0,r]);
            world = m4.multiply(rot, world);
            world = m4.multiply(m4.rotationY(performance.now()*0.001), world);
            engine.setWorldMatrix(world);    
            engine.gl.drawElements(
                engine.gl.TRIANGLES, 
                bufferInfo.numElements, 
                engine.gl.UNSIGNED_SHORT, 
                0);
        });
    },
};

SlideManager.slides["slide4"] = {
    initialize : function(engine) {
        this.material = engine.resourceManager.createStandardMaterial(0.8,0.2,0.9);
    },
    draw : function(engine) {

        const m4 = twgl.m4;

        engine.setMaterial(this.material);
        const bufferInfo = engine.resourceManager.getGeometry("pentagon");
        engine.setBuffers(bufferInfo);
        
        const ph = regularPolyhedra.dodecahedron;
        const r = PolygonEdge * ph.inRadius / ph.edgeLength;
        ph.rotations.forEach(rot => {
            let world = m4.translation([0,0,r]);
            world = m4.multiply(rot, world);
            world = m4.multiply(m4.rotationY(performance.now()*0.001), world);
            engine.setWorldMatrix(world);    
            engine.gl.drawElements(
                engine.gl.TRIANGLES, 
                bufferInfo.numElements, 
                engine.gl.UNSIGNED_SHORT, 
                0);
        });
    },
};

SlideManager.slides["slide5"] = {
    initialize : function(engine) {
        this.material = engine.resourceManager.createStandardMaterial(0.1,0.8,0.9);
    },
    draw : function(engine) {

        const m4 = twgl.m4;

        engine.setMaterial(this.material);
        const bufferInfo = engine.resourceManager.getGeometry("triangle");
        engine.setBuffers(bufferInfo);
        
        const ph = regularPolyhedra.icosahedron;
        const r = PolygonEdge * ph.inRadius / ph.edgeLength;
        ph.rotations.forEach(rot => {
            let world = m4.translation([0,0,r]);
            world = m4.multiply(rot, world);
            world = m4.multiply(m4.rotationY(performance.now()*0.001), world);
            engine.setWorldMatrix(world);    
            engine.gl.drawElements(
                engine.gl.TRIANGLES, 
                bufferInfo.numElements, 
                engine.gl.UNSIGNED_SHORT, 
                0);
        });
    },
};

SlideManager.slides["cube-grid"] = {
    initialize : function(engine) {

        const material = new Material(engine.resourceManager.getShaderProgram("instanced"));
        material.uniforms.u_diffuseColor = [0.7,0.7,0.7,1.0];        
        this.material = material;
    },
    draw : function(engine) {

        const m4 = twgl.m4;

        engine.setMaterial(this.material);
        const bufferInfo = engine.resourceManager.getGeometry("cube-grid-cell");


        engine.setBuffers(bufferInfo);
        
        let world = m4.rotationY(performance.now()*0.0001);
        world = m4.multiply(m4.rotationX(0.01), world);


        
        engine.setWorldMatrix(world);    
        /*
        engine.gl.drawElements(
            engine.gl.TRIANGLES, 
            bufferInfo.numElements, 
            engine.gl.UNSIGNED_SHORT, 
            0);
        */
        const m = 7;
        const instanceCount = 1+m*(6+m*(6+8*m));
        twgl.drawBufferInfo(engine.gl, bufferInfo, engine.gl.TRIANGLES, 
           bufferInfo.numelements, 0, instanceCount);

        engine.gl.vertexAttribDivisor(0,0);

    },
};


SlideManager.slides["534-grid"] = {
    initialize : function(engine) {

        const material = new Material(engine.resourceManager.getShaderProgram("hyperbolic"));
        // material.uniforms.u_diffuseColor = [0.7,0.7,0.7,1.0];        
        this.material = material;

        const bufferInfo = engine.resourceManager.getGeometry("534-grid-cell");
        const hMatrix = bufferInfo.extraData.hMatrix;
        const mats = regularPolyhedra.dodecahedron.rotations.map(r=>twgl.m4.multiply(r, hMatrix));    
        this.nodes = createCellsTransformations(mats);

        const nodesBuffers = [];
        this.nodes.forEach(node => {
            node.mat.forEach(v=>nodesBuffers.push(v));
        });

        twgl.setAttribInfoBufferFromArray(engine.gl, bufferInfo.attribs.cellMatrix, nodesBuffers);
    },

    draw : function(engine) {
        const m4 = twgl.m4;
        const gl = engine.gl;

        let world = m4.rotationY(performance.now()*0.0001);
        world = m4.multiply(m4.rotationX(0.05), world);


        engine.setMaterial(this.material);

        const bufferInfo = engine.resourceManager.getGeometry("534-grid-cell");



        engine.camera.cameraMatrix = m4.lookAt([0,0,-0.8],[0,0,0],[0,1,0]);

        engine.setWorldMatrix(world);
        engine.setBuffers(bufferInfo);


        var prog = this.material.program;

/*
        var nodes2 = [];
        const zero = twgl.v3.create(0,0,0);
        nodes.forEach(node => {
            var mat = m4.multiply(v.worldViewProjection, node.mat);
            var p = m4.transformPoint(mat, zero);
            if(p[2]>0) nodes2.push(node);
        });
        */

        let hMatrix = twgl.m4.identity();
        // hMatrix = KPoint.translationV3(grid534.pts[0], twgl.v3.create(0,0,0));
        
        twgl.setUniforms(prog.pInfo, {  hMatrix : hMatrix  });
        // twgl.drawBufferInfo(engine.gl, bufferInfo, engine.gl.TRIANGLES, bufferInfo.numelements);


        twgl.drawBufferInfo(engine.gl, bufferInfo, engine.gl.TRIANGLES, 
            bufferInfo.numElements, 0, this.nodes.length);
 
         engine.gl.vertexAttribDivisor(0,0);

         /*
        this.nodes.forEach(node => {
            twgl.setUniforms(prog.pInfo, {  hMatrix : node.mat });
            twgl.drawBufferInfo(engine.gl, bufferInfo, engine.gl.TRIANGLES, bufferInfo.numelements);
    
        });
        */
       // engine.drawGrid(world);

        

    }
};


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

function createCellsTransformations(mats) {
    
    const startTime = performance.now();
    const v3 = twgl.v3;
    const m4 = twgl.m4;
    
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

    /*
    mats.forEach(mat => { nodes.push({mat:mat}); });
    return nodes;
    */

    var todo = [nodes[0]];
    var nextTodo = [];
    const rEps = 0.001;

    const rMax = 0.999;
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

const slideMngr = new SlideManager({
    slides:[
        "hpolyhedra",
        "534-grid",
        "cube-grid",
        "slide1",
        "slide2",
        "slide3",
        "slide4",
        "slide5",
    ]
});



function initialize() {
    const engine = new Engine("viewer");
    const meter = createFpsMeter();

    const gl = engine.gl;
    slideMngr.initialize(engine);

    /*
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
   */

    // const bufferInfo = engine.resourceManager.getGeometry("three-cubes");
    
    
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


    /*
    const vertexArrayInfo = twgl.createVertexArrayInfo(
        gl, 
        material1.program.pInfo, 
        bufferInfo);

    scene1.vertexArrayInfo = vertexArrayInfo;
    scene1.material1 = material1;

    */

    document.addEventListener("keydown", e=>{
        if(e.code == "ArrowDown") slideMngr.nextSlide();
        else if(e.code == "ArrowUp") slideMngr.prevSlide();
        console.log(e);
    });

    const canvas = engine.canvas;
    canvas.addEventListener('mousedown', onMouseDown, false);



    window.ggl = gl;
    gl.enable(gl.DEPTH_TEST);
    gl.enable(gl.CULL_FACE);
    gl.clearColor(0,0,0,1);

    function render(time) {
        meter.tickStart();
        time *= 0.001;
        let err = gl.getError();
        if(err != gl.NO_ERROR) throw "GL ERROR " + err;

        twgl.resizeCanvasToDisplaySize(gl.canvas);
        gl.viewport(0, 0, gl.canvas.width, gl.canvas.height);
  
        gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);
  
        camera.update();

        slideMngr.draw(engine);

        meter.tick();
        err = gl.getError();
        if(err != gl.NO_ERROR) throw "GL ERROR " + err;
        requestAnimationFrame(render);
      }
      requestAnimationFrame(render);    
}

function getMousePos(e) {
    return {x:e.offsetX, y:e.offsetY};
}
function onMouseDown(e) {
    e.preventDefault();
    let lastMousePos = getMousePos(e);

    function onMouseDrag(e) { 
        e.preventDefault();
        let mousePos = getMousePos(e);
        // console.log("drag", mousePos.x-lastMousePos.x, mousePos.y-lastMousePos.y);
        const slide = slideMngr.currentSlide;
        if(slide && slide.onDrag) slide.onDrag(mousePos.x-lastMousePos.x, mousePos.y-lastMousePos.y);
        
        lastMousePos.x = mousePos.x;
        lastMousePos.y = mousePos.y;
    }
    function onMouseUp(e) { 
        e.preventDefault();
        document.removeEventListener('mousemove', onMouseDrag, false);
        document.removeEventListener('mouseup', onMouseUp, false);  
    }
    
    document.addEventListener('mousemove', onMouseDrag, false);
    document.addEventListener('mouseup', onMouseUp, false);

}


function dopo() {


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



}