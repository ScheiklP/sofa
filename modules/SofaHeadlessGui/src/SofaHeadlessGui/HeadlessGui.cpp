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
#include "HeadlessGui.h"
#include <sofa/helper/AdvancedTimer.h>
#include <sofa/gui/ArgumentParser.h>
#include <sofa/helper/Utils.h>
using sofa::helper::Utils;
using namespace sofa::type;
#include <sofa/simulation/Simulation.h>
#include <sofa/simulation/Node.h>

#include <boost/program_options.hpp>

namespace sofa
{

namespace gui
{

namespace hGui
{

GLsizei HeadlessGui::s_width = 1920;
GLsizei HeadlessGui::s_height = 1080;
const static int n_colour_channels = 4;

using namespace sofa::defaulttype;
using sofa::simulation::getSimulation;

static sofa::core::ObjectFactory::ClassEntry::SPtr classVisualModel;
typedef GLXContext (*glXCreateContextAttribsARBProc)(Display*, GLXFBConfig, GLXContext, Bool, const int*);
typedef Bool (*glXMakeContextCurrentARBProc)(Display*, GLXDrawable, GLXDrawable, GLXContext);
static glXCreateContextAttribsARBProc glXCreateContextAttribsARB = 0;
static glXMakeContextCurrentARBProc glXMakeContextCurrentARB = 0;

// Class
HeadlessGui::HeadlessGui()
{
    groot = nullptr;
    initTexturesDone = false;
    vparams = core::visual::VisualParams::defaultInstance();
    vparams->drawTool() = &drawTool;

    int nvals = n_colour_channels * s_width * s_height;
    currentFrame = new GLubyte[nvals];
    currentDepthFrame = new GLubyte[s_width*s_height];
}

HeadlessGui::~HeadlessGui()
{
    delete[] currentFrame;
    delete[] currentDepthFrame;

    glDeleteFramebuffers(1, &fbo);
    glDeleteRenderbuffers(1, &rbo_color);
    glDeleteRenderbuffers(1, &rbo_depth);
}

int HeadlessGui::RegisterGUIParameters(sofa::gui::ArgumentParser* argumentParser)
{
    argumentParser->addArgument(cxxopts::value<GLsizei>(s_width)->default_value("1920"),
                                "width", "(only HeadLessRecorder) video or picture width");
    argumentParser->addArgument(cxxopts::value<GLsizei>(s_height)->default_value("1080"),
                                "height", "(only HeadLessRecorder) video or picture height");

    return 0;
}

BaseGUI* HeadlessGui::CreateGUI(const char* /*name*/, sofa::simulation::NodeSPtr groot, const char* filename)
{
    int context_attribs[] = {
        GLX_CONTEXT_MAJOR_VERSION_ARB, 3,
        GLX_CONTEXT_MINOR_VERSION_ARB, 0,
        None
    };

    Display* m_display;
    int fbcount = 0;
    GLXFBConfig* fbc = nullptr;
    GLXContext ctx;
    GLXPbuffer pbuf;

    /* open display */
    if ( ! (m_display = XOpenDisplay(0)) ){
        fprintf(stderr, "Failed to open display\n");
        exit(1);
    }

    /* get framebuffer configs, any is usable (might want to add proper attribs) */
    if ( !(fbc = glXChooseFBConfig(m_display, DefaultScreen(m_display), nullptr, &fbcount) ) ){
        fprintf(stderr, "Failed to get FBConfig\n");
        exit(1);
    }

    /* get the required extensions */
    glXCreateContextAttribsARB = (glXCreateContextAttribsARBProc)glXGetProcAddressARB( (const GLubyte *) "glXCreateContextAttribsARB");
    glXMakeContextCurrentARB = (glXMakeContextCurrentARBProc)glXGetProcAddressARB( (const GLubyte *) "glXMakeContextCurrent");
    if ( !(glXCreateContextAttribsARB && glXMakeContextCurrentARB) ){
        fprintf(stderr, "missing support for GLX_ARB_create_context\n");
        XFree(fbc);
        exit(1);
    }

    /* create a context using glXCreateContextAttribsARB */
    if ( !( ctx = glXCreateContextAttribsARB(m_display, fbc[0], 0, True, context_attribs)) ){
        fprintf(stderr, "Failed to create opengl context\n");
        XFree(fbc);
        exit(1);
    }

    /* create temporary pbuffer */
    int pbuffer_attribs[] = {
        GLX_PBUFFER_WIDTH, s_width,
        GLX_PBUFFER_HEIGHT, s_height,
        None
    };
    pbuf = glXCreatePbuffer(m_display, fbc[0], pbuffer_attribs);

    XFree(fbc);
    XSync(m_display, False);

    /* try to make it the current context */
    if ( !glXMakeContextCurrent(m_display, pbuf, pbuf, ctx) ){
        /* some drivers does not support context without default framebuffer, so fallback on
                    * using the default window.
                    */
        if ( !glXMakeContextCurrent(m_display, DefaultRootWindow(m_display), DefaultRootWindow(m_display), ctx) ){
            fprintf(stderr, "failed to make current\n");
            exit(1);
        }
    }

    GLenum err = glewInit();
    if (GLEW_OK != err)
    {
        std:: cout << "GLEW Error: " << glewGetErrorString(err) << std::endl;
        exit(EXIT_FAILURE);
    }

    HeadlessGui* gui = new HeadlessGui();
    gui->setScene(groot, filename);
    gui->initializeGL();
    gui->setUpCamera();

    return gui;
}

int HeadlessGui::closeGUI()
{
    delete this;
    return 0;
}

// -----------------------------------------------------------------
// --- OpenGL stuff
// -----------------------------------------------------------------
void HeadlessGui::initializeGL(void)
{
    static GLfloat    specular[4];
    static GLfloat    ambientLight[4];
    static GLfloat    diffuseLight[4];
    static GLfloat    lightPosition[4];
    static GLfloat    lmodel_ambient[]    = {0.0f, 0.0f, 0.0f, 0.0f};
    static GLfloat    lmodel_twoside[]    = {GL_FALSE};
    static GLfloat    lmodel_local[]        = {GL_FALSE};
    static bool       initialized            = false;

    if (!initialized)
    {
        lightPosition[0] = -0.7f;
        lightPosition[1] = 0.3f;
        lightPosition[2] = 0.0f;
        lightPosition[3] = 1.0f;

        ambientLight[0] = 0.5f;
        ambientLight[1] = 0.5f;
        ambientLight[2] = 0.5f;
        ambientLight[3] = 1.0f;

        diffuseLight[0] = 0.9f;
        diffuseLight[1] = 0.9f;
        diffuseLight[2] = 0.9f;
        diffuseLight[3] = 1.0f;

        specular[0] = 1.0f;
        specular[1] = 1.0f;
        specular[2] = 1.0f;
        specular[3] = 1.0f;

        // Set light model
        glLightModelfv(GL_LIGHT_MODEL_LOCAL_VIEWER, lmodel_local);
        glLightModelfv(GL_LIGHT_MODEL_TWO_SIDE, lmodel_twoside);
        glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lmodel_ambient);

        // Setup 'light 0'
        glLightfv(GL_LIGHT0, GL_AMBIENT, ambientLight);
        glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuseLight);
        glLightfv(GL_LIGHT0, GL_SPECULAR, specular);
        glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
        glEnable(GL_LIGHT0);

