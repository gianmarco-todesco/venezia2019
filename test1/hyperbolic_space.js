H = {};

H.dot = function(a,b) {
    return a.x*b.x + a.y*b.y + a.z*b.z - a.w*b.w;
}

H.toBall = function(p) {
    const q = H.dot(p,p);
    return p.scale(1.0/(Math.sqrt(-q)+1)); 
}

H.H3toR3 = function(p) {
    return H.toBall(p.scale(1.0/p.w));
}

H.normalize = function(p) {
    return p.scale(1.0/p.w);
}

H.reflection = function(p) {
    const s = -2.0/H.dot(p,p);
    return BABYLON.Matrix.FromValues(
        1.0 + p.x*p.x*s, p.x*p.y*s, p.x*p.z*s, -p.x*p.w*s,
        p.y*p.x*s, 1.0 + p.y*p.y*s, p.y*p.z*s, -p.y*p.w*s,
        p.z*p.x*s, p.z*p.y*s, 1.0 + p.z*p.z*s, -p.z*p.w*s,
        p.w*p.x*s, p.w*p.y*s, p.w*p.z*s, 1.0 - p.w*p.w*s
    );
}

H.apply = function(M, p) {
    return new BABYLON.Vector4(
        M.m[0]*p.x+M.m[1]*p.y+M.m[2]*p.z+M.m[3]*p.w,
        M.m[4]*p.x+M.m[5]*p.y+M.m[6]*p.z+M.m[7]*p.w,
        M.m[8]*p.x+M.m[9]*p.y+M.m[10]*p.z+M.m[11]*p.w,
        M.m[12]*p.x+M.m[13]*p.y+M.m[14]*p.z+M.m[15]*p.w
    );
}

H.distance = function(a,b) {
    const aa = H.dot(a,a);
    const bb = H.dot(b,b);
    const ab = H.dot(a,b);
    const q = ab/Math.sqrt(aa*bb);
    return Math.acosh(q);
}


H.midPoint = function(a,b) {
    const aa = H.dot(a,a);
    const bb = H.dot(b,b);
    const ab = H.dot(a,b);
    return a.scale(Math.sqrt(bb*ab)).add(
        b.scale(Math.sqrt(aa*ab))
    );
}

H.translation = function(a,b) {
    const m = H.midPoint(a,b);
    return H.reflection(m).multiply(H.reflection(a));
}

H.sum = function(pts) {
    let c = pts.reduce((a,b)=>a.add(b));
    return H.normalize(c);
    // c = c.scale(1.0/Math.sqrt(-H.dot(c,c)));
}

