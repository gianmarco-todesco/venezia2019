
SlideManager.slides["pdisk2"] = {

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
        this.disk = createDisk(engine.gl);

        this.hline = createHLine(engine.gl);
        window.eng = engine;
        this.fooPoint = [0,0,0];
        this.hlinePosition = new Float32Array(4*200);
        this.radius = 0.696;

        this.currentMatrix = twgl.m4.identity();

    },

    draw : function(engine) {
        const m4 = twgl.m4;
        const v3 = twgl.v3;
        
        const gl = engine.gl;

        engine.setMaterial(this.mat2D);
        const sc = 2.5;
        const worldMatrix = m4.scaling([sc,sc,sc]);
        engine.setWorldMatrix(worldMatrix);

        engine.gl.disable(engine.gl.DEPTH_TEST);


        twgl.setUniforms(this.mat2D.program.pInfo, {  u_color : [0.9,0.9,0.9,1]  });
        engine.setBuffers(this.disk);
        twgl.drawBufferInfo(engine.gl, this.disk, engine.gl.TRIANGLES);

        twgl.setUniforms(this.mat2D.program.pInfo, {  u_color : [0.0,0.0,0.4,1]  });
        engine.setBuffers(this.circle);
        twgl.drawBufferInfo(engine.gl, this.circle, engine.gl.TRIANGLES);

        /*
        engine.setMaterial(this.matH2);
        engine.setWorldMatrix(m4.identity());
        engine.setBuffers(this.hline);
        */
        engine.setMaterial(this.mat2D);
        twgl.setUniforms(this.mat2D.program.pInfo, {  u_color : [0,0.6,0.9,1]  });
        
        engine.setWorldMatrix(worldMatrix);

        const startTime = performance.now();

        const edgeCount = 8;

        
        const r = this.radius;
        const pp = [];
        for(let i=0; i<edgeCount; i++) {
            const phi = 2*Math.PI*i/edgeCount;
            pp.push(r*Math.cos(phi), r*Math.sin(phi));
        }

        const mats = [];
        for(let i=0; i<edgeCount; i++) {
            const i1 = (i+1)%edgeCount;
            let pm = [(pp[2*i]+pp[2*i1])*0.5, (pp[2*i+1]+pp[2*i1+1])*0.5,0];
            mats.push(KPoint.reflectionV3(pm));            
        }

        const me = this;

        function drawHPolygon() {
            for(let i=0; i<edgeCount; i++) {
                const i1 = (i+1)%edgeCount;
                me.drawHSegment(engine, pp[2*i], pp[2*i+1], pp[2*i1], pp[2*i1+1]);
            }
        }

        

        this.currentMatrix = twgl.m4.identity();
        drawHPolygon();

        mats.forEach(mat => { this.currentMatrix = mat; drawHPolygon(); });
        

        for(var i=0;i<2;i++) {
            for(let j=2;j<6;j++) {
                this.currentMatrix = m4.multiply(mats[i], mats[j]);
                drawHPolygon();
            }    
        }

        window.tt = performance.now()-startTime;

        /*
        const hMatrix = KPoint.translationV3(this.fooPoint, [0,0,0]);
        twgl.setUniforms(this.matH2.program.pInfo, {  u_hMatrix : hMatrix  });
        window.hMatrix2 = hMatrix;

        twgl.drawBufferInfo(engine.gl, this.hline, engine.gl.TRIANGLES);
        */
        engine.gl.enable(engine.gl.DEPTH_TEST);

    },

    drawHSegment(engine, x0, y0, x1, y1) {
        const m = 200;
        const thickness = 0.01;


        
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
            const d = thickness * pp[3*i+2];
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
        this.disk = createDisk(engine.gl);

        this.hline = createHLine(engine.gl);
        this.hPolygon = createHPolygon(engine.gl);
        window.eng = engine;
        this.fooPoint = [0,0,0];
        this.hlinePosition = new Float32Array(4*200);
        this.radius = 0.696;
        this.hMatrix = twgl.m4.identity();

        this.currentMatrix = twgl.m4.identity();
        this.pos = [0,0,0];

    },

    draw : function(engine) {
        const m4 = twgl.m4;
        const v3 = twgl.v3;
        
        const gl = engine.gl;

        engine.setMaterial(this.mat2D);
        const sc = 2.5;
        const worldMatrix = m4.scaling([sc,sc,sc]);
        engine.setWorldMatrix(worldMatrix);

        engine.gl.disable(engine.gl.DEPTH_TEST);


        twgl.setUniforms(this.mat2D.program.pInfo, {  u_color : [0.9,0.9,0.9,1]  });
        engine.setBuffers(this.disk);
        twgl.drawBufferInfo(engine.gl, this.disk, engine.gl.TRIANGLES);

        twgl.setUniforms(this.mat2D.program.pInfo, {  u_color : [0.0,0.0,0.4,1]  });
        engine.setBuffers(this.circle);
        twgl.drawBufferInfo(engine.gl, this.circle, engine.gl.TRIANGLES);



        engine.setMaterial(this.matH2);
        engine.setWorldMatrix(worldMatrix);
        engine.setBuffers(this.hPolygon);


        twgl.setUniforms(this.matH2.program.pInfo, {  u_hMatrix : this.hMatrix  });
        engine.draw();

    
        const matrices = this.hPolygon.edgeCenters.map(p=> KPoint.reflectionV3(p));

        const pInfo = this.matH2.program.pInfo;
        const hMatrix = this.hMatrix;

        function draw(mat) {
            twgl.setUniforms(pInfo, {  u_hMatrix : m4.multiply(hMatrix,mat) });
            engine.draw();
        }
        for(let i=0; i<matrices.length; i++) draw(matrices[i]);
       
        for(let i=0; i<matrices.length; i++) {
            for(let j=4; j<8; j++)  
                draw(m4.multiply(matrices[i], matrices[j]));
        
        }






/*
        engine.setMaterial(this.mat2D);
        twgl.setUniforms(this.mat2D.program.pInfo, {  u_color : [0,0.6,0.9,1]  });
        
        engine.setWorldMatrix(worldMatrix);

        const startTime = performance.now();

        const edgeCount = 8;

        
        const r = this.radius;
        const pp = [];
        for(let i=0; i<edgeCount; i++) {
            const phi = 2*Math.PI*i/edgeCount;
            pp.push(r*Math.cos(phi), r*Math.sin(phi));
        }

        const mats = [];
        for(let i=0; i<edgeCount; i++) {
            const i1 = (i+1)%edgeCount;
            let pm = [(pp[2*i]+pp[2*i1])*0.5, (pp[2*i+1]+pp[2*i1+1])*0.5,0];
            mats.push(KPoint.reflectionV3(pm));            
        }

        const me = this;

        function drawHPolygon() {
            for(let i=0; i<edgeCount; i++) {
                const i1 = (i+1)%edgeCount;
                me.drawHSegment(engine, pp[2*i], pp[2*i+1], pp[2*i1], pp[2*i1+1]);
            }
        }

        

        this.currentMatrix = twgl.m4.identity();
        drawHPolygon();

        mats.forEach(mat => { this.currentMatrix = mat; drawHPolygon(); });
        

        for(var i=0;i<2;i++) {
            for(let j=2;j<6;j++) {
                this.currentMatrix = m4.multiply(mats[i], mats[j]);
                drawHPolygon();
            }    
        }
        window.tt = performance.now()-startTime;
*/

        /*
        const hMatrix = KPoint.translationV3(this.fooPoint, [0,0,0]);
        twgl.setUniforms(this.matH2.program.pInfo, {  u_hMatrix : hMatrix  });
        window.hMatrix2 = hMatrix;

        twgl.drawBufferInfo(engine.gl, this.hline, engine.gl.TRIANGLES);
        */
        engine.gl.enable(engine.gl.DEPTH_TEST);

    },

    drawHSegment(engine, x0, y0, x1, y1) {
        const m = 200;
        const thickness = 0.01;


        
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
            const d = thickness * pp[3*i+2];
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
        // this.radius += 0.001*dx;
        this.pos[0] += 0.001*dx;
        this.pos[1] += 0.001*dy;
        this.hMatrix = KPoint.translationV3([0,0,0], this.pos);
        
    }


};


