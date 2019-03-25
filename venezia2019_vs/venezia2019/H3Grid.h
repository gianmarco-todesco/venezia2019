#ifndef H3GRID_INCLUDED
#define H3GRID_INCLUDED

#include <qvector3d.h>
#include <qvector4d.h>
#include <qmatrix4x4.h>
#include <qlist.h>
#include <qvector.h>
#include <qpair.h>


class GridMatrices {
public:
    QList<QMatrix4x4> m_vertexMatrices;
    QList<QMatrix4x4> m_edgeMatrices;

    void clear() { m_vertexMatrices.clear(); m_edgeMatrices.clear(); }
};

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

    // roto-traslazione di un cubo posto nell'origine sul vertice index-esimo
    QMatrix4x4 getCellVertexMatrix(int index);

    QMatrix4x4 getCornerMatrix();

    QMatrix4x4 getEdgeMatrix();


    void addDodVertices(GridMatrices &matrices);
    void addEdgeAndVertex(GridMatrices &matrices, const QMatrix4x4 &mat);

    void flower(GridMatrices &matrices, const QMatrix4x4 &mat, int level);

};



#endif
