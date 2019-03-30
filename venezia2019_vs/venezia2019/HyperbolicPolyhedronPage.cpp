#include "HyperbolicPolyhedronPage.h"

#include "Polyhedra.h"
#include "Gutil.h"
#include "Point3.h"

#include "Mesh.h"
#include "H3.h"

#include <assert.h>
#include <QKeyEvent>
#include <QWheelEvent>
#include <QPainter>

#include <vector>

#include <QGLShaderProgram>
#include <qmatrix4x4.h>
#include <qvector.h>
#include <qlist.h>
#include <qmap.h>



HyperbolicPolyhedronPage::HyperbolicPolyhedronPage()
: m_cameraDistance(15)
, m_theta(10)
, m_phi(20)
, m_rotating(true)
, m_parameter(0)
, m_status(0)
{
    m_hMatrix.setToIdentity();
}

HyperbolicPolyhedronPage::~HyperbolicPolyhedronPage()
{
}

void HyperbolicPolyhedronPage::initializeGL()
{
    // m_mesh.makeSphere(1,20,20);
    double d = 0.7;
    int m = 40;
    double q = d / (m-1);
    m_mesh.m_hasTexCoords = true;
    m_mesh.addFace(QVector3D(-d,-d, 0), QVector3D(2,0,0)*q, QVector3D(0.0,2,0)*q, m,m);
    m_mesh.createBuffers();

    m_outSphereMesh.makeSphere(5.0, 50,50);

    createHLineMesh();


    m_h3ShaderProgram = loadProgram("h3basic");
    m_stdShaderProgram = loadProgram("basic");


    QImage img(256,256, QImage::Format_ARGB32_Premultiplied);
    img.fill(QColor(255,255,255));
    QPainter pa;
    pa.begin(&img);
    pa.setFont(QFont("Arial",80,QFont::Bold));
    pa.setPen(Qt::black);
    // pa.drawRect(0,0,255,255);
    // pa.fillRect(0,0,128,256, QColor(100,100,0));
    //pa.drawText(QRect(0,0,256,256), Qt::AlignCenter, "Hello!");
    pa.end();   
    m_texture1.createTexture(img);

    img = QImage(256,256, QImage::Format_ARGB32_Premultiplied);
    img.fill(QColor(160,160,160));
    pa.begin(&img);
    pa.setPen(QPen(Qt::black,2));
    pa.drawRect(0,0,255,255);
    pa.end();   
    m_texture2.createTexture(img);

    /*
    img = QImage(32,32, QImage::Format_ARGB32_Premultiplied);
    img.fill(QColor(200,120,20));
    pa.begin(&img);
    pa.setPen(QPen(Qt::black,2));
    pa.drawRect(0,0,255,255);
    pa.end();   
    m_texture2.createTexture(img);
    */
}

void HyperbolicPolyhedronPage::start()
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

    m_hMatrix.setToIdentity();  
    m_clock.start();

}

