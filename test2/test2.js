var canvas, engine, scene, camera;
var meter;

var vertexMat, edgeMat;


var A,B;



document.addEventListener("DOMContentLoaded", function() {
    meter = new FPSMeter({graph:   1});


    canvas = document.getElementById("renderCanvas"); 
    engine = new BABYLON.Engine(canvas, true); 
    scene = new BABYLON.Scene(engine);
    scene.clearColor.set(0,0,0);
    camera = new BABYLON.ArcRotateCamera("Camera", 
        0.3, 0.3, 3, 
        new BABYLON.Vector3(0,0,0), scene);
    camera.attachControl(canvas, true);
    camera.wheelPrecision = 50;
    camera.minZ = 0.01;

    var light1 = new BABYLON.HemisphericLight("light1", new BABYLON.Vector3(1, 1, 0), scene);
    var light2 = new BABYLON.PointLight("light2", new BABYLON.Vector3(0, 0, 0), scene);
    light2.parent = camera;


    createAxes(1);
    createSphere(1);

    vertexMat = new BABYLON.StandardMaterial("vertexMat", scene);
    vertexMat.diffuseColor = new BABYLON.Color3(0.9, 0.2, 0.2);
    
    edgeMat = new BABYLON.StandardMaterial("edgeMat", scene);
    edgeMat.diffuseColor = new BABYLON.Color3(0.3, 0.4, 0.4);
    
    createModel();

    engine.runRenderLoop(function () { meter.tickStart(); scene.render(); meter.tick(); });
    window.addEventListener("resize", function () { engine.resize(); });
});

function computeRadius() {
    var m = 4; // voglio 4 dodecaedri attorno ad ogni spigolo
    const dod = regularPolyhedra.dodecahedron;
    const phi = Math.acos(BABYLON.Vector3.Dot(
        dod.centers[0].clone().normalize(), 
        dod.centers[1].clone().normalize()));
    // phi è il supplementare dell'angolo diedro del dod
    
    const theta = Math.acos(BABYLON.Vector3.Dot(
        dod.vertices[0], 
        dod.vertices[1]));
    // theta = angolo sotteso da un lato
    
    const d1 = Math.acosh(1.0/(Math.tan(phi*0.5)*Math.tan(Math.PI/m)));
    // distanza (iperbolica) fra il centro e il punto medio di uno spigolo
    // uso : cosh(hyp) = cot(A)*cot(B)
    console.log(d1);

    const d2 = Math.atanh(Math.tanh(d1)/Math.cos(theta/2));
    // distanza (iperbolica) fra il centro e un vertice
    // uso: cos(A) = tanh(b)/tanh(c)
    console.log(d2);

    
    const tmp = Math.exp(2*d2);
    const d3 = (tmp-1)/(tmp+1);
    // distanza (euclidea) dal centro di un punto nel modello di Klein
    // che abbia d2 come distanza iperbolica.
    // uso la formula per la dist. hyp. del modello di klein: 
    // d=1/2log((aq*pb)/(ap*qb)), con d=d2, ap=pb=1, pq=d3, qb=1-d3
    console.log("d3=",d3);

    return d3;
}

