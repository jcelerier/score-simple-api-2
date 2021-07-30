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
   * UUIDs are important to uniquely identify plug-ins: you can use uuidgen for instance.
   */
  meta_attribute(pretty_name, "My pretty distortion");
  meta_attribute(script_name, disto_123);
  meta_attribute(category, Audio);
  meta_attribute(kind, AudioEffect);
  meta_attribute(author, "<AUTHOR>");
  meta_attribute(description, "<DESCRIPTION>");
  meta_attribute(uuid, "b9237d2a-1651-4dcc-920b-80e5e619c6c4");

  /**
   * This is used to define the sample-accuracy level of this plug-in ;
   * does the function run for each sample, every time a control changes,
   * for each buffer, etc...
   */
  using control_policy = ossia::safe_nodes::last_tick;

  /** We define the input ports of our process: in this case,
   *  there's an audio input, a gain slider.
   */
  struct {
    struct {
      meta_attribute(name, "In");
      multichannel_audio_view samples;
    } audio;

    struct {
      meta_control(Control::FloatSlider, "Gain", 0.f, 100.f, 10.f);

      float value = 10.f;
    } gain;

  } inputs;

  /** And the output ports: only an audio output on this one **/
  struct {
    struct {
      meta_attribute(name, "Out")
      multichannel_audio samples;
    } audio;

  } outputs;

  /** Will be called upon creation, and whenever the buffer size / sample rate changes **/
  void reset(ossia::exec_state_facade st)
  {
    // Reserve memory for two channels
    inputs.audio.samples.reserve(2, st.bufferSize());
    outputs.audio.samples.reserve(2, st.bufferSize());
  }

  /** Our actual function for transforming the inputs into outputs **/
  void run(
      const ossia::token_request& t,
      ossia::exec_state_facade st
  )
  {
    auto& gain = inputs.gain;
    auto& p1 = inputs.audio;
    auto& p2 = outputs.audio;

    // Allocate outputs
    const auto chans = p1.samples.size();
    p2.samples.resize(chans);

    // When do we start writing, and for how many samples
    const int64_t N = t.physical_write_duration(st.modelToSamples());
    const int64_t first_pos = t.physical_start(st.modelToSamples());
    // Process the input buffer
    for (std::size_t i = 0; i < chans; i++)
    {
      auto& in = p1.samples[i];
      auto& out = p2.samples[i];
      out.resize(in.size());

      const int64_t samples = in.size();
      int64_t max = std::min(N, samples);

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
