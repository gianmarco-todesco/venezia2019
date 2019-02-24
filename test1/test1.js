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

    var light1 = new BABYLON.HemisphericLight("light1", new BABYLON.Vector3(1, 1, 0), scene);
    var light2 = new BABYLON.PointLight("light2", new BABYLON.Vector3(0, 0, 0), scene);
    light2.parent = camera;


    createAxes(1);

    vertexMat = new BABYLON.StandardMaterial("vertexMat", scene);
    vertexMat.diffuseColor = new BABYLON.Color3(0.9, 0.2, 0.2);
    
    edgeMat = new BABYLON.StandardMaterial("edgeMat", scene);
    edgeMat.diffuseColor = new BABYLON.Color3(0.3, 0.4, 0.4);
    
    makeDod(BABYLON.Matrix.Identity());

    var hpb = new BABYLON.Vector4(0.0,0,0,1);
    
    const dodData = regularPolyhedra.dodecahedron;
    var hpa = getFaceCenter(dodData, 0);
    console.log(hpa);

    A = H.translation(hpa,hpb);

    B = BABYLON.Matrix.Identity().clone();
    B.m[10] = -1;



    makeDod(multiplyMatrices(A.clone().invert(), B, A));

    C = getFaceMatrix(dodData,1);
    
    const M1 = multiplyMatrices(C, A.clone().invert(), B, A);
    makeDod(M1);
    makeDod(multiplyMatrices(M1,M1));
    

    var pa = new BABYLON.Vector4(0,0,0,1);
    var pb = getFaceCenter(dodData, 0);
    

    engine.runRenderLoop(function () { meter.tickStart(); scene.render(); meter.tick(); });
    window.addEventListener("resize", function () { engine.resize(); });
});

const sc = 0.85;
function getFaceCenter(ph, i) {
    const A = BABYLON.Matrix.Identity();
    var hpts = [];
    // const sc = 0.95;
    ph.vertices.forEach(p=> {
        var hp = applyMatrix(A, new BABYLON.Vector4(p.x*sc,p.y*sc,p.z*sc,1));
        hpts.push(hp);
    });
    var face = ph.faces[i];
    var c = new BABYLON.Vector4(0,0,0,1);
    for(var i=0;i<face.length;i++) {
        var hp = hpts[face[i]];
        c.x += hp.x/hp.w;
        c.y += hp.y/hp.w;
        c.z += hp.z/hp.w;
    }
    const im = 1.0/face.length;
    c.x *= im;
    c.y *= im;
    c.z *= im;
    return c;
}


function getFaceMatrix(ph, i) {
    var face = ph.faces[i];
    var pts = ph.vertices.map(p=>p.clone());
    var c = new BABYLON.Vector3(0,0,0);
    for(var i=0;i<face.length;i++) {
        c.addInPlace(pts[face[i]]);
    }
    c.scaleInPlace(1.0/face.length);
    var e2 = c.clone().normalize();
    var e0 = pts[face[0]].subtract(c).normalize();
    console.log(e0);
    console.log(BABYLON.Vector3.Dot(e0,e2));
    var e1 = BABYLON.Vector3.Cross(e2,e0).normalize();
    console.log(e0);
    console.log(e1);
    console.log(e2);
    var mat = BABYLON.Matrix.FromValues(
        e0.x,e0.y,e0.z,0,
        e1.x,e1.y,e1.z,0,
        e2.x,e2.y,e2.z,0,
        0,0,0,1
    );
    return mat;
}


function makeDod(A) {
    var dodData = regularPolyhedra.dodecahedron;
    var hpts = [];
    dodData.vertices.forEach(p=>{
        var hp = applyMatrix(A, new BABYLON.Vector4(p.x*sc,p.y*sc,p.z*sc,1));
        hpts.push(hp);
        makeHVertex(hp);
    });
    dodData.edges.forEach(e=>{
        const a = e[0];
        const b = e[1];
        makeHEdge(hpts[a], hpts[b]);
    });


    /*
    
    var face = regularPolyhedra.dodecahedron.faces[0];
    var c = new BABYLON.Vector4(0,0,0,1);
    for(var i=0;i<5;i++) {
        var p = pts[face[i]];
        c.x += 0.2*p.x/p.w;
        c.y += 0.2*p.y/p.w;
        c.z += 0.2*p.z/p.w;
    }
    createHPoint(c);
    var R = H.reflection(c);
    for(var i=0; i<pts.length; i++) {
        var p = H.normalize(H.apply(R, pts[i]));
        pts[i] = p;
    }


    pts.forEach(p=>{createHPoint(p);});
    dodData.edges.forEach(e=>{
        const a = e[0];
        const b = e[1];
        createHLine(pts[a], pts[b]);
    });

    */
}


