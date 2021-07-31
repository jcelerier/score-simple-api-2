#pragma once
#include <SimpleApi2/Attributes.hpp>
#include <ossia/detail/timed_vec.hpp>
#include <rnd/random.hpp>

namespace SimpleApi2
{

struct ValueGeneratorExample
{
  meta_attribute(pretty_name, "My example generator");
  meta_attribute(script_name, effect_gen);
  meta_attribute(category, Debug);
  meta_attribute(kind, Other);
  meta_attribute(author, "<AUTHOR>");
  meta_attribute(description, "<DESCRIPTION>");
  meta_attribute(uuid, "c519b3c4-326e-4e80-8dec-d465264c5b08");

  /**
   * Here we define a single output, which allows writing
   * sample-accurate data to an output port of the node.
   * Timestamps start from zero (at the beginning of a buffer) to N:
   * i ∈ [0; N( in the usual mathematic notation.
   */
  struct {
    struct {
      // Give a name to our parameter to show the user
      meta_attribute(name, "Out");

      // The data type used must conform to std::map<int64_t, your_type>
      ossia::timed_vec<int> values;
    } value;
  } outputs;

  // Note that buffer sizes aren't necessarily powers-of-two: N can be 1 for instance.
  void operator()(std::size_t N)
  {
    // 0 : first sample of the buffer.
    outputs.value.values[0] = rnd::rand(0, 100);
  }
};
}
