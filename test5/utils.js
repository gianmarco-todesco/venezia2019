"use strict";

// from http://www.euclideanspace.com/maths/geometry/rotations/conversions/matrixToQuaternion/index.htm


function quaternionFromRotationMatrix(m) {
    const m00 = m[0 * 4 + 0];
    const m01 = m[0 * 4 + 1];
    const m02 = m[0 * 4 + 2];
    const m10 = m[1 * 4 + 0];
    const m11 = m[1 * 4 + 1];
    const m12 = m[1 * 4 + 2];
    const m20 = m[2 * 4 + 0];
    const m21 = m[2 * 4 + 1];
    const m22 = m[2 * 4 + 2];

    const tr = m00 + m11 + m22;
    let qw,qx,qy,qz;

    if (tr > 0) { 
        const S = Math.sqrt(tr+1.0) * 2.0; // S=4*qw 
        qw = 0.25 * S;
        qx = (m21 - m12) / S;
        qy = (m02 - m20) / S; 
        qz = (m10 - m01) / S; 
    } else if ( m00 > m11 && m00 > m22) { 
        const S = Math.sqrt(1.0 + m00 - m11 - m22) * 2.0; // S=4*qx 
        qw = (m21 - m12) / S;
        qx = 0.25 * S;
        qy = (m01 + m10) / S; 
        qz = (m02 + m20) / S; 
    } else if (m11 > m22) { 
        const S = Math.sqrt(1.0 + m11 - m00 - m22) * 2; // S=4*qy
        qw = (m02 - m20) / S;
        qx = (m01 + m10) / S; 
        qy = 0.25 * S;
        qz = (m12 + m21) / S; 
    } else { 
        const S = Math.sqrt(1.0 + m22 - m00 - m11) * 2; // S=4*qz
        qw = (m10 - m01) / S;
        qx = (m02 + m20) / S;
        qy = (m12 + m21) / S;
        qz = 0.25 * S;
    }
    return new Quaternion(qw,qx,qy,qz);
}

function interpolateRotation(mat1, mat2, s) {
    const q1 = quaternionFromRotationMatrix(mat1);
    const q2 = quaternionFromRotationMatrix(mat2);
    const q = q1.slerp(q2)(s);
    return q.toMatrix4();
}