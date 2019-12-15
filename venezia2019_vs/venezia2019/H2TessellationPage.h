#pragma once


#include "Page.h"
#include "PDiskPage.h"

#include <QString>
#include <QMatrix4x4>


class H2TessellationPage : public PDiskPage {
protected:
    int m_n, m_q;
    bool m_veryDetailed;
public:
    H2TessellationPage(int n, int q, bool veryDetailed);
    ~H2TessellationPage();
    
    void resizeGL(int width, int height);
    
    void paintGL();

};
