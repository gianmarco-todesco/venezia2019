#ifndef OVERLAY_H
#define OVERLAY_H

#include "Gutil.h"
#include <QImage>
#include <QList>
#include <QTime>

class OverlayPanel {
    GLuint m_textureId;
    int m_width, m_height;

public:
    OverlayPanel(const QImage &image);
    OverlayPanel(const QString imageFileName);



};



class Overlay {
    QList<OverlayPanel*> m_panels;
    QTime m_clock;
public:

    Overlay();
    ~Overlay();

    void initializeGL();

    void add(OverlayPanel *panel);
    void remove(OverlayPanel *panel);

    void draw(int width, int height);
};

#endif

