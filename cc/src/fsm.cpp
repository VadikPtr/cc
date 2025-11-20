#include "cc/fsm.hpp"

void FSMTimedTransition::start(Time duration) {
  end  = Time::now() + duration;
  diff = duration;
}

void FSMTimedTransition::reset() {
  end = diff = Time();
}

bool FSMTimedTransition::has_value() const {
  return end.has_value();
}

bool FSMTimedTransition::ended(const Time& now) const {
  return now > end;
}

f32 FSMTimedTransition::progress(const Time& now) const {
  if (ended(now)) {
    return 1.0f;
  }
  return 1.0f - ((end - now) / diff);
}