void HyperbolicPolyhedronPage::resizeGL(int width, int height)
{
    double aspect = (float)width/height;
    glViewport(0,0,width,height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45, aspect, 1.0, 70.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void HyperbolicPolyhedronPage::createHLineMesh()
{
    m_hlineMesh.m_hasTexCoords = true;
    int m = 5, n = 100;
    double r = 0.015;
    QList<QVector4D> pts;
    QList<QVector3D> nrms;
    for(int i=0; i<m; i++)
    {
        double phi = 2*M_PI*i/(m-1);
        double cs = cos(phi), sn = sin(phi);
        nrms.append(QVector3D(cs, 0, sn));
        pts.append(QVector4D(cs*r, 0, sn*r, 1.0));
    }
    for(int i= 0; i<n; i++)
    {
        double y = 0.99999 * (-1.0+2.0*i/(n-1));
        QMatrix4x4 mat = H3::KModel::translation(QVector3D(0,0,0), QVector3D(0,y,0));
        for(int j=0; j<m; j++)
        {
            m_hlineMesh.addVertex(mat.map(pts[j]).toVector3DAffine(), nrms[j], QPointF( (double)i/(n-1), (double)j/(m-1)));
        }
    }
    m_hlineMesh.addTriangleGrid(n,m);
    m_hlineMesh.createBuffers();
}


void HyperbolicPolyhedronPage::paintGL()
{
    glClearColor(0,0,0,1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    drawBackground();
    
    glPushMatrix();
    glTranslated(0,0,-m_cameraDistance);
    glRotated(m_theta, 1,0,0);
    glRotated(m_phi, 0,1,0);

    GLfloat specular[] =  { 0.7f, 0.7f, 0.7f, 1.0f};
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular);
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 90.0);

    draw();
    drawOutSphere();

    glPopMatrix();
}

QVector3D rainbow(double t)
{
    const QVector3D colors[] = {
        QVector3D(1,0,0),
        QVector3D(1,1,0),
        QVector3D(0,1,0),
        QVector3D(0,1,1),
        QVector3D(0,0,1),
        QVector3D(1,0,1),
    };
    t = (t-floor(t)) * 6;
    int k = (int)floor(t);
    t -= k;
    int k1 = (k+1)%6;
    return colors[k] * (1-t) + colors[k1] * t;
}

QMatrix4x4 makeTranslation(double x, double y, double z)
{
    double kx =  x / sqrt(1.0 + x*x);
    double ky =  y / sqrt(1.0 + y*y);
    double kz =  z / sqrt(1.0 + z*z);
    return H3::KModel::translation(QVector3D(0,0,0), QVector3D(kx,ky,kz));
}

void HyperbolicPolyhedronPage::draw()
{
    QGLShaderProgram *prog;

    if(m_status < 4)
    {
        m_hMatrix = makeTranslation(0,0,m_parameter);
    }
    else
    {
        m_hMatrix.setToIdentity();
    }

    //double z0 = m_parameter / sqrt(1.0 + m_parameter*m_parameter);
    //QMatrix4x4 globalMat = H3::KModel::translation(QVector3D(0,0,0), QVector3D(0,0,z0));
    if(m_status == 0)
    {
        // solo punti
        setColor(0,0.5,0.9);
        double r = 0.5;
        drawHPoint(QVector3D(r,r,-r));
        drawHPoint(QVector3D(r,-r,r));
        drawHPoint(QVector3D(-r,r,r));
        drawHPoint(QVector3D(-r,-r,-r));
    }
    else if(m_status == 1) {
        // punto e retta.
        setColor(0,0.5,0.9);
        drawHPoint(QVector3D(0,0,0));
        QMatrix4x4 identity; identity.setToIdentity();
        drawHLine(identity);
    }
    else if(m_status == 2) {
        // diversi punti e diverse rette

        m_hMatrix = makeTranslation(m_hPan.x(),0,m_hPan.y());
        

        QVector3D p0(-0.5,0,0), p1(0.5,0,0);
        QVector3D p(0.2,0.4,0.1);
        setColor(0,0.5,0.9);
        drawHPoint(p0);
        drawHPoint(p1);
        drawHPoint(p);
        drawHLine(p0,p);
        drawHLine(p1,p);
        drawHLine(p0,p1);
    }
    else if(m_status == 3) {
        // draw faces
        
        m_texture2.bind();    
        prog = m_h3ShaderProgram;
        prog->bind();
        // setViewUniforms(prog);
        prog->setUniformValue("u_texture", 0);
        prog->setUniformValue("u_texScale", QPointF(1,1));

        glEnableClientState(GL_NORMAL_ARRAY);
        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        m_mesh.bind();

        int m = 10;
        for(int i=0; i<=m;i++)
        {

            prog->setUniformValue("u_color", rainbow(0.5*(double)i/m));
            double z = 0.9*(i-5.0)/5.0;
            QMatrix4x4 mat = H3::KModel::translation(QVector3D(0,0,0), QVector3D(0,0,z));
            prog->setUniformValue("hMatrix", m_hMatrix * mat);
            m_mesh.draw();
        }
        glDisableClientState(GL_NORMAL_ARRAY);
        glDisableClientState(GL_VERTEX_ARRAY);
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);    
        m_mesh.release();
        prog->release();
        m_texture2.release();
    }
    else
    {
        // hpolyhedron 

        m_texture1.bind();    
        prog = m_h3ShaderProgram;
        prog->bind();
        // setViewUniforms(prog);
        prog->setUniformValue("u_texture", 0);
        prog->setUniformValue("u_texScale", QPointF(20,20));   
        prog->setUniformValue("hMatrix", m_hMatrix);
        prog->setUniformValue("u_color", QVector3D(1,1,1));

        double radius = qMax(0.1, qMin(0.99, 0.4 + m_parameter));
        setColor(1,0,1);
        Polyhedron *ph = makeDodecahedron();
        ph->computeFaceVertices();
        ph->scale(radius / ph->getVertex(0).m_pos.length());
        drawHPolyhedron(ph);

        m_texture1.release();
        prog->release();

        if(m_status == 5)
        {
            // vertex angle
            const Polyhedron::Face &face = ph->getFace(0);

            QVector3D p0 = ph->getVertex(face.m_vertices[1]).m_pos;
            QVector3D p1 = ph->getVertex(face.m_vertices[0]).m_pos;
            QVector3D p2 = ph->getVertex(face.m_vertices[2]).m_pos;
            drawHLine(p0,p1);
            drawHLine(p0,p2);

            drawHAngle(p0,p0-p1,p0-p2);

        } else if(m_status == 6)
        {
            int n = ph->getVertexCount();
            for(int i=0;i<n;i++)
            {
                const QVector3D p1 = ph->getVertex(i).m_pos;
                QVector3D p2;
                double maxDist = 0;
                int k = -1;
                for(int j=0; j<n; j++)
                {
                    const QVector3D q = ph->getVertex(j).m_pos;
                    double d = (p1-q).length();
                    if(d>maxDist) { k= j; maxDist = d; p2 = q; }
                }
                if(i<k) {
                    drawHLine(p1,p2);
                }
            }
        }
        else if(m_status == 7)
        {
            // dihedral angle
            const Polyhedron::Edge &edge = ph->getEdge(0);
            QVector3D midPoint = (
                ph->getVertex(edge.m_a).m_pos + 
                ph->getVertex(edge.m_b).m_pos)*0.5;
            QList<int> fis;
            for(int i=0;i<ph->getFaceCount();i++)
            {
                bool found = false;
                const Polyhedron::Face &face = ph->getFace(i);
                for(int j=0;j<face.m_edges.size();j++) 
                    if(face.m_edges[j]==0)
                    {
                        found=true;
                        break;
                    }
                if(found)
                    fis.append(i);
            }
            if(fis.count() == 2)
            {
                const QVector3D p0 = getFaceCenter(ph, fis[0]);
                const QVector3D p1 = getFaceCenter(ph, fis[1]);
                drawHLine(midPoint,p0);
                drawHLine(midPoint,p1);
                drawHAngle(midPoint,midPoint-p0,midPoint-p1);
            }

        } else if(m_status == 6)
        delete ph;
    }
    /*
    else if(m_status==8) 
    {
        double z0 = m_parameter / sqrt(1.0 + m_parameter*m_parameter);
        QMatrix4x4 globalMat = H3::KModel::translation(QVector3D(0,0,0), QVector3D(0,0,z0));

        QVector<QVector3D> pts;
        QVector3D p0(0.5,0.3,0.1);

        for(int i=0;i<5;i++) 
        {
            const double phi = 2*M_PI*i/5;
            pts.append(QVector3D(cos(phi)*0.2+0.3, 0, sin(phi)*0.2-0.1));
        }
        drawHPoint(p0);
        for(int i=0;i<5;i++) 
        {
            drawHPoint(pts[i]);
            drawHLine(p0,pts[i]);
            drawHLine(pts[i], pts[(i+1)%5]);
        }
    }

    else if(m_status == 8)
    {
        m_texture1.bind();    
        prog = m_h3ShaderProgram;
        prog->bind();
        // setViewUniforms(prog);
        prog->setUniformValue("u_texture", 0);
        prog->setUniformValue("u_texScale", QPointF(20,20));   
        prog->setUniformValue("hMatrix", m_hMatrix);
        prog->setUniformValue("u_color", QVector3D(1,1,1));

        
        setColor(1,0,1);
        Polyhedron *ph = makeDodecahedron();
        ph->computeFaceVertices();
        double radius = 0.8 + m_parameter;
        ph->scale(radius / ph->getVertex(0).m_pos.length());
        drawHPolyhedron(ph);

        const Polyhedron::Face &face = ph->getFace(0);

        QVector3D p0 = ph->getVertex(face.m_vertices[1]).m_pos;
        QVector3D p1 = ph->getVertex(face.m_vertices[0]).m_pos;
        QVector3D p2 = ph->getVertex(face.m_vertices[2]).m_pos;
        drawHLine(p0,p1);
        drawHLine(p0,p2);

        drawHAngle(p0,p0-p1,p0-p2);
        

        delete ph;

        m_texture1.release();
        prog->release();

    }
    */
}


