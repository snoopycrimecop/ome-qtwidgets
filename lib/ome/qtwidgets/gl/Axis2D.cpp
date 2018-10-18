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

#include <ome/qtwidgets/gl/Axis2D.h>
#include <ome/qtwidgets/gl/Util.h>

#include <iostream>

namespace ome
{
  namespace qtwidgets
  {
    namespace gl
    {

      Axis2D::Axis2D(std::shared_ptr<ome::files::FormatReader>  reader,
                     ome::files::dimension_size_type            series,
                     ome::files::dimension_size_type            resolution,
                     QObject                                   *parent):
        QObject(parent),
        vertices(),
        xaxis_vertices(QOpenGLBuffer::VertexBuffer),
        yaxis_vertices(QOpenGLBuffer::VertexBuffer),
        axis_elements(QOpenGLBuffer::IndexBuffer),
        reader(reader),
        series(series),
        resolution(resolution),
        axis_shader(new glsl::GLFlatShader2D(this))
      {
        initializeOpenGLFunctions();
      }

      Axis2D::~Axis2D()
      {
      }

      void
      Axis2D::create()
      {
        ome::files::dimension_size_type oldseries = reader->getSeries();
        reader->setSeries(series);
        reader->setResolution(resolution);
        setSize(glm::vec2(-(reader->getSizeX()/2.0f), reader->getSizeX()/2.0f),
                glm::vec2(-(reader->getSizeY()/2.0f), reader->getSizeY()/2.0f),
                glm::vec2(-(reader->getSizeX()/2.0f)-12.0, -(reader->getSizeY()/2.0f)-12.0),
                glm::vec2(-6.0, 6.0));
        reader->setSeries(oldseries);
      }

      void
      Axis2D::setSize(glm::vec2 xlim,
                      glm::vec2 ylim,
                      glm::vec2 soff,
                      glm::vec2 slim)
      {
        GLfloat swid(slim[1] - slim[0]);
        GLfloat smid(slim[0] + (swid/2.0));
        GLfloat arrowwid(swid * 5);
        GLfloat arrowlen(arrowwid * 1.5);
        GLfloat xoff(static_cast<GLfloat>(soff[0]));
        GLfloat yoff(static_cast<GLfloat>(soff[1]));

        const std::array<GLfloat, 14> xaxis_vertices_a
        {
          // Arrow shaft
          xlim[0], yoff+slim[0],
          xlim[1]-arrowlen, yoff+slim[0],
          xlim[1]-arrowlen, yoff+slim[1],
          xlim[0], yoff+slim[1],
          // Arrow head
          xlim[1]-arrowlen, yoff+smid+(arrowwid/2.0f),
          xlim[1]-arrowlen, yoff+smid-(arrowwid/2.0f),
          xlim[1], yoff+smid
        };

        const std::array<GLfloat, 14> yaxis_vertices_a
        {
          // Arrow shaft
          xoff+slim[1], ylim[0],
          xoff+slim[1], ylim[1]-arrowlen,
          xoff+slim[0], ylim[1]-arrowlen,
          xoff+slim[0], ylim[0],
          // Arrow head
          xoff+smid-(arrowwid/2.0f), ylim[1]-arrowlen,
          xoff+smid+(arrowwid/2.0f), ylim[1]-arrowlen,
          xoff+smid, ylim[1]
        };

        vertices.create();
        vertices.bind();

        xaxis_vertices.create();
        xaxis_vertices.setUsagePattern(QOpenGLBuffer::StaticDraw);
        xaxis_vertices.bind();
        xaxis_vertices.allocate(xaxis_vertices_a.data(), sizeof(GLfloat) * xaxis_vertices_a.size());

        yaxis_vertices.create();
        yaxis_vertices.setUsagePattern(QOpenGLBuffer::StaticDraw);
        yaxis_vertices.bind();
        yaxis_vertices.allocate(yaxis_vertices_a.data(), sizeof(GLfloat) * yaxis_vertices_a.size());

        std::array<GLushort, 9> axis_elements_a
        {
          // Arrow shaft
          0,  1,  2,
          2,  3,  0,
          // Arrow head
          4,  5,  6
        };

        axis_elements.create();
        axis_elements.setUsagePattern(QOpenGLBuffer::StaticDraw);
        axis_elements.bind();
        axis_elements.allocate(axis_elements_a.data(), sizeof(GLushort) * axis_elements_a.size());
      }

      void
      Axis2D::render(const glm::mat4& mvp)
      {
        axis_shader->bind();

        vertices.bind();

        // Render x axis
        axis_shader->setModelViewProjection(mvp);
        axis_shader->setColour(glm::vec4(1.0, 0.0, 0.0, 1.0));
        axis_shader->setOffset(glm::vec2(0.0, -40.0));
        axis_shader->enableCoords();
        axis_shader->setCoords(xaxis_vertices, 0, 2, 0);

        // Push each element to the vertex shader
        axis_elements.bind();
        glDrawElements(GL_TRIANGLES, axis_elements.size()/sizeof(GLushort), GL_UNSIGNED_SHORT, 0);
        check_gl("Axis X draw elements");

        axis_shader->disableCoords();

        // Render y axis
        axis_shader->bind();
        axis_shader->setModelViewProjection(mvp);
        axis_shader->setColour(glm::vec4(0.0, 1.0, 0.0, 1.0));
        axis_shader->setOffset(glm::vec2(-40.0, 0.0));
        axis_shader->enableCoords();
        axis_shader->setCoords(yaxis_vertices, 0, 2, 0);

        // Push each element to the vertex shader
        axis_elements.bind();
        glDrawElements(GL_TRIANGLES, axis_elements.size()/sizeof(GLushort), GL_UNSIGNED_SHORT, 0);
        check_gl("Axis Y draw elements");

        axis_shader->disableCoords();
        vertices.release();
        axis_shader->release();
      }

    }
  }
}
