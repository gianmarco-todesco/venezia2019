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


namespace polydron {

    class Piece;
    
    class Behavior {
    public:
        const Piece *piece;
        virtual QMatrix4x4 getMatrix(int time) const = 0;
        // virtual QString getName() const = 0;
        Behavior(Piece *p) : piece(p) {}
    };

    class PieceType {
    public:
        const int index;
        Mesh mesh;
        Color color;
        double radius;
        QList<Piece*> pieces;
        PieceType(int i) : index(i), color(1,1,1), radius(0) {}
    };

    class Piece {
    public:
        const int index;
        const PieceType *type;
        QMatrix4x4 worldMatrix;
        Behavior *behaviour;
        Piece(PieceType *t, int i) : type(t), index(i), behaviour(0) {}
        ~Piece() { delete behaviour; }

        void setBehavior(Behavior *b) { delete behaviour; behaviour = b; }
        void animate(int t) {
            if(behaviour) worldMatrix = behaviour->getMatrix(t);
        };
    };


    class RestBehavior : public Behavior {

    public:
        RestBehavior(Piece *p) 
            : Behavior(p)
        {
        }

        QMatrix4x4 getMatrix(int time) const {
           
            double t = (double)piece->index / (double)piece->type->pieces.count();
            double phi = time*0.0001 + 2*M_PI*t;
            double r = piece->type->radius*4;
            QMatrix4x4 matrix; matrix.setToIdentity();
            matrix.translate(r*cos(phi), r*sin(phi), -20);
            matrix.rotate(phi*180/M_PI, 0,0,1);
            
            return matrix;
        }
    };

    class PolyhedronBehavior : public Behavior {
        QMatrix4x4 m_faceMatrix;
    public:
        PolyhedronBehavior(Piece *p, const Polyhedron *ph, int faceIndex) 
        : Behavior(p)
        {
            m_faceMatrix = getFaceMatrix(ph, faceIndex);
        }

        QMatrix4x4 getMatrix(int time) const {
            QMatrix4x4 matrix;
            // matrix.setToIdentity();
            
            // matrix.translate(0,0,3);
            matrix.rotate(time * 0.01, 0,1,0);
            matrix = matrix * m_faceMatrix;
            return matrix;
        }

    };

    class TransitionBehavior : public Behavior {
        Behavior *m_b1, *m_b2;
        double m_start, m_duration;
    public:
        TransitionBehavior(Piece *p, Behavior *b1, Behavior *b2, double start, double duration)
            : Behavior(p)
            , m_b1(b1)
            , m_b2(b2)
            , m_start(start)
            , m_duration(duration)
        {
        }

        QMatrix4x4 getMatrix(int time) const {
            double t = ((double)time*0.001 - m_start) / m_duration;
            if(t<=0) return m_b1->getMatrix(time);
            else if(t>=1) return m_b2->getMatrix(time);
            QMatrix4x4 mat1 = m_b1->getMatrix(time);
            QMatrix4x4 mat2 = m_b2->getMatrix(time);

            QMatrix4x4 mat = slerp(mat1, mat2, t);
            return mat;
        }

    };

};

using namespace polydron;






class Polydron {
public:
    const double edgeLength;

    

    QList<PieceType*> m_types;
    QList<Piece*> m_pieces;
    QList<Polyhedron*> m_polyhedra;
    QMap<int,int> m_edgeCountTable;

    Polydron();
    ~Polydron();
    
    void buildPolyhedra();
    void buildPieces();
    void buildPieces(int edgeCount, int count, const Color color, double radius);
    void animate(int t);
    void draw();

    void place(int time);
    void place(int time, int phIndex);

};


Polydron::Polydron()
    : edgeLength(3)
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
    buildPieces(3, 20, colors[0], 3.7);
    buildPieces(4, 6, colors[1], 1);
    buildPieces(5, 12, colors[2], 2.3);
    buildPieces(10, 12, colors[3], 5);
}

void Polydron::buildPieces(int edgeCount, int count, Color color, double radius)
{
    int index = m_types.count();
    m_edgeCountTable[edgeCount] = index;
    PieceType *type = new PieceType(index);
    type->color = color;
    type->radius = radius;
    double r = edgeLength*0.5/sin(M_PI/edgeCount);
    type->mesh.makePrism(r,0.03,edgeCount);
    m_types.append(type);
    for(int i=0;i<count; i++)
    {
        Piece *piece = new Piece(type, i);
        type->pieces.append(piece);
        piece->worldMatrix.setToIdentity();
        piece->worldMatrix.translate(2*i, index*2, 0);
        m_pieces.append(piece);
    }
    
    place(0);
}

void Polydron::animate(int t)
{
    foreach(Piece*piece, m_pieces) piece->animate(t);
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


void Polydron::place(int time)
{
    foreach(Piece *piece, m_pieces)
    {
        piece->setBehavior(new RestBehavior(piece));
    }
}

void Polydron::place(int time, int phIndex)
{
    QVector<int> used(m_types.count(), 0);
    if(phIndex<0 || phIndex>=m_polyhedra.count()) return;
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
        Behavior *b1 = new RestBehavior(piece);
        Behavior *b2 = new PolyhedronBehavior(piece, ph, i);
        double start = time * 0.001 + j * 0.5;
        piece->setBehavior(new TransitionBehavior(piece, b1,b2, start, 1));
    }
    foreach(Piece*piece, m_pieces)
    {
        if(piece->index >= used[piece->type->index])
            piece->setBehavior(new RestBehavior(piece));
    }
}



// =============================================================================




PolydronPage::PolydronPage()
: m_cameraDistance(15)
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
    m_polydron->buildPieces();
}

void PolydronPage::start()
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

  
    m_clock.start();

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

    m_polydron->animate(m_clock.elapsed());

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
  // updateGL();
}

void PolydronPage::keyPressEvent(QKeyEvent *e)
{
    int time = m_clock.elapsed();
    if(e->key() == Qt::Key_0) m_polydron->place(time);
    else if(e->key() == Qt::Key_1) m_polydron->place(time,0);
    else if(e->key() == Qt::Key_2) m_polydron->place(time,1);
    else if(e->key() == Qt::Key_3) m_polydron->place(time,2);
    else if(e->key() == Qt::Key_4) m_polydron->place(time,3);
    else if(e->key() == Qt::Key_5) m_polydron->place(time,4);
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
