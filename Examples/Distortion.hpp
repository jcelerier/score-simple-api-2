#pragma once
#include <SimpleApi2/Attributes.hpp>
#include <SimpleApi2/Concepts.hpp>
#include <Control/Widgets.hpp>
#include <cmath>


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

  ossia::audio_channel temp_buffer;
  /** Will be called upon creation, and whenever the buffer size / sample rate changes **/
  void reset(ossia::exec_state_facade st)
  {
    // Reserve memory for two channels, for the input and the output.
    inputs.audio.samples.reserve(2, st.bufferSize());
    outputs.audio.samples.reserve(2, st.bufferSize());
    temp_buffer.reserve(st.bufferSize());

    // Note that channels in score are dynamic ; your process should not expect
    // a fixed number of channels, even across a single execution: it could be applied to 2 channels
    // at the start of a score, and 5 channels at the end.

    // It is still a work-in-progress to define a real-time-safe way to support this dynamic behaviour;
    // it is however guaranteed that there won't be other allocations during execution if
    // enough space had been reserved.
  }

  /**
   * Our actual function for transforming the inputs into outputs.
   *
   * Note that when using multichannel_audio_view and multichannel_audio,
   * there's no guarantee on the size of the data: on a token_request for 64 samples,
   * there may be for instance only 23 samples of 2-channel input,
   * while the output of the node could be 10 channels.
   *
   * It is up to the author to set-up the output according to its wishes for the plug-in.
   **/
  void operator()(int64_t N)
  {
    auto& gain = inputs.gain;
    auto& input = inputs.audio.samples;
    auto& output = outputs.audio.samples;

    // How many input channels
    const auto chans = input.channels();

    // First sum all the input channels into a mono buffer
    temp_buffer.clear();
    for (std::size_t i = 0; i < chans; i++)
    {
      // The buffers are accessed through spans.
      std::span<const double> in = input[i];

      // Filled with zeros
      temp_buffer.resize(std::max(temp_buffer.size(), in.size()));

      // Sum samples from the input buffer
      const int64_t samples_to_read = std::min(N, int64_t(in.size()));
      for(int64_t j = 0; j < samples_to_read; j++)
      {
        temp_buffer[j] += in[j];
      }
    }

    // Then output a stereo buffer of that with distortion, and reversed phase.
    // We fix the output channels to 2 for this example.
    const auto output_chans = 2;

    // We could have made things a bit simpler by just resizing our temp_buffer to N
    // and using that for the output ; this may save a few samples of silence though.
    const int64_t samples_to_write = std::min(N, int64_t(temp_buffer.size()));

    output.resize(output_chans, samples_to_write);

    // Write to the channels once they are allocated:
    std::span<double> out_l = output[0];
    std::span<double> out_r = output[1];

    for(int64_t j = 0; j < samples_to_write; j++)
    {
      using namespace std;
      out_l[j] = tanh(temp_buffer[j] * gain.value);
      out_r[j] = 1. - out_l[j];
    }
  }
};
}
