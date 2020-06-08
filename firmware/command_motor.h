#ifndef __COMMAND_MOTOR_H__
#define __COMMAND_MOTOR_H__

#include <memory>   // for std::shared_ptr
#include "command_base.h"
#include "hardware_interface.h"
#include "net_interface.h"
#include "debug_interface.h"

namespace Command {

class Motor: public Base {
  public:

  Motor( std::shared_ptr<HWI> hwiArg, std::shared_ptr<DebugInterface> debugArg, std::shared_ptr<NetInterface> netArg, HWI::Pin pin0,  HWI::Pin pin1);

  virtual Time::TimeUS execute() override;
  virtual const char* debugName() override;

  void setSpeed( int percent );

  enum class Dir {
    FORWARD,
    BACKWARDS
  };

  enum class Pulse {
    FORWARD,
    BACKWARD,
    NONE
  };

  void doPulse( Pulse pulse );
  void doPulseIfChanged( Pulse pulse );

  private:

  std::shared_ptr<HWI> hwi;
  std::shared_ptr<DebugInterface> debug;
  std::shared_ptr<NetInterface> net;
  const HWI::Pin pin0;
  const HWI::Pin pin1;

  Dir dir;
  unsigned speed;
  int counter;
  Pulse lastPulse;
};

};

#endif
