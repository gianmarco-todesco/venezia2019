#include "PolydronPage.h"

#include "Polyhedra.h"
#include "Gutil.h"
#include "Point3.h"

#include "Mesh.h"

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

    enum TypeId {
        T_NONE = 0,
        T_TRIANGLE,
        T_SQUARE,
        T_PENTAGON,
        T_DECAGON,
        T_TRIANGLE2
    };

    class MyPolyhedron {
    public:

        QList<QPair<TypeId, QMatrix4x4> > faces;
        double radius;

        MyPolyhedron() : radius(1) {}
    
        static QMatrix4x4 computeMatrixFromFace(const QVector3D &center, const QVector3D &p1, const QVector3D &p2);
        static QMatrix4x4 computeMatrixFromTriangle(const QVector3D &p0, const QVector3D &p1, const QVector3D &p2);

        static MyPolyhedron*make(const Polyhedron *ph, double edgeLength);
        static MyPolyhedron*makeDeltahedron(double edgeLength, int index);
        static MyPolyhedron*makeImpossible(double edgeLength, double parameter);

    };

    QMatrix4x4 MyPolyhedron::computeMatrixFromFace(const QVector3D &center, const QVector3D &p1, const QVector3D &p2)
    {
        QVector3D e0 = (p1-center).normalized();
        QVector3D e1 = p2-center;
        e1 = (e1 - e0*QVector3D::dotProduct(e0,e1)).normalized();
        QVector3D e2 = QVector3D::crossProduct(e0,e1);
        return QMatrix4x4(
            e0.x(),e1.x(),e2.x(),center.x(),
            e0.y(),e1.y(),e2.y(),center.y(),
            e0.z(),e1.z(),e2.z(),center.z(),
            0,0,0,1);
    }

    QMatrix4x4 MyPolyhedron::computeMatrixFromTriangle(const QVector3D &p0, const QVector3D &p1, const QVector3D &p2)
    {
        return computeMatrixFromFace((p0+p1+p2)*(1.0/3.0), p0,p1);
    }

    MyPolyhedron*MyPolyhedron::make(const Polyhedron *ph, double edgeLength)
    {
        MyPolyhedron *myph = new MyPolyhedron();
        const Polyhedron::Edge &e = ph->getEdge(0);
        double L = (ph->getVertex(e.m_a).m_pos - ph->getVertex(e.m_b).m_pos).length();
        double sc = edgeLength / L;
        ph->computeFaceVertices();
        for(int i=0;i<ph->getFaceCount();i++)
        {
            const Polyhedron::Face &face = ph->getFace(i);
            QVector3D fc = sc*getFaceCenter(ph,i);
            QVector3D p0 = sc*ph->getVertex(face.m_vertices[0]).m_pos;
            QVector3D p1 = sc*ph->getVertex(face.m_vertices[1]).m_pos;
            int m = (int)face.m_edges.size();
            assert(m==3 || m==4 || m==5 || m==10);
            TypeId typeId = m<=4 ? (m==3 ? T_TRIANGLE : T_SQUARE) : (m==5 ? T_PENTAGON : T_DECAGON);
            QMatrix4x4 matrix = computeMatrixFromFace(fc,p0,p1);

            myph->faces.append(qMakePair(typeId, matrix));
        }
        myph->radius = (ph->getVertex(0).m_pos * sc).length();
        return myph;
    }

    MyPolyhedron*MyPolyhedron::makeDeltahedron(double edgeLength, int m)
    {
        if(m<1)m=1; else if(m>5)m=5;
        double d = edgeLength*0.5;
        QVector3D p0(0,d,0), p1(0,-d,0);
        double r = edgeLength*sqrt(3.0)*0.5;
        QList<QVector3D> pts;
        double dphi = acos(1.0/3.0);
        QVector3D center(0,0,0);
        for(int i=0;i<=m;i++)
        {
            double phi = dphi*i;
            QVector3D p(r*cos(phi),0,r*sin(phi));
            center += p;
            pts.append(QVector3D(r*cos(phi),0,r*sin(phi)));
        }
        center *= 1.0/(m+3);
        p0 -= center;
        p1 -= center;
        for(int i=0;i<=m;i++) pts[i] -= center;
        
        MyPolyhedron *ph = new MyPolyhedron();
        ph->radius = d;
        for(int i=0;i<=m;i++) {
            double r = pts[i].length();
            if(r>ph->radius) ph->radius = r;
        }
        TypeId typeId = (m%2)==0 ? T_TRIANGLE2 : T_TRIANGLE;
        ph->faces.append(qMakePair(typeId, computeMatrixFromTriangle(p0,p1,pts[0])));
        ph->faces.append(qMakePair(typeId, computeMatrixFromTriangle(p1,p0,pts.back())));
        for(int i=0;i<m;i++) 
        {
            ph->faces.append(qMakePair(typeId, computeMatrixFromTriangle(p0,pts[i],pts[i+1])));
            ph->faces.append(qMakePair(typeId, computeMatrixFromTriangle(p1,pts[i+1],pts[i])));
        }
        return ph;
    }

    MyPolyhedron*MyPolyhedron::makeImpossible(double edgeLength, double parameter)
    {
        MyPolyhedron *ph = new MyPolyhedron();
        ph->radius = 4 * edgeLength;
        const double decagonPos = 1.879*edgeLength;
        const double trianglePos = 1.768 * edgeLength;
        const double squarePos = 3.032 * edgeLength;

        // decagoni
        for(int i=0;i<12;i++)
        {
            QMatrix4x4 mat; mat.setToIdentity();
            int k = i/4;
            if(k==1) mat.rotate(90,0,0,1);
            else if(k==2) mat.rotate(90,1,0,0); 
            mat.rotate(90*(i%4),0,1,0);
            mat.translate(decagonPos,0,decagonPos);
            mat.rotate(45,0,1,0);
            ph->faces.append(qMakePair(T_DECAGON, mat));
        }

        // triangoli sui vertici del cubo
        QVector3D e2 = QVector3D(1,1,1).normalized();
        QVector3D e0 = QVector3D(0,1,0);
        e0 = (e0 - e2*QVector3D::dotProduct(e0,e2)).normalized();
        QVector3D e1 = QVector3D::crossProduct(e2,e0);
        QMatrix4x4 vertexMat(
            e0.x(),e1.x(),e2.x(), 0,
            e0.y(),e1.y(),e2.y(), 0,
            e0.z(),e1.z(),e2.z(), 0,
            0,0,0,1);

        for(int i=0;i<8;i++)
        {
            QMatrix4x4 mat; mat.setToIdentity();
            if(i>=4) mat.rotate(180,1,0,0);
            mat.rotate(90*i,0,1,0);
            double d = trianglePos;
            mat.translate(d,d,d);
            mat = mat * vertexMat;
            ph->faces.append(qMakePair(T_TRIANGLE, mat));
        }

        // quadrati (e triangoli) sulle facce del cubo
        QMatrix4x4 foldedTriangle;
        foldedTriangle.setToIdentity();
        foldedTriangle.translate(edgeLength*0.5,0,0);
        foldedTriangle.rotate(14,0,1,0);
        foldedTriangle.translate(edgeLength*sqrt(3.0)/6,0,0);
            
        for(int i=0;i<6;i++)
        {
            QMatrix4x4 mat; mat.setToIdentity();
            if(1<=i && i<=4)
            {
                mat.rotate(90*i,0,0,1);
                mat.rotate(90,1,0,0);
            }
            else if(i==5) mat.rotate(180,1,0,0);
            mat.translate(0,0,squarePos);
            ph->faces.append(qMakePair(T_SQUARE, mat));

            for(int j=0; j<4; j++) 
            {
                QMatrix4x4 triMat;
                triMat.setToIdentity();
                triMat.rotate(45+90*j,0,0,1);
                ph->faces.append(qMakePair(T_TRIANGLE2, mat * triMat * foldedTriangle));
            }
        }
    

        return ph;
    }


    //-------------------------------------------------------------------------

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
        TypeId typeId;
        const int index;
        Mesh mesh;
        Color color;
        double radius;
        QList<Piece*> pieces;
        PieceType(int i) : index(i), color(1,1,1), radius(0), typeId(T_NONE) {}
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
        PolyhedronBehavior(Piece *p, const MyPolyhedron *ph, int faceIndex, QMatrix4x4 *phMatrix) 
        : Behavior(p)
        , m_phMatrix(phMatrix)
        {
            m_faceMatrix = ph->faces[faceIndex].second;
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

    namespace {
        inline double step(double x, double a, double b) { 
            if(x<=a) return 0; 
            else if(x>=b) return 1;
            else return (x-a)/(b-a);
        }
        inline double smooth(double x) { return x<0 ? 0 : x>1 ? 1 : 0.5*(1-cos(x*M_PI)); }
    }
    void TransitionBehavior::blend(const QMatrix4x4 oldMatrix, const QMatrix4x4 newMatrix, double t) 
    {
        double t0 = smooth(step(t, 0,0.8));
        double t1 = smooth(step(t, 0.75,1.0));

        QMatrix4x4 newMatrix2; newMatrix2.setToIdentity();
        newMatrix2.translate(0,0,(1-t1)*4);
        newMatrix2 = newMatrix * newMatrix2;
        QMatrix4x4 mat = slerp(oldMatrix, newMatrix2, t0);
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
    QList<MyPolyhedron*> m_polyhedra;
    QMap<TypeId,int> m_typeIdTable;

    struct {
        QMatrix4x4 worldMatrix;
        QMatrix4x4 manualRotation;
        QMatrix4x4 translation;
        double distance;
    } m_polyhedronPlacement;


    bool m_automaticRotation;
    double m_oldTime;

    Polydron();
    ~Polydron();
    
    void buildPolyhedra();
    void buildPieces();
    void buildPieces(TypeId typeId, int edgeCount, int count, const Color color, double radius);
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
    m_polyhedronPlacement.distance = 0;
}

Polydron::~Polydron()
{
    foreach(Piece*piece, m_pieces) delete piece; 
    m_pieces.clear();
    
    foreach(PieceType*type, m_types) delete type;
    m_types.clear();
    
    foreach(MyPolyhedron*ph, m_polyhedra) delete ph;
    m_polyhedra.clear();    
    
}

void Polydron::buildPolyhedra()
{
    Polyhedron *ph;

    ph = makeTetrahedron(); 
    m_polyhedra.append(MyPolyhedron::make(ph, edgeLength)); delete ph;

    ph = makeCube(); 
    m_polyhedra.append(MyPolyhedron::make(ph, edgeLength)); delete ph;
    
    ph = makeOctahedron();
    m_polyhedra.append(MyPolyhedron::make(ph, edgeLength)); delete ph;

    ph = makeDodecahedron();
    m_polyhedra.append(MyPolyhedron::make(ph, edgeLength)); delete ph;

    ph = makeIcosahedron();
    m_polyhedra.append(MyPolyhedron::make(ph, edgeLength)); delete ph;

    ph = makeCuboctahedron();
    m_polyhedra.append(MyPolyhedron::make(ph, edgeLength)); delete ph;

    ph = makeTruncatedDodecahedron();
    m_polyhedra.append(MyPolyhedron::make(ph, edgeLength)); delete ph;

    m_polyhedra.append(MyPolyhedron::makeDeltahedron(edgeLength,2));
    m_polyhedra.append(MyPolyhedron::makeDeltahedron(edgeLength,3));
    m_polyhedra.append(MyPolyhedron::makeDeltahedron(edgeLength,4));
    m_polyhedra.append(MyPolyhedron::makeImpossible(edgeLength, 1.0));

}


void Polydron::buildPieces()
{
    const Color colors[5] = {
        Color(0.8,0.2,0.2),
        Color(0.8,0.6,0.1),
        Color(0.2,0.4,0.8),
        Color(0.1,0.2,0.9),
        Color(0.8,0.5,0.1)
    };
    buildPieces(T_TRIANGLE, 3, 20, colors[0], 3.1);
    buildPieces(T_SQUARE, 4, 6, colors[1], 1);
    buildPieces(T_PENTAGON, 5, 12, colors[2], 2.2);
    buildPieces(T_DECAGON, 10, 12, colors[3], 5.5);
    buildPieces(T_TRIANGLE2, 3, 24, colors[4], 3.8);
}

void Polydron::buildPieces(TypeId typeId, int edgeCount, int count, Color color, double radius)
{
    int index = m_types.count();
    m_typeIdTable[typeId] = index;
    PieceType *type = new PieceType(index);
    type->color = color;
    type->radius = radius;
    type->typeId = typeId;
    double r = edgeLength*0.5/sin(M_PI/edgeCount);
    if(typeId == T_TRIANGLE2) r *= 1.03;
    
    type->mesh.makePolydronPiece(r,0.03,edgeCount);
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
    const MyPolyhedron *ph = m_polyhedra[phIndex];

    double distance = ph->radius * 2;
    m_polyhedronPlacement.distance = distance;
    m_polyhedronPlacement.translation.setToIdentity();
    m_polyhedronPlacement.translation.translate(0,0,-distance);
    m_polyhedronPlacement.manualRotation.setToIdentity();
    m_polyhedronPlacement.worldMatrix = m_polyhedronPlacement.translation;
    
    for(int i=0;i<ph->faces.count();i++) 
    {
        TypeId typeId = ph->faces[i].first;
        if(!m_typeIdTable.contains(typeId)) continue;
        int typeIndex = m_typeIdTable[typeId];
        int j = used[typeIndex]++;
        const PieceType *type = m_types[typeIndex];
        assert(j<type->pieces.count());
        Piece *piece = type->pieces[j];
        piece->addBehaviour(
            new PolyhedronBehavior(
                piece, ph, i, &m_polyhedronPlacement.worldMatrix), 
                time, time + j * 0.1);
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
, m_parameter(1)
{
    m_polydron = new Polydron();
}

PolydronPage::~PolydronPage()
{
    delete m_polydron; m_polydron = 0;
}

void PolydronPage::initializeGL()
{
    QImage img(256,256, QImage::Format_ARGB32_Premultiplied);
    img.fill(Qt::white);
    QPainter pa;
    pa.begin(&img);
    int d = 4;
    pa.fillRect(0,128,256,64, QColor(127,127,127));
    pa.fillRect(0,0,256,d, Qt::black);
    pa.fillRect(0,64-d,256,d*2, Qt::black);
    pa.fillRect(0,128-d,256,d*2, Qt::black);
    pa.fillRect(0,192-d,256,d*2, Qt::black);
    pa.fillRect(0,255-d,256,d, Qt::black);
    /*
    pa.setFont(QFont("Arial", 30));
    pa.drawText(QRect(0,0,256,64),Qt::AlignCenter, "Uno");
    pa.drawText(QRect(0,64,256,64),Qt::AlignCenter, "Due");
    pa.drawText(QRect(0,128,256,64),Qt::AlignCenter, "Tre");
    */
    pa.end();   
    m_texture1.createTexture(img);


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
    GLfloat spec[] =  { 0.2f, 0.2f, 0.2f, 1.0f};
    GLfloat lpos[]   = { 5, 7, 10, 1.0f};
    glLightfv(GL_LIGHT0, GL_AMBIENT, lcolor);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lcolor);
    glLightfv(GL_LIGHT0, GL_SPECULAR, spec);
    glLightfv(GL_LIGHT0, GL_POSITION, lpos);

    lpos[0] = -5;
    glLightfv(GL_LIGHT1, GL_AMBIENT, lcolor);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, lcolor);
    glLightfv(GL_LIGHT1, GL_SPECULAR, spec);
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
    

    GLfloat specular[] =  { 0.2f, 0.2f, 0.2f, 1.0f};
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular);
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 120.0);

    // draw();
    glPushMatrix();
    glMultMatrixd((m_polydron->m_polyhedronPlacement.translation * m_polydron->m_polyhedronPlacement.manualRotation).constData());
    // drawAxes();
    glPopMatrix();

    glEnableClientState(GL_NORMAL_ARRAY);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glEnable(GL_CULL_FACE);

    m_polydron->animate(m_clock.elapsed()*0.001);

    // drawPolyhedron();
    m_texture1.bind();  
    glEnable(GL_TEXTURE_2D);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    
    m_polydron->draw();
    glDisable(GL_TEXTURE_2D);
    m_texture1.release();

    glDisable(GL_CULL_FACE);
    
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);

    glPopMatrix();
}

