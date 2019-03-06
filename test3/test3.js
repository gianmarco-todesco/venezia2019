
var canvas, engine, scene, camera;

document.addEventListener("DOMContentLoaded", function() {
    canvas = document.getElementById('renderCanvas');
    engine = new BABYLON.Engine(canvas, true, {preserveDrawingBuffer: true});
    scene = new BABYLON.Scene(engine);
    camera = new BABYLON.ArcRotateCamera("Camera", 0, Math.PI / 2, 5, BABYLON.Vector3.Zero(), scene);
    camera.setTarget(BABYLON.Vector3.Zero());
    camera.attachControl(canvas, false);
    camera.wheelPrecision = 50;
    var light = new BABYLON.HemisphericLight('light1', new BABYLON.Vector3(0, 1, 0), scene);
    
    
    


    BABYLON.Effect.ShadersStore["customVertexShader"]=  `
        precision highp float;
        attribute vec3 position;
        attribute vec3 normal;
        attribute vec2 uv;
        uniform mat4 worldViewProjection;
        uniform mat4 hworld;


        varying vec3 vPosition;
        varying vec3 vNormal;
        varying vec2 vUV;

        vec4 toBall(vec4 p) {  

            float k = 5.0 / (1.0 + sqrt(1.0 - (p.x*p.x + p.y*p.y + p.z*p.z)));
            // return vec4(p.xyz * k, 1.0);
            return p*3.0;
        }


        void main() {
            vec4 outPosition = toBall(worldViewProjection * vec4(position, 1.0));
            gl_Position = outPosition;
            vUV = uv;
            vNormal = normal;
            vPosition = position;
        }
    `;              


    BABYLON.Effect.ShadersStore["customFragmentShader"]= `
        precision highp float;

        // Varying
        varying vec3 vPosition;
        varying vec3 vNormal;
        varying vec2 vUV;
        
        // Uniforms
        uniform mat4 world;
        
        // Refs
        uniform vec3 cameraPosition;
    

        void main(void) {
            vec3 vLightPosition = vec3(0,20,10);
    
            // World values
            vec3 vPositionW = vec3(world * vec4(vPosition, 1.0));
            vec3 vNormalW = normalize(vec3(world * vec4(vNormal, 0.0)));
            vec3 viewDirectionW = normalize(cameraPosition - vPositionW);
            
            // Light
            vec3 lightVectorW = normalize(vLightPosition - vPositionW);
            vec3 color = vec3(0.5,0.8,0.9);
            
            // diffuse
            float ndl = max(0., dot(vNormalW, lightVectorW));
            
            // Specular
            vec3 angleW = normalize(viewDirectionW + lightVectorW);
            float specComp = max(0., dot(vNormalW, angleW));
            specComp = pow(specComp, max(1., 64.)) * 0.3;
            
            gl_FragColor = vec4(color * ndl + vec3(specComp), 1.);            
        }
    `;

    var shaderMaterial = new BABYLON.ShaderMaterial("shader", scene, {
            vertex: "custom",
            fragment: "custom",
        },
        {
            attributes: ["position", "normal", "uv"],
            uniforms: ["hworld", "world", "worldView", "worldViewProjection", "view", "projection"]
        });
    //shaderMaterial.setFloat("time", 0);
    shaderMaterial.setVector3("cameraPosition", BABYLON.Vector3.Zero());
    shaderMaterial.backFaceCulling = false;

    var hmatrix = BABYLON.Matrix.Identity();
    shaderMaterial.setMatrix("hworld", hmatrix);

    window.shaderMaterial = shaderMaterial;

    var mat = shaderMaterial;

    var dummy = window.dummy = new BABYLON.Mesh("dummy", scene);
    
    function addEdge(p0,p1) {
        var pts = [];
        var m = 30;
        for(var i = 0; i<m; i++) {
            pts.push(BABYLON.Vector3.Lerp(p0,p1,i/(m-1)));
        }
        const tube = BABYLON.MeshBuilder.CreateTube("edge", {
            path: pts,
            radius: 0.02,
        }, scene);
        tube.material = mat;   
        tube.parent = dummy; 
    }

    const r = 0.7/Math.sqrt(3.0);

    var pts = [
        new BABYLON.Vector3(-r,-r,-r),
        new BABYLON.Vector3(-r,-r, r),
        new BABYLON.Vector3(-r, r,-r),
        new BABYLON.Vector3(-r, r, r),
        new BABYLON.Vector3( r,-r,-r),
        new BABYLON.Vector3( r,-r, r),
        new BABYLON.Vector3( r, r,-r),
        new BABYLON.Vector3( r, r, r)        
    ];
    [[0,1],[1,3],[3,2],[2,0], [4,5],[5,7],[7,6],[6,4], [0,4],[1,5],[2,6],[3,7]].forEach(e=>{
        addEdge(pts[e[0]], pts[e[1]]);
    });


    /*
    var sphere = BABYLON.MeshBuilder.CreateSphere('sphere1', { diameter: 1.0 }, scene);
    sphere.material = mat;
        
    var torus = BABYLON.MeshBuilder.CreateTorus('torus1', {
        thickness: 0.7, 
        diameter: 3.5,
        tessellation: 40
    }, scene);
    torus.material = mat;

    */

    scene.registerBeforeRender(function(){
        /*
        var t = performance.now();
        var x = Math.cos(t*0.01)*0.1;
        var mat = KPoint.translation(new KPoint(0,0,0), new KPoint(x,0,0));
        shaderMaterial.setMatrix("hworld", mat);
        */
    });



    engine.runRenderLoop(function(){
        scene.render();
    });
    window.addEventListener('resize', function(){
        engine.resize();
    });
});


function foo(x) {
    var mat = KPoint.translation(new KPoint(0,0,0), new KPoint(x,0,0));
    shaderMaterial.setMatrix("hworld", mat);
}


function KPoint(x,y,z) {
    this.x = x;
    this.y = y;
    this.z = z;
}

KPoint.dot = function(a,b) {
    if(!(a instanceof KPoint && b instanceof KPoint)) throw "Expected KPoints";
    return a.x*b.x+a.y*b.y+a.z*b.z-1;
}
KPoint.midPoint = function(a,b) {
    if(!(a instanceof KPoint && b instanceof KPoint)) throw "Expected KPoints";
    const aa = KPoint.dot(a,a);
    const bb = KPoint.dot(b,b);
    const ab = KPoint.dot(a,b);
    const sa = Math.sqrt(bb*ab);
    const sb = Math.sqrt(aa*ab);
    return new KPoint(
        a.x*sa+b.x*sb,
        a.y*sa+b.y*sb,
        a.z*sa+b.z*sb        
    );
}

KPoint.reflection = function(p) {
    if(!(p instanceof KPoint)) throw "Expected KPoint";
    const s = -2.0/KPoint.dot(p,p);
    return BABYLON.Matrix.FromValues(
        1.0 + p.x*p.x*s, p.x*p.y*s, p.x*p.z*s, -p.x*s,
        p.y*p.x*s, 1.0 + p.y*p.y*s, p.y*p.z*s, -p.y*s,
        p.z*p.x*s, p.z*p.y*s, 1.0 + p.z*p.z*s, -p.z*s,
        p.x*s, p.y*s, p.z*s, 1.0 - s
    );
}


KPoint.translation = function(a,b) {
    const m = KPoint.midPoint(a,b);
    return KPoint.reflection(m).multiply(KPoint.reflection(a));
}
