#pragma once


#include "Page.h"
#include "PDiskPage.h"

#include <QString>
#include <QMatrix4x4>


class Fig8aPage : public PDiskPage {
  
public:
    Fig8aPage();
    ~Fig8aPage();
    
    void savePictures();

    void resizeGL(int width, int height);
    
    void paintGL();

};
