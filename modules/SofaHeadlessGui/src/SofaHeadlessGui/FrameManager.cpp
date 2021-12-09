#include "FrameManager.h"

namespace sofa
{

namespace gui
{

namespace frameManager
{

FrameManager::FrameManager(GLubyte *frame, int rows, int columns, int channels) : m_rows(rows), m_columns(columns), m_channels(channels)
{
    // Order of indices in memory (most- to least-rapidly varying): k, j, i
    FrameManager::m_stride_channels = 1;            //stride in k
    FrameManager::m_stride_rows = -columns * channels; //stride in i (vertical)
    FrameManager::m_stride_columns = channels;         //stride in j (horizontal)

    FrameManager::m_frame = &frame[(rows - 1) * columns * channels]; //if dim has a negative stride, (dim_length - 1) * dim_stride
}

FrameManager::FrameManager(GLubyte *frame, int rows, int columns, int channels, bool torch) : m_rows(channels), m_columns(rows), m_channels(columns), m_torch(torch)
{
    // Order of indices in memory (most- to least-rapidly varying): k, j, i
    FrameManager::m_stride_channels = channels;            //stride in k
    FrameManager::m_stride_rows = 1; //stride in i (vertical)
    FrameManager::m_stride_columns = -columns*channels;         //stride in j (horizontal)

    FrameManager::m_frame = &frame[m_rows * (m_columns -1) * m_channels]; //if dim has a negative stride, (dim_length - 1) * dim_stride
}

} // namespace frameManager

} // namespace gui

} // namespace sofa
