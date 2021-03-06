#ifndef CIRCLELIMIT3VIEWER_H
#define CIRCLELIMIT3VIEWER_H

#include "HTessellationViewer.h"

class CircleLimit3Tessellation;

class CircleLimit3Viewer : public HTessellationViewer
{
  CircleLimit3Tessellation *m_tess;
  Texture *m_fish[4];
  int m_maxFaceCount;
  bool m_figureMode;

public:
  int m_status;


  CircleLimit3Viewer(bool figureMode = false);
  ~CircleLimit3Viewer();

  bool onKeyDown(int key);

  void foo();

  void draw();
  void drawTess();
  void drawTess2(double thickness);

  void loadTextures();
};

#endif // CIRCLELIMIT3VIEWER_H
