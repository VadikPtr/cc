#pragma once
#include "cc/common.hpp"
#include "cc/time.hpp"
#include "cc/ifsm.hpp"

#define mInState(name)                                            \
  StateDecl state_in_##name##_ = states[u32(name)].in_state =     \
      &std::remove_reference_t<decltype(*this)>::state_in_##name; \
  void state_in_##name()

#define mEnterState(name)                                            \
  StateDecl state_enter_##name##_ = states[u32(name)].enter_state =  \
      &std::remove_reference_t<decltype(*this)>::state_enter_##name; \
  void state_enter_##name()

#define mExitState(name)                                            \
  StateDecl state_exit_##name##_ = states[u32(name)].exit_state =   \
      &std::remove_reference_t<decltype(*this)>::state_exit_##name; \
  void state_exit_##name()

#define mInTransition(...)                                              \
  StateDecl state_decl_in_transition_ = do_timed_transition =           \
      &std::remove_reference_t<decltype(*this)>::state_func_transition; \
  void state_func_transition(__VA_ARGS__)


class FSMTimedTransition {
  Time end  = Time();
  Time diff = Time();

 public:
  void start(Time duration);
  void reset();
  bool has_value() const;
  bool ended(const Time& now = Time::now()) const;
  f32  progress(const Time& now = Time::now()) const;  // returns [0, 1]
};


template <typename TState, typename TInstance>
class Fsm : public IFsm {
 private:
  enum class StateMachineState { InState, Transitioning, Sleeping, Final };
  StateMachineState  self_state_ = StateMachineState::InState;
  FSMTimedTransition timed_transition_;
  FSMTimedTransition sleep_;

 protected:
  struct StateInfo {
    void (TInstance::*in_state)()    = nullptr;
    void (TInstance::*enter_state)() = nullptr;
    void (TInstance::*exit_state)()  = nullptr;
  };

  StateInfo states[u32(TState::Final)]                 = {};
  void (TInstance::*do_timed_transition)(f32 progress) = nullptr;
  TState current                                       = TState::Init;
  TState to_state;

  struct StateDecl {
    template <typename TAny>
    StateDecl([[maybe_unused]] TAny any) {}
  };

 public:
  void sleep(Time duration) {
    sleep_.start(duration);
    self_state_ = StateMachineState::Sleeping;
  }

  TState state() const { return current; }
  bool   is_done() const override { return self_state_ == StateMachineState::Final; }

  void to(TState state) {
    to_state = state;
    sleep_.reset();
    if (not timed_transition_.has_value()) {
      enter_new_state(to_state);
    } else {
      self_state_ = StateMachineState::Transitioning;
    }
  }

  void to(TState state, Time duration) {
    timed_transition_.start(duration);
    to(state);
  }

  void reset() override {
    current     = TState::Init;
    self_state_ = StateMachineState::InState;
    timed_transition_.reset();
    sleep_.reset();
  }

  void wake_up() override {
    if (self_state_ == StateMachineState::Transitioning) {
      enter_new_state(to_state);
    }
    self_state_ = StateMachineState::InState;
    timed_transition_.reset();
    sleep_.reset();
  }

  void update() override {
    if (self_state_ == StateMachineState::Final) {
      return;
    }

    TInstance* self        = static_cast<TInstance*>(this);
    Time       now         = Time::now();
    bool       switch_next = false;

    if (self_state_ == StateMachineState::Sleeping) {
      if (sleep_.ended(now)) {
        self_state_ = StateMachineState::InState;
      }
    }

    if (self_state_ == StateMachineState::Transitioning) {
      // if (to_state == TState::Final) {
      //   self_state_ = StateMachineState::Final;
      //   current     = TState::Final;
      //   return;
      // }
      if (timed_transition_.has_value()) {
        if (timed_transition_.ended(now)) {
          switch_next = true;
        } else {
          if (do_timed_transition) {
            f32 progress = timed_transition_.progress(now);
            (self->*do_timed_transition)(progress);
          }
        }
      } else {
        switch_next = true;
      }
    }

    if (switch_next) {
      enter_new_state(to_state);
      timed_transition_.reset();
      sleep_.reset();
    }

    if (self_state_ == StateMachineState::InState) {
      if (current == TState::Final) {
        self_state_ = StateMachineState::Final;
        return;
      }
      StateInfo& info = states[u32(current)];
      if (info.in_state) {
        (self->*info.in_state)();
      }
    }
  }

 private:
  void enter_new_state(TState next) {
    TInstance* self = static_cast<TInstance*>(this);
    if (StateInfo& prev = states[u32(current)]; prev.exit_state) {
      (self->*prev.exit_state)();
    }
    current = next;
    if (current == TState::Final) {
      self_state_ = StateMachineState::Final;
      return;
    }
    self_state_ = StateMachineState::InState;
    if (StateInfo& cur = states[u32(current)]; cur.enter_state) {
      (self->*cur.enter_state)();
    }
  }
};
