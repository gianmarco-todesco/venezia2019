#ifndef TEXTURE_H
#define TEXTURE_H

#include <QtOpenGL>
#include <map>
#include <QImage>

class Texture
{
public:
  Texture(const QString& fileName);
  virtual ~Texture();

  void load();
  virtual void bind();
  virtual void unbind();
  virtual bool failed() const {return m_failed;}

  static Texture *get(QString fileName);
  static void discard(Texture *texture);

protected:
  GLuint m_texture;
  bool m_failed;
  bool m_loaded;
  QString m_fileName;

private:
  static std::map<QString, Texture *> m_textures;

};


// fatta per paura di rompere le pagine vecchie. Bisogna unificare Texture, MyTexture e OverlayPanel
class MyTexture {
    GLuint m_textureId;
public:
    MyTexture();
    ~MyTexture();

    void createTexture(const QImage &img);
    void destroyTexture();

    void bind();
    void release();
};

#endif // TEXTURE_H
