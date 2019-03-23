"use strict";

SlideManager.slides["surfaces"] = {

    initialize : function(engine) {
        this.phi = 0.0;
        this.theta = 0.0;

        this.vertexMaterial = this.resourceManager.createStandardMaterial(0.8,0.3,0.1);
        this.computeWorldMatrix();
        this.grid = new Grid();
    },

    draw : function(engine) {
        const m4 = twgl.m4;
        engine.drawAxes(this.world);


        this.drawGrid();

        

        /*
        this.drawVertices([
            [0,0,0],
            [1,0,0],
            [0,1,0],
            [1,1,1]
        ]);

        this.drawEdges([
            [[0,0,0],[1,0,0]],
            [[0,0,0],[0,1,0]],
            [[0,0,0],[1,1,1]],            
        ]);
        */

        /*
        const pts = [];
        for(let i=0; i<10; i++) {
            for(let j=0;j<10;j++) {
                pts.push([i+j*0.5,j*Math.sqrt(3)/2,0]);
            }
        }

        engine.setUniforms({u_diffuseColor: [0.3,0.1,0.1,1]});
        this.drawVertices(pts);

        const edges = [];
        for(let i=0; i+1<10; i++) {
            for(let j=0;j+1<10;j++) {
                edges.push([ [i+j*0.5,j*Math.sqrt(3)/2,0], [i+1+j*0.5,j*Math.sqrt(3)/2,0]]);
                edges.push([ [i+j*0.5,j*Math.sqrt(3)/2,0], [i+(j+1)*0.5,(j+1)*Math.sqrt(3)/2,0]]);
            }
        }
        engine.setUniforms({u_diffuseColor: [0.1,0.4,0.8,1]});
        this.drawEdges(edges);
*/



    },

    drawGrid() {
        this.grid.evolve();
        const engine = this.engine;
        engine.setMaterial(this.vertexMaterial);
        engine.setUniforms({u_diffuseColor: [0.3,0.1,0.1,1]});
        this.drawVertices(this.grid.vertices);

        engine.setUniforms({u_diffuseColor: [0.1,0.4,0.8,1]});
        this.drawEdges(this.grid.edges.map(e=>[e[0].pos, e[1].pos]));
        

        engine.setUniforms({u_diffuseColor: [0.1,0.9,0.1,1]});
        engine.setWorldMatrix(twgl.m4.identity());

        
        this.drawTriangles(this.grid.faces.map(face=>[face[0].pos,face[1].pos,face[2].pos]));
        



    },

    computeWorldMatrix() {
        const m4 = twgl.m4;
        this.world = m4.multiply(m4.rotationX(this.theta), m4.rotationY(this.phi));
    },

    drawVertices(vertices) {
        const m4 = twgl.m4;
        const r = 0.05;
        const engine = this.engine;
        const scaling = m4.scaling([r,r,r]);
        const world = this.world;

        const colors = {
            5 : [0.2,0.2,0.8,1.0],
            6 : [0.4,0.4,0.4,1.0],
            7 : [0.9,0.1,0.9,1.0],
        };

        engine.setBuffers(this.resourceManager.getGeometry("sphere"));
        vertices.forEach(vertex => {
            const mat = m4.multiply(m4.translation(vertex.pos), scaling);
            engine.setWorldMatrix(m4.multiply(world, mat));
            let color = colors[vertex.maxDegree] || [0.8,0.8,0.8,1.0];
            engine.setUniforms({u_diffuseColor: color});
            engine.draw(); 
        });
    },

    drawEdges(edges) {
        const m4 = twgl.m4;
        const v3 = twgl.v3;
        const r = 0.015;
        const engine = this.engine;
        const world = this.world;
        engine.setBuffers(this.resourceManager.getGeometry("cylinder"));
        edges.forEach(e => {
            const p0 = e[0];
            const p1 = e[1];
            const p01 = v3.subtract(p1,p0);
            const d = v3.length(p01);
            
            const pm = v3.lerp(p0,p1,0.5);
            const mat = createAlignMatrix(pm, v3.divScalar(p01,d));
            const scaling = m4.scaling([r,d,r]);
            engine.setWorldMatrix(m4.multiply(world, m4.multiply(mat, scaling)));
            engine.draw();             
        });
    },

    drawTriangles(triangles) {
        const bufferSize = 500; // max numero di facce che entrano nel buffer
        const v3 = twgl.v3;


        if(!this.triangleGeometry) {
            const arrays = this.triangleGeometryArrays = {
                position : new Float32Array(bufferSize*18),
                normal : new Float32Array(bufferSize*18),
                uvs : new Float32Array(bufferSize*12),
                indices : [],
            };
            for(let i=0; i<bufferSize; i++) {
                [0,0, 1,0, 1,1,  0,0, 1,0, 1,1].forEach((v,j)=>arrays.uvs[12*i+j]=v);
                const k = i*6;
                arrays.indices.push(k,k+1,k+2, k+3,k+5,k+4);
            } 
            this.triangleGeometry = twgl.createBufferInfoFromArrays(this.gl, arrays);
        }
        
        const arrays = this.triangleGeometryArrays;

        const thickness = 0.001;
        let k = 0;

        const n = Math.min(bufferSize, triangles.length);
        const positionBuffer = arrays.position;
        const normalBuffer = arrays.normal;
        
        for(let i=0; i<n; i++) {
            const triangle = triangles[i];
            const [p0,p1,p2] = triangle;
            const norm = v3.normalize(v3.cross(v3.subtract(p1,p0), v3.subtract(p2,p0)));
            const offset = v3.mulScalar(norm, thickness * 0.5);
            const upFace = [p0,p1,p2].map(p=>v3.add(p,offset));
            const dnFace = [p0,p1,p2].map(p=>v3.subtract(p, offset));
            const dnNorm = v3.mulScalar(norm, -1);
            const k3 = i*18;
            positionBuffer.set(upFace[0], k3);
            positionBuffer.set(upFace[1], k3+3);
            positionBuffer.set(upFace[2], k3+6);
            positionBuffer.set(dnFace[0], k3+9);
            positionBuffer.set(dnFace[1], k3+12);
            positionBuffer.set(dnFace[2], k3+15);
            for(let j=0;j<3;j++) normalBuffer.set(norm,k3+3*j);
            for(let j=0;j<3;j++) normalBuffer.set(dnNorm,k3+3*j+9);
        }
        const attribs = this.triangleGeometry.attribs;
        twgl.setAttribInfoBufferFromArray(this.gl, attribs.position, positionBuffer);
        twgl.setAttribInfoBufferFromArray(this.gl, attribs.normal, normalBuffer);

        this.engine.setWorldMatrix(this.world);
        this.engine.setBuffers(this.triangleGeometry);
        twgl.drawBufferInfo(this.gl, this.triangleGeometry, this.gl.TRIANGLES, n*6);
    },


    onDrag(dx,dy) {
        this.phi += dx*0.005;
        this.theta += dy*0.005;
        this.computeWorldMatrix();
    },

    onKeyDown(e) {
        if(e.key=="+") this.grid.grow();
        else if(e.key=="1") this.grid.reset(1);
        else if(e.key=="2") this.grid.reset(2);
        else if(e.key=="3") this.grid.reset(3);
        else if(e.key=="4") this.grid.reset(4);
    }
};



