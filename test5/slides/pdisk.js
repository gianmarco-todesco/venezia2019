
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


        this.tess = new H2Tessellation(8,3);
        
        this.hline = createHLine(engine.gl);
        this.hPolygon = createHPolygon(this.tess, engine.gl);
        window.eng = engine;
        this.fooPoint = [0,0,0];
        this.hlinePosition = new Float32Array(4*200);
        this.radius = 0.696;
        this.hMatrix = twgl.m4.identity();

        this.currentMatrix = twgl.m4.identity();
        this.pos = [0,0,0];

        this.index = 0;

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

        const cells = this.tess.cells;
        const pInfo = this.matH2.program.pInfo;
        const hMatrix = this.hMatrix;
        const currentIndex = this.index;
        cells.map((cell,index) => {

            const mat = cell.matrix;
            twgl.setUniforms(pInfo, {  
                u_hMatrix : m4.multiply(hMatrix, mat),
                u_color : [0.03,0.5,0.8,1]
             });
            if(index != currentIndex) engine.draw();    
        });
        if(0<=currentIndex && currentIndex < cells.length) {
            const mat = cells[currentIndex].matrix;
            twgl.setUniforms(pInfo, {  
                u_hMatrix : m4.multiply(hMatrix, mat),
                u_color : [0.9,0.6,0.1,1]
             });
            engine.draw();        
        }



        /*

        const matrices = [];
        


        for(let i=0; i<8; i++) {
            const mat_a = m4.multiply(hMatrix, matrices[i]);
            for(let j=2; j<6; j++) {
                twgl.setUniforms(pInfo, {  u_hMatrix : m4.multiply(mat_a, matrices[j]) });
                engine.draw();           
            }


        }
        */

        engine.drawGrid(m4.identity());





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
        this.hMatrix = twgl.m4.multiply(KPoint.translationV3([dx*0.01,-dy*0.01,0],[0,0,0]), this.hMatrix);
        
        this.recenter();
    },

    onKeyDown(e) {
        if(e.key == "+") this.index++;        
        else if(e.key == "a") {
            const m4 = twgl.m4;
            const mat = m4.multiply(this.hMatrix, this.tess.cells[this.index].matrix);
            this.hMatrix = twgl.m4.multiply(this.hMatrix, m4.inverse(mat));
            console.log(this.hMatrix);
        } else if(e.key == "b") {
            this.recenter();
        }
    },

    recenter() {
        const m4 = twgl.m4;
        let minDist2 = 0;
        let closestCell = null;
        for(let i=0; i<9; i++) {
            const p = m4.transformPoint(m4.multiply(this.hMatrix, this.tess.cells[i].matrix), [0,0,0]);
            const d2 = p[0]*p[0]+p[1]*p[1];
            if(closestCell == null || d2<minDist2) { minDist2 = d2; closestCell = i; }            
        }
        this.hMatrix = m4.multiply(this.hMatrix, this.tess.cells[closestCell].matrix);        
    }


};

//-----------------------------------------------------------------------------

function createCircle(gl) {
    const m = 150;
    const arrays = {
        position: { numComponents: 2, data: [] },
        indices: []
    };
    const r1 = 1.0, r2 = 1.01;
    for(let i=0;i<m;i++) {
        const phi = Math.PI*2*i/(m-1);
        const cs = Math.cos(phi), sn = Math.sin(phi);
        arrays.position.data.push( cs * r1, sn * r1, cs * r2, sn * r2);
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
        arrays.indices.push(0,1 + i, 2 + i);
    }

    return twgl.createBufferInfoFromArrays(gl, arrays);
}

// obsolete?
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

