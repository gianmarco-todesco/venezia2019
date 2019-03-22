
SlideManager.slides["pdisk"] = {

    initialize : function(engine) {

        this.mat2D = new Material(engine.resourceManager.getShaderProgram("2D"));
        this.mat2D.uniforms.u_color = [1,1,1,1];

        this.matH2 = new Material(engine.resourceManager.getShaderProgram("H2"));
        this.matH2.uniforms.u_color = [0,0.5,1,1];
        this.matH2.uniforms.u_hMatrix = twgl.m4.identity();

        /*
        this.model = createHPolyhedraModel(engine.gl);
        twgl.setAttribInfoBufferFromArray(engine.gl, this.model.attribs.cellMatrix, twgl.m4.identity());
        this.gl = engine.gl;
        this.r = 0.95;
        this.delta = 0.0;
        */

        this.circle = createCircle(engine.gl);
        this.hline = createHLine(engine.gl);
        window.eng = engine;
        this.fooPoint = [0,0,0];
        this.hlinePosition = new Float32Array(4*200);
        this.radius = 0.5;

        this.currentMatrix = twgl.m4.identity();

    },

    draw : function(engine) {
        const m4 = twgl.m4;
        const v3 = twgl.v3;
        
        const gl = engine.gl;

        engine.setMaterial(this.mat2D);
        engine.setWorldMatrix(m4.identity());
        engine.setBuffers(this.circle);
        twgl.drawBufferInfo(engine.gl, this.circle, engine.gl.TRIANGLES);

        /*
        engine.setMaterial(this.matH2);
        engine.setWorldMatrix(m4.identity());
        engine.setBuffers(this.hline);
        */
        engine.setMaterial(this.mat2D);
        twgl.setUniforms(this.mat2D.program.pInfo, {  u_color : [0,0.6,0.9,1]  });
        
        engine.setWorldMatrix(twgl.m4.identity());

        const r = this.radius;
        const pp = [];
        for(let i=0; i<5; i++) {
            const phi = 2*Math.PI*i/5;
            pp.push(r*Math.cos(phi), r*Math.sin(phi));
        }

        const startTime = performance.now();
        this.currentMatrix = twgl.m4.identity();
        for(let i=0; i<5; i++) {
            const i1 = (i+1)%5;
            this.drawHSegment(engine, pp[2*i], pp[2*i+1], pp[2*i1], pp[2*i1+1]);
        }
        let pm = [(pp[0]+pp[2])*0.5, (pp[1]+pp[3])*0.5,0];
        this.currentMatrix = KPoint.reflectionV3(pm);
        for(let i=0; i<5; i++) {
            const i1 = (i+1)%5;
            this.drawHSegment(engine, pp[2*i], pp[2*i+1], pp[2*i1], pp[2*i1+1]);
        }
        pm = [(pp[2]+pp[4])*0.5, (pp[3]+pp[5])*0.5,0];
        this.currentMatrix = KPoint.reflectionV3(pm);
        for(let i=0; i<5; i++) {
            const i1 = (i+1)%5;
            this.drawHSegment(engine, pp[2*i], pp[2*i+1], pp[2*i1], pp[2*i1+1]);
        }
        window.tt = performance.now()-startTime;

        /*
        const hMatrix = KPoint.translationV3(this.fooPoint, [0,0,0]);
        twgl.setUniforms(this.matH2.program.pInfo, {  u_hMatrix : hMatrix  });
        window.hMatrix2 = hMatrix;

        twgl.drawBufferInfo(engine.gl, this.hline, engine.gl.TRIANGLES);
        */

    },

    drawHSegment(engine, x0, y0, x1, y1) {
        const m = 200;



        
        var pp = [];
        const mat = this.currentMatrix;

        for(var i=0;i<m;i++)
        {
            const t = i/(m-1);
            let x = (1-t)*x0 + t*x1;
            let y = (1-t)*y0 + t*y1;

            let xx = mat[0]*x+mat[4]*y+mat[12];
            let yy = mat[1]*x+mat[5]*y+mat[13];
            let w = mat[3]*x+mat[7]*y+mat[15];
            x = xx/w;
            y = yy/w;

            const s2 = Math.min(1.0, x*x + y*y);
            const k = 1.0 / (1.0 + Math.sqrt(1.0 - s2));        
            x *= k;
            y *= k;
            pp.push(x,y,1.0-Math.sqrt(x*x+y*y));
        }

        const position = this.hlinePosition;
        

        for(let i=0;i<m;i++) {
            const x = pp[3*i];
            const y = pp[3*i+1];
            const i1 = Math.max(0,i-1);
            const i2 = Math.min(m-1,i+1);
            const dx = pp[3*i2] - pp[3*i1];
            const dy = pp[3*i2+1] - pp[3*i1+1];
            const dd = Math.sqrt(dx*dx+dy*dy);
            const d = 0.02 * pp[3*i+2];
            const ux = -d*dy/dd, uy = d*dx/dd;            
            position[i*4] = x-ux;
            position[i*4+1] = y-uy;
            position[i*4+2] = x+ux;
            position[i*4+3] = y+uy;
        }

        twgl.setAttribInfoBufferFromArray(
            engine.gl, 
            this.hline.attribs.position, 
            position);
        engine.setBuffers(this.hline);
        twgl.drawBufferInfo(engine.gl, this.hline, engine.gl.TRIANGLES);
    
    },

    onDrag(dx,dy) {
        this.radius += 0.001*dx;
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

function createHLine(gl) {
    const m = 200;
    const d = 0.03;
    const x0 = 1.0, y0 = 0.0, x1 = -Math.sqrt(0.5), y1 = Math.sqrt(0.5);
    const dx = x1-x0, dy = y1-y0;
    const dd = Math.sqrt(dx*dx+dy*dy);
    const ux = -d*dy/dd, uy = d*dx/dd;
    

    const arrays = {
        position: { numComponents: 2, data: [] },
        indices: []
    };
    for(let i=0;i<m;i++) {
        const t = i/(m-1);
        const x = (1-t)*x0 + t*x1;
        const y = (1-t)*y0 + t*y1;
        
        arrays.position.data.push(x-ux,y-uy,x+ux,y+uy); 
    }
    for(let i=0;i+1<m;i++) {
        arrays.indices.push(2*i,2*i+1,2*i+3, 2*i, 2*i+3, 2*i+2 );
    }

    return twgl.createBufferInfoFromArrays(gl, arrays);

}