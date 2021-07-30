#pragma once
#include <SimpleApi2/Concepts.hpp>
#include <cmath>
#include <Control/Widgets.hpp>
#include <ossia/dataflow/safe_nodes/tick_policies.hpp>

struct multichannel_audio_view {
  const ossia::audio_vector* buffer{};
  const ossia::audio_channel& operator[](std::size_t i) const noexcept { return (*buffer)[i]; };
  std::size_t size() const noexcept { return buffer->size(); }
};

struct multichannel_audio {
  ossia::audio_vector* buffer{};
  ossia::audio_channel& operator[](int i) const noexcept { return (*buffer)[i];};
  std::size_t size() const noexcept { return buffer->size(); }
  void resize(std::size_t i) const noexcept { return buffer->resize(i); }
};

namespace SimpleApi2
{
struct Distortion
{
  /**
   * An audio effect plug-in must provide some metadata: name, author, etc.
   */
  struct Metadata
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

  struct inputs {
    struct {
      constant name() { return "In"; }
      multichannel_audio_view samples;
    } audio;

    struct gain : control_input {
      constant control = Control::FloatSlider{ "Gain", 0.f, 100.f, 10.f };

      float value = 10.f;
    } gain;
  } inputs;

  struct outputs {
    struct {
      constant name() { return "Out"; }
      multichannel_audio samples;
    } audio;

  } outputs;

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
