/******************************************************************************
*       SOFA, Simulation Open-Framework Architecture, development version     *
*                (c) 2006-2019 INRIA, USTL, UJF, CNRS, MGH                    *
*                                                                             *
* This program is free software; you can redistribute it and/or modify it     *
* under the terms of the GNU General Public License as published by the Free  *
* Software Foundation; either version 2 of the License, or (at your option)   *
* any later version.                                                          *
*                                                                             *
* This program is distributed in the hope that it will be useful, but WITHOUT *
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or       *
* FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for    *
* more details.                                                               *
*                                                                             *
* You should have received a copy of the GNU General Public License along     *
* with this program. If not, see <http://www.gnu.org/licenses/>.              *
*******************************************************************************
* Authors: The SOFA Team and external contributors (see Authors.txt)          *
*                                                                             *
* Contact information: contact@sofa-framework.org                             *
******************************************************************************/
#ifndef SOFA_GUI_HEADLESSGUI_H
#define SOFA_GUI_HEADLESSGUI_H

#include <sofa/gui/BaseGUI.h>
#include <sofa/gui/ArgumentParser.h>
#include <sofa/simulation/fwd.h>
#include <sofa/core/visual/VisualParams.h>
#include <sofa/gl/DrawToolGL.h>
#include <SofaBaseVisual/InteractiveCamera.h>
#include <sofa/core/ObjectFactory.h>
#include "FrameManager.h"

#include <csignal>

#include <iostream>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <memory>

// OPENGL
#define GL_GLEXT_PROTOTYPES 1
#include <GL/gl.h>
#include <GL/glx.h>

// SCREENSHOT
#include <sofa/helper/io/Image.h>
#include <sofa/helper/system/SetDirectory.h>

namespace sofa
{

namespace gui
{

namespace hGui
{

enum class RecordMode { wallclocktime, simulationtime, timeinterval };

// class VideoRecorderFFmpeg;

class HeadlessGui : public sofa::gui::BaseGUI
{

public:
    typedef sofa::core::visual::VisualParams VisualParams;
    typedef sofa::gl::DrawToolGL   DrawToolGL;

    HeadlessGui();
    ~HeadlessGui();

    int mainLoop() override;
    void redraw() override;
    void resetView();
    void saveView();
    void initializeGL();
    void paintGL();
    void newView();


    sofa::gui::frameManager::FrameManager getVisual();
    sofa::gui::frameManager::FrameManager getVisualChannelFirst();
    sofa::gui::frameManager::FrameManager getVisualDepth();

    // Virtual from BaseGUI
    void setScene(sofa::simulation::NodeSPtr scene, const char* filename=nullptr, bool temporaryFile=false) override;
    virtual sofa::simulation::Node* currentSimulation() override;
    virtual int closeGUI() override;
    virtual void setViewerResolution(int width, int height) override;

    // Needed for the registration
    static BaseGUI* CreateGUI(const char* name, sofa::simulation::NodeSPtr groot = nullptr, const char* filename = nullptr);
    static int RegisterGUIParameters(sofa::gui::ArgumentParser* argumentParser);

private:
    void setUpCamera();
    void displayOBJs();
    void drawScene();
    void calcProjection();

    VisualParams* vparams;
    DrawToolGL   drawTool;

    sofa::simulation::NodeSPtr groot;
    std::string sceneFileName;
    sofa::component::visualmodel::BaseCamera::SPtr currentCamera;

    GLuint fbo{};
    GLuint rbo_color{}, rbo_depth{};
    double lastProjectionMatrix[16]{};
    double lastModelviewMatrix[16]{};

    GLubyte *currentFrame;
    GLubyte *currentDepthFrame;

    bool initTexturesDone;

    static GLsizei s_height;
    static GLsizei s_width;
};

} // namespace hGui

} // namespace gui

} // namespace sofa

#endif
