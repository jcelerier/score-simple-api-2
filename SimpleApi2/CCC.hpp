#pragma once
#include <SimpleApi2/Concepts.hpp>
#include <cmath>
#include <Control/Widgets.hpp>
#include <ossia/dataflow/safe_nodes/tick_policies.hpp>

namespace SimpleApi2
{
struct CCC
{
  struct Metadata
  {
    static const constexpr auto prettyName = "CCC";
    static const constexpr auto objectKey = "CCC";
    static const constexpr auto category = "Audio";
    static const constexpr auto author = "Peter Castine";
    static const constexpr auto kind = Process::ProcessCategory::Generator;
    static const constexpr auto description = "1/f noise, using the Schuster/Procaccia deterministic (chaotic) algorithm";
    static const constexpr auto tags = std::array<const char*, 0>{};
    static const uuid_constexpr auto uuid
        = make_uuid("9db0af3c-8573-4541-95d4-cf7902cdbedb");
  };

  struct inputs {
    struct bang : control_input {
      constant control = Control::ImpulseButton{ "Bang" };
      ossia::safe_nodes::timed_vec<ossia::impulse> value;
    } bang;
  } inputs;

  struct outputs {
    struct main_outlet : value_output {
      constant name = "Output";
    } out;
  } outputs;

  double current_value{0.1234};

  void run(
      const ossia::token_request& t,
      ossia::exec_state_facade st
      )
  {
    if(inputs.bang.value.empty())
      return;

    // CCC algorithm
    {
      const double kMinPink = 1.0 / 525288.0;

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

    int64_t timestamp = t.physical_start(st.modelToSamples());
    outputs.out.port->write_value(current_value, timestamp);
  }
};
}
