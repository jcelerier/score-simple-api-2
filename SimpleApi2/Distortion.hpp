#pragma once
#include <SimpleApi2/Concepts.hpp>
#include <cmath>
#include <Control/Widgets.hpp>
#include <ossia/dataflow/safe_nodes/tick_policies.hpp>

namespace SimpleApi2
{
struct Distortion
{
  /**
   * An audio effect plug-in must provide some metadata: name, author, etc.
   */
  struct Metadata : SimpleApi2::Meta_base
  {
    static const constexpr auto prettyName = "Distortion";
    static const constexpr auto objectKey = "Distortion";
    static const constexpr auto category = "Audio";
    static const constexpr auto author = "<AUTHOR>";
    static const constexpr auto kind = Process::ProcessCategory::AudioEffect;
    static const constexpr auto description = "<DESCRIPTION>";
    static const constexpr auto tags = std::array<const char*, 0>{};
    static const uuid_constexpr auto uuid
        = make_uuid("9d4e59a6-e062-49cf-98f3-1c17a932bfff");
  };

  /**
   * This is used to define the sample-accuracy level of this plug-in ;
   * does the function run for each sample, every time a control changes,
   * for each buffer, etc...
   */
  using control_policy = ossia::safe_nodes::last_tick;

  struct audio_inputs {
    struct main_inlet : audio_input {
      constant name = "In";
    } p1;
  } audio_inputs;

  struct audio_outputs {
    struct main_outlet : audio_output {
      constant name = "Out";
    } p2;
  } audio_outputs;

/*
  struct midi_inputs { } midi_inputs;
  struct midi_outputs { } midi_outputs;
  struct value_inputs { } value_inputs;
  struct value_outputs { } value_outputs;
*/

  struct control_inputs {
    struct gain : control_input{
      constant control = Control::FloatSlider{
          "Gain", 0.f, 100.f, 10.f};

      float value = 10.f;
    } gain;
  } control_inputs;

  struct control_outputs { } control_outputs;


  void run(
      const ossia::token_request& t,
      ossia::exec_state_facade st
  )
  {
    auto& gain = control_inputs.gain;
    auto& p1 = audio_inputs.p1;
    auto& p2 = audio_outputs.p2;

    // Allocate outputs
    const auto chans = p1.port->samples.size();
    p2.port->samples.resize(chans);

    // When do we start writing, and for how many samples
    const int64_t N = t.physical_write_duration(st.modelToSamples());
    const int64_t first_pos = t.physical_start(st.modelToSamples());

    // Process the input buffer
    for (std::size_t i = 0; i < chans; i++)
    {
      auto& in = p1.port->samples[i];
      auto& out = p2.port->samples[i];

      const int64_t samples = in.size();
      int64_t max = std::min(N, samples);

      out.resize(samples);

      for (int64_t j = first_pos; j < max; j++)
      {
        // Very crude distortion
        using namespace std;
        out[j] = tanh(in[j] * gain.value);
      }
    }
  }
};
}
