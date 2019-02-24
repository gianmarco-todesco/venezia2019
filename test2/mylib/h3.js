//-----------------------------------------

function HPoint(x,y,z) {
    this.x = x;
    this.y = y;
    this.z = z;
    this.w = Math.sqrt(1 + x*x+y*y+z*z);    
}

HPoint.dot = function(a,b) {
    if(!(a instanceof HPoint && b instanceof HPoint)) throw "Expected HPoint";    
    return a.x*b.x+a.y*b.y+a.z*b.z-a.w*b.w;
}

HPoint.prototype.check = function() {
    const d = HPoint.dot(this, this);
    if(Math.abs(d+1)>1.0e-10)
        throw "Bad HPoint";
} 


HPoint.distance = function(a,b) {
    if(!(a instanceof HPoint && b instanceof HPoint)) throw "Expected HPoint";    
    const d = Math.max(1.0, -HPoint.dot(a,b));
    return Math.acosh(d);
}



HPoint.prototype.toBPoint = function() {
    const k = 1.0/(1.0+this.w);
    return new BPoint(this.x*k, this.y*k, this.z*k);
}

HPoint.prototype.toKPoint = function() {
    const iw = 1.0/this.w;
    return new KPoint(this.x*iw, this.y*iw, this.z*iw);
}

HPoint.midPoint = function(a,b) {
    var p = new HPoint(
        a.x+b.x,
        a.y+b.y,
        a.z+b.z,
        a.w+b.w
    );
    const x = a.x + b.x;
    const x = a.x + b.x;
    const x = a.x + b.x;
    const x = a.x + b.x;
    
    const sc = 1.0 / Math.sqrt()
}

//-----------------------------------------

function BPoint(x,y,z) {
    this.x = x;
    this.y = y;
    this.z = z;
}


BPoint.prototype.check = function() {
    const d = this.x*this.x+this.y*this.y+this.z*this.z;
    if(d>=1.0)
        throw "Bad BPoint";
} 

BPoint.prototype.toHPoint = function() {
    const s = this.x*this.x+this.y*this.y+this.z*this.z;
    const k = 2.0/(1-s);
    return new HPoint(this.x*k, this.y*k, this.z*k, k-1.0);
}

BPoint.prototype.toKPoint = function() {
    const u2 = this.x*this.x+this.y*this.y+this.z*this.z;
    const k = 2/(1+u2);
    return new KPoint(k*this.x, k*this.y, k*this.z);
}

//-----------------------------------------

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

KPoint.prototype.toHPoint = function() {
    const k = Math.sqrt(1.0/(1.0 - (this.x*this.x+this.y*this.y+this.z*this.z)));
    return new HPoint(this.x*k, this.y*k, this.z*k);
}
KPoint.prototype.toBPoint = function() {
    const s2 = Math.min(1.0, this.x*this.x+this.y*this.y+this.z*this.z);
    const k = (1 - Math.sqrt(1-s2))/s2;
    return new BPoint(this.x*k, this.y*k, this.z*k);
}

KPoint.prototype.transformed = function(M) {
    const m = M.m;
    const v = this;
    const w = [
        v.x*m[ 0] + v.y*m[ 4] + v.z*m[ 8] + m[12],
        v.x*m[ 1] + v.y*m[ 5] + v.z*m[ 9] + m[13],
        v.x*m[ 2] + v.y*m[ 6] + v.z*m[10] + m[14],
        v.x*m[ 3] + v.y*m[ 7] + v.z*m[11] + m[15]
    ];
    const s = 1.0/w[3];
    return new KPoint(w[0]*s,w[1]*s,w[2]*s);
}

/*
KPoint.distance = function(p,q) {
    if(p instanceof KPoint && q instanceof KPoint) {
        // A,B sono le intersezioni della retta che passa per pq
        // e la sfera unitaria
        const p2 = p.x*p.x+p.y*p.y+p.z*p.z;
        const q2 = q.x*q.x+q.y*q.y+q.z*q.z;
        const pq = p.x*q.x+p.y*q.y+p.z*q.z;
        const a = p2+q2-2*pq;
        const b = pq-q2;
        const c = q2-1;
        const r = Math.sqrt(b*b-a*c);
        const t0 = (-b+r)/a, t1 = (-b-r)/a;
        const A = {x: p.x*t0 + q.x*(1-t0), y: p.y*t0 + q.y*(1-t0), z: p.z*t0 + q.z*(1-t0)};
        const B = {x: p.x*t1 + q.x*(1-t1), y: p.y*t1 + q.y*(1-t1), z: p.z*t1 + q.z*(1-t1)};
        // d(p,q) = 1/2 log((QA*PB)/(PA*QB))


    } else {
        throw "arguments are not KPoints";
    }
}
*/
KPoint.dot = function(a,b) {
    if(!(a instanceof KPoint && b instanceof KPoint)) throw "Expected KPoints";
    return a.x*b.x+a.y*b.y+a.z*b.z-1;
}
KPoint.distance = function(p,q) {
    if(!(p instanceof KPoint && q instanceof KPoint)) throw "Expected KPoints";
    return HPoint.distance(p.toHPoint(), q.toHPoint());
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



//-----------------------------------------

function testPointsConversions(h) {
    var b = h.toBPoint();
    var k = h.toKPoint();
    console.log(HPoint.dist(h, b.toHPoint()));
    console.log(HPoint.dist(h, k.toHPoint()));

    var b2 = k.toBPoint();
    var k2 = b.toKPoint();
    console.log(HPoint.dist(h, b2.toHPoint()));
    console.log(HPoint.dist(h, k2.toHPoint()));

    
}

//-----------------------------------------