void HyperbolicPolyhedronPage::drawHPoint(const QVector3D &pos)
{
    drawSphere(H3::KModel::toBall(m_hMatrix.map(QVector4D(pos,1.0))), 0.1);
}
    

void HyperbolicPolyhedronPage::drawHLine(const QMatrix4x4 &matrix)
{
    QList<QMatrix4x4> matrices; matrices << matrix;
    drawHLines(matrices);
}

void HyperbolicPolyhedronPage::drawHLine(const QVector3D &p0, const QVector3D&p1)
{
    QList<QPair<QVector3D, QVector3D> > pointPairs;
    pointPairs << qMakePair(p0,p1);
    drawHLines(pointPairs);
}

void HyperbolicPolyhedronPage::drawHLines(const QList<QMatrix4x4> &matrices)
{
    // 
    m_texture1.bind();    
    QGLShaderProgram *prog = m_h3ShaderProgram;
    prog->bind();
    // setViewUniforms(prog);
    prog->setUniformValue("u_texture", 0);
    prog->setUniformValue("u_texScale", QPointF(20,20));
    prog->setUniformValue("u_color", QVector3D(0.05,0.3,0.8));
    setColor(1,1,1);
    glEnableClientState(GL_NORMAL_ARRAY);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    m_hlineMesh.bind();

    foreach(QMatrix4x4 mat, matrices)
    {
        prog->setUniformValue("hMatrix", m_hMatrix * mat);
        m_hlineMesh.draw();
    }
    m_hlineMesh.release();
    
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    prog->release();
    m_texture1.release();
}