void PolydronPage::drawPolyhedron()
{
    QMatrix4x4 phMatrix = 
        m_polydron->m_polyhedronPlacement.translation * 
        m_polydron->m_polyhedronPlacement.manualRotation;
    MyPolyhedron *ph = MyPolyhedron::makeImpossible(m_polydron->edgeLength, m_parameter);

    for(int i=0; i<m_polydron->m_types.count(); i++)
    {
        PieceType *pieceType = m_polydron->m_types[i];
        setColor(pieceType->color);
        Mesh *mesh = &pieceType->mesh;
        mesh->bind();        
        for(int i=0;i<ph->faces.count();i++)
        {
            if(pieceType->typeId == ph->faces[i].first)
            {
                glPushMatrix();
                glMultMatrixd((phMatrix * ph->faces[i].second).constData());
                mesh->draw();
                glPopMatrix();
            }
        }
        mesh->release();
    }
    
    delete ph;
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
      m_parameter +=  0.001*delta.x();
      qDebug() << m_parameter;
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

    else if(e->key() == Qt::Key_8) m_polydron->makePolyhedron(time,7);

    else if(e->key() == Qt::Key_Q) m_polydron->makePolyhedron(time,8);
    else if(e->key() == Qt::Key_W) m_polydron->makePolyhedron(time,9);
    else if(e->key() == Qt::Key_E) m_polydron->makePolyhedron(time,10);
    // else if(e->key() == Qt::Key_R) m_polydron->makePolyhedron(time,11);
    else {
      e->ignore();
    }
    updateGL();
}

void PolydronPage::wheelEvent(QWheelEvent*e)
{
    double distance = m_polydron->m_polyhedronPlacement.distance - e->delta() * 0.01;
    // distance = clamp(distance, 1,150);
    m_polydron->m_polyhedronPlacement.distance = distance;
    m_polydron->m_polyhedronPlacement.translation.setToIdentity();
    m_polydron->m_polyhedronPlacement.translation.translate(0,0,-distance);
    updateGL();
}
