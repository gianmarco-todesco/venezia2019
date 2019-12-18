#pragma once


#include "Page.h"
#include "HyperbolicPolyhedronPage.h"

#include <QString>
#include <QMatrix4x4>


class Fig9Page : public HyperbolicPolyhedronPage {
  
public:
    Fig9Page();
    ~Fig9Page();

    void paintGL();    
    void savePictures();

    virtual Fig9Page* clone() const = 0;
    virtual QString getFigureName() const = 0;
    virtual void draw() = 0;

};


class Fig9aPage : public Fig9Page {
public:
    void draw();
    Fig9aPage();
    Fig9Page* clone() const { return new Fig9aPage(); }
    QString getFigureName() const { return "Fig9a.png"; } 
};

class Fig9bPage : public Fig9Page {
public:
    Fig9bPage();
    void draw();
    Fig9Page* clone() const { return new Fig9bPage(); }
    QString getFigureName() const { return "Fig9b.png"; } 
};

class Fig9cPage : public Fig9Page {
public:
    Fig9cPage();
    void draw();
    Fig9Page* clone() const { return new Fig9cPage(); }
    QString getFigureName() const { return "Fig9c.png"; } 
};