        // Define background color
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

        // frame buffer
        glGenFramebuffers(1, &fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);

        // color render buffer
        glGenRenderbuffers(1, &rbo_color);
        glBindRenderbuffer(GL_RENDERBUFFER, rbo_color);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_RGB, s_width, s_height);
        glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, rbo_color);

        /* Depth renderbuffer. */
        glGenRenderbuffers(1, &rbo_depth);
        glBindRenderbuffer(GL_RENDERBUFFER, rbo_depth);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, s_width, s_height);
        glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo_depth);
        glReadBuffer(GL_COLOR_ATTACHMENT0);

        glEnable(GL_DEPTH_TEST);

        initialized = true;
    }

    // switch to preset view
    resetView();
}

int HeadlessGui::mainLoop()
{
    return 0;
}

void HeadlessGui::redraw()
{
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear ( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    paintGL();
    glFlush();
}

void HeadlessGui::displayOBJs()
{
    vparams->sceneBBox() = groot->f_bbox.getValue();
    if (!initTexturesDone)
    {
        simulation::getSimulation()->initTextures(groot.get());
        initTexturesDone = true;
    } else
    {
        simulation::getSimulation()->draw(vparams,groot.get());
    }
}

void HeadlessGui::drawScene(void)
{
    if (!groot) return;
    if(!currentCamera)
    {
        msg_error("HeadlessGui") << "ERROR: no camera defined";
        return;
    }

    calcProjection();
    glLoadIdentity();

    GLdouble mat[16];
    currentCamera->getOpenGLModelViewMatrix(mat);
    glMultMatrixd(mat);
    displayOBJs();
}

void HeadlessGui::calcProjection()
{
    double xNear, yNear;
    double xFactor = 1.0, yFactor = 1.0;
    double offset;
    double xForeground, yForeground, zForeground, xBackground, yBackground, zBackground;
    Vector3 center;

    /// Camera part
    if (!currentCamera)
        return;

    if (groot && (!groot->f_bbox.getValue().isValid()))
    {
        vparams->sceneBBox() = groot->f_bbox.getValue();
        currentCamera->setBoundingBox(vparams->sceneBBox().minBBox(), vparams->sceneBBox().maxBBox());
    }
    currentCamera->computeZ();

    vparams->zNear() = currentCamera->getZNear();
    vparams->zFar() = currentCamera->getZFar();

    xNear = 0.35 * vparams->zNear();
    yNear = 0.35 * vparams->zNear();
    offset = 0.001 * vparams->zNear(); // for foreground and background planes

    if ((s_height != 0) && (s_width != 0))
    {
        if (s_height > s_width)
        {
            xFactor = 1.0;
            yFactor = (double) s_height / (double) s_width;
        }
        else
        {
            xFactor = (double) s_width / (double) s_height;
            yFactor = 1.0;
        }
    }
    vparams->viewport() = sofa::type::make_array(0, 0, s_width, s_height);

    glViewport(0, 0, s_width, s_height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    xFactor *= 0.01;
    yFactor *= 0.01;

    zForeground = -vparams->zNear() - offset;
    zBackground = -vparams->zFar() + offset;

    if (currentCamera->getCameraType() == core::visual::VisualParams::PERSPECTIVE_TYPE)
        gluPerspective(currentCamera->getFieldOfView(), (double) s_width / (double) s_height, vparams->zNear(), vparams->zFar());
    else
    {
        float ratio = (float)( vparams->zFar() / (vparams->zNear() * 20) );
        Vector3 tcenter = vparams->sceneTransform() * center;
        if (tcenter[2] < 0.0)
        {
            ratio = (float)( -300 * (tcenter.norm2()) / tcenter[2] );
        }
        glOrtho((-xNear * xFactor) * ratio, (xNear * xFactor) * ratio, (-yNear * yFactor) * ratio, (yNear * yFactor) * ratio,
                vparams->zNear(), vparams->zFar());
    }

    xForeground = -zForeground * xNear / vparams->zNear();
    yForeground = -zForeground * yNear / vparams->zNear();
    xBackground = -zBackground * xNear / vparams->zNear();
    yBackground = -zBackground * yNear / vparams->zNear();

    xForeground *= xFactor;
    yForeground *= yFactor;
    xBackground *= xFactor;
    yBackground *= yFactor;

    glGetDoublev(GL_PROJECTION_MATRIX,lastProjectionMatrix);

    glMatrixMode(GL_MODELVIEW);
}

void HeadlessGui::paintGL()
{
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClearDepth(1.0);
    drawScene();
}

void HeadlessGui::resetView()
{
    bool fileRead = false;

    if (!sceneFileName.empty())
    {
        std::string viewFileName = sceneFileName + ".view";
        fileRead = currentCamera->importParametersFromFile(viewFileName);
    }

    //if there is no .view file , look at the center of the scene bounding box
    // and with a Up vector in the same axis as the gravity
    if (!fileRead)
    {
        newView();
    }
    redraw();
}

void HeadlessGui::newView()
{
    if (!currentCamera || !groot)
        return;

    currentCamera->setDefaultView(groot->getGravity());
}

void HeadlessGui::setScene(sofa::simulation::NodeSPtr scene, const char* filename, bool)
{
    std::ostringstream ofilename;

    sceneFileName = (filename==nullptr)?"":filename;
    if (!sceneFileName.empty())
    {
        const char* begin = sceneFileName.c_str();
        const char* end = strrchr(begin,'.');
        if (!end) end = begin + sceneFileName.length();
        ofilename << std::string(begin, end);
        ofilename << "_";
    }
    else
        ofilename << "scene_";

    groot = scene;
    initTexturesDone = false;

    //Camera initialization
    if (groot)
    {
        groot->get(currentCamera);
        if (!currentCamera)
        {
            currentCamera = sofa::core::objectmodel::New<component::visualmodel::InteractiveCamera>();
            currentCamera->setName(core::objectmodel::Base::shortName(currentCamera.get()));
            groot->addObject(currentCamera);
            currentCamera->p_position.forceSet();
            currentCamera->p_orientation.forceSet();
            currentCamera->bwdInit();
            resetView();
        }

        vparams->sceneBBox() = groot->f_bbox.getValue();
        currentCamera->setBoundingBox(vparams->sceneBBox().minBBox(), vparams->sceneBBox().maxBBox());

    }
    redraw();
}

void HeadlessGui::setUpCamera()
{
    if(currentCamera)
        currentCamera->setViewport(s_width, s_height);
    calcProjection();
}

sofa::simulation::Node* HeadlessGui::currentSimulation()
{
    return groot.get();
}

void HeadlessGui::setViewerResolution(int /*width*/, int /*height*/)
{
}

sofa::gui::frameManager::FrameManager HeadlessGui::getVisual()
{
    getSimulation()->updateVisual(groot.get());
    redraw();
    glReadPixels(0, 0, s_width, s_height, GL_RGB, GL_UNSIGNED_BYTE, currentFrame);
    return sofa::gui::frameManager::FrameManager { currentFrame, s_height, s_width, 3 };
}

sofa::gui::frameManager::FrameManager HeadlessGui::getVisualChannelFirst()
{
    getSimulation()->updateVisual(groot.get());
    redraw();
    glReadPixels(0, 0, s_width, s_height, GL_RGB, GL_UNSIGNED_BYTE, currentFrame);
    return sofa::gui::frameManager::FrameManager { currentFrame, s_height, s_width, 3, true};
}


sofa::gui::frameManager::FrameManager HeadlessGui::getVisualDepth()
{
    getSimulation()->updateVisual(groot.get());
    redraw();
    //TODO Do this better
    glReadPixels(0, 0, s_width, s_height, GL_RGBA, GL_UNSIGNED_BYTE, currentFrame);
    glReadPixels(0, 0, s_width, s_height, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, currentDepthFrame);
    for(int i = 0; i < s_width*s_height; ++i){ currentFrame[3 + i * 4] = currentDepthFrame[i];}
    return sofa::gui::frameManager::FrameManager { currentFrame, s_height, s_width, 4 };
}

} // namespace hGui

} // namespace gui

} // namespace sofa


