/*
 * #%L
 * OME-QTWIDGETS C++ library for display of OME-Files pixel data and metadata.
 * %%
 * Copyright © 2014 - 2015 Open Microscopy Environment:
 *   - Massachusetts Institute of Technology
 *   - National Institutes of Health
 *   - University of Dundee
 *   - Board of Regents of the University of Wisconsin-Madison
 *   - Glencoe Software, Inc.
 * Copyright © 2018 Quantitative Imaging Systems, LLC
 * %%
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDERS OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * The views and conclusions contained in the software and documentation are
 * those of the authors and should not be interpreted as representing official
 * policies, either expressed or implied, of any organization.
 * #L%
 */

#include <ome/qtwidgets/glm.h>
#include <ome/qtwidgets/glsl/GLImageShader2D.h>
#include <ome/qtwidgets/gl/Util.h>

#include <iostream>
#include <sstream>

using ome::qtwidgets::gl::check_gl;

namespace ome
{
  namespace qtwidgets
  {
    namespace glsl
    {

      GLImageShader2D::GLImageShader2D(QObject *parent,
                                       bool     rgb):
        QOpenGLShaderProgram(parent),
        vshader(),
        fshader(),
        attr_coords(),
        attr_texcoords(),
        uniform_mvp(),
        uniform_texture(),
        uniform_lut(),
        uniform_min(),
        uniform_max(),
        rgb(rgb)
      {
        initializeOpenGLFunctions();

        vshader = new QOpenGLShader(QOpenGLShader::Vertex, this);

        vshader->compileSourceCode
          ("#version 410 core\n"
           "\n"
           "layout (location = 0) in vec2 coord2d;\n"
           "layout (location = 1) in vec2 texcoord;\n"
           "uniform mat4 mvp;\n"
           "\n"
           "out VertexData\n"
           "{\n"
           "  vec2 f_texcoord;\n"
           "} outData;\n"
           "\n"
           "void main(void) {\n"
           "  gl_Position = mvp * vec4(coord2d, 0.0, 1.0);\n"
           "  outData.f_texcoord = texcoord;\n"
           "}\n");

        if (!vshader->isCompiled())
          {
            std::cerr << "GLImageShader2D: Failed to compile vertex shader\n" << vshader->log().toStdString() << std::endl;
          }

        fshader = new QOpenGLShader(QOpenGLShader::Fragment, this);
        if (!rgb)
          {
            fshader->compileSourceCode
              ("#version 410 core\n"
               "\n"
               "uniform sampler2D tex;\n"
               "uniform sampler1DArray lut;\n"
               "uniform vec3 texmin;\n"
               "uniform vec3 texmax;\n"
               "uniform vec3 correction;\n"
               "\n"
               "in VertexData\n"
               "{\n"
               "  vec2 f_texcoord;\n"
               "} inData;\n"
               "\n"
               "out vec4 outputColour;\n"
               "\n"
               "void main(void) {\n"
               "  vec2 flipped_texcoord = vec2(inData.f_texcoord.x, 1.0 - inData.f_texcoord.y);\n"
               "  vec4 texval = texture(tex, flipped_texcoord);\n"
               "\n"
               "  outputColour = texture(lut, vec2(((((texval[0] * correction[0]) - texmin[0]) / (texmax[0] - texmin[0]))), 0.0));\n"
               "}\n");
          }
        else
          {
            fshader->compileSourceCode
              ("#version 410 core\n"
               "\n"
               "uniform sampler2D tex;\n"
               "\n"
               "in VertexData\n"
               "{\n"
               "  vec2 f_texcoord;\n"
               "} inData;\n"
               "\n"
               "out vec4 outputColour;\n"
               "\n"
               "void main(void) {\n"
               "  vec2 flipped_texcoord = vec2(inData.f_texcoord.x, 1.0 - inData.f_texcoord.y);\n"
               "  vec4 texval = texture(tex, flipped_texcoord);\n"
               "\n"
               "  outputColour = vec4(texval[0], texval[1], texval[2], 1.0);\n"
               "}\n");
          }

        if (!fshader->isCompiled())
          {
            std::cerr << "GLImageShader2D: Failed to compile fragment shader\n" << fshader->log().toStdString() << std::endl;
          }

        addShader(vshader);
        addShader(fshader);
        link();

        if (!isLinked())
          {
            std::cerr << "GLImageShader2D: Failed to link shader program\n" << log().toStdString() << std::endl;
          }

        attr_coords = attributeLocation("coord2d");
        if (attr_coords == -1)
          std::cerr << "GLImageShader2D: Failed to bind coordinates" << std::endl;

        attr_texcoords = attributeLocation("texcoord");
        if (attr_texcoords == -1)
          std::cerr << "GLImageShader2D: Failed to bind texture coordinates" << std::endl;

        uniform_mvp = uniformLocation("mvp");
        if (uniform_mvp == -1)
          std::cerr << "GLImageShader2D: Failed to bind transform" << std::endl;

        uniform_texture = uniformLocation("tex");
        if (uniform_texture == -1)
          std::cerr << "GLImageShader2D: Failed to bind texture uniform " << std::endl;

        if (!rgb)
          {
            uniform_lut = uniformLocation("lut");
            if (uniform_lut == -1)
              std::cerr << "GLImageShader2D: Failed to bind lut uniform " << std::endl;

            uniform_min = uniformLocation("texmin");
            if (uniform_min == -1)
              std::cerr << "GLImageShader2D: Failed to bind min uniform " << std::endl;

            uniform_max = uniformLocation("texmax");
            if (uniform_max == -1)
              std::cerr << "GLImageShader2D: Failed to bind max uniform " << std::endl;

            uniform_corr = uniformLocation("correction");
            if (uniform_corr == -1)
              std::cerr << "GLImageShader2D: Failed to bind correction uniform " << std::endl;
          }
      }

