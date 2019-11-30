#pragma once


#include "Page.h"
#include "PDiskPage.h"

#include <QString>
#include <QMatrix4x4>


class Fig8bPage : public PDiskPage {
  
public:
    Fig8bPage();
    ~Fig8bPage();
    
    void savePictures();

    void resizeGL(int width, int height);
    
    void paintGL();

};
