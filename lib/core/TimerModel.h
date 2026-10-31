#pragma once
#include <stdint.h>

class TimerModel {
 public:
  enum class State { Setting, Running, Paused, Done };

  void turn(int delta);            // Setting only: adjusts minutes, clamped to 1..180
  void turnToStep(int dir, int step);  // Setting only: moves to the next multiple of `step` in direction dir (+1/-1)
  void click(uint32_t nowMs);      // Setting->Running, Running<->Paused, Done->Setting
  static constexpr int kDefaultMinutes = 25;
  void resetMinutes();             // Setting only: back to kDefaultMinutes
  void cancel();                   // any state -> Setting (keeps the chosen minutes)
  void tick(uint32_t nowMs);       // advances a running timer; Running->Done at zero

  State state() const;
  int setMinutes() const;
  uint32_t remainingMs() const;
  uint32_t totalMs() const;
  int minutesLeft() const;         // ceiling of remaining time in minutes
  bool inLastMinute() const;       // running/paused with 0 < remaining <= 60 s

 private:
  void settle(uint32_t nowMs);

  State state_ = State::Setting;
  int minutes_ = 25;
  uint32_t total_ = 25u * 60000u;
  uint32_t remaining_ = 25u * 60000u;
  uint32_t startMs_ = 0;
  uint32_t baseRemaining_ = 0;
};
