#ifndef DILAY_TOOL_SCULPT_BEHAVIOR
#define DILAY_TOOL_SCULPT_BEHAVIOR

#include <glm/fwd.hpp>
#include "macro.hpp"

class ConfigProxy;
class QDoubleSpinBox;
class PrimRay;
class SculptBrush;
class State;
class ViewCursor;
class ViewPropertiesPart;
class ViewToolTip;
class WingedFaceIntersection;

class ToolSculptBehavior {
  public:
    DECLARE_BIG3_VIRTUAL (ToolSculptBehavior, ConfigProxy&, State&, const char*)

    void setupCursor           ();
    void setupProperties       (ViewPropertiesPart&);
    void setupToolTip          (ViewToolTip&);
    void render                () const;
    void mouseMoveEvent        (const glm::ivec2&, bool);
    void mouseLeftPressEvent   (const glm::ivec2&);
    void mouseLeftReleaseEvent (const glm::ivec2&);
    void mouseWheelEvent       (bool);
    void close                 ();

  protected:
    ConfigProxy& config              () const;
    State&       state               () const;
    ViewCursor&  cursor              () const;
    bool         intersectsSelection (const PrimRay&, WingedFaceIntersection&) const;
    bool         intersectsSelection (const glm::ivec2&, WingedFaceIntersection&) const;
    void         brushFromCache      (SculptBrush&) const;
    void         sculpt              (const SculptBrush&);
    void         sculpt              ();

  private:
    IMPLEMENTATION

    virtual const char*  key                      () const = 0;
    virtual SculptBrush& brush                    () const = 0;
    virtual void         runSetupProperties       (ViewPropertiesPart&) = 0;
    virtual void         runSetupToolTip          (ViewToolTip&) = 0;
    virtual void         runMouseLeftPressEvent   (const glm::ivec2&) = 0;
    virtual void         runMouseMoveEvent        (const glm::ivec2&, bool) = 0;
    virtual void         runMouseLeftReleaseEvent (const glm::ivec2&) {}
    virtual void         runRender                () const {}
};

#define DECLARE_TOOL_SCULPT_BEHAVIOR(name,theKey,otherMethods)                              \
  class name : public ToolSculptBehavior {                                                  \
    public:  DECLARE_BIG3 (name, ConfigProxy&, State&)                                      \
    private: IMPLEMENTATION                                                                 \
             const char*  key                    () const { return theKey ; }               \
             SculptBrush& brush                  () const;                                  \
             void         runSetupProperties     (ViewPropertiesPart&);                     \
             void         runSetupToolTip        (ViewToolTip&);                            \
             void         runMouseLeftPressEvent (const glm::ivec2&);                       \
             void         runMouseMoveEvent      (const glm::ivec2&, bool);                 \
             otherMethods                                                                   \
  };

#define DECLARE_TOOL_SCULPT_BEHAVIOR_RUN_MOUSE_LEFT_RELEASE_EVENT \
  void runMouseLeftReleaseEvent (const glm::ivec2&);

#define DECLARE_TOOL_SCULPT_BEHAVIOR_RUN_RENDER \
  void runRender () const;

#define DELEGATE_TOOL_SCULPT_BEHAVIOR(name)                                                 \
  DELEGATE_BIG3_BASE ( name, (ConfigProxy& c, State& s), (this)                             \
                     , ToolSculptBehavior, (c,s,this->key ()) )                             \
  GETTER_CONST   (SculptBrush&, name, brush)                                                \
  DELEGATE1      (void, name, runSetupProperties, ViewPropertiesPart&);                     \
  DELEGATE1      (void, name, runSetupToolTip, ViewToolTip&);                               \
  DELEGATE1      (void, name, runMouseLeftPressEvent, const glm::ivec2&)                    \
  DELEGATE2      (void, name, runMouseMoveEvent, const glm::ivec2&, bool)                    

#define DELEGATE_TOOL_SCULPT_BEHAVIOR_RUN_MOUSE_LEFT_RELEASE_EVENT(n) \
  DELEGATE1 (void, n, runMouseLeftReleaseEvent, const glm::ivec2&)

#define DELEGATE_TOOL_SCULPT_BEHAVIOR_RUN_RENDER(n) \
  DELEGATE_CONST (void, n, runRender)

#endif