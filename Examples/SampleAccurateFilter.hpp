#pragma once
#include <oscr/Attributes.hpp>
#include <cmath>

namespace examples
{

struct SampleAccurateFilterExample
{
  meta_attribute(pretty_name, "My sample-accurate filter");
  meta_attribute(script_name, sample_acc_filt);
  meta_attribute(category, Demo);
  meta_attribute(kind, Other);
  meta_attribute(author, "<AUTHOR>");
  meta_attribute(description, "<DESCRIPTION>");
  meta_attribute(uuid, "43818edd-63de-458b-a6a5-08033cefc051");

  /**
   * Here we define an input and an output pair.
   */
  struct {
    struct {
      // Give a name to our parameter to show the user
      meta_attribute(name, "In");

      // The "event" attribute on input is used by score to check whether the value has to be read on every tick
      // even if no messages were received, or if it will only be set whenever a new message is received.
      // In the default case (event == false), `values` will never be empty if connected to, say, an OSC address or MIDI CC ;
      // the running value is always copied at the start of the array.
      // If one sets event == true, then `values` will only contain something at the tick which follows a network / MIDI / ... message.
      meta_attribute(event, true);

      // The data type used must conform to std::map<int64_t, your_type>
      ossia::timed_vec<float> values;
    } value;
  } inputs;

  struct {
    struct {
      // Give a name to our parameter to show the user
      meta_attribute(name, "Out");

      // The data type used must conform to std::map<int64_t, your_type>
      ossia::timed_vec<float> values;
    } value;
  } outputs;

  void operator()()
  {
    // The output is copied at the same timestamp at which each input happened.
    for(auto& [timestamp, value]: inputs.value.values)
    {
      outputs.value.values[timestamp] = cos(value * value) * cos(value);
    }
  }
};
}
