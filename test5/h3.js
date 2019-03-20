
function KPoint(x,y,z) {
    this.x = x;
    this.y = y;
    this.z = z;
}


KPoint.prototype.check = function() {
    const d = this.x*this.x+this.y*this.y+this.z*this.z;
    if(d>=1.0)
        throw "Bad KPoint";
} 

KPoint.dot = function(a,b) {
    if(!(a instanceof KPoint && b instanceof KPoint)) throw "Expected KPoints";
    return a.x*b.x+a.y*b.y+a.z*b.z-1;
}

// attenzione! non sembra funzionare!!
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
    return [
        1.0 + p.x*p.x*s, p.x*p.y*s, p.x*p.z*s, p.x*s,
        p.y*p.x*s, 1.0 + p.y*p.y*s, p.y*p.z*s, p.y*s,
        p.z*p.x*s, p.z*p.y*s, 1.0 + p.z*p.z*s, p.z*s,
        -p.x*s, -p.y*s, -p.z*s, 1.0 - s
    ];
}

/*

KPoint.translation = function(a,b) {
    const m4 = twgl.m4;
    const m = KPoint.midPoint(a,b);
    return m4.multiply(KPoint.reflection(a), KPoint.reflection(m));
}

*/

KPoint.translation = function(a,b) {
    const m4 = twgl.m4;
    const m = KPoint.midPoint(a,b);
    return m4.multiply(KPoint.reflection(a), KPoint.reflection(m));
}

/*
KPoint.snort = function(a) {
    const m4 = twgl.m4;
    return m4.multiply(KPoint.reflection(a), KPoint.reflection(m));
}
*/



function computeGrid534Radius() {
    const v3 = twgl.v3;

    var m = 4; // voglio 4 dodecaedri attorno ad ogni spigolo
    const dod = regularPolyhedra.dodecahedron;
    const phi = Math.acos(v3.dot(
        v3.normalize(dod.centers[0]),
        v3.normalize(dod.centers[1])));
    // phi è il supplementare dell'angolo diedro del dod
    
    const theta = Math.acos(v3.dot(
        v3.normalize(dod.vertices[0]),
        v3.normalize(dod.vertices[1])));
    // theta = angolo sotteso da un lato
    
    const d1 = Math.acosh(1.0/(Math.tan(phi*0.5)*Math.tan(Math.PI/m)));
    // distanza (iperbolica) fra il centro e il punto medio di uno spigolo
    // uso : cosh(hyp) = cot(A)*cot(B)
    // console.log(d1);

    const d2 = Math.atanh(Math.tanh(d1)/Math.cos(theta/2));
    // distanza (iperbolica) fra il centro e un vertice
    // uso: cos(A) = tanh(b)/tanh(c)
    // console.log(d2);

    
    const tmp = Math.exp(2*d2);
    const d3 = (tmp-1)/(tmp+1);
    // distanza (euclidea) dal centro di un punto nel modello di Klein
    // che abbia d2 come distanza iperbolica.
    // uso la formula per la dist. hyp. del modello di klein: 
    // d=1/2log((aq*pb)/(ap*qb)), con d=d2, ap=pb=1, pq=d3, qb=1-d3
    // console.log("d3=",d3);

    return d3;
}
