#ifndef SCREENVIEW_HPP
#define SCREENVIEW_HPP

#include <gui_generated/screen_screen/screenViewBase.hpp>
#include <gui/screen_screen/screenPresenter.hpp>

class screenView : public screenViewBase
{
public:
    screenView();
    virtual ~screenView() {}
    virtual void setupScreen();
    virtual void tearDownScreen();
    virtual void handleTickEvent();
    virtual void Status_editor( const char* Edit_string, touchgfx::Unicode::UnicodeChar *Buffer, uint16_t Buffer_size,  touchgfx::TextAreaWithOneWildcard Buffer_name);
    virtual void Update_progress(touchgfx::BoxProgress& progressBar, uint16_t val);
protected:
};

#endif // SCREENVIEW_HPP
