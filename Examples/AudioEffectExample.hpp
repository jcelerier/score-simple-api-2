#pragma once
#include <SimpleApi2/Attributes.hpp>
#include <Control/Widgets.hpp>

namespace SimpleApi2
{

struct AudioEffectExample
{
  meta_attribute(pretty_name, "My example effect");
  meta_attribute(script_name, effect_123);
  meta_attribute(category, Audio);
  meta_attribute(kind, AudioEffect);
  meta_attribute(author, "<AUTHOR>");
  meta_attribute(description, "<DESCRIPTION>");
  meta_attribute(uuid, "c8b57fff-c34c-4772-8f72-fe5267527ece");

  struct {
    struct {
      meta_attribute(name, "In");
      const double** samples{};
      std::size_t channels{};
    } audio;

    struct {
      meta_control(Control::FloatSlider, "Gain", 0.f, 100.f, 10.f);

      float value = 10.f;
    } gain;
  } inputs;

  struct {
    struct {
      meta_attribute(name, "Out");
      double** samples{};
      std::size_t channels{};
    } audio;
  } outputs;

  void run(std::size_t N)
  {
    auto& gain = inputs.gain;
    auto& p1 = inputs.audio;
    auto& p2 = outputs.audio;

    const auto chans = p1.channels;

    // Process the input buffer
    for (std::size_t i = 0; i < chans; i++)
    {
      auto& in = p1.samples[i];
      auto& out = p2.samples[i];

      for (std::size_t j = 0; j < N; j++)
      {
        out[j] = in[j] * gain.value;
      }
    }
  }
};
}
