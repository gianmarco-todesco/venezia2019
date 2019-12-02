#ifndef OVERLAY_H
#define OVERLAY_H

#include "Gutil.h"
#include <QImage>
#include <QList>
#include <QTime>
#include <QPointF>
#define _USE_MATH_DEFINES
#include <math.h>

class Viewer;

class OverlayAnimation;

class OverlayPanel {
    GLuint m_textureId;
    int m_width, m_height;
    double m_size; // percentage
    QPointF m_pos; // percentage
    bool m_hasAlpha;

public:
    OverlayPanel();
    ~OverlayPanel();

    void setSize(double sz) { m_size = sz; }
    void setPosition(double x, double y) { m_pos = QPointF(x,y); }
    void setPosition(const QPointF &pos) { m_pos = pos; }
    QPointF getPosition() const { return m_pos; }

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
    QList<OverlayAnimation*> m_animations;
public:

    Overlay(Viewer *);
    ~Overlay();

    void initializeGL();

    void clear() {
        removeAll();
    }

    void move(OverlayPanel *panel, 
        const QPointF &p0, const QPointF &p1, 
        double duration = 1.0,
        double delay = 0.0);

    void addAndMove(OverlayPanel *panel, 
        const QPointF &p0, const QPointF &p1, 
        double duration = 1.0,
        double delay = 0.0);

    void moveAndRemove(OverlayPanel *panel, 
        const QPointF &p0, const QPointF &p1, 
        double duration = 1.0,
        double delay = 0.0);


    void add(OverlayPanel *panel);
    void add(OverlayPanel *panel, const QPointF &pos) {
        panel->setPosition(pos); add(panel);
    }

    void remove(OverlayPanel *panel);
    void removeAll();

    void draw(const QSize &winSize);

    void addAnimation(OverlayAnimation *animation);

    void animate();
};

class OverlayAnimation {
public:
    virtual ~OverlayAnimation() {}
    double m_startTime;
    OverlayAnimation() : m_startTime(0) {}
    virtual bool tick(Overlay *overlay, double time) = 0; // return true when finished

};


class OverlayMovement : public OverlayAnimation {
public:
    OverlayPanel*m_panel;
    const QPointF m_p0;
    const QPointF m_p1; 
    double m_duration, m_delay;
    bool m_deleteOnFinish;

    OverlayMovement(
        OverlayPanel*panel, 
        const QPointF &p0, 
        const QPointF &p1, 
        double duration)
        : m_panel(panel)
        , m_p0(p0)
        , m_p1(p1)
        , m_duration(duration)
        , m_delay(0)
        , m_deleteOnFinish(false)
    {}
    bool tick(Overlay *overlay, double time);
};


#endif

