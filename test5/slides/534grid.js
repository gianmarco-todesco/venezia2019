
SlideManager.slides["534-grid"] = {
    initialize(engine) {

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
        this.world = twgl.m4.identity();
        this.hMatrix = twgl.m4.identity();
    },

    start() {
        this.gl.clearColor(0.4,0.4,0.4,1);
    },

    onDrag(dx,dy) {
        const m4 = twgl.m4;
        this.hMatrix = m4.multiply(m4.multiply(
            m4.rotationX(dy*0.01),
            m4.rotationY(-dx*0.01)
        ), this.hMatrix);
    },

    onKeyDown(e) {
        if(e.key == "s") {
            const m4 = twgl.m4;
            this.hMatrix = m4.multiply(KPoint.translationV3([0,0,0],[0,0,0.01]), this.hMatrix);
        } else if(e.key == "d") {
            const m4 = twgl.m4;
            this.hMatrix = m4.multiply(KPoint.translationV3([0,0,0],[0,0,-0.01]), this.hMatrix);
        } else if(e.key == "a") {
            const m4 = twgl.m4;
            this.hMatrix = m4.multiply(KPoint.translationV3([0,0,0],[-0.01,0,0.]), this.hMatrix);
        } else if(e.key == "f") {
            const m4 = twgl.m4;
            this.hMatrix = m4.multiply(KPoint.translationV3([0,0,0],[ 0.01,0,0.]), this.hMatrix);
        } else if(e.key == "1") {
            const m4 = twgl.m4;
            this.hMatrix = m4.multiply(this.hMatrix, this.nodes[1].mat);
        } else if(e.key == "2") {
            const m4 = twgl.m4;
            this.hMatrix = m4.multiply(this.hMatrix, m4.inverse(this.nodes[1].mat));
        }


    },

    draw : function(engine) {
        const m4 = twgl.m4;
        const gl = engine.gl;

        let world = this.world;
        
        engine.camera.cameraMatrix = m4.lookAt([0,0,-0.8],[0,0,0],[0,1,0]);

        engine.setMaterial(this.material);




        engine.setWorldMatrix(world);
        const bufferInfo = engine.resourceManager.getGeometry("534-grid-cell");
        engine.setBuffers(bufferInfo);



        // let hMatrix = twgl.m4.identity();
        // hMatrix = KPoint.translationV3(grid534.pts[0], twgl.v3.create(0,0,0));
        

        engine.setUniforms({  hMatrix : this.hMatrix  });

        twgl.drawBufferInfo(engine.gl, bufferInfo, engine.gl.TRIANGLES, 
            bufferInfo.numElements, 0, 13); 
         engine.gl.vertexAttribDivisor(0,0);



        const bufferInfo2 = engine.resourceManager.getGeometry("534-foo");
        engine.setBuffers(bufferInfo2);

        const nodesBuffers = [];
        this.nodes.forEach(node => {
            node.mat.forEach(v=>nodesBuffers.push(v));
        });
        twgl.setAttribInfoBufferFromArray(engine.gl, bufferInfo2.attribs.cellMatrix, nodesBuffers);
        

        twgl.drawBufferInfo(engine.gl, bufferInfo2, engine.gl.TRIANGLES, 
            bufferInfo2.numElements, 0, 1);


         
         /*
        this.nodes.forEach(node => {
            twgl.setUniforms(prog.pInfo, {  hMatrix : node.mat });
            twgl.drawBufferInfo(engine.gl, bufferInfo, engine.gl.TRIANGLES, bufferInfo.numelements);
    
        });
        */
        // engine.drawGrid(world);

        

    },


    recenter() {

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