void HyperbolicPolyhedronPage::drawHLines(const QList<QPair<QVector3D, QVector3D> > &pointPairs)
{
    QList<QMatrix4x4> matrices;
    for(int i=0;i<pointPairs.count();i++)
    {
        QVector3D p0 = pointPairs[i].first;
        QVector3D p1 = pointPairs[i].second;
        if((p0-p1).length()<0.00001) continue;

        QVector3D e = (p1-p0).normalized();
        QVector3D center = p0 + e* QVector3D::dotProduct(e,-p0);
        p0 = center;

        double c2 = center.lengthSquared();
        if(c2>=1.0) continue;
        double s = sqrt(0.999 - c2);
        p1 = center + s * e;
        

        QMatrix4x4 mat;
        mat = H3::KModel::translation(QVector3D(0,0,0), p0);

        QVector3D du = mat.inverted().map(QVector4D(p1,1.0)).toVector3DAffine();
        if(du.lengthSquared()>0.0001)
        {
            du.normalize();
            if(du.y()<0) du = -du;
            double theta = acos(du.y());
            double phi = atan2(du.z(),du.x());

            QMatrix4x4 rot; 
            rot.setToIdentity(); 
            rot.rotate(180*theta/M_PI,0,0,1);
            rot.rotate(180*phi/M_PI,0,1,0);

            mat = mat * rot.inverted();
        }

        matrices.append(mat);

    }
    drawHLines(matrices);
}
    
void HyperbolicPolyhedronPage::drawHAngle(
    const QVector3D &kp, 
    const QVector3D &kd1, 
    const QVector3D &kd2)
{
    glDisable(GL_LIGHTING);
    glColor3d(1,0,1);

    const QVector3D p = H3::KModel::toBall(kp);
    const QVector3D d1 = (H3::KModel::toBall(kp+kd1*0.001)-p).normalized();
    const QVector3D d2 = (H3::KModel::toBall(kp+kd2*0.001)-p).normalized();


    glBegin(GL_LINE_STRIP);
    glVertex(p+d1);
    glVertex(p);
    glVertex(p+d2);
    glEnd();

    QVector3D e0 = d1.normalized();
    QVector3D e1 = (d2-e0*QVector3D::dotProduct(e0,d2)).normalized();
    double phi = acos(QVector3D::dotProduct(d1,d2));
    double r1 = 1;
    double r2 = 1.5;
    double dphi = 5 * M_PI/180;
    QVector<QVector3D> pts;
    double a = 0;
    for(;;a+=dphi)
    {
        if(a>phi) a=phi;
        double cs = cos(a);
        double sn = sin(a);
        pts.append(p + e0*(cs*r1) + e1*(sn*r1));
        pts.append(p + e0*(cs*r2) + e1*(sn*r2));
        if(a>=phi) break;
    }
    int m = pts.count()/2;
    glBegin(GL_LINE_STRIP);
    for(int i=0;i<m;i++) glVertex(pts[2*i]);
    glEnd();
    glBegin(GL_LINE_STRIP);
    for(int i=0;i<m;i++) glVertex(pts[2*i+1]);
    glEnd();
    glBegin(GL_LINES);
    for(int i=0;i<m;i++) {glVertex(pts[2*i]);glVertex(pts[2*i+1]);}
    glEnd();


    glEnable(GL_LIGHTING);

}