      GLImageShader2D::~GLImageShader2D()
      {
      }

      void
      GLImageShader2D::enableCoords()
      {
        enableAttributeArray(attr_coords);
      }

      void
      GLImageShader2D::disableCoords()
      {
        disableAttributeArray(attr_coords);
      }

      void
      GLImageShader2D::setCoords(const GLfloat *offset, int tupleSize, int stride)
      {
        setAttributeArray(attr_coords, offset, tupleSize, stride);
      }

      void
      GLImageShader2D::setCoords(QOpenGLBuffer& coords, const GLfloat *offset, int tupleSize, int stride)
      {
        coords.bind();
        setCoords(offset, tupleSize, stride);
        coords.release();
      }

      void
      GLImageShader2D::enableTexCoords()
      {
        enableAttributeArray(attr_texcoords);
      }

      void
      GLImageShader2D::disableTexCoords()
      {
        disableAttributeArray(attr_texcoords);
      }

      void
      GLImageShader2D::setTexCoords(const GLfloat *offset,
                                    int            tupleSize,
                                    int            stride)
      {
        setAttributeArray(attr_texcoords, offset, tupleSize, stride);
      }

      void
      GLImageShader2D::setTexCoords(QOpenGLBuffer&  texcoords,
                                    const GLfloat  *offset,
                                    int             tupleSize,
                                    int             stride)
      {
        texcoords.bind();
        setTexCoords(offset, tupleSize, stride);
        texcoords.release();
      }

      void
      GLImageShader2D::setTexture(int texunit)
      {
        glUniform1i(uniform_texture, texunit);
        check_gl("Set image texture");
      }

      void
      GLImageShader2D::setMin(const glm::vec3& min)
      {
        if (!rgb)
          {
            glUniform3fv(uniform_min, 1, glm::value_ptr(min));
            check_gl("Set min range");
          }
      }

      void
      GLImageShader2D::setMax(const glm::vec3& max)
      {
        if (!rgb)
          {
            glUniform3fv(uniform_max, 1, glm::value_ptr(max));
            check_gl("Set max range");
          }
      }

      void
      GLImageShader2D::setCorrection(const glm::vec3& corr)
      {
        if (!rgb)
          {
            glUniform3fv(uniform_corr, 1, glm::value_ptr(corr));
            check_gl("Set correction multiplier");
          }
      }

      void
      GLImageShader2D::setLUT(int texunit)
      {
        if (!rgb)
          {
            glUniform1i(uniform_lut, texunit);
            check_gl("Set LUT texture");
          }
      }

      void
      GLImageShader2D::setModelViewProjection(const glm::mat4& mvp)
      {
        glUniformMatrix4fv(uniform_mvp, 1, GL_FALSE, glm::value_ptr(mvp));
        check_gl("Set image2d uniform mvp");
      }

    }
  }
}
