#include "App.h"

namespace {
Page nextPage(Page p) {
  switch (p) {
    case Page::Timeline: return Page::Timer;
    case Page::Timer: return Page::Clock;
    default: return Page::Timeline;
  }
}
}  // namespace

void App::onInput(const InputEvent &ev, uint32_t nowMs, const LocalTime &now) {
  if (overlay_) {
    // Any button press dismisses the stand-up overlay and is otherwise swallowed;
    // a rotary turn is ignored so an accidental bump cannot dismiss it.
    if (ev.type == InputEvent::Type::Press) {
      overlay_ = false;
      standup_.acknowledge(now);
    }
    return;
  }

  if (ev.type == InputEvent::Type::Press && ev.button == Button::Menu) {
    page_ = nextPage(page_);
    return;
  }

  if (page_ == Page::Timer) {
    if (ev.type == InputEvent::Type::Turn) {
      timer_.turn(accel_.step(ev.dir, nowMs));
    } else if (ev.button == Button::Click) {
      timer_.click(nowMs);
    } else if (ev.button == Button::Exit) {
      timer_.cancel();
    }
  }
}

void App::tick(uint32_t nowMs, const LocalTime &now) {
  timer_.tick(nowMs);
  if (!overlay_ && standup_.due(now)) overlay_ = true;
}
