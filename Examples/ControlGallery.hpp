#pragma once
#include <oscr/Attributes.hpp>
#include <oscr/Widgets.hpp>
#include <ossia/detail/logger.hpp>
#include <ossia/network/value/format_value.hpp>
#include <boost/pfr.hpp>

#include <cmath>

namespace examples
{

struct ControlGallery
{
  meta_attribute(pretty_name, "Control gallery");
  meta_attribute(script_name, control_gallery);
  meta_attribute(category, Demo);
  meta_attribute(kind, Other);
  meta_attribute(author, "<AUTHOR>");
  meta_attribute(description, "<DESCRIPTION>");
  meta_attribute(uuid, "a9b0e2c6-61e9-45df-a75d-27abf7fb43d7");

  struct {
    //! Buttons are level-triggers: true as long as the button is pressed
    struct {
      meta_control(Control::Button, "Press me ! (Button)");
      ossia::timed_vec<bool> values{};
    } button;

    //! In contrast, impulses are edge-triggers: there is only a value at the moment of the click.
    struct {
      meta_control(Control::ImpulseButton, "Press me ! (Impulse)");
      ossia::timed_vec<ossia::impulse> values{};
    } impulse_button;

    //! Common widgets
    struct {
      meta_control(Control::FloatSlider, "Float slider", 0., 1., 0.5);
      ossia::timed_vec<float> values{};
    } float_slider;
    struct {
      meta_control(Control::FloatKnob, "Float knob", 0., 1., 0.5);
      ossia::timed_vec<float> values{};
    } float_knob;
    struct {
      meta_control(Control::LogFloatSlider, "Float slider (log)", 0., 1., 0.5);
      ossia::timed_vec<float> values{};
    } log_float_slider;

    struct {
      meta_control(Control::IntSlider, "Int slider", 0, 1000, 10);
      ossia::timed_vec<int> values{};
    } int_slider;
    struct {
      meta_control(Control::IntSpinBox, "Int spinbox", 0, 1000, 10);
      ossia::timed_vec<int> values{};
    } int_spinbox;

    //! Will look like a checkbox
    struct {
      meta_control(Control::Toggle, "Toggle", true);
      ossia::timed_vec<bool> values{};
    } toggle;

    //! Same, but allows to choose what is displayed.
    struct {
      meta_control(Control::ChooserToggle, "Chooser toggle", {"Falsey", "Truey"}, false);
      ossia::timed_vec<bool> values{};
    } chooser_toggle;

    //! Allows to edit some text.
    struct {
      meta_control(Control::LineEdit, "Line edit", "Henlo");
      ossia::timed_vec<std::string> values{};
    } lineedit;

    //! First member of the pair is the text, second is the value.
    struct {
      static const constexpr std::array<std::pair<const char*, float>, 3> choices() {
        return {{{"Foo", -10.f}, {"Bar", 0.f}, {"Baz", 10.f}}};
      };
      meta_control(Control::ComboBox, "Combo box", 1, choices());
      ossia::timed_vec<float> values{};
    } combobox;

    //! Will give the string as a result.
    struct {
      static const constexpr std::array<const char*, 4> choices() {
        return {"Roses", "Red", "Violets", "Blue"};
      };
      meta_control(Control::Enum, "Enum", 1, choices());
      ossia::timed_vec<std::string> values{};
    } enumeration;

    //! Same as Enum but won't reject strings that are not part of the list.
    struct {
      static const constexpr std::array<const char*, 3> choices() {
        return {"Square", "Sine", "Triangle"};
      };
      meta_control(Control::UnvalidatedEnum, "Unchecked enum", 1, choices());
      ossia::timed_vec<std::string> values{};
    } unvalidated_enumeration;

    //! It's also possible to use this which will define an enum type and
    //! map to it automatically.
    //! e.g. in source one can then do:
    //!
    //!   auto& param = inputs.simpler_enumeration;
    //!   using enum_type = decltype(param)::enum_type;
    //!   switch(param.value) {
    //!      case enum_type::Square:
    //!        ...
    //!   }
    //!
    //! OSC messages can use either the int index or the string.
    struct {
      meta_enum("Simple Enum", 1, Square, Peg, Round, Hole);
      ossia::timed_vec<enum_type> values{};
    } simpler_enumeration;

    //! Crosshair XY chooser
    struct {
      meta_control(Control::XYSlider, "XY", -5.f, 5.f, 0.f);
      ossia::timed_vec<ossia::vec2f> values{};
    } position;

    //! Color chooser. Colors are in 8-bit RGBA by default.
    struct {
      meta_control(Control::HSVSlider, "Color");
      ossia::timed_vec<ossia::vec4f> values{};
    } color;
  } inputs;

  void operator()()
  {
    const bool has_impulse = !inputs.impulse_button.values.empty();
    const bool has_button = ossia::any_of(inputs.button.values, [] (const auto& p) { return p.second == true; });

    if(!has_impulse && !has_button)
      return;

    ossia::logger().debug("");
    boost::pfr::for_each_field(
        inputs,
        [] (const auto& input) {
          {
            auto val = input.values.begin()->second;
            if constexpr(!std::is_same_v<decltype(val), ossia::impulse>)
              ossia::logger().critical("changed: {} {}", input.control().name, val);
          }
    });
  }
};

}