class Grid {
    constructor() {
        window.grid = this;
        this.type = 1;
        this.edgeLength = 0.5;
        this.nextVertexId = 0;

        this.vertices = [];
        this.faces = [];
        this.edges = [];

        this.reset(1);
    }

    reset(type) {
        if(type<1 || type>4) type=1;
        this.type = type;
        this.vertices = [];
        this.faces = [];
        this.edges = [];
        this.degrees = [[5,5],[5,5],[6,6],[5,6],[7,6]][type];
        this.addFirstTriangle();
    }


    addVertex(p, maxDegree) {
        const id = this.nextVertexId++;
        const v = {
            pos : p,
            maxDegree, 
            id, 
            links : []
        };
        this.vertices.push(v);
        return v;
    }




    addFirstTriangle() {
        const r = 0.5*this.edgeLength/Math.sin(Math.PI/3);

        
        for(let i=0; i<3; i++) {
            const phi = Math.PI*2*i/3;
            const p = [Math.cos(phi)*r, 0, Math.sin(phi)*r];
            this.addVertex(p, this.degrees[i==0 ? 0 : 1]);
        }
        for(let i=0; i<3; i++) {
            const vi = this.vertices[i];
            vi.links.push(this.vertices[(i+1)%3], this.vertices[(i+2)%3]);
            for(let j=i+1; j<3; j++) {
                const edge = [vi,this.vertices[j]];
                this.edges.push(edge);                
            }
        }
        this.faces.push([0,1,2].map(i=>this.vertices[i]));
    }

