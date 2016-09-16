#ifndef GLUTILS_HPP
#define GLUTILS_HPP

#include <QVector3D>

// Modern OpenGL
#include <QOpenGLFunctions_3_3_Core>
#include <QOpenGLShader>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLFramebufferObject>
#include <QOpenGLTexture>

using oglShader		= QOpenGLShader;
using oglShaderP	= QOpenGLShaderProgram;
using oglVAO		= QOpenGLVertexArrayObject;
using oglBuffer		= QOpenGLBuffer;
using oglFBO		= QOpenGLFramebufferObject;
using oglTexture	= QOpenGLTexture;
using oglFuncs		= QOpenGLFunctions_3_3_Core;

#endif // GLUTILS_HPP
