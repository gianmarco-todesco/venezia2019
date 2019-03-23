

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


const slideMngr = new SlideManager({
    slides:[
        "534-grid",
        "pdisk",
        "surfaces",
        "polydron",
        "cube-grid",
        "hpolyhedra",
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
        else {
            const slide = slideMngr.currentSlide;
            if(slide && slide.onKeyDown)  slide.onKeyDown(e);
            // console.log(e);
        }
    });

    const canvas = engine.canvas;
    canvas.addEventListener('mousedown', onMouseDown, false);



    window.ggl = gl;
    gl.enable(gl.DEPTH_TEST);
    gl.enable(gl.CULL_FACE);
    // gl.clearColor(0.3,0.3,0.3,1);

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