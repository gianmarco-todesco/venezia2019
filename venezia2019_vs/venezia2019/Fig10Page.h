#pragma once


#include "Page.h"

#include <QString>
#include <QMatrix4x4>
#include "HyperbolicPolyhedronPage.h"



class Fig10Page : public HyperbolicPolyhedronPage {
protected:
    double m_radius;  
    int m_edgeIndex;
public:
    Fig10Page(double radius);
    ~Fig10Page();

    void paintGL();    
    void savePictures();
    void draw();

    virtual Fig10Page* clone() const = 0;
    virtual QString getFigureName() const = 0;
  
    void keyPressEvent(QKeyEvent *e);
};


class Fig10aPage : public Fig10Page {
public:
    Fig10aPage();
    Fig10Page* clone() const { return new Fig10aPage(); }
    QString getFigureName() const { return "Fig10a.png"; } 
};

class Fig10bPage : public Fig10Page {
public:
    Fig10bPage();
    void draw();
    Fig10Page* clone() const { return new Fig10bPage(); }
    QString getFigureName() const { return "Fig10b.png"; } 
};

