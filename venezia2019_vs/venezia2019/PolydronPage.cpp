#include "PolydronPage.h"

#include "Polyhedra.h"
#include "Gutil.h"
#include "Point3.h"

#include "Mesh.h"

#define _USE_MATH_DEFINES
#include <math.h>
#include <assert.h>
#include <QKeyEvent>
#include <QWheelEvent>

#include <vector>

#include <QGLShaderProgram>
#include <qmatrix4x4.h>
#include <qvector.h>
#include <qlist.h>
#include <qmap.h>


class Polydron {
public:
    const double edgeLength;

    class Piece;
    struct PieceType {
        Mesh mesh;
        Color color;
        QList<Piece*> pieces;
    };

    struct Piece {
        PieceType *type;
        QMatrix4x4 worldMatrix;
    };


    QList<PieceType*> m_types;
    QList<Piece*> m_pieces;
    QList<Polyhedron*> m_polyhedra;
    QMap<int,int> m_edgeCountTable;

    Polydron();
    ~Polydron();
    
    void buildPolyhedra();
    void buildPieces();
    void buildPieces(int edgeCount, int count, const Color color);
    void draw();

    void place();
    void place(int phIndex);

};


Polydron::Polydron()
    : edgeLength(0.6)
{
    buildPolyhedra();
}

Polydron::~Polydron()
{
    foreach(Piece*piece, m_pieces) delete piece; 
    m_pieces.clear();
    
    foreach(PieceType*type, m_types) delete type;
    m_types.clear();
    
    foreach(Polyhedron*ph, m_polyhedra) delete ph;
    m_polyhedra.clear();    
    
}

void Polydron::buildPolyhedra()
{
    m_polyhedra.append(makeTetrahedron());
    m_polyhedra.append(makeCube());
    m_polyhedra.append(makeOctahedron());
    m_polyhedra.append(makeDodecahedron());
    m_polyhedra.append(makeIcosahedron());

    foreach(Polyhedron *ph, m_polyhedra) 
    {
        const Polyhedron::Edge &e = ph->getEdge(0);
        double L = (ph->getVertex(e.m_a).m_pos - ph->getVertex(e.m_b).m_pos).length();
        ph->computeFaceVertices();
        ph->scale(edgeLength / L);
    }
}


void Polydron::buildPieces()
{
    const Color colors[4] = {
        Color(0.8,0.2,0.2),
        Color(0.8,0.6,0.1),
        Color(0.2,0.4,0.8),
        Color(0.8,0.2,0.9)
    };
    buildPieces(3, 20, colors[0]);
    buildPieces(4, 6, colors[1]);
    buildPieces(5, 12, colors[2]);
    buildPieces(10, 12, colors[3]);
}

void Polydron::buildPieces(int edgeCount, int count, Color color)
{
    int index = m_types.count();
    m_edgeCountTable[edgeCount] = index;
    PieceType *type = new PieceType();
    type->color = color;
    double r = edgeLength*0.5/sin(M_PI/edgeCount);
    type->mesh.makePrism(r,0.03,edgeCount);
    m_types.append(type);
    for(int i=0;i<count; i++)
    {
        Piece *piece = new Piece();
        piece->type = type;
        type->pieces.append(piece);
        piece->worldMatrix.setToIdentity();
        piece->worldMatrix.translate(2*i, index*2, 0);
        m_pieces.append(piece);
    }
    place();
}

void Polydron::draw()
{
    foreach(PieceType*type, m_types)
    {
        type->mesh.bind();
        setColor(type->color);
        foreach(Piece *piece, type->pieces) {
            glPushMatrix();
            glMultMatrixd(piece->worldMatrix.constData());
            type->mesh.draw();            
            glPopMatrix();
        }
        type->mesh.release();
    }
}


void Polydron::place()
{
    int n = m_types.count();
    for(int i=0;i<n;i++)
    {
        int m = m_types[i]->pieces.count();
        for(int j=0; j<m; j++) 
        {
            QMatrix4x4 mat;mat.setToIdentity();
            mat.translate(2.5*(j-(m-1)*0.5), 2.5*(i-(n-1)*0.5),0);
            m_types[i]->pieces[j]->worldMatrix = mat;
        }
    }
}

void Polydron::place(int phIndex)
{
    QVector<int> used(m_types.count(), 0);
    QMap<int,int> usedCount;
    if(phIndex<0 || phIndex>=m_polyhedra.count()) return;
    place();
    const Polyhedron *ph = m_polyhedra[phIndex];
    for(int i=0;i<ph->getFaceCount();i++) 
    {
        const Polyhedron::Indices &ii = ph->getFace(i).m_vertices;
        int m = (int)ii.size();
        if(!m_edgeCountTable.contains(m)) continue;
        int typeIndex = m_edgeCountTable[m];
        int j = used[typeIndex]++;
        const PieceType *type = m_types[typeIndex];
        assert(j<type->pieces.count());
        Piece *piece = type->pieces[j];
        piece->worldMatrix = getFaceMatrix(ph, i);
    }
}



