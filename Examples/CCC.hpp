#pragma once
#include <SimpleApi2/Concepts.hpp>
#include <cmath>
#include <Control/Widgets.hpp>
#include <ossia/dataflow/safe_nodes/tick_policies.hpp>

namespace SimpleApi2
{
struct CCC
{
  meta_attribute(pretty_name, "CCC");
  meta_attribute(script_name, "CCC");
  meta_attribute(category, Audio);
  meta_attribute(kind, Generator);
  meta_attribute(author, "Peter Castine");
  meta_attribute(description, "1/f noise, using the Schuster/Procaccia deterministic (chaotic) algorithm");
  meta_attribute(uuid, "9db0af3c-8573-4541-95d4-cf7902cdbedb");

  struct {
    struct bang : control_input {
      meta_control(Control::ImpulseButton, "Bang");

      ossia::safe_nodes::timed_vec<ossia::impulse> value;
    } bang;
  } inputs;

  struct {
    struct main_outlet : value_output {
      meta_attribute(name, "Out");
    } out;
  } outputs;

  double current_value{0.1234};

  void run(
      const ossia::token_request& t,
      ossia::exec_state_facade st
      )
  {
    for(auto& [timestamp, value] : inputs.bang.value)
    {
      // CCC algorithm
      {
        constexpr double kMinPink = 1.0 / 525288.0;

        double curVal = this->current_value;

        // Sanity check... due to limitations in accuracy, we can die at very small values.
        // Also, we prefer to only "nudge" the value towards chaos...
        if (curVal <= kMinPink) {
          if (curVal == 0.0)	curVal  = kMinPink;
          else				curVal += curVal;
        }

        curVal = curVal * curVal + curVal;
        if (curVal >= 1.0)					// Cheaper than fmod(), and works quite nicely
          curVal -= 1.0;					// in the range of values that can occur.

        this->current_value = curVal;
      }

      outputs.out.port->write_value(current_value, timestamp);
    }
  }
};
}
