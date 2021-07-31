#pragma once
#include <SimpleApi2/Attributes.hpp>
#include <Control/Widgets.hpp>
#include <ossia/detail/timed_vec.hpp>

namespace SimpleApi2
{
struct CCC
{
  meta_attribute(pretty_name, "CCC");
  meta_attribute(script_name, CCC);
  meta_attribute(category, Control);
  meta_attribute(kind, Generator);
  meta_attribute(author, "Peter Castine");
  meta_attribute(description, "1/f noise, using the Schuster/Procaccia deterministic (chaotic) algorithm");
  meta_attribute(uuid, "9db0af3c-8573-4541-95d4-cf7902cdbedb");

  struct {
    struct {
      meta_control(Control::ImpulseButton, "Bang");

      ossia::timed_vec<ossia::impulse> values;
    } bang;
  } inputs;

  struct {
    struct {
      meta_attribute(name, "Out");
      ossia::timed_vec<float> values;
    } out;
  } outputs;

  double current_value{0.1234};

  void operator()()
  {
    for(auto& [timestamp, value] : inputs.bang.values)
    {
      // CCC algorithm, copied verbatim from the LitterPower source code.
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

      outputs.out.values[timestamp] = current_value;
    }
  }
};
}
