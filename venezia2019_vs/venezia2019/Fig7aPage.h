#pragma once


#include "Page.h"
#include "PDiskPage.h"

#include <QString>
#include <QMatrix4x4>


class Fig7aPage : public PDiskPage {
  
public:
    Fig7aPage();
    ~Fig7aPage();
    
    void savePictures();

    void resizeGL(int width, int height);
    
    void paintGL();

};
