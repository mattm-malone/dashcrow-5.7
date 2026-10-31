#pragma once
#include "TimerModel.h"
#include "TimerPage.h"
#include "Types.h"

TimerView makeTimerView(const TimerModel &timer, const LocalTime &now);
