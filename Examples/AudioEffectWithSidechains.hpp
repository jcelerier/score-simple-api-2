#pragma once
#include <oscr/Attributes.hpp>
#include <oscr/Widgets.hpp>

namespace examples
{
/**
 * This example exhibits a more advanced multi-channel processor, with side-chain inputs and outputs.
 */
struct AudioSidechainExample
{
  meta_attribute(pretty_name, "Sidechain example");
  meta_attribute(script_name, effect_123);
  meta_attribute(category, Demo);
  meta_attribute(kind, AudioEffect);
  meta_attribute(author, "<AUTHOR>");
  meta_attribute(description, "<DESCRIPTION>");
  meta_attribute(uuid, "6fcfa5ad-ac5e-4851-a7bc-72f6fbf57dcd");

  struct {
    // Note: one could of course define helper types.
    // The point of these examples is to show that the helper types are strictly
    // syntax sugar ; what matters is the shape of our inputs / outputs.
    struct {
      meta_attribute(name, "In");

      const double** samples{};
      std::size_t channels{};
    } audio;

    struct {
      meta_attribute(name, "Sidechain");

      const double** samples{};
      std::size_t channels{};
    } sidechain;

    struct {
      meta_control(Control::FloatSlider, "Gain", 0.f, 100.f, 10.f);

      float value = 10.f;
    } gain;
  } inputs;

  struct {
    struct {
      meta_attribute(name, "Out");

      // Here we say: use the same number of channels than the input "audio".
      meta_attribute(mimic_channels, audio);

      double** samples{};
    } audio;

    struct {
      meta_attribute(name, "Mono downmix");

      // Here we say: allocate one channel.
      meta_attribute(want_channels, 1);

      double** samples{};
    } side_out;

  } outputs;

  void operator()(std::size_t N)
  {
    auto& gain = inputs.gain;
    auto& p1 = inputs.audio;
    auto& sc = inputs.sidechain;
    auto& p2 = outputs.audio;
    auto& mono = outputs.side_out.samples[0];

    const auto chans = p1.channels;

    // Process the input buffer
    for (std::size_t i = 0; i < chans; i++)
    {
      auto& in = p1.samples[i];
      auto& out = p2.samples[i];

      // If there's enough channels in the sidechain, use it
      if(sc.channels > i)
      {
        auto& sidechain = sc.samples[i];
        for (std::size_t j = 0; j < N; j++)
        {
          out[j] = std::abs(sidechain[j] * gain.value) > 0.5 ? in[j] * gain.value : 0.0;
          mono[j] += out[j];
        }
      }
      else
      {
        // Don't use the sidechain
        for (std::size_t j = 0; j < N; j++)
        {
          out[j] = in[j] * gain.value;
          mono[j] += out[j];
        }
      }
    }
  }
};
}
