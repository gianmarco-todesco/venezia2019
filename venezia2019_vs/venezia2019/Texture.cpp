#include "Texture.h"
#include <QImage>
#include <gl/GLU.h>
#include <assert.h>

Texture::Texture(const QString& fileName)
: m_failed(false)
, m_loaded(false)
, m_fileName(fileName)
{
}

void Texture::load()
{
  m_loaded = true;
  m_failed = false;

  QImage image(m_fileName);

  if (image.isNull())
  {
    m_failed = true;
    return;
  }

  glGenTextures(1, &m_texture);

  image = image.convertToFormat(QImage::Format_ARGB32);

  //qDebug() << "Image size:" << image.width() << "x" << image.height();
  int width = image.width();
  int height = image.height();
  //  image = image.scaled(width, height, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

  glBindTexture(GL_TEXTURE_2D, m_texture);

  // Works on x86, so probably works on all little-endian systems.
  // Does it work on big-endian systems?

  //gluBuild2DMipmaps( GL_TEXTURE_2D, 4, width, height,
  //                 GL_BGRA_EXT, GL_UNSIGNED_BYTE, image.bits() ); // GL_BGRA

  gluBuild2DMipmaps( GL_TEXTURE_2D, 4, width, height,
                   GL_BGRA_EXT, GL_UNSIGNED_BYTE, image.bits() );

  //glTexImage2D(GL_TEXTURE_2D, 0, 4, image.width(), image.height(), 0,
  //  GL_BGRA, GL_UNSIGNED_BYTE, image.bits());

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  //glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
  glBindTexture(GL_TEXTURE_2D, 0);
}

Texture::~Texture()
{
  if(m_loaded)
    glDeleteTextures(1, &m_texture);
}

void Texture::bind()
{
  if(!m_loaded) load();
  if(!m_failed)
  {
    glBindTexture(GL_TEXTURE_2D, m_texture);
    glEnable(GL_TEXTURE_2D);
  }
}

void Texture::unbind()
{
  if(!m_failed)
  {
    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_TEXTURE_2D);
  }
}


std::map<QString, Texture*> Texture::m_textures;

Texture *Texture::get(QString fileName)
{
  std::map<QString, Texture*>::iterator it;
  it = m_textures.find(fileName);
  if(it != m_textures.end()) return it->second;
  Texture *texture = new Texture(fileName);
  m_textures[fileName] = texture;
  return texture;
}

void Texture::discard(Texture *texture)
{
  std::map<QString, Texture*>::iterator it;
  for(it = m_textures.begin(); it != m_textures.end();)
  {
    if(it->second == texture)
    {
      delete it->second;
      m_textures.erase(it++);
    }
    else
      ++it;
  }
}


//=============================================================================


MyTexture::MyTexture()
    : m_textureId(0)
{
}

MyTexture::~MyTexture()
{
    if(m_textureId) destroyTexture();
}


void MyTexture::createTexture(const QImage &img)
{
    GLuint err;
    err = glGetError();
    assert(err == GL_NO_ERROR);
    QImage glImg = img.convertToFormat(QImage::Format_ARGB32);
    glGenTextures(1, &m_textureId);
    glBindTexture(GL_TEXTURE_2D, m_textureId);
    gluBuild2DMipmaps( GL_TEXTURE_2D, 4, glImg.width(), glImg.height(),
                   GL_BGRA_EXT, GL_UNSIGNED_BYTE, glImg.bits() );
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    //glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
    glBindTexture(GL_TEXTURE_2D, 0);
    err = glGetError();
    assert(err == GL_NO_ERROR);

}

void MyTexture::destroyTexture()
{
    if(m_textureId) 
    {
        glDeleteTextures(1, &m_textureId);
        m_textureId = 0;
    }
}


void MyTexture::bind()
{
    glBindTexture(GL_TEXTURE_2D, m_textureId);
}

void MyTexture::release()
{
    glBindTexture(GL_TEXTURE_2D, 0);
}

