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

#ifndef OME_QTWIDGETS_GL_AXIS2D_H
#define OME_QTWIDGETS_GL_AXIS2D_H

#include <memory>

#include <QtCore/QObject>
#include <QtGui/QOpenGLVertexArrayObject>
#include <QtGui/QOpenGLBuffer>
#include <QtGui/QOpenGLShader>
#include <QtGui/QOpenGLFunctions_3_3_Core>

#include <ome/files/Types.h>
#include <ome/files/FormatReader.h>

#include <ome/qtwidgets/glm.h>
#include <ome/qtwidgets/glsl/GLFlatShader2D.h>

namespace ome
{
  namespace qtwidgets
  {
    namespace gl
    {

      /**
       * 2D (xy) axis renderer.
       *
       * Draws x and y axes for the specified image.
       */
      class Axis2D : public QObject, protected QOpenGLFunctions_3_3_Core
      {
        Q_OBJECT

      public:
        /**
         * Create a 2D axis.
         *
         * The size and position will be taken from the specified image.
         *
         * @param reader the image reader.
         * @param series the image series.
         * @param resolution the image resolution.
         * @param parent the parent of this object.
         */
        explicit Axis2D(std::shared_ptr<ome::files::FormatReader>  reader,
                        ome::files::dimension_size_type            series,
                        ome::files::dimension_size_type            resolution,
                        QObject                                   *parent = 0);

        /// Destructor.
        virtual
        ~Axis2D();

        /**
         * Create GL buffers.
         *
         * @note Requires a valid GL context.  Must be called before
         * rendering.
         */
        void
        create();

        /**
         * Render the axis.
         *
         * @param mvp the model view projection matrix.
         */
        void
        render(const glm::mat4& mvp);

      private:
        /**
         * Set the size of the x and y axes.
         *
         * @param xlim the x axis limits (range).
         * @param ylim the y axis limits (range).
         * @param soff shaft offset (x and y offsets from midline).
         * @param slim the axis shaft width (min and max from midline).
         */
        virtual
        void
        setSize(glm::vec2 xlim,
                glm::vec2 ylim,
                glm::vec2 soff,
                glm::vec2 slim);

        /// The vertex array.
        QOpenGLVertexArrayObject vertices;
        /// The vertices for the x axis.
        QOpenGLBuffer xaxis_vertices;
        /// The vertices for the y axis.
        QOpenGLBuffer yaxis_vertices;
        /// The elements for both axes.
        QOpenGLBuffer axis_elements;
        /// The image reader.
        std::shared_ptr<ome::files::FormatReader> reader;
        /// The image series.
        ome::files::dimension_size_type series;
        /// The image resolution.
        ome::files::dimension_size_type resolution;
        /// The shader program for axis rendering.
        glsl::GLFlatShader2D *axis_shader;
      };

    }
  }
}

#endif // OME_QTWIDGETS_GL_AXIS2D_H

/*
 * Local Variables:
 * mode:C++
 * End:
 */
