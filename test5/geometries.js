"use strict";


ResourceStore.geometries["three-cubes"] = function(gl) {
    const arrays = GeometryBuilder.createCubeArray(gl, 0.5);
    arrays.instancePos = {
        numComponents: 4,
        data: [0,0,0,1, 1,1,1,1, 2,2,2,1],
        divisor: 1
    };
    return twgl.createBufferInfoFromArrays(gl, arrays);
};

const PolygonHeight = 0.01;
const PolygonEdge = 1;

ResourceStore.geometries["sphere"] = function(gl) {
    return twgl.primitives.createSphereBufferInfo(gl, 1, 20, 20);
}

ResourceStore.geometries["cylinder"] = function(gl) {
    return twgl.primitives.createCylinderBufferInfo(gl, 1, 1, 20, 20);
}


ResourceStore.geometries["triangle"] = function(gl) {
    const r = PolygonEdge*0.5/Math.sin(Math.PI/3);
    const arrays = GeometryBuilder.createThickFaceArray(gl, 3, r, PolygonHeight);
    return twgl.createBufferInfoFromArrays(gl, arrays);
};

ResourceStore.geometries["square"] = function(gl) {
    const r = PolygonEdge/Math.sqrt(2);
    const arrays = GeometryBuilder.createThickFaceArray(gl, 4, r, PolygonHeight);
    return twgl.createBufferInfoFromArrays(gl, arrays);
};

ResourceStore.geometries["pentagon"] = function(gl) {
    const r = PolygonEdge*0.5/Math.sin(Math.PI/5);
    const arrays = GeometryBuilder.createThickFaceArray(gl, 5, r, PolygonHeight);
    return twgl.createBufferInfoFromArrays(gl, arrays);
};

ResourceStore.geometries["decagon"] = function(gl) {
    const r = PolygonEdge*0.5/Math.sin(Math.PI/10);
    const arrays = GeometryBuilder.createThickFaceArray(gl, 10, r, PolygonHeight);
    return twgl.createBufferInfoFromArrays(gl, arrays);
};

ResourceStore.geometries["cube-grid-cell"] = function(gl) {
    const m4 = twgl.m4;
    const gb = new GeometryBuilder(gl);

    const a = 0.8, b = 1.0, c = 0.95;

    // cubi per i vertici
    var i,j ;
    const faceMatrices = [
        m4.identity(),
        m4.rotationX(Math.PI),
        m4.rotationX(Math.PI/2),
        m4.rotationX(-Math.PI/2),
        m4.rotationY(Math.PI/2),
        m4.rotationY(-Math.PI/2),
    ];
    for(i=0;i<6;i++) {
        gb.pushMatrix();
        gb.multmatrix(faceMatrices[i]);
        gb.setNormal(0,0,1);
        gb.addQuads([
            -b,-b,-a, -a,-b,-a, -a,-a,-a, -b,-a,-a,
             a,-b,-a,  b,-b,-a,  b,-a,-a,  a,-a,-a,
            -b, a,-a, -a, a,-a, -a, b,-a, -b, b,-a,
             a, a,-a,  b, a,-a,  b, b,-a,  a, b,-a,
        ]);    
        gb.popMatrix();
    }

    // parallelepipedi per gli spigoli
    const edgeMatrices = [
        m4.identity(),
        m4.rotationX(Math.PI/2),
        m4.rotationY(Math.PI/2),
    ]
    for(i=0;i<3;i++) {
        gb.pushMatrix();
        gb.multmatrix(edgeMatrices[i]);
        for(j=0; j<4; j++) {
            gb.pushMatrix();
            gb.rotateZ(Math.PI*0.5*j);
            gb.setNormal(1,0,0);
            gb.addQuads([
                -c,-b,-b, -c,-c,-b, -c,-c, b, -c,-b, b,
                -c, c,-b, -c, b,-b, -c, b, b, -c, c, b,
            ]);
            gb.popMatrix();
        }
        gb.popMatrix();    
    }

    const offsets = [];
    var m = 7;
    for(let dx = -m; dx<=m; dx++)
    for(let dy = -m; dy<=m; dy++)
    for(let dz = -m; dz<=m; dz++) {
        offsets.push(dx*2,dy*2,dz*2);
        // offsets.push(0,0,0);
    }

    let arrays = gb.arrays;

    arrays.instancePos = {
        numComponents: 3,
        data: offsets,
        divisor: 1
    };

    
    return twgl.createBufferInfoFromArrays(gl, arrays);
};



ResourceStore.geometries["534-grid-cell"] = function(gl) {
    const m4 = twgl.m4;
    const v3 = twgl.v3;

    const gb = new GeometryBuilder2(gl);

    const { pts, centers } = grid534;

    // const nodes = fooBar(dod, hMatrix);

    regularPolyhedra.dodecahedron.edges.forEach(e => {
        var p0 = pts[e[0]];
        var p1 = pts[e[1]];

        var pa = centers[e[2]];
        var pb = centers[e[3]];

        var p2 = v3.lerp(p0,pa,0.05);
        var p3 = v3.lerp(p1,pa,0.05);
        var p4 = v3.lerp(p0,pb,0.05);
        var p5 = v3.lerp(p1,pb,0.05);
        
        gb.addStrip(p2,p3,p5,p4, 30);
    });


    // lavoro sui vertici


    for(let i=0; i<pts.length; i++) {
        const vv = [i]; 
        // aggiungo gli indici dei vertici adiacenti a 0
        regularPolyhedra.dodecahedron.edges.forEach(e=>{
            if(e[0]==i) vv.push(e[1]);
            else if(e[1]==i) vv.push(e[0]);
        });
    
    
        const mat = KPoint.translationV3(pts[i], [0,0,0]);
    
        const pp = vv.map(j=>m4.transformPoint(mat, pts[j]));
        const q = pp[0];
        let e0 = v3.normalize(v3.subtract(pp[1],pp[0]));
        let e1 = v3.normalize(v3.subtract(pp[2],pp[0]));
        let e2 = v3.normalize(v3.subtract(pp[3],pp[0]));
        if(v3.dot(e2,v3.cross(e0,e1))<0) { let tmp=e2;e2=e1;e1=tmp; }
        const mat2 = [
            e0[0],e0[1],e0[2],0, 
            e1[0],e1[1],e1[2],0,
            e2[0],e2[1],e2[2],0,
            q[0],q[1],q[2],1
        ];
    
        const mat3 = m4.multiply(m4.inverse(mat), mat2);
    
        const d = 0.02;
        const d1 = d * 1.12;
        [
            [[0,0,d], [d,0,d], [d1,d1,d1], [0,d,d]],    
            [[0,d,0], [0,d,d], [d1,d1,d1], [d,d,0]],
            [[d,0,0], [d,d,0], [d1,d1,d1], [d,0,d]],        
        ].forEach(q => {
            const qq = q.map(p => m4.transformPoint(mat3,p));
            gb.addQuad(qq[0],qq[1],qq[2],qq[3]);
        });
    
    }


    // console.log(v3.dot(e0,e1), v3.dot(e0,e2), v3.dot(e1,e2));






    /*
    const cellMatrix = [1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1];
    dod.rotations.forEach(rot => {
        const mat = m4.multiply(rot, hMatrix);
        mat.forEach(v => cellMatrix.push(v));
    });

    */


    gb.arrays.cellMatrix = {
        numComponents: 16,
        data: [],
        divisor: 1
    };

    const result = gb.createBuffer();
    result.extraData = {
        hMatrix : grid534.hMatrix
    };
    return result;
};

