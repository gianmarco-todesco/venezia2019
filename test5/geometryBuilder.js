"use strict";


class GeometryBuilder {

    constructor(gl) {
        this.gl = gl;
        this.matrixStack = [twgl.m4.identity()];
        this.arrays = {
            position:[],
            normal:[],
            texcoord: [],
            indices:[]
        };
        this.vCount = 0;
    }

    getTopMatrix() { return this.matrixStack[this.matrixStack.length-1]; }

    setNormal(x,y,z) {
        this.normal = twgl.m4.transformNormal(this.getTopMatrix(), twgl.v3.create(x,y,z));
    }

    addVertex(x,y,z, u,v) {
        const p = twgl.m4.transformPoint(this.getTopMatrix(), twgl.v3.create(x,y,z));
        this.arrays.position.push(p[0],p[1],p[2]);
        this.arrays.normal.push(this.normal[0],this.normal[1],this.normal[2]);
        this.arrays.texcoord.push(u,v);
        const index = this.vCount++;
        return index;
    }

    
    pushMatrix() {  this.matrixStack.push(twgl.m4.copy(this.getTopMatrix())); }
    popMatrix() { this.matrixStack.pop(); };

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
        const uvs = [0,0,1,0,1,1,0,1];
        for(var i=0; i+11<v.length; i+=12) {
            const k = this.vCount;
            for(var j=0;j<4;j++) this.addVertex(v[i+j*3], v[i+j*3+1], v[i+j*3+2], uvs[j*2], uvs[j*2+1]);
            this.addQuadIndices(k,k+1,k+2,k+3);
        }
    }
    
    createBuffer() {
        return twgl.createBufferInfoFromArrays(this.gl, this.arrays);
    }
}



GeometryBuilder.createCubeArray = function(gl, r) {

    const arrays = {
        position: [r, r, -r, r, r, r, r, -r, r, r, -r, -r, -r, r, r, -r, r, -r, -r, -r, -r, -r, -r, r, -r, r, r, r, r, r, r, r, -r, -r, r, -r, -r, -r, -r, r, -r, -r, r, -r, r, -r, -r, r, r, r, r, -r, r, r, -r, -r, r, r, -r, r, -r, r, -r, r, r, -r, r, -r, -r, -r, -r, -r],
        normal:   [1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1],
        texcoord: [1, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1],
        indices:  [0, 1, 2, 0, 2, 3, 4, 5, 6, 4, 6, 7, 8, 9, 10, 8, 10, 11, 12, 13, 14, 12, 14, 15, 16, 17, 18, 16, 18, 19, 20, 21, 22, 20, 22, 23],
    };

    return arrays;
};




GeometryBuilder.createThickFaceArray = function(gl, n, r, h) {
    const gb = new GeometryBuilder(gl);

    const cssn = [];
    const pts = [];
    let i;

    for( i=0;i<n;i++) {        
        let phi = 2*Math.PI*i/n, cs = Math.cos(phi), sn = Math.sin(phi);
        cssn.push(cs,sn);
        pts.push(r*cs, r*sn);
    }


    gb.setNormal(0,0,1);
    gb.addVertex(0,0,h,0.5,0.5);
    for( i = 0; i<n; i++) gb.addVertex(pts[i*2],pts[i*2+1],h,cssn[i*2],cssn[i*2+1]);        
    for( i = 0; i<n; i++) gb.addTriangleIndices(0,1+i,1+((i+1)%n));
    gb.setNormal(0,0,-1)
    gb.addVertex(0,0,-h,0.5,0.5);
    for( i = n-1; i>=0; i--) gb.addVertex(pts[i*2],pts[i*2+1],-h,cssn[i*2],cssn[i*2+1]);        
    for( i = 0; i<n; i++) gb.addTriangleIndices(n+1,n+2+i,n+2+((i+1)%n));

    for( i = 0; i<n; i++) {
        let phi = 2*Math.PI*(i+0.5)/n, cs = Math.cos(phi), sn = Math.sin(phi);
        gb.setNormal(cs,sn,0);
        const j = (i+1)%n;
        gb.addQuads([
            pts[i*2],pts[i*2+1],h,
            pts[i*2],pts[i*2+1],-h,
            pts[j*2],pts[j*2+1],-h,
            pts[j*2],pts[j*2+1],h
        ]);
    }

    return gb.arrays;
}


    /*
    return arrays;
        instancePos: {
            numComponents: 4,
            data: [0,0,0,1, 1,1,1,1, 2,2,2,1],
            divisor: 1
        }
      };
    }




    const bufferInfo = twgl.createBufferInfoFromArrays(gl, arrays);
GeometryBuilder
}
*/
