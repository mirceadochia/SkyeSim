#ifndef FGEVENTHANDLER_H
#define FGEVENTHANDLER_H 1

#include <map>
#include <osg/Quat>
#include <osgGA/GUIEventHandler>
#include <osgViewer/ViewerEventHandlers>
#include <simgear/structure/OSGVersion.hxx>

#include <Main/fg_os.hxx>

namespace flightgear
{
    class FGStatsHandler : public osgViewer::StatsHandler
    {
        public:
            FGStatsHandler()
            {
                // Adjust font type/size for >=OSG3.
                // OSG defaults aren't working/available for FG.
                _font = "Fonts/helvetica_medium.txf";
                _characterSize = 12.0f;
            }
    };

class FGEventHandler : public osgGA::GUIEventHandler {
public:
    FGEventHandler();
    
    virtual ~FGEventHandler() {}
    
    virtual const char* className() const {return "FGEventHandler"; }
#if 0
    virtual void init(const osgGA::GUIEventAdapter& ea,
		      osgGA::GUIActionAdapter& us);
#endif
    virtual bool handle(const osgGA::GUIEventAdapter& ea,
			osgGA::GUIActionAdapter& us);

    void setIdleHandler(fgIdleHandler idleHandler)
	{
	    this->idleHandler = idleHandler;
	}

    fgIdleHandler getIdleHandler() const
	{
	    return idleHandler;
	}

    void setKeyHandler(fgKeyHandler keyHandler)
	{
	    this->keyHandler = keyHandler;
	}

    fgKeyHandler getKeyHandler() const
	{
	    return keyHandler;
	}

    void setMouseClickHandler(fgMouseClickHandler mouseClickHandler)
	{
	    this->mouseClickHandler = mouseClickHandler;
	}
    
    fgMouseClickHandler getMouseClickHandler()
	{
	    return mouseClickHandler;
	}

    void setMouseMotionHandler(fgMouseMotionHandler mouseMotionHandler)
	{
	    this->mouseMotionHandler = mouseMotionHandler;
	}
    
    fgMouseMotionHandler getMouseMotionHandler()
	{
	    return mouseMotionHandler;
	}

    void setChangeStatsCameraRenderOrder(bool c)
    {
        changeStatsCameraRenderOrder = c;
    }

    int getCurrentModifiers() const
	{
	    return currentModifiers;
	}

    void setMouseWarped()
	{
	    mouseWarped = true;
	}

    /** Whether or not resizing is supported. It might not be when
     * using multiple displays.
     */
    bool getResizable() { return resizable; }
    void setResizable(bool _resizable) { resizable = _resizable; }

    void reset();
    
    static int translateKey(const osgGA::GUIEventAdapter& ea);
    static int translateModifiers(const osgGA::GUIEventAdapter& ea);
protected:
    osg::ref_ptr<osg::Node> _node;
    fgIdleHandler idleHandler;
    fgKeyHandler keyHandler;
    fgMouseClickHandler mouseClickHandler;
    fgMouseMotionHandler mouseMotionHandler;
    osg::ref_ptr<FGStatsHandler> statsHandler;
    osg::ref_ptr<osgGA::GUIEventAdapter> statsEvent;
    int statsType;
    int currentModifiers;
    
    void handleKey(const osgGA::GUIEventAdapter& ea, int& key, int& modifiers);
    bool resizable;
    bool mouseWarped;
    // workaround for osgViewer double scroll events
    bool scrollButtonPressed;
    int release_keys[128];
    void handleStats(osgGA::GUIActionAdapter& us);
    bool changeStatsCameraRenderOrder;
    SGPropertyNode_ptr _display, _print;
};

void eventToWindowCoords(const osgGA::GUIEventAdapter* ea, double& x, double& y);
void eventToWindowCoordsYDown(const osgGA::GUIEventAdapter* ea,
                              double& x, double& y);
}
#endif
