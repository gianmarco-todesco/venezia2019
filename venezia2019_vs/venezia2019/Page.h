#ifndef PAGE_H
#define PAGE_H

#include <qlist.h>

class Viewer;
class QMouseEvent;
class QShowEvent;
class QKeyEvent;
class QWheelEvent;
class QGLShaderProgram;
class Overlay;

class Page {
    Viewer *m_viewer;
    int m_width, m_height;
public:
    Page();
    virtual ~Page();

    int width() const { return m_width; }
    int height() const { return m_height; }
    void setSize(int width, int height);

    Viewer *v() const { return m_viewer; }
    void setViewer(Viewer*viewer) { m_viewer = viewer; }
    
    virtual void start() {}
    virtual void stop() {}

    virtual void initializeGL() {}
    virtual void resizeGL(int width, int height) {}
    virtual void paintGL() {}
    Overlay *getOverlay() const;

    void drawBackground();
    void drawAxes();
    QGLShaderProgram *loadProgram(QString name);
    void setViewUniforms(QGLShaderProgram*);

    virtual void mousePressEvent(QMouseEvent *e) {}
    virtual void mouseMoveEvent(QMouseEvent *e) {}
    virtual void mouseReleaseEvent(QMouseEvent *e) {}

    virtual void keyPressEvent(QKeyEvent *e) {}
    virtual void wheelEvent(QWheelEvent*) {}

    // forse non serve più 
    void updateGL();

    virtual void savePictures() {}
    void savePicture(const QString &path, int border = 0);
};

class Presentation {

    QList<Page*> m_pages;
    Page m_emptyPage;
    Page *m_currentPage;
    int m_currentPageIndex;
    Viewer *m_viewer;
    bool m_started;
    int m_width, m_height;

public:
    Presentation(Viewer *viewer);
    ~Presentation();

    void addPage(Page*page);
    void buildPages();

    Page*p() const { return m_currentPage; }
    Page*getCurrentPage() const { return m_currentPage; }

    void initializeGL();
    void resizeGL(int width, int height);
    void paintGL();
    
    void setPage(int index);
    void nextPage();
    void prevPage();
private:
    void start(Page *page);
    void stop(Page *page);


};

#endif

