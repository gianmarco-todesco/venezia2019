
const hpolyhedraSlide = SlideManager.slides["hpolyhedra"] = {

    initialize : function(engine) {

        this.material = new Material(engine.resourceManager.getShaderProgram("hyperbolic"));
        this.model = createHPolyhedraModel(engine.gl);
        twgl.setAttribInfoBufferFromArray(engine.gl, this.model.attribs.cellMatrix, twgl.m4.identity());
        this.gl = engine.gl;
        this.r = 0.95;
        this.delta = 0.0;

    },

    draw : function(engine) {
        const m4 = twgl.m4;
        const v3 = twgl.v3;
        
        const gl = engine.gl;

        let world = m4.rotationY(performance.now()*0.001);
        world = m4.multiply(m4.rotationX(0.05), world);

        engine.setMaterial(this.material);
        engine.setWorldMatrix(world);


        engine.setBuffers(this.model);

        const p = v3.mulScalar(this.model.extraData.firstVertex, this.r * this.delta);
        const hMatrix = KPoint.translationV3(p, [0,0,0]);
        twgl.setUniforms(this.material.program.pInfo, {  hMatrix : hMatrix  });
        
        twgl.drawBufferInfo(engine.gl, this.model, engine.gl.TRIANGLES,  this.model.numElements);
        

        engine.drawGrid(world);

        

    },

    onDrag : function(dx,dy) {
        this.setRadius(this.r + dx * 0.001);
        this.setDelta(this.delta + dy * 0.01);
    },

    setDelta(delta) {
        this.delta = Math.max(0.0, Math.min(1.0, delta));
    },

    setRadius(r) {
        this.r = Math.max(0.001, Math.min(0.999, r));
        this.model.extraData.update(this.gl, this.r);
    }
};


function foo(r) {
    hpolyhedraSlide.model.update(hpolyhedraSlide.gl, r);
}



function createHPolyhedraModel(gl) {
    const m4 = twgl.m4;
    const v3 = twgl.v3;
    const gb = new GeometryBuilder2(gl);

    const d = 0.95;

    const ph = regularPolyhedra.dodecahedron;
    ph.faces.forEach((f,fIndex)=>{
        const c = v3.mulScalar(ph.centers[fIndex], d);
        const pts = f.map(j=>v3.mulScalar(ph.vertices[j], d));
        const m = f.length;
        for(let j=0; j<m; j++) {
            // console.log(c, pts[j]);
            gb.addTriangleMesh(pts[j], c, pts[(j+1)%m], 20);

        }
    });

/*
    // gb.addQuadMesh([-d,-d,-d], [ d,-d,-d], [ d, d,-d], [-d, d,-d], 10,10);
    gb.addTriangleMesh([-d,-d, d], [ d,-d, d], [ 0, 0, d], 10);
    gb.addTriangleMesh([ d, d, d], [-d, d, d], [ 0, 0, d], 10);
    */

    gb.arrays.cellMatrix = {
        numComponents: 16,
        data: m4.identity(),
        divisor: 1
    };

    const result = gb.createBuffer();
    result.extraData = {
        buffers : {
            position : new Float32Array(gb.arrays.position),
            position_du : new Float32Array(gb.arrays.position_du.data),
            position_dv : new Float32Array(gb.arrays.position_dv.data),    
        },
        update : function(gl, r) {
            const sc = r/d;
            const buffers = result.extraData.buffers;
            twgl.setAttribInfoBufferFromArray(gl, result.attribs.position, buffers.position.map(x=>x*sc));
            twgl.setAttribInfoBufferFromArray(gl, result.attribs.position_du, buffers.position_du.map(x=>x*sc));
            twgl.setAttribInfoBufferFromArray(gl, result.attribs.position_dv, buffers.position_dv.map(x=>x*sc));
        },
        firstVertex : ph.vertices[0],
        
    };
    return result;
}