void HyperbolicPolyhedronPage::drawOutSphere()
{

    QGLShaderProgram *prog = m_stdShaderProgram;
    prog->bind();
    setViewUniforms(prog);
    setColor(0.05,0.08,0.08);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);
    glEnableClientState(GL_NORMAL_ARRAY);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnable(GL_CULL_FACE);
    glDepthMask(false);
    m_outSphereMesh.bind();

    m_outSphereMesh.draw();
    m_outSphereMesh.release();
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisable(GL_BLEND);
    glDisable(GL_CULL_FACE);

    prog->release();
    glDepthMask(true);

    glDisable(GL_LIGHTING);

    int m = 200;
    double r = 5.002;

    QVector<QPair<double, double> > cssn;
    for(int i=0;i<m;i++) {
        double phi = M_PI * 2 * i / (m-1);
        cssn.append(qMakePair(cos(phi), sin(phi)));
    }
    glColor3d(0.5,0.5,0.5);    
    glBegin(GL_LINE_STRIP);
    for(int i=0;i<m;i++) glVertex3d(cssn[i].first*r,cssn[i].second*r,0);
    glEnd();
    glBegin(GL_LINE_STRIP);
    for(int i=0;i<m;i++) glVertex3d(cssn[i].first*r,0,cssn[i].second*r);
    glEnd();
    glBegin(GL_LINE_STRIP);
    for(int i=0;i<m;i++) glVertex3d(0,cssn[i].first*r,cssn[i].second*r);
    glEnd();

    r = 5;
    double rr = r/sqrt(-r*r+m_cameraDistance*m_cameraDistance);
    glPushMatrix();
    glLoadIdentity();
    // glTranslated(0,0,-m_cameraDistance);
    double z = 10;
    glBegin(GL_LINE_STRIP);
    for(int i=0;i<m;i++) glVertex3d(z*rr*cssn[i].first,z*rr*cssn[i].second,-z);       
    glEnd();
    
    
    glPopMatrix();
    
    glBegin(GL_LINES);
        glColor3d(1,0,0);
        glVertex3d(0,0,0);
        glVertex3d(r,0,0);
        glColor3d(0,1,0);
        glVertex3d(0,0,0);
        glVertex3d(0,r,0);
        glColor3d(0,0,1);
        glVertex3d(0,0,0);
        glVertex3d(0,0,r);

    glEnd();

    glEnable(GL_LIGHTING);
}

namespace  {
    inline void addVertex(QVector<GLfloat> &buffer, 
        const QVector3D &p, 
        const QVector3D &normal)
    {
        buffer << p.x() << p.y() << p.z() << normal.x() << normal.y() << normal.z();    
    }
    inline void addVertex(QVector<GLfloat> &buffer, 
        const QVector3D &p, 
        const QVector3D &normal,
        const QPointF &uv)
    {
        buffer << p.x() << p.y() << p.z() << normal.x() << normal.y() << normal.z() << uv.x() << uv.y();    
    }
}

void HyperbolicPolyhedronPage::drawHPolygon(const QList<QVector3D> &pts)
{
    setColor(0.8,0.5,0.1);
    m_h3ShaderProgram->setUniformValue("u_color", QVector3D(0.8,0.5,0.05));
    
    const int n = pts.count();
    QVector3D center;
    foreach(QVector3D p, pts) center+=p;
    center *= 1.0/n;
    QVector3D normal = QVector3D::crossProduct(pts[1]-center, pts[0]-center).normalized();
    QVector<GLushort> indices;
    QVector<GLfloat> buffer;
    QList<QVector3D> qs;
    qs.append(center);
    addVertex(buffer, center, normal);
    int m = 30;
    for(int i=0;i<n;i++)
    {
        for(int j=1;j<=m;j++)
        {
            double t = (double)j/m;
            QVector3D q = (1-t)*center + t*pts[i];
            qs.append(q);
            addVertex(buffer, q, normal);
        }
    }
    int k = 1+n*m;
    for(int i=0; i<n; i++)
    {
        int i1 = (i+1)%n;        
        indices << 0 << i*m+1 << i1*m+1;
        for(int j=1;j<m;j++) 
        {
            int a = i*m+j, b = i*m+j+1, c = i1*m+j, d = i1*m+j+1;   
            int as = a, bs = b;
            double t;
            for(int s=1; s<j; s++)
            {
                t = (double)s/j;
                addVertex(buffer, qs[a]*(1-t)+qs[c]*t, normal);
                t = (double)s/(j+1);
                addVertex(buffer, qs[b]*(1-t)+qs[d]*t, normal);
                indices << as << bs << k+1 << as << k+1 << k;
                as = k;
                bs = k+1;
                k+=2;
            }
            t = (double)j/(j+1);
            addVertex(buffer, qs[b]*(1-t)+qs[d]*t, normal);
            indices << as << bs << k << c << as << k << c << k << d;
            k++;
        }
    }
    glEnableClientState(GL_NORMAL_ARRAY);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnable(GL_CULL_FACE);
    glNormalPointer(GL_FLOAT, sizeof(GL_FLOAT)*6, &buffer[0]+3);
    glVertexPointer(3, GL_FLOAT, sizeof(GL_FLOAT)*6, &buffer[0]);
    //glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_SHORT, &indices[0]);
    // glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
    glDisable(GL_CULL_FACE);
    
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);

}

