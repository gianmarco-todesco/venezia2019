#ifndef OVERLAY_H
#define OVERLAY_H

#include "Gutil.h"
#include <QImage>
#include <QList>
#include <QTime>
#include <QPointF>

class Viewer;


class OverlayPanel {
    GLuint m_textureId;
    int m_width, m_height;
    double m_size; // percentage
    QPointF m_pos; // percentage
    bool m_hasAlpha;

public:
    // nota: il pannello va creato dentro la initializeGL()
    OverlayPanel(const QImage &image);

    void setSize(double sz) { m_size = sz; }
    void setPosition(double x, double y) { m_pos = QPointF(x,y); }

    bool hasAlpha() const { return m_hasAlpha; }
    void setHasAlpha(bool hasAlpha) { m_hasAlpha = hasAlpha; }

    void draw(const QSize &winSize);

    void createTexture(const QImage &img);
    void deleteTexture();

   
};



class Overlay {
    QList<OverlayPanel*> m_panels;
    QTime m_clock;
    Viewer *m_viewer;
public:

    Overlay(Viewer *);
    ~Overlay();

    void initializeGL();

    void add(OverlayPanel *panel);
    void remove(OverlayPanel *panel);

    void draw(const QSize &winSize);
};

#endif

