#pragma once

struct IFsm {
  virtual ~IFsm()              = default;
  virtual bool is_done() const = 0;
  virtual void update()        = 0;
  virtual void reset()         = 0;
  virtual void wake_up()       = 0;
};
