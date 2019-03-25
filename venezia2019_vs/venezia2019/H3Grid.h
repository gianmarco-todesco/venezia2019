#ifndef H3GRID_INCLUDED
#define H3GRID_INCLUDED

#include <qvector3d.h>
#include <qvector4d.h>
#include <qmatrix4x4.h>
#include <qlist.h>
#include <qvector.h>
#include <qpair.h>

class H3Grid534 {
public:
    double radius; // distanza euclidea dei vertici dal centro nel modello di klein
    double edgeLength; // lunghezza iperbolica di un lato
    QVector<QMatrix4x4> dirMatrices;

    QVector<QVector4D> m_pts;
    QVector<QPair<int, int> > m_edges;
    QVector<QVector<int> > m_adjVertTb;
    H3Grid534();
    


    void compute(); 
    QMatrix4x4 getCornerMatrix();

    QMatrix4x4 getEdgeMatrix();
};



#endif
