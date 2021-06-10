#pragma once

#include <boost/pfr.hpp>
#include <ossia/dataflow/safe_nodes/port.hpp>
namespace SimpleApi2
{

template<typename>
struct get_type_list;

template<typename T>
struct get_control_value_type
{
  using control_type = decltype(std::remove_reference_t<T>::control);
  using type = typename control_type::type;

};
template<template<typename ...> typename Tuple, typename ... T>
struct get_type_list<Tuple<T...>>
{
  using type = Tuple<typename get_control_value_type<T>::type...>;
};


template <typename Node_T>
struct info_functions_2
{
  using controls_type = decltype(Node_T::control_inputs);
  using controls_tuple = decltype(boost::pfr::structure_to_tuple(std::declval<controls_type&>()));
  using controls_values_type = typename SimpleApi2::get_type_list<controls_tuple>::type;

  using control_outs_type = decltype(Node_T::control_outputs);
  using control_outs_tuple = decltype(boost::pfr::structure_to_tuple(std::declval<control_outs_type&>()));
  using control_outs_values_type = typename SimpleApi2::get_type_list<control_outs_tuple>::type;

  static constexpr auto audio_in_count = [] {
    if constexpr(requires { Node_T::audio_inputs; })
      return boost::pfr::tuple_size_v<decltype(Node_T::audio_inputs)>;
    else
      return 0; }();
  static constexpr auto audio_out_count = [] {
    if constexpr(requires { Node_T::audio_outputs; })
      return boost::pfr::tuple_size_v<decltype(Node_T::audio_outputs)>;
    else
      return 0; }();

  static constexpr auto midi_in_count = [] {
    if constexpr(requires { Node_T::midi_inputs; })
      return boost::pfr::tuple_size_v<decltype(Node_T::midi_inputs)>;
    else
      return 0; }();
  static constexpr auto midi_out_count = [] {
    if constexpr(requires { Node_T::midi_outputs; })
      return boost::pfr::tuple_size_v<decltype(Node_T::midi_outputs)>;
    else
      return 0; }();

  static constexpr auto value_in_count = [] {
    if constexpr(requires { Node_T::value_inputs; })
      return boost::pfr::tuple_size_v<decltype(Node_T::value_inputs)>;
    else
      return 0; }();
  static constexpr auto value_out_count = [] {
    if constexpr(requires { Node_T::value_outputs; })
      return boost::pfr::tuple_size_v<decltype(Node_T::value_outputs)>;
    else
      return 0; }();

  static constexpr auto control_in_count = [] {
    if constexpr(requires { Node_T::control_inputs; })
      return boost::pfr::tuple_size_v<decltype(Node_T::control_inputs)>;
    else
      return 0; }();
  static constexpr auto control_out_count = [] {
    if constexpr(requires { Node_T::control_outputs; })
      return boost::pfr::tuple_size_v<decltype(Node_T::control_outputs)>;
    else
      return 0; }();

  static constexpr auto categorize_inlet(std::size_t i)
  {
    if (i < audio_in_count)
      return ossia::safe_nodes::inlet_kind::audio_in;
    else if (i < audio_in_count + midi_in_count)
      return ossia::safe_nodes::inlet_kind::midi_in;
    else if (i < audio_in_count + midi_in_count + value_in_count)
      return ossia::safe_nodes::inlet_kind::value_in;
    else if (i < audio_in_count + midi_in_count + value_in_count + control_in_count)
      return ossia::safe_nodes::inlet_kind::control_in;
    else
      throw std::runtime_error("Invalid input number");
  }

  static constexpr auto categorize_outlet(std::size_t i)
  {
    if (i < audio_out_count)
      return ossia::safe_nodes::outlet_kind::audio_out;
    else if (i < audio_out_count + midi_out_count)
      return ossia::safe_nodes::outlet_kind::midi_out;
    else if (i < audio_out_count + midi_out_count + value_out_count)
      return ossia::safe_nodes::outlet_kind::value_out;
    else if (i < audio_out_count + midi_out_count + value_out_count + control_out_count)
      return ossia::safe_nodes::outlet_kind::control_out;
    else
      throw std::runtime_error("Invalid output number");
  }

  static constexpr auto control_start
      = audio_in_count + midi_in_count + value_in_count;

  static constexpr auto control_out_start
      = audio_out_count + midi_out_count + value_out_count;

  static constexpr auto inlet_size = control_start + control_in_count;

  static constexpr auto outlet_size = control_out_start + control_out_count;
};

}