void HyperbolicPolyhedronPage::drawHPolyhedron(const Polyhedron *ph)
{
    for(int i=0;i<ph->getFaceCount();i++)
    {
        const Polyhedron::Face &face = ph->getFace(i);
        QList<QVector3D> pts;
        for(int j=0;j<face.m_vertices.size();j++) 
            pts.append(ph->getVertex(face.m_vertices[j]).m_pos);
        drawHPolygon(pts);
    }
}



void HyperbolicPolyhedronPage::mousePressEvent(QMouseEvent *e)
{
      m_lastPos = e->pos();
      m_rotating = e->button() == Qt::RightButton;
}

void HyperbolicPolyhedronPage::mouseReleaseEvent(QMouseEvent *e)
{

}


void HyperbolicPolyhedronPage::mouseMoveEvent(QMouseEvent *e)
{
  QPoint delta = m_lastPos - e->pos();
  m_lastPos = e->pos();


  if(!m_rotating)
  {
      m_parameter += 0.005*delta.x();
      m_hPan += 0.005*QPointF(delta);

  }
  else
  {
    m_phi -= 0.25*delta.x();
    m_theta -= 0.25*delta.y();

  }
  // updateGL();
}

void HyperbolicPolyhedronPage::keyPressEvent(QKeyEvent *e)
{
    if(e->key() == Qt::Key_Right) setStatus(m_status+1);
    else if(e->key() == Qt::Key_Left) setStatus(m_status-1);
    e->ignore();
}

void HyperbolicPolyhedronPage::wheelEvent(QWheelEvent*e)
{
    m_cameraDistance = clamp(m_cameraDistance - e->delta() * 0.01, 1,50);
}


void HyperbolicPolyhedronPage::setStatus(int status)
{
    m_hPan = QPointF();
    m_parameter = 0;
    m_hMatrix.setToIdentity();
    m_status = qMax(0, status);
}


#ifdef DOPO


    else if(m_status == 4) {

        double z0 = m_parameter / sqrt(1.0 + m_parameter*m_parameter);
        QMatrix4x4 globalMat = H3::KModel::translation(QVector3D(0,0,0), QVector3D(0,0,z0));
        /*
        // 
        m_texture1.bind();    
        prog = m_h3ShaderProgram;
        prog->bind();
        // setViewUniforms(prog);
        prog->setUniformValue("u_texture", 0);
        prog->setUniformValue("u_texScale", QPointF(20,20));
        setColor(1,1,1);
        glEnableClientState(GL_NORMAL_ARRAY);
        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        m_hlineMesh.bind();

        prog->setUniformValue("hMatrix", globalMat);
        m_hlineMesh.draw();
        prog->setUniformValue("hMatrix", H3::KModel::translation(QVector3D(0,0,0), QVector3D(0.1,0,0)));
        m_hlineMesh.draw();
        m_hlineMesh.release();
    
        glDisableClientState(GL_NORMAL_ARRAY);
        glDisableClientState(GL_VERTEX_ARRAY);
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
        prog->release();
        m_texture1.release();
        */
        QList<QMatrix4x4> matrices;
        matrices << globalMat;
        for(int i=0;i<10;i++) 
            matrices << H3::KModel::translation(QVector3D(0,0,0), QVector3D(0.05*i,0,0));
        drawHLines(matrices);
    }

#endif
