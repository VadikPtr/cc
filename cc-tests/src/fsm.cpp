#include "cc/test.hpp"
#include "cc/all.hpp"

namespace {
  enum class FsmState {
    Init,
    Running,
    Final,
  };
  struct ExampleFsm : Fsm<FsmState, ExampleFsm> {
    using enum FsmState;
    mInState(Init) { to(Running); }
    mInState(Running) { to(Final); }
  };
}  // namespace

mTestCase(fsm_example) {
  ExampleFsm fsm;
  mRequire(not fsm.is_done());
  mRequire(fsm.state() == FsmState::Init);

  fsm.update();
  mRequire(not fsm.is_done());
  mRequire(fsm.state() == FsmState::Running);

  fsm.update();
  mRequire(fsm.is_done());
  mRequire(fsm.state() == FsmState::Final);

  fsm.update();
  mRequire(fsm.is_done());
  mRequire(fsm.state() == FsmState::Final);
}
