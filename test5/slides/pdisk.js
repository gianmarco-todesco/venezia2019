
SlideManager.slides["pdisk"] = {

    initialize : function(engine) {

        this.mat2D = new Material(engine.resourceManager.getShaderProgram("2D"));
        this.mat2D.uniforms.u_color = [1,1,1,1];

        /*
        this.model = createHPolyhedraModel(engine.gl);
        twgl.setAttribInfoBufferFromArray(engine.gl, this.model.attribs.cellMatrix, twgl.m4.identity());
        this.gl = engine.gl;
        this.r = 0.95;
        this.delta = 0.0;
        */

        this.circle = createCircle(engine.gl);
        
        window.eng = engine;

    },

    draw : function(engine) {
        const m4 = twgl.m4;
        const v3 = twgl.v3;
        
        const gl = engine.gl;

        engine.setMaterial(this.mat2D);
        engine.setWorldMatrix(m4.identity());
        engine.setBuffers(this.circle);
        twgl.drawBufferInfo(engine.gl, this.circle, engine.gl.TRIANGLES);
    }
};


function createCircle(gl) {
    const m = 100;
    const arrays = {
        position: { numComponents: 2, data: [] },
        indices: []
    };
    const r1 = 1.0, r2 = 1.05;
    for(let i=0;i<m;i++) {
        const phi = Math.PI*2*i/(m-1);
        const cs = Math.cos(phi), sn = Math.sin(phi);
        arrays.position.data.push( cs * r2, sn * r2, cs * r1, sn * r1);
    }
    for(let i=0;i+1<m;i++) {
        arrays.indices.push(2*i,2*i+1,2*i+3, 2*i, 2*i+3, 2*i+2 );
    }

    return twgl.createBufferInfoFromArrays(gl, arrays);

}