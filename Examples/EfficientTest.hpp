#pragma once
#include <SimpleApi2/Concepts.hpp>
#include <cmath>
#include <Control/Widgets.hpp>
#include <ossia/dataflow/safe_nodes/tick_policies.hpp>


namespace SimpleApi2
{
struct EfficientTest
{
  meta_attribute(pretty_name, "Efficient Test plugin");
  meta_attribute(script_name, efficient_test_plugin);
  meta_attribute(category, Audio);
  meta_attribute(kind, AudioEffect);
  meta_attribute(author, "<AUTHOR>");
  meta_attribute(description, "<DESCRIPTION>");
  meta_attribute(uuid, "be958cc2-0545-4a90-bf45-c73db0845d9c");

  struct {
    struct {
      meta_attribute(name, "Audio In");
      const ossia::audio_port* port{};
    } audio;

    struct {
      meta_attribute(name, "Value In");
      const ossia::value_port* port{};
    } value;

    struct {
      meta_attribute(name, "MIDI In");
      const ossia::midi_port* port{};
    } midi;

  } inputs;

  struct {
    struct {
      meta_attribute(name, "Audio In");
      ossia::audio_port* port{};
    } audio;

    struct {
      meta_attribute(name, "Value In");
      ossia::value_port* port{};
    } value;

    struct {
      meta_attribute(name, "MIDI In");
      ossia::midi_port* port{};
    } midi;

  } outputs;


  void run(
      const ossia::token_request& t,
      ossia::exec_state_facade st
  )
  {
    outputs.audio.port->samples = inputs.audio.port->samples;
    outputs.value.port->get_data() = inputs.value.port->get_data();
    outputs.midi.port->messages = inputs.midi.port->messages;
  }
};
}
