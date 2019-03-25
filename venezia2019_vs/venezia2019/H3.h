#ifndef H3_INCLUDED
#define H3_INCLUDED

#include <qvector3d.h>
#include <qmatrix4x4.h>
#include <math.h>

inline double asinh(double x) { return log(x + sqrt(x*x + 1.0)); }
inline double acosh(double x) { return log(x + sqrt(x*x - 1.0)); }
inline double atanh(double x) { return (log(1+x) - log(1-x))/2; }

namespace H3 {

    namespace KModel {

        inline double dot(const QVector3D &a, const QVector3D &b) {
            return 1.0 - QVector3D::dotProduct(a,b);
        }

        inline double dot(const QVector3D &a) {
            return 1.0 - a.lengthSquared();
        }

        inline double distance(const QVector3D &a, const QVector3D &b) {
            const double ab = dot(a,b);
            const double aa = dot(a);
            const double bb = dot(b);
            const double v = ab/sqrt(aa*bb);
            return acosh(v);
        }

        inline QVector3D midPoint(const QVector3D &a, const QVector3D &b) {
            double aa = sqrt(1 - a.lengthSquared());
            double bb = sqrt(1 - b.lengthSquared());
            double den = 1.0 / (aa + bb);
            double sa = bb * den;
            double sb = aa * den;
    
            return a*sa + b*sb;
        }

        inline QMatrix4x4 reflection(const QVector3D &p) {
            const double s = 2.0/dot(p);
            return QMatrix4x4(
                1.0 + p.x()*p.x()*s, p.x()*p.y()*s, p.x()*p.z()*s, p.x()*s,
                p.y()*p.x()*s, 1.0 + p.y()*p.y()*s, p.y()*p.z()*s, p.y()*s,
                p.z()*p.x()*s, p.z()*p.y()*s, 1.0 + p.z()*p.z()*s, p.z()*s,
                -p.x()*s, -p.y()*s, -p.z()*s, 1.0 - s
            );
        }

        inline QMatrix4x4 translation(const QVector3D &p0, const QVector3D &p1) {
            return reflection(p0) * reflection(midPoint(p0,p1));
        }

        // the euclidean distance OP : O = disk center, hyperbolic distance of OP = hdist
        inline double getRadius(double hdist) {
            // uso la formula per la dist. hyp. del modello di klein: 
            // d=1/2 log((AQ*PB)/(AP*QB)), 
            // Q è l'origine, quindi: AQ=QB=1, PB=1+r, AP=1-r
            // epx(2*hdist) = (1+r)/(1-r)
            const double tmp = exp(2*hdist);
            return (tmp-1)/(tmp+1);
        }

        
        inline QVector3D toBall(const QVector4D &p, double scaleFactor = 5.0) 
        {
            QVector3D q = p.toVector3DAffine();
            double s2 = qMin(1.0, q.lengthSquared()); 
            double k = scaleFactor / (1.0 + sqrt(1.0 - s2));        
            return q*k;
        }


    } // namespace KModel

    // return the hyperbolic length of the Hyptenuse of a right triangle with angles alpha & beta 
    // (angles in radiant)
    inline double getHypotenuseLengthFromAngles(double alpha, double beta) {
        // cosh(hypotenuse) = cot(A)cot(B)
        return acosh(1.0/(tan(alpha)*tan(beta)));
    }

    inline double getHypotenuseLengthFromAdjancentCathetus(double alpha, double cathetusLength) {
        // uso: cos(alpha) = tanh(adjacentCathetus)/tanh(hypotenuse)
        return atanh( tanh(cathetusLength) / cos(alpha) );
    }



} // namespace H3

#endif