function createCircle(gl) {
    const m = 150;
    const arrays = {
        position: { numComponents: 2, data: [] },
        indices: []
    };
    const r1 = 1.0, r2 = 1.03;
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

function createDisk(gl) {
    const m = 100;
    const arrays = {
        position: { numComponents: 2, data: [] },
        indices: []
    };
    const r1 = 1.0;
    arrays.position.data.push(0,0);
    for(let i=0;i<m;i++) {
        const phi = Math.PI*2*i/(m-1);
        const cs = Math.cos(phi), sn = Math.sin(phi);
        arrays.position.data.push( cs * r1, sn * r1);
    }
    for(let i=0;i+1<m;i++) {
        arrays.indices.push(0,2 + i, 1 + i);
    }

    return twgl.createBufferInfoFromArrays(gl, arrays);
}

function createHLine(gl) {
    const m = 100;
    const d = 0.05;
    const length = 1.0;
    const x0 = -length/2, x1 = length/2;

    const arrays = {
        position: { numComponents: 2, data: [] },
        indices: []
    };
    for(let i=0;i<m;i++) {
        const t = i/(m-1);
        const x = (1-t)*x0 + t*length*x1;
        arrays.position.data.push(x,-d,x,d); 
    }
    for(let i=0;i+1<m;i++) {
        arrays.indices.push(2*i,2*i+1,2*i+3, 2*i, 2*i+3, 2*i+2 );
    }

    return twgl.createBufferInfoFromArrays(gl, arrays);

}

function createHPolygon(gl) {
    const n = 8;
    const m = 50;
    const d = 0.01;
    const r = 0.69;

    const arrays = {
        position: { numComponents: 2, data: [] },
        indices: []
    };

    const pts = [];
    const pts2 = [];
    for(let i=0;i<n;i++) { 
        const phi = Math.PI*2*i/n;
        const cs = Math.cos(phi), sn = Math.sin(phi);
        pts.push(cs*(r-d), sn*(r-d), cs*(r+d), sn*(r+d));
        pts2.push(cs*r,sn*r);
    }

    const edgeCenters = [];
    for(let i=0;i<n;i++) { 
        const i1 = (i+1)%n;
        edgeCenters.push([
            (pts2[2*i]+pts2[2*i1])*0.5,
            (pts2[2*i+1]+pts2[2*i1+1])*0.5,
            0
        ]);
    }

    for(let i=0;i<n;i++) {
        const i1 = (i+1)%n;
        const x0 = pts[i*4], y0 = pts[i*4+1], x1 = pts[i1*4], y1 = pts[i1*4+1], 
            x2 = pts[i1*4+2], y2 = pts[i1*4+3], x3 = pts[i*4+2], y3 = pts[i*4+3];
        for(let j=0; j<m; j++) {
            const t = j/m;
            const x01 = x0*(1-t)+x1*t;
            const y01 = y0*(1-t)+y1*t;            
            const x32 = x3*(1-t)+x2*t;
            const y32 = y3*(1-t)+y2*t;
            arrays.position.data.push(x01,y01,x32,y32);            
        }
    }

    
    for(let i=0;i<m*n;i++) {
        const i1 = (i+1)%(m*n);
        arrays.indices.push(2*i,2*i1+1,2*i+1, 2*i, 2*i1, 2*i1+1);
    }

    const result = twgl.createBufferInfoFromArrays(gl, arrays);
    result.edgeCenters = edgeCenters;
    return result;
}


class H2Tessellation {

    constructor(p,q) {
        this.p = p; // edges per face
        this.q = q; // faces around vertex
        this.faces = [];
        this.edgeTableRight = {};
        this.edgeTableLeft = {};
    }

    addFace(matrix) {
        const k = this.faces.length;
        this.faces.push(matrix);
    }

    getId(a) { return "e_"+a[0]+"_"+a[1]; }

    linkFaces(table, a,b) {
        const a_id = this.getId(a);
        const b_id = this.getId(b);
        if(this.edgeTable[a_id] === undefined) {
            if(this.edgeTable[b_id]) throw "uff";
            this.edgeTable[a_id] = [b[0],b[1],2];
            this.edgeTable[b_id] = [a[0],a[1],2];
        } else {

        }
        



    }
}