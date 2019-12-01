#pragma once


#include "Page.h"
#include "PDiskPage.h"

#include <QString>
#include <QMatrix4x4>


class Fig2Page : public PDiskPage {
  
public:
    Fig2Page();
    ~Fig2Page();
    
    void savePictures();

    void resizeGL(int width, int height);
    
    void paintGL();

};
