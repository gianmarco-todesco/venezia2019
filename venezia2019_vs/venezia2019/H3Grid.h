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

class H3Grid534;

class H3Grid {
public:


    struct Vertex {
        QMatrix4x4 matrix;
        int links[6];
        int channels[24];

    };

    const H3Grid534 &m_gridData;

    QList<Vertex> m_vertices;
    QList<QMatrix4x4> m_edgeMatrices;
    QMatrix4x4 m_rotations[6];

    H3Grid(const H3Grid534 &gridData) : m_gridData(gridData) { 
        clear(); 
        initRotations();
    }

    void clear() { 
        m_vertices.clear(); 
        m_edgeMatrices.clear(); 
    }

    void createFirstVertex();
    void addVertex(int vIndex, int direction);


    // dato un vertice e un canale (direzione + side), segue la faccia aggiornando vIndex e channelIndex. Ritorna il numero di passi
    // per una faccia chiusa ritorna 5 : vIndex e' l'ultimo vertice della catena e channelIndex "punta" al vertice di partenza
    // nota: se la faccia è già chiusa ritorna 0
    int getOtherEnd(int &vIndex, int &channelIndex);

    void foo(int vIndex);
    void closeIfNeeded(int vIndex, int dir);

    static int innerLink(int j);

private:
    void initRotations();

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