function createHPolygon(tess, gl) {
    const m = 50;
    const d = 0.01;

    const arrays = {
        position: { numComponents: 2, data: [] },
        indices: []
    };

    const n = tess.p;
    const r = tess.radius;
    const pts = tess.pts;
    for(let i=0;i<n;i++) {
        const i1 = (i+1)%n;
        const x0 = pts[i][0]/r, y0 = pts[i][1]/r, x1 = pts[i1][0]/r, y1 = pts[i1][1]/r;
        const x2 = x0 * (r-d), y2 = y0 * (r-d), x3 = x1 * (r-d), y3 = y1 * (r-d);
        const x4 = x0 * (r+d), y4 = y0 * (r+d), x5 = x1 * (r+d), y5 = y1 * (r+d);

        for(let j=0; j<m; j++) {
            const t = j/m;
            arrays.position.data.push(
                x2*(1-t)+x3*t,
                y2*(1-t)+y3*t,
                x4*(1-t)+x5*t,
                y4*(1-t)+y5*t
                );
        }
    }

    
    for(let i=0;i<m*n;i++) {
        const i1 = (i+1)%(m*n);
        arrays.indices.push(2*i,2*i+1,2*i1+1, 2*i1, 2*i, 2*i1+1);
    }

    const result = twgl.createBufferInfoFromArrays(gl, arrays);
    return result;
}


class H2Tessellation {

    constructor(p,q) {
        this.p = p; // edges per face
        this.q = q; // faces around vertex

        this.radius = this.computePolygonRadius();
        this.buildPolygon();
        this.buildCells();
    }

    // return the euclidean distance in the K-Model between a vertex and the center 
    computePolygonRadius() {
        // radius = hypotenuse, angles : A=pi/p, B=pi/q
        // cosh(hypotenuse) = cot(A)cot(B)
        const dist = Math.acosh(1.0/(Math.tan(Math.PI/this.p)*Math.tan(Math.PI/this.q)));
        // dist is the hyperbolic distance
        const tmp = Math.exp(2*dist);
        const radius = (tmp-1)/(tmp+1);
        // distanza (euclidea) dal centro di un punto nel modello di Klein
        // che abbia dist come distanza iperbolica.
        // uso la formula per la dist. hyp. del modello di klein: 
        // d=1/2log((aq*pb)/(ap*qb)), con d=d2, ap=pb=1, pq=d3, qb=1-d3
        
        return radius;
    }

    buildPolygon() {
        const r = this.radius;
        const pts = this.pts = [];
        const n = this.p;
        for(let i=0; i<n; i++) {
            const phi = Math.PI*2*i/n;
            pts.push([Math.cos(phi)*r, Math.sin(phi)*r,0]);
        }
        const edgeCenters = this.edgeCenters = [];
        for(let i=0; i<n; i++) {
            const i1 = (i+1)%n;
            // nota: per ragioni di simmetria posso calcolare i punti medi con il modo semplice
            edgeCenters.push([
                (pts[i][0]+pts[i1][0])*0.5,
                (pts[i][1]+pts[i1][1])*0.5,
                0
            ]);
        }
    }

    buildCells() {
        // ATTENZIONE: funziona solo per {8,3}!!!
        const m4 = twgl.m4;
        const cells = this.cells = [];
        cells.push({matrix: m4.identity(), type: 0});

        const matrix = KPoint.reflectionV3(this.edgeCenters[0]);
        cells.push({matrix: matrix, type: 1});
        const n = this.p;

        for(let i=1;i<n;i++) {
            const childMatrix = m4.multiply(m4.rotationZ(2*Math.PI*i/n), matrix);
            cells.push({matrix: childMatrix, type: 1});
        }

        let curType = 2;

        function addLevel(level, first, matrix) {
            if(level > 0) {
                const j1 = first ? 3 : 2;
                for(let j=j1; j<6; j++) {
                    const childMatrix = m4.multiply(matrix, cells[j].matrix);
                    cells.push({matrix: childMatrix, type: curType});
                    addLevel(level-1, j==j1, childMatrix);
                }
            }
        }

        for(let i=0; i<n; i++) {
            addLevel(1, false, cells[i].matrix);
        }

        /*
        for(let i=0; i<n; i++) {
            const mat1 = matrices[i];
            for(let j=2;j<6;j++) {
                const mat2 = m4.multiply(matrices[i], matrices[j]);
                matrices.push(mat2);
                if(j==2) {
                    for(let k=3; k<6; k++)
                        matrices.push(m4.multiply(mat2, matrices[k]));
                } else {
                    for(let k=2; k<6; k++)
                        matrices.push(m4.multiply(mat2, matrices[k]));
                }

            }
        }
        */

        console.log("Cell count = ", cells.length)    
    }

}