// =============================================================================




PolydronPage::PolydronPage()
: OpenGLPage()
, m_cameraDistance(15)
, m_theta(0)
, m_phi(0)
, m_rotating(true)
{
    m_polydron = new Polydron();
}

PolydronPage::~PolydronPage()
{
    delete m_polydron; m_polydron = 0;
}

void PolydronPage::initializeGL()
{
  glEnable(GL_LIGHTING);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_LIGHT0);
  // glEnable(GL_LIGHT1);
  glLightModelf(GL_LIGHT_MODEL_TWO_SIDE,1.0);
  glLightModelf(GL_LIGHT_MODEL_LOCAL_VIEWER,1.0);

  GLfloat lcolor[] =  { 0.7f, 0.7f, 0.7f, 1.0f};
  GLfloat lpos[]   = { 5, 7, 10, 1.0f};
  glLightfv(GL_LIGHT0, GL_AMBIENT, lcolor);
  glLightfv(GL_LIGHT0, GL_DIFFUSE, lcolor);
  glLightfv(GL_LIGHT0, GL_SPECULAR, lcolor);
  glLightfv(GL_LIGHT0, GL_POSITION, lpos);

  lpos[0] = -5;
  glLightfv(GL_LIGHT1, GL_AMBIENT, lcolor);
  glLightfv(GL_LIGHT1, GL_DIFFUSE, lcolor);
  glLightfv(GL_LIGHT1, GL_SPECULAR, lcolor);
  glLightfv(GL_LIGHT1, GL_POSITION, lpos);

  m_polydron->buildPieces();

}

void PolydronPage::resizeGL(int width, int height)
{
    double aspect = (float)width/height;
    glViewport(0,0,width,height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45, aspect, 1.0, 70.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}


void PolydronPage::paintGL()
{
    glClearColor(0,0,0,1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    drawBackground();

    glPushMatrix();
    glTranslated(0,0,-m_cameraDistance);
    glRotated(m_theta,1,0,0);
    glRotated(m_phi,0,1,0);


    GLfloat specular[] =  { 0.7f, 0.7f, 0.7f, 1.0f};
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular);
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 90.0);

    // draw();
    drawAxes();

    glEnableClientState(GL_NORMAL_ARRAY);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnable(GL_CULL_FACE);

    m_polydron->draw();
    glDisable(GL_CULL_FACE);
    
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);

    glPopMatrix();
}

void PolydronPage::draw()
{
    Polyhedron *ph = makeDodecahedron();
    ph->computeFaceVertices();


    double sc = 5.0 / ph->getVertex(0).m_pos.length();
    for(int i=0;i<ph->getVertexCount();i++) 
        ph->getVertex(i).m_pos *= sc;

    for(int i=0;i<ph->getVertexCount();i++)
    {
        setColor(0.5,0.8,0.1);
        drawSphere(ph->getVertex(i).m_pos, 0.1);
    }

    for(int i=0;i<ph->getEdgeCount();i++)
    {
        const Polyhedron::Edge &edge = ph->getEdge(i);
        QVector3D p0 = ph->getVertex(edge.m_a).m_pos;
        QVector3D p1 = ph->getVertex(edge.m_b).m_pos;
        setColor(0.2,0.2,0.1);
        drawCylinder(p0,p1,0.05);
    }
    delete ph;
  
}



void PolydronPage::mousePressEvent(QMouseEvent *e)
{
      m_lastPos = e->pos();
      m_rotating = e->button() == Qt::RightButton;
}

void PolydronPage::mouseReleaseEvent(QMouseEvent *e)
{

}

void PolydronPage::mouseMoveEvent(QMouseEvent *e)
{
  QPoint delta = m_lastPos - e->pos();
  m_lastPos = e->pos();
  if(!m_rotating)
  {
  }
  else
  {
    m_phi -= 0.25*delta.x();
    m_theta -= 0.25*delta.y();
  }
  updateGL();
}

void PolydronPage::keyPressEvent(QKeyEvent *e)
{
    if(e->key() == Qt::Key_0) m_polydron->place();
    else if(e->key() == Qt::Key_1) m_polydron->place(0);
    else if(e->key() == Qt::Key_2) m_polydron->place(1);
    else if(e->key() == Qt::Key_3) m_polydron->place(2);
    else if(e->key() == Qt::Key_4) m_polydron->place(3);
    else if(e->key() == Qt::Key_5) m_polydron->place(4);
    else {
      e->ignore();
    }
    updateGL();
}

void PolydronPage::wheelEvent(QWheelEvent*e)
{
    m_cameraDistance = clamp(m_cameraDistance - e->delta() * 0.01, 1,50);
    updateGL();
}

void PolydronPage::showEvent(QShowEvent*)
{
    setFocus();
}

void PolydronPage::hideEvent(QHideEvent*)
{
}
