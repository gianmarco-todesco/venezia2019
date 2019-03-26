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
        Piece * const piece;

        virtual void compute(double time) = 0;
        virtual bool isTransition() const { return false; }
        virtual bool hasFinished() const { return false; }
        virtual bool isRest() const { return false; }
        Behavior(Piece *p) : piece(p) {}
        virtual ~Behavior() {}
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
        ~Piece() { delete behaviour; behaviour=0; }
        
        Behavior *compute(Behavior *b, double time);
        void compute(double time);
        void addBehaviour(Behavior *b, double time, double startTime);
    };


    class RestBehavior : public Behavior {

    public:
        RestBehavior(Piece *p) 
            : Behavior(p)
        {
        }
        bool isRest() const { return true; }
        void compute(double time) {
           
            double t = (double)piece->index / (double)piece->type->pieces.count();
            double phi = 0.2*time + 2*M_PI*t;
            double r = piece->type->radius*4;
            QMatrix4x4 matrix; matrix.setToIdentity();
            matrix.translate(r*cos(phi), r*sin(phi), -30);
            matrix.rotate(phi*180/M_PI, 0,0,1);
            piece->worldMatrix = matrix;
        }
    };



    class PolyhedronBehavior : public Behavior {
        QMatrix4x4 m_faceMatrix;
        QMatrix4x4 *m_phMatrix;
    public:
        PolyhedronBehavior(Piece *p, const Polyhedron *ph, int faceIndex, QMatrix4x4 *phMatrix) 
        : Behavior(p)
        , m_phMatrix(phMatrix)
        {
            m_faceMatrix = getFaceMatrix(ph, faceIndex);
        }

        void compute(double time) {

            piece->worldMatrix = (*m_phMatrix) * m_faceMatrix;
        }
    };

    class TransitionBehavior : public Behavior {
        Behavior *m_oldBehavior, *m_newBehavior;
        double m_startTime;
        bool m_started, m_finished;
    public:
        TransitionBehavior(Piece *p, Behavior *b1, Behavior *b2, double startTime)
            : Behavior(p)
            , m_oldBehavior(b1)
            , m_newBehavior(b2)
            , m_startTime(startTime)
            , m_started(false)
            , m_finished(false)
        {
        }
        ~TransitionBehavior()
        {
            delete m_oldBehavior;
            delete m_newBehavior;
        }

        virtual bool isTransition() const { return true; }

        double getStartTime() const { return m_startTime; }

        Behavior *getOldBehaviour() const { return m_oldBehavior; }
        Behavior *getNewBehaviour() const { return m_newBehavior; }
        bool hasFinished() const { return m_finished; }
        bool hasStarted() const { return m_started; }

        Behavior *removeNewBehaviour() { Behavior *b = m_newBehavior; m_newBehavior = 0; return b; }
        Behavior *removeOldBehaviour() { Behavior *b = m_oldBehavior; m_oldBehavior = 0; return b; }
        
        void compute(double time) {
            if(m_finished) m_newBehavior->compute(time);
            else if(time <= m_startTime) m_oldBehavior->compute(time);
            else 
            {
                m_oldBehavior = piece->compute(m_oldBehavior, time);
                if(m_oldBehavior->isTransition()) return;
                if(!m_started)
                {
                    if(m_startTime<time) m_startTime = time;
                     m_started = true;
                }
                double t = (time - m_startTime) / 0.5;
                if(t<=0) return; // this should never happen
                else if(t>=1)
                {
                    m_finished = true;
                    m_newBehavior->compute(time); 
                }
                else 
                {
                    const QMatrix4x4 oldMatrix = piece->worldMatrix;
                    m_newBehavior->compute(time);
                    const QMatrix4x4 newMatrix = piece->worldMatrix;
                    blend(oldMatrix, newMatrix, t);
                }
            }
        }
        void blend(const QMatrix4x4 oldMatrix, const QMatrix4x4 newMatrix, double t);
    };

    void TransitionBehavior::blend(const QMatrix4x4 oldMatrix, const QMatrix4x4 newMatrix, double t) 
    {
        QMatrix4x4 mat = slerp(oldMatrix, newMatrix, t);
        piece->worldMatrix = mat;
    }

    Behavior *Piece::compute(Behavior *b, double time)
    {
        if(!b) return b;
        b->compute(time);
        if(!b->isTransition()) return b;
        TransitionBehavior *tb = dynamic_cast<TransitionBehavior *>(b);
        assert(tb);
        if(tb->hasFinished()) 
        {
            b = tb->removeNewBehaviour();
            delete tb;
        }
        return b;
    }

    void Piece::compute(double time)
    {
        behaviour = compute(behaviour, time);
    }

    void Piece::addBehaviour(Behavior *newBehavior, double time, double startTime)
    {
        assert(!newBehavior->isTransition());
        if(!behaviour)
        {
            // nessun vecchio comportamento ( non dovrebbe succedere)
            // parto subito
            behaviour = newBehavior;
        }
        else
        {
            if(!behaviour->isTransition())
            {
                // il vecchio comportamento è semplice: creo una transizione
                behaviour = new TransitionBehavior(this, behaviour, newBehavior, startTime);
            }
            else
            {
                // il vecchio comportamento e' una transizione
                TransitionBehavior *oldTb = dynamic_cast<TransitionBehavior*>(behaviour);
                assert(oldTb);
                if(!oldTb->hasStarted())
                {
                    // non è ancora partita
                    behaviour = new TransitionBehavior(this, oldTb->removeNewBehaviour(), newBehavior, startTime);
                    delete oldTb;
                }
                else
                {
                    // è già partita: mi accodo:
                    behaviour = new TransitionBehavior(this, behaviour, newBehavior, startTime);
                }               
            }
        }
    }


} // namespace polydron