function createModel() {
    const builder = new Builder();

    const D = computeRadius();
    const dod = regularPolyhedra.dodecahedron;

    
    const model = builder.createHPolyhedronModel(dod, D);

    const p0 = model.pts[0];
    console.log("p0:", KPoint.distance(p0, new KPoint(0,0,0) ));
    console.log("p0(h):", HPoint.distance(p0.toHPoint(), new HPoint(0,0,0) ));
    
    console.log("check:", 0.5*Math.log((1+0.842)/(1-0.842)));


    
    
    builder.createHPolyhedron(model, BABYLON.Matrix.Identity());
    
    
    builder.createVertex(model.pts[0]);
    builder.createVertex(model.pts[1]);

    // per ragioni di simmetria il punto medio di un segmento
    // dovrebbe coincidere con quello euclideo
    const pm = new KPoint(
        (model.pts[0].x + model.pts[1].x)*0.5,
        (model.pts[0].y + model.pts[1].y)*0.5,
        (model.pts[0].z + model.pts[1].z)*0.5
    );
    builder.createVertex(pm);
    console.log(HPoint.distance(new HPoint(0,0,0), pm.toHPoint()));

    let matrix = KPoint.translation(new KPoint(0,0,0),model.fpts[0]);
    matrix = BABYLON.Matrix.RotationZ(Math.PI/5).multiply(matrix);
    

    // builder.createHPolyhedron(model, matrix);

    
    var rot = new BABYLON.Matrix();

    window.model = model;
    /*
    model.rotations[1].toRotationMatrix(rot);
    builder.createHPolyhedron(model, rot.multiply(matrix));
    */

    /*
    for(var i=0; i<12;i++) {
        model.rotations[i].toRotationMatrix(rot);
        builder.createHPolyhedron(model, matrix.multiply(rot));     
    }
    */

    model.rotations[0].toRotationMatrix(rot);
    var matrix1 = matrix.multiply(rot);
    builder.createHPolyhedron(model, matrix1);     

    model.rotations[7].toRotationMatrix(rot);
    var matrix2 = matrix.multiply(rot);
    
    builder.createHPolyhedron(model, 
        matrix2.multiply(matrix1));

    model.rotations[1].toRotationMatrix(rot);
    var matrix3 = matrix.multiply(rot);
    builder.createHPolyhedron(model, matrix3);     
            

    /*
    let i=0;
    model.rotations.forEach(R=>{
        console.log("n",i++);
        var rot = new BABYLON.Matrix();
        R.toRotationMatrix(rot);
        var ph = builder.createHPolyhedron(model, matrix);

        ph.rotationQuaternion = R;


        // builder.createHPolyhedron(model, matrix.multiply(rot));

    });
    */
    
    const boxMat = new BABYLON.StandardMaterial("boxmat", scene);
    boxMat.diffuseColor.set(0.2,0.4,0.5);

    /*
    [1,2].forEach(i=>{
        var box = new BABYLON.MeshBuilder.CreateBox("box", {size:0.2}, scene);
        box.material = boxMat;
        box.position.set(0.1,0.1,0.5);
        box.setPivotPoint(box.position.scale(-1));
        // box.rotation.y = Math.PI/2;
        box.rotationQuaternion = model.rotations[i];

    });
*/
    /*
    model.rotations.forEach(q=>{
        var box = new BABYLON.MeshBuilder.CreateBox("box", {size:0.2}, scene);
        box.material = boxMat;
        box.position.set(0.1,0.1,0.5);
        box.setPivotPoint(box.position.scale(-1));
        // box.rotation.y = Math.PI/2;
        box.rotationQuaternion = q;
    }); 
    */


    /*
    builder.createHPolyhedron(model, KPoint.translation(new KPoint(0,0,0),model.fpts[0]));
    
    let p = new KPoint(0,0,0.25);
    p = model.pts[0];
    builder.createHPolyhedron(model, KPoint.reflection(p));
    */

}

function Builder() {
    this.matrix = BABYLON.Matrix.Identity();
}

Builder.prototype.toBabylon = function(p) {
    if(!(p instanceof KPoint)) throw "Expected KPoint";
    var pb = p.transformed(this.matrix).toBPoint();
    const v = new BABYLON.Vector3(pb.x,pb.y,pb.z);
    return v;
}


Builder.prototype.createVertex = function(hp) {
    var sph = BABYLON.MeshBuilder.CreateSphere("s", {diameter:0.02}, scene);
    sph.position.copyFrom(this.toBabylon(hp));
    sph.material = vertexMat;
    sph.parent = this.mesh;
    return sph;
}

Builder.prototype.createEdge = function(kpa, kpb, m) {
    var kp = new KPoint(0,0,0);
    var pts = [];
    for(var i=0; i<m; i++) {
        const t = i/(m-1);
        const it = 1.0-t;
        kp.x = kpa.x*it+kpb.x*t;
        kp.y = kpa.y*it+kpb.y*t;
        kp.z = kpa.z*it+kpb.z*t;
        pts.push(this.toBabylon(kp));        
    }
    const tube = BABYLON.MeshBuilder.CreateTube("edge", {
            path: pts,
            radius: 0.005,
        }, scene);
    tube.material = edgeMat;
    tube.parent = this.mesh;
    // mat.wireframe = true;
    return tube;    
}

Builder.prototype.createHPolyhedronModel = function(data, r) {
    if(r>1.0) throw "Radius must be <= 1.0"; 
    const p0 = data.vertices[0];
    const sc = r;
    const model = {};
    model.rotations = data.rotations;
    const pts = model.pts = [];
    data.vertices.forEach(v => {
        const hp = new KPoint(sc*v.x,sc*v.y,sc*v.z);
        pts.push(hp);
    });
    model.fpts = data.centers.map(p=>new KPoint(sc*p.x,sc*p.y,sc*p.z));
    const edges = model.edges = [];
    data.edges.forEach(e=>{
        const [a,b] = e;
        edges.push({a:a,b:b});
    });
    return model;        
}

Builder.prototype.createHPolyhedron = function(model, matrix) {
    this.matrix = matrix;
    this.mesh = new BABYLON.Mesh("pivot", scene);
    const me = this;
    // model.pts.forEach(p=>{ me.createVertex(p); });
    // model.fpts.forEach(p=>{ me.createVertex(p); });
    // model.fpts.forEach(p=>{ me.createVertex(p); });
    model.edges.forEach(e=> { me.createEdge(model.pts[e.a], model.pts[e.b],20)});
    
    
    return this.mesh;
}



