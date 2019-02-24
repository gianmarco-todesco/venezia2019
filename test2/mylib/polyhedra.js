
function PolyhedronData(pts, faces) {
    const p0 = new BABYLON.Vector3(pts[0][0],pts[0][1],pts[0][2]);
    const sc = 1.0/p0.length();
    this.vertices = pts.map(c=>new BABYLON.Vector3(sc*c[0],sc*c[1],sc*c[2]));
    this.faces = faces;
    const edges = this.edges = [];
    const edgesTb = {};
    this.centers = [];
    this.rotations = [];
    const me = this;
    faces.forEach(f=>{

        const m = f.length;
        for(var i=0;i<m;i++) {
            const a = f[i];
            const b = f[(i+1)%m];
            const abId = a>b ? a+"_"+b : b+"_"+a;
            if(!edgesTb[abId]) { edgesTb[abId] = [a,b]; edges.push([a,b]); }; 
        }

        const fPts = f.map(i=>me.vertices[i]);
        const pc = fPts.reduce((a,b)=>a.add(b)).scale(1.0/m);
        me.centers.push(pc);

        const pa = fPts[0];
        const e0 = pa.subtract(pc).normalize();
        const e2 = pc.clone().normalize();
        const e1 = BABYLON.Vector3.Cross(e2,e0);
        const vrot = BABYLON.Vector3.RotationFromAxis(e0,e1,e2);
        const orientation = BABYLON.Quaternion.RotationQuaternionFromAxis(e0,e1,e2);
        // const orientation = BABYLON.Quaternion.FromEulerVector(vrot);
        this.rotations.push(orientation);


    });

}

const regularPolyhedra = {
    dodecahedron : new PolyhedronData(
        [
            [0.607062,0,0.794654], [0.982247,0,0.187592], [0.187592,0.57735,0.794654], [0.303531,0.934172,0.187592],
            [-0.491124,0.356822,0.794654], [-0.794655,0.57735,0.187592], [-0.491123,-0.356822,0.794654], [-0.794654,-0.577351,0.187592], 
            [0.187593,-0.57735,0.794654], [0.303532,-0.934172,0.187592], [-0.303532,0.934172,-0.187592], [-0.187593,0.57735,-0.794654], 
            [0.794654,0.577351,-0.187592], [0.491123,0.356822,-0.794654], [0.794655,-0.57735,-0.187592], [0.491124,-0.356822,-0.794654], 
            [-0.303531,-0.934172,-0.187592], [-0.187592,-0.57735,-0.794654], [-0.982247,0,-0.187592], [-0.607062,0,-0.794654]
        ],
        [
            [0,2,4,6,8], [2,0,1,12,3], [0,8,9,14,1], [8,6,7,16,9], [6,4,5,18,7], [4,2,3,10,5], 
            [3,12,13,11,10], [1,14,15,13,12], [9,16,17,15,14], [7,18,19,17,16], [5,10,11,19,18], [11,13,15,17,19]
        ]        
    )
};