    addTriangle(v1,v2,p) {
        assert(v1 && v2 && v1.links && v2.links && 
            v1.links[v1.links.length-1]==v2 &&
            v2.links[0] == v1, "addTriangle");
        const degs = [[]]
        const maxDegree = this.degrees[v1.maxDegree == 6 && v2.maxDegree == 6 ? 0 : 1];

        const v = this.addVertex(p, maxDegree); v.links.push(v1,v2);
        v1.links.push(v);
        v2.links.splice(0,0,v);
        this.edges.push([v,v1]);
        this.edges.push([v,v2]);
        this.faces.push([v1,v2,v]);        
    }

    closeTriangle(v1,v2) {
        assert(v1 && v2 && v1.links && v2.links && 
            v1.links[v1.links.length-1]==v2.links[0], "closeTriangle");   
        const v = v2.links[0];
        v.closed = true;
        v1.links.push(v2);
        v2.links.splice(0,0,v1);
        this.edges.push([v1,v2]);
        this.faces.push([v2,v1,v]); 
        
    }

    growTriangle(v1,v2) {
        const v3 = twgl.v3;

        assert(v1 && v2 && v1.links && v2.links && 
            v1.links[v1.links.length-1]==v2 &&
            v2.links[0] == v1, "growTriangle");
        
        const other = v2.links[1];
        const e0 = v3.normalize(v3.subtract(v2.pos, v1.pos));
        let e1 = v3.subtract(v1.pos, other.pos);
        e1 = v3.normalize(v3.subtract(e1, v3.mulScalar(e0, v3.dot(e0,e1))));
        
        const pm = v3.lerp(v1.pos,v2.pos,0.5);
        const height = this.edgeLength * Math.sqrt(3) / 2;
        const p = v3.add(pm, v3.mulScalar(e1, height));
        
        this.addTriangle(v1,v2, v3.add(p, this.rndPoint(0.02)));
    }

    grow() {
        let best = null;
        let maxDegree = 0;
        this.vertices.forEach(vertex => {
            const m = vertex.links.length;
            if(!vertex.closed) {
                if(best == null || m>maxDegree) { maxDegree = m; best = vertex}
            }
        });
        if(!best) return;

        const v = best;
        const m = v.links.length;
        if(m == v.maxDegree) {
            this.closeTriangle(v.links[0], v.links[m-1]);
        } else {
            const other0 = v.links[0], other1 = v.links[m-1];
            if(other0.links.length > other1.links.length) {
                this.growTriangle(other0, v);        
            } else {
                this.growTriangle(v, other1);        
            }
    
        }

        this.vertices.forEach(vertex => {
            if(!vertex.closed && vertex.links.length == vertex.maxDegree) {
                const m = vertex.links.length;
                this.closeTriangle(vertex.links[0], vertex.links[m-1]);
            }
        });

        console.log(this.faces.length," faces");

    }

    rndPoint(r) {
        return [
            r*(Math.random()*2.0-1.0),
            r*(Math.random()*2.0-1.0),
            r*(Math.random()*2.0-1.0)            
        ];
    }

