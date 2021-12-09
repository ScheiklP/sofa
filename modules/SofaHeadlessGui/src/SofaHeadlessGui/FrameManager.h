#ifndef SOFA_GUI_FRAMEMANAGER_H
#define SOFA_GUI_FRAMEMANAGER_H

#define GL_GLEXT_PROTOTYPES 1
#include <GL/gl.h>
#include <GL/glx.h>

namespace sofa
{

namespace gui
{

namespace frameManager
{

class FrameManager
{
public:
    FrameManager(GLubyte *frame, int rows, int columns, int channels);
    FrameManager(GLubyte *frame, int rows, int columns, int channels, bool torch);
    GLubyte* frame() { return m_frame; }
    int rows() const { return m_rows; }
    int columns() const { return m_columns; }
    int channels() const { return m_channels; }
    int stride_rows() const { return m_stride_rows; }
    int stride_columns() const { return m_stride_columns; }
    int stride_channels() const { return m_stride_channels; }

private:
    int m_rows, m_columns, m_channels;
    int m_stride_rows, m_stride_columns, m_stride_channels;
    bool m_torch;
    GLubyte *m_frame;
};

} // namespace frameManager

} // namespace gui

} // namespace sofa

#endif
