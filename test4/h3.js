
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
