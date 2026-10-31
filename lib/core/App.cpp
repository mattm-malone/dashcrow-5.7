#include "App.h"

namespace {
constexpr int kMaxDayOffset = 7;

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

  if (page_ == Page::Timeline) {
    // Day scrolling is one day per step regardless of how fast the rotary is pressed.
    if (ev.type == InputEvent::Type::Turn) {
      dayOffset_ += ev.dir;
      if (dayOffset_ > kMaxDayOffset) dayOffset_ = kMaxDayOffset;
      if (dayOffset_ < -kMaxDayOffset) dayOffset_ = -kMaxDayOffset;
    } else if (ev.button == Button::Click) {
      dayOffset_ = 0;
    }
    return;
  }

  if (page_ == Page::Timer) {
    if (ev.type == InputEvent::Type::Turn) {
      // First press moves one minute; a held key's repeats jump to the next multiple of five.
      if (ev.repeat) timer_.turnToStep(ev.dir, 5);
      else timer_.turn(ev.dir);
    } else if (ev.button == Button::Click) {
      timer_.click(nowMs);
    } else if (ev.button == Button::ClickLong) {
      // Holding the dial: abandon a running/paused/finished timer, or put the minutes back to the default.
      if (timer_.state() == TimerModel::State::Setting) timer_.resetMinutes();
      else timer_.cancel();
    } else if (ev.button == Button::Exit) {
      style_ = TimerStyle((uint8_t(style_) + 1) % uint8_t(TimerStyle::kCount));
    }
  }
}

void App::tick(uint32_t nowMs, const LocalTime &now) {
  const bool wasRunning = timer_.state() == TimerModel::State::Running;
  timer_.tick(nowMs);
  // The device has no buzzer: a timer that finishes while another page is showing must be seen.
  if (wasRunning && timer_.state() == TimerModel::State::Done) page_ = Page::Timer;
  if (!overlay_ && standup_.due(now)) overlay_ = true;
}
