SlideManager.slides["polydron"] = {
    initialize(engine) {
        const createMaterial = (r,g,b) => this.resourceManager.createStandardMaterial(r,g,b);
        const getGeometry = (name) => this.resourceManager.getGeometry(name);

        this.pieceFamilies = [
            new PolydronPieceFamily(getGeometry("triangle"), createMaterial(0.8,0.2,0.2), 20),
            new PolydronPieceFamily(getGeometry("square"), createMaterial(0.8,0.5,0.2), 6),
            new PolydronPieceFamily(getGeometry("pentagon"), createMaterial(0.8,0.2,0.9), 12),
            new PolydronPieceFamily(getGeometry("decagon"), createMaterial(0.8,0.78,0.1), 12)
        ];
        this.pieces = [];
        const me = this;
        let globalId = 0;
        this.pieceFamilies.forEach((family, i) => {
            family.index = i;
            let pieceId = 0;
            family.pieces.forEach(piece=>{
                piece.globalId = globalId++;
                me.pieces.push(piece);
            });
        });
        this.pieceTotalCount = globalId;

        this.tetrahedronAction = this.makeRegularPolyhedronAction(regularPolyhedra.tetrahedron);
        this.cubeAction = this.makeRegularPolyhedronAction(regularPolyhedra.cube);
        this.octahedronAction = this.makeRegularPolyhedronAction(regularPolyhedra.octahedron);
        this.dodecahedronAction = this.makeRegularPolyhedronAction(regularPolyhedra.dodecahedron);
        this.icosahedronAction = this.makeRegularPolyhedronAction(regularPolyhedra.icosahedron);
        
    },
    start() {
        this.gl.clearColor(0.3,0.6,0.65,1.0);
    },
    draw (engine) {
        
        this.time = performance.now();
        const me = this;
        this.pieceFamilies.forEach(family => {
            engine.setMaterial(family.material);
            engine.setBuffers(family.geometry);    
            family.pieces.forEach(piece => {
                me.executeAction(piece);
                engine.setWorldMatrix(piece.world);
                engine.draw();            
            });    
        });

        //const ph = regularPolyhedra.tetrahedron;
        //const r = PolygonEdge * ph.inRadius / ph.edgeLength;




           

        
        
        /*
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
            / *
                engine.setBuffers(this.vertexArrayInfo);
                twgl.drawBufferInfo(engine.gl, this.vertexArrayInfo, engine.gl.TRIANGLES, 
                this.vertexArrayInfo.numelements, 0, 3);
                // twgl.drawBufferInfo(engine.gl, bufferInfo2, engine.gl.TRIANGLES);
            * /
        });
        */
    },

    baseAction(time, piece) {
        const m4 = twgl.m4;
        const psi = time*0.0002;
        const r = [4.3,1.5,3,7][piece.family.index];
        const t = piece.index / piece.family.pieces.length;
        let world = m4.translation([r,0,12]);
        world = m4.multiply(m4.rotationZ(psi + t * Math.PI*2), world);
        piece.world = world;
    },

    makeRegularPolyhedronAction(ph) {
        const m4 = twgl.m4;
        const rotations = ph.rotations;
        const r = PolygonEdge * ph.inRadius / ph.edgeLength;
        

        return function(time, piece) {
            const index = piece.index;
            const psi = time*0.0002;
            let world = m4.translation([0,0,r * 1.05]);
            world = m4.multiply(rotations[index], world);
            world = m4.multiply(m4.rotationY(psi), world);
            piece.world = world;
        }
    },

        /*

        const pieces = this.pieceFamilies[2].pieces; // pentagons
        const ph = regularPolyhedra.dodecahedron;

        ph.rotations.forEach((rot,i) => {
            const piece = pieces[i];

            let world = m4.translation([0,0,r * 1.1]);
            world = m4.multiply(rot, world);
            world = m4.multiply(m4.rotationY(psi), world);

            const oldWorld = piece.world;

            const mat = interpolateRotation(world, oldWorld, param);
            for(let j=12; j<15; j++) mat[j] = world[j] * (1-param) + oldWorld[j] * param;            
            piece.world = mat;
            
        }); 
    },
        */       


    /*

    base(piece) {
        const m4 = twgl.m4;
        const psi = performance.now()*0.0002;
        const me = this;
        
            const m = family.pieces.length;

            family.pieces.forEach((piece,j) => {
                const t = j /m;
                let world = m4.translation([r,0,12]);
                world = m4.multiply(m4.rotationZ(psi + t * Math.PI*2), world);
                piece.world = world;
            });    
        });

    },
    */
    executeAction(piece) {
        const action = piece.action;
        const time = this.time;
        if(action) {
            if(typeof(action) == "function") {
                action(time, piece);
            } else {
                if(this.time > action.t1) {
                    action.newAction(time, piece);
                    piece.action = action.newAction;
                } else if(this.time <action.t0) {
                    action.oldAction(time, piece);
                } else {
                    const t = (this.time - action.t0) / ( action.t1 - action.t0);
                    action.oldAction(time, piece);
                    const world0 = piece.world;
                    action.newAction(time, piece);
                    const world1 = piece.world;
                    const mat = interpolateRotation(world0, world1, t);
                    for(let j=12; j<15; j++) mat[j] = world0[j] * (1-t) + world1[j] * t;  
                    piece.world = mat;                      
                }
            }
        } else {
            this.baseAction(time, piece);
        }
    },

    myAction() {
        const m4 = twgl.m4;
        const psi = performance.now()*0.0002;
        const param = 0.5*(1+Math.cos(performance.now()*0.005));

        const pieces = this.pieceFamilies[2].pieces; // pentagons
        const ph = regularPolyhedra.dodecahedron;

        const r = PolygonEdge * ph.inRadius / ph.edgeLength;
        ph.rotations.forEach((rot,i) => {
            const piece = pieces[i];

            let world = m4.translation([0,0,r * 1.1]);
            world = m4.multiply(rot, world);
            world = m4.multiply(m4.rotationY(psi), world);

            const oldWorld = piece.world;

            const mat = interpolateRotation(world, oldWorld, param);
            for(let j=12; j<15; j++) mat[j] = world[j] * (1-param) + oldWorld[j] * param;            
            piece.world = mat;
            
        });

    },
    onKeyDown(e) {
        if(e.key == "1") this.setTetrahedron();
        else if(e.key == "2") this.setCube();
        else if(e.key == "3") this.setOctahedron();
        else if(e.key == "4") this.setDodecahedron();
        else if(e.key == "5") this.setIcosahedron();
        console.log(e);
    },

    setAction(piece, action, dt, duration) {
        if(piece.action == null) piece.action = this.baseAction;
        if(action == piece.action) return;
        const t0 = this.time + dt;
        const t1 = t0 + duration;
        piece.action = {
            oldAction : piece.action,
            newAction : action,
            t0,t1
        };
    },

    setCube() {
        const me = this;
        this.pieces.forEach(piece => {
            me.setAction(piece, piece.family.index == 1 ? me.cubeAction : me.baseAction, piece.index*100, 500);
        });
    },

    setDodecahedron() {
        const me = this;
        this.pieces.forEach(piece => {
            me.setAction(piece, piece.family.index == 2 ? me.dodecahedronAction : me.baseAction, piece.index*100, 500);
        });
    },
    setOctahedron() {
        const me = this;
        this.pieces.forEach(piece => {
            me.setAction(piece, piece.family.index == 0 && piece.index<8 ? me.octahedronAction : me.baseAction, piece.index*100, 500);
        });
    },
    setIcosahedron() {
        const me = this;
        this.pieces.forEach(piece => {
            me.setAction(piece, piece.family.index == 0  ? me.icosahedronAction : me.baseAction, piece.index*100, 500);
        });
    },


};

class PolydronPieceFamily {
    constructor(geometry, material, count) {
        this.geometry = geometry;
        this.material = material;
        this.pieces = [];
        const me = this;
        for(let i=0; i<count; i++) {
            this.pieces.push({
                family:me, 
                index:i,
                action: null,
            });
        }
    }
};