function makeCubeGrid() {
    var n = 20;
    
    var box = BABYLON.MeshBuilder.CreateBox("box", {size:0.5}, scene);
    box.material = new BABYLON.StandardMaterial("a",scene);
    box.material.diffuseColor.set(0.8,0.5,0.04);
    box.scaling.set((n-1)*2,0.1,0.1);
    var bar = null;
    for(var k=0;k<3;k++) {
        for(var i=0;i<n;i++) {
            for(var j=0;j<n;j++) {
                if(bar) bar = box.createInstance("o"+i+"_"+j+"_"+k);
                else bar = box;
                if(k==0) {
                    bar.position.x = 0;
                    bar.position.y = i-(n-1)/2;
                    bar.position.z = j-(n-1)/2;                    
                } else if(k==1) {
                    bar.rotation.z=Math.PI/2;
                    bar.position.x = i-(n-1)/2;
                    bar.position.y = 0;
                    bar.position.z = j-(n-1)/2;                    
                } else {
                    bar.rotation.y=Math.PI/2;
                    bar.position.x = i-(n-1)/2;
                    bar.position.y = j-(n-1)/2;                    
                    bar.position.z = 0;
                }
            }
        }    
    }

}




var makeTextPlane = function(text, color, size) {
    var dynamicTexture = new BABYLON.DynamicTexture("DynamicTexture", 50, scene, true);
    dynamicTexture.hasAlpha = true;
    dynamicTexture.drawText(text, 5, 40, "bold 36px Arial", color , "transparent", true);
    var plane = BABYLON.Mesh.CreatePlane("TextPlane", size, scene, true);
    plane.material = new BABYLON.StandardMaterial("TextPlaneMaterial", scene);
    plane.material.backFaceCulling = false;
    plane.material.specularColor = new BABYLON.Color3(0, 0, 0);
    plane.material.diffuseTexture = dynamicTexture;
    return plane;
 };

let tube;
let tubeCount = 0;



function makeHVertex(hp) {    
    const sphere = BABYLON.Mesh.CreateSphere(
        'sphere1', 16, 0.05, scene);
    sphere.position.copyFrom(H.H3toR3(hp));
    sphere.material = vertexMat;
    return sphere;
}

function makeHEdge(hpa, hpb) {
    const tubeId = ++tubeCount;
    const myPath = [];
    const n = 100;
    for(let i=0;i<=n;i++) {
        const t = i/n, it = 1-t;
        const hp = new BABYLON.Vector4(
            it*hpa.x+t*hpb.x,
            it*hpa.y+t*hpb.y,
            it*hpa.z+t*hpb.z,
            it*hpa.w+t*hpb.w
        );
        myPath.push(H.H3toR3(hp));
    }
    const tube = BABYLON.MeshBuilder.CreateTube(
        "tube"+tubeId, {
            path: myPath,
            radius: 0.01,
        }, scene);
    tube.material = edgeMat;
    // mat.wireframe = true;
    return tube;
}


/*
function v4(x,y,z,w) { return new BABYLON.Vector4(x,y,z,w); }

toWorld(H.toBall(p.scale(1.0/p.w)));

function createHLine(pa,pb) {

}

function createHPoint(p) {    
    const q = toWorld(H.toBall(p.scale(1.0/p.w)));
    const sphere = BABYLON.Mesh.CreateSphere('sphere1', 16, 0.1, scene);
    sphere.position.copyFrom(q);
    const mat = new BABYLON.StandardMaterial("matp", scene);
    sphere.material = mat;
    mat.diffuseColor = new BABYLON.Color3(1, 0, 0);
    return sphere;
}

*/