    evolve() {
        const v3 = twgl.v3;
        const me = this;
        for(let iter = 0; iter<2; iter++) {
            this.vertices.forEach(vertex => {
                const oldPos = vertex.pos;
                const energy1 = me.computeEnergy(vertex);            
                vertex.pos = v3.add(oldPos, this.rndPoint(0.01));
                const energy2 = me.computeEnergy(vertex);
                const delta = energy2 - energy1;
                if(delta > 0.0 && Math.exp(-delta*10) < Math.random()) 
                    vertex.pos = oldPos;
            });    
        }

        const center = v3.create(0,0,0);
        this.vertices.forEach(vertex => v3.add(center,vertex.pos,center));
        v3.divScalar(center, this.vertices.length, center);

        this.vertices.forEach(vertex => v3.subtract(vertex.pos, center, vertex.pos));                
    }

    evolve_old() {
        const v3 = twgl.v3;
        const me = this;
        this.vertices.forEach(vertex => {
            const p0 = vertex.pos;
            const force = v3.create(0,0,0);
            vertex.links.forEach(other => {
                const p1 = other.pos;
                const p01 = v3.subtract(p1,p0);
                const dist = v3.length(p01);
                const dir = v3.divScalar(p01, dist);
                const v = (dist - me.edgeLength)*0.001;
                v3.add(force, v3.mulScalar(dir, v), force);    
            });
            for(let j=0; j+1<vertex.links.length;j++) {
                const vrt1 = vertex.links[j];
                const vrt2 = vertex.links[j+1];
                const k = vrt2.links.indexOf(vrt1);
                if(k+1<vrt2.links.length) {
                    // assert vrt2.links[0] == vrt1
                    const vrt3 = vrt2.links[k+1];
                    const p1 = vrt1.pos;
                    const p2 = vrt2.pos;
                    const p3 = vrt3.pos;
                    const normal = v3.normalize(v3.cross(v3.subtract(p1,p0), v3.subtract(p2,p0)));
                    const pm = v3.lerp(p1,p2,0.5);
                    const z = v3.dot(v3.subtract(p3,pm), normal);
                    const v = -z * 0.00001;
                    v3.add(force, v3.mulScalar(normal, v), force);    
                }
                
            }
            vertex.force = force;
            if(!vertex.speed) vertex.speed = v3.create(0,0,0);
            v3.add(v3.mulScalar(vertex.speed, 0.99), vertex.force, vertex.speed);
        });
        this.vertices.forEach(vertex => {
            v3.add(vertex.pos, vertex.speed, vertex.pos);   
            v3.add(vertex.pos, this.rndPoint(0.001), vertex.pos);         
        });
    }


    computeEnergy(vertex) {
        const v3 = twgl.v3;
        let energy = 0;
        const m = vertex.links.length;
        const p0 = vertex.pos;
        const edgeLength = this.edgeLength;

        vertex.links.forEach(other => {
            const dist = v3.distance(p0,other.pos);
            energy += 100.0 * Math.pow(dist-edgeLength,2);
        });
        
        if(vertex.closed) {
            const center = v3.create(0,0,0);
            vertex.links.forEach(other => { 
                v3.add(center, other.pos, center);
            });
            v3.divScalar(center, vertex.links.length, center);
            energy += 10 * v3.distance(center, p0);
        }
                
        const nrms = [];
        const faceCount = vertex.closed ? m : m-1;
        for(let i=0; i<faceCount; i++) {
            const p1 = vertex.links[i].pos;
            const p2 = vertex.links[(i+1)%m].pos;
            const nrm = v3.cross(v3.subtract(p1,p0), v3.subtract(p2,p0));            
            nrms.push(nrm);
        }
        const facePairCount = vertex.closed ? faceCount : faceCount-1;
        for(let i=0; i<facePairCount; i++) {
            const pd = Math.max(1.0, v3.dot(nrms[i], nrms[(i+1)%nrms.length]));
             
            energy += 10.0 * Math.pow((1-pd), 3.0);
        }
        return energy;
    }
}