using namespace polydron;






class Polydron {
public:
    const double edgeLength;

    QList<PieceType*> m_types;
    QList<Piece*> m_pieces;
    QList<Polyhedron*> m_polyhedra;
    QMap<int,int> m_edgeCountTable;

    struct {
        QMatrix4x4 worldMatrix;
        QMatrix4x4 manualRotation;
        QMatrix4x4 translation;
    } m_polyhedronPlacement;


    bool m_automaticRotation;
    double m_oldTime;

    Polydron();
    ~Polydron();
    
    void buildPolyhedra();
    void buildPieces();
    void buildPieces(int edgeCount, int count, const Color color, double radius);
    void animate(double time);
    void draw();

    void rest(double time);
    void makePolyhedron(double time, int phIndex);

};


Polydron::Polydron()
    : edgeLength(3)
    , m_automaticRotation(true)
    , m_oldTime(0)
{
    buildPolyhedra();
    m_polyhedronPlacement.manualRotation.setToIdentity();
    m_polyhedronPlacement.worldMatrix.setToIdentity();
    m_polyhedronPlacement.translation.setToIdentity();
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

    m_polyhedra.append(makeCuboctahedron());
    m_polyhedra.append(makeTruncatedDodecahedron());

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
    
    rest(0);
}

void Polydron::animate(double time)
{
    double dt = time - m_oldTime; m_oldTime = time;

    QMatrix4x4 spin; spin.setToIdentity();
    spin.rotate(time*10, 0,1,0);

    m_polyhedronPlacement.worldMatrix = 
        m_polyhedronPlacement.translation * 
        m_polyhedronPlacement.manualRotation * 
        spin;

    foreach(Piece*piece, m_pieces) piece->compute(time);
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


void Polydron::rest(double time)
{
    foreach(Piece *piece, m_pieces)
    {
        piece->addBehaviour(new RestBehavior(piece), time, time);
    }
}

void Polydron::makePolyhedron(double time, int phIndex)
{
    QVector<int> used(m_types.count(), 0);
    if(phIndex<0 || phIndex>=m_polyhedra.count()) return;
    const Polyhedron *ph = m_polyhedra[phIndex];

    double distance = ph->getVertex(0).m_pos.length() * 2;
    m_polyhedronPlacement.translation.setToIdentity();
    m_polyhedronPlacement.translation.translate(0,0,-distance);
    m_polyhedronPlacement.manualRotation.setToIdentity();
    m_polyhedronPlacement.worldMatrix = m_polyhedronPlacement.translation;
    
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
        piece->addBehaviour(new PolyhedronBehavior(piece, ph, i, &m_polyhedronPlacement.worldMatrix), time, time + j * 0.5);
    }
    foreach(Piece*piece, m_pieces)
    {
        if(piece->index >= used[piece->type->index])
        {
            if(!piece->behaviour || !piece->behaviour->isRest())
                piece->addBehaviour(new RestBehavior(piece), time, time);
        }
    }
}



// =============================================================================




PolydronPage::PolydronPage()
: m_cameraDistance(25)
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
    

    GLfloat specular[] =  { 0.7f, 0.7f, 0.7f, 1.0f};
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular);
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 90.0);

    // draw();
    drawAxes();

    glEnableClientState(GL_NORMAL_ARRAY);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnable(GL_CULL_FACE);

    m_polydron->animate(m_clock.elapsed()*0.001);

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
      m_polydron->m_automaticRotation = false;
}

void PolydronPage::mouseReleaseEvent(QMouseEvent *e)
{
      m_polydron->m_automaticRotation = true;

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

    QMatrix4x4 &rot = m_polydron->m_polyhedronPlacement.manualRotation;
    rot.setToIdentity();
    rot.rotate(m_theta, 1,0,0);
    rot.rotate(m_phi, 0,1,0);



  }
  // updateGL();
}

void PolydronPage::keyPressEvent(QKeyEvent *e)
{
    double time = 0.001 * m_clock.elapsed();
    if(e->key() == Qt::Key_0) m_polydron->rest(time);
    else if(e->key() == Qt::Key_1) m_polydron->makePolyhedron(time,0);
    else if(e->key() == Qt::Key_2) m_polydron->makePolyhedron(time,1);
    else if(e->key() == Qt::Key_3) m_polydron->makePolyhedron(time,2);
    else if(e->key() == Qt::Key_4) m_polydron->makePolyhedron(time,3);
    else if(e->key() == Qt::Key_5) m_polydron->makePolyhedron(time,4);
    else if(e->key() == Qt::Key_6) m_polydron->makePolyhedron(time,5);
    else if(e->key() == Qt::Key_7) m_polydron->makePolyhedron(time,6);
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
