#pragma once
#include <SimpleApi2/Attributes.hpp>
#include <Control/Widgets.hpp>

namespace SimpleApi2
{
/**
 * This example exhibits a more advanced multi-channel processor, with side-chain inputs and outputs.
 */
struct AudioSidechainExample
{
  meta_attribute(pretty_name, "Sidechain example");
  meta_attribute(script_name, effect_123);
  meta_attribute(category, Audio);
  meta_attribute(kind, AudioEffect);
  meta_attribute(author, "<AUTHOR>");
  meta_attribute(description, "<DESCRIPTION>");
  meta_attribute(uuid, "b9f63462-e41e-4a5a-9f41-f322a4699862");

  /**
   * Here we have a special case, which happens to be the most common case in audio
   * development. If our inputs start with an audio port of the shape
   *
   *     const double** samples;
   *     std::size_t channels;
   *
   * and our outputs starts with an audio port of shape
   *
   *     double** samples;
   *
   * then it is assumed that we are writing an effect processor, where the outputs
   * should match the inputs. There will be as many output channels as input channels,
   * with enough samples allocated to write from 0 to N.
   */
  struct {
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
      meta_attribute(use_channels, audio);

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
