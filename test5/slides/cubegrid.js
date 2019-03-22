"use strict";


SlideManager.slides["cube-grid"] = {
    initialize : function(engine) {

        const material = new Material(engine.resourceManager.getShaderProgram("instanced"));
        material.uniforms.u_diffuseColor = [0.7,0.7,0.7,1.0];        
        this.material = material;
    },
    start: function() {
        console.log("start----------------------");
        this.gl.clearColor(0,0,0,1);
        if(!this.title) this.title = createTitle();        
    },
    stop: function() {
        console.log("stop----------------------");
        const title = this.title;
        this.title = null;
        if(title) {
            title.style.transition="transform 1s ease-in-out";
            title.style.transform="translate(400px,0)";
            setTimeout(()=>{ title.parentNode.removeChild(title); }, 1000);    
        }

    },
    draw : function(engine) {

        const m4 = twgl.m4;

        engine.setMaterial(this.material);
        const bufferInfo = engine.resourceManager.getGeometry("cube-grid-cell");


        engine.setBuffers(bufferInfo);
        
        let world = m4.rotationY(performance.now()*0.0001);
        world = m4.multiply(m4.rotationX(0.01), world);


        
        engine.setWorldMatrix(world);    
        /*
        engine.gl.drawElements(
            engine.gl.TRIANGLES, 
            bufferInfo.numElements, 
            engine.gl.UNSIGNED_SHORT, 
            0);
        */
        const m = 7;
        const instanceCount = 1+m*(6+m*(6+8*m));
        twgl.drawBufferInfo(engine.gl, bufferInfo, engine.gl.TRIANGLES, 
           bufferInfo.numelements, 0, instanceCount);

        engine.gl.vertexAttribDivisor(0,0);

    },
};


function createTitle() {
    var elem = document.createElement("div");
    /*
    elem.innerHTML = `
    <h1>Hyperbolic Honeycomb</h1>
    <h2>Gian Marco Todesco</h2>
    <p>From errors to ...</p>
    `;
    */
   elem.innerHTML = `
   <div class="uplayer">
   <img src="images/cubic-space-division.jpg" height="200px"/>
   <div style="color:white">
   <p>Cubic space division</p>
   <p>M.C.Escher 1953</p>
   </div>
   </div>
   `;

    document.body.appendChild(elem);
    elem.style.position = "absolute";
    elem.style.left = "100px";
    elem.style.top = "100px";
    return elem;
}