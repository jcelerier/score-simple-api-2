#pragma once
#include <SimpleApi2/Concepts.hpp>
#include <SimpleApi2/Metadatas.hpp>
#include <ossia/dataflow/fx_node.hpp>
#include <ossia/dataflow/graph_node.hpp>
#include <ossia/dataflow/node_process.hpp>
#include <ossia/dataflow/port.hpp>
#include <ossia/dataflow/safe_nodes/tick_policies.hpp>
#include <ossia/detail/algorithms.hpp>
#include <ossia/detail/apply_type.hpp>
#include <ossia/detail/lockfree_queue.hpp>
#include <ossia/detail/for_each_in_tuple.hpp>
#include <ossia/dataflow/safe_nodes/node.hpp>
#include <ossia/network/dataspace/dataspace_visitors.hpp>

#include <boost/mp11/algorithm.hpp>
#include <boost/pfr.hpp>
#include <bitset>

namespace ossia
{

template <class F, template<class...> class T1, class... T1s, template<class...> class T2, class... T2s>
void for_each_in_tuples(T1<T1s...>&& t1, T2<T2s...>& t2, F&& func)
{
  static_assert(sizeof...(T1s) == sizeof...(T2s));

  if constexpr(sizeof...(T1s) > 0)
  {
    using namespace std;
    for_each_in_tuples_impl(std::move(t1), t2, forward<F>(func), make_index_sequence<sizeof...(T1s)>());
  }
}

}
namespace SimpleApi2
{

template <typename T, typename = void>
struct has_event_policy : std::false_type
{
};
template <typename T>
struct has_event_policy<T, std::void_t<typename T::event_policy>>
    : std::true_type
{
};

template <typename T, typename = void>
struct has_audio_policy : std::false_type
{
};
template <typename T>
struct has_audio_policy<T, std::void_t<typename T::audio_policy>>
    : std::true_type
{
};

template <typename T, typename = void>
struct has_control_policy : std::false_type
{
};
template <typename T>
struct has_control_policy<T, std::void_t<typename T::control_policy>>
    : std::true_type
{
};

template<typename Exec_T>
struct ExecInitFunc
{
  Exec_T& self;

  void operator()(const AudioInput auto& in)
  {
  }
  void operator()(const AudioOutput auto& out)
  {
  }
  void operator()(const ValueInput auto& in)
  {
  }
  void operator()(const ValueOutput auto& out)
  {
  }
  void operator()(const MidiInput auto& in)
  {
  }
  void operator()(const MidiOutput auto& out)
  {
  }
  void operator()(const ControlInput auto& ctrl)
  {
  }
  void operator()(const ControlOutput auto& ctrl)
  {
  }
};

template<typename Exec_T>
struct InitPorts
{
  Exec_T& self;
  ossia::inlets& inlets;
  ossia::outlets& outlets;

  void operator()(AudioInput auto& in, ossia::audio_inlet& port) const noexcept
  {
    inlets.push_back(std::addressof(port));

    in.samples.buffer = std::addressof(port->samples);
  }

  void operator()(AudioOutput auto& out, ossia::audio_outlet& port) const noexcept
  {
    outlets.push_back(std::addressof(port));

    out.samples.buffer = std::addressof(port->samples);
  }

  void operator()(ValueInput auto& in, ossia::value_inlet& port) const noexcept
  {
    inlets.push_back(std::addressof(port));

    if constexpr(requires { in.is_event; }) {
      port->is_event = in.is_event;
    }
    in.port = std::addressof(*port);
  }

  void operator()(ValueOutput auto& out, ossia::value_outlet& port) const noexcept
  {
    outlets.push_back(std::addressof(port));

    out.port = std::addressof(*port);

    if constexpr(requires { out.type; }) {
      if(!out.type.empty())
        port->type = ossia::parse_dataspace(out.type);
    }
  }

  void operator()(MidiInput auto& in, ossia::midi_inlet& port) const noexcept
  {
    inlets.push_back(std::addressof(port));

    in.port = std::addressof(*port);
  }

  void operator()(MidiOutput auto& out, ossia::midi_outlet& port) const noexcept
  {
    outlets.push_back(std::addressof(port));

    out.port = std::addressof(*port);
  }

  void operator()(ControlInput auto& ctrl, ossia::value_inlet& port) const noexcept
  {
    inlets.push_back(std::addressof(port));

    port->is_event = true;
    //ctrl.port = std::addressof(*port);

    ctrl.control.setup_exec(port);
  }

  void operator()(ControlOutput auto& ctrl, ossia::value_outlet& port) const noexcept
  {
    outlets.push_back(std::addressof(port));

    //ctrl.port = std::addressof(*port);
    ctrl.control.setup_exec(port);
  }
};


template<typename Exec_T>
struct PreparePorts
{
  Exec_T& self;

  void operator()(AudioInput auto& in, ossia::audio_inlet& port) const noexcept
  {
  }

  void operator()(AudioOutput auto& out, ossia::audio_outlet& port) const noexcept
  {
  }

  void operator()(ValueInput auto& in, ossia::value_inlet& port) const noexcept
  {
  }

  void operator()(ValueOutput auto& out, ossia::value_outlet& port) const noexcept
  {
  }

  void operator()(MidiInput auto& in, ossia::midi_inlet& port) const noexcept
  {
  }

  void operator()(MidiOutput auto& out, ossia::midi_outlet& port) const noexcept
  {
  }

  void operator()(ControlInput auto& ctrl, ossia::value_inlet& port) const noexcept
  {
  }

  void operator()(ControlOutput auto& ctrl, ossia::value_outlet& port) const noexcept
  {
  }

};
template <typename Node_T>
class safe_node final : public ossia::nonowning_graph_node
{
public:
  Node_T state;

  using info = info_functions_2<Node_T>;
  typename info::ossia_inputs_tuple input_ports;
  typename info::ossia_outputs_tuple output_ports;

  // std::tuple<float, int...> : current running values of the controls
  using control_input_values_type = typename info::control_input_values_type;
  control_input_values_type control_input;

  // bitset : 1 if the control has changed since the last tick, 0 else
  using control_input_changed_list = std::bitset<info::control_in_count>;
  control_input_changed_list control_input_changed;

  // holds the std::tuple<timed_vec<float>, ...>
  using control_input_timed_t = typename ossia::apply_type<control_input_values_type, ossia::safe_nodes::timed_vec>::type;
  control_input_timed_t control_input_timed;


  // std::tuple<float, int...> : current running values of the controls
  using control_output_values_type = typename info::control_output_values_type;
  control_output_values_type control_output;

  // bitset : 1 if the control has changed since the last tick, 0 else
  using control_output_changed_list = std::bitset<info::control_in_count>;
  control_output_changed_list control_output_changed;

  // holds the std::tuple<timed_vec<float>, ...>
  using control_output_timed_t = typename ossia::apply_type<control_output_values_type, ossia::safe_nodes::timed_vec>::type;
  control_output_timed_t control_output_timed;


  // used to communicate control changes from / to the ui
  ossia::spsc_queue<control_input_values_type> control_ins_queue;
  ossia::spsc_queue<control_output_values_type> control_outs_queue;

  int sampleRate{}, bufferSize{};

  safe_node(ossia::exec_state_facade st) noexcept
      : sampleRate{st.sampleRate()}
      , bufferSize{st.bufferSize()}
  {
    m_inlets.reserve(info::inlet_size);
    m_outlets.reserve(info::outlet_size);

    InitPorts<safe_node> port_init_func{*this, this->m_inlets, this->m_outlets};
    if constexpr(requires { state.inputs; })
    {
      ossia::for_each_in_tuples(boost::pfr::detail::tie_as_tuple(state.inputs), this->input_ports, port_init_func);
    }
    if constexpr(requires { state.outputs; })
    {
      ossia::for_each_in_tuples(boost::pfr::detail::tie_as_tuple(state.outputs), this->output_ports, port_init_func);
    }
  }

  template <typename T, std::size_t ControlN>
  void control_updated_from_ui(T&& v)
  {
    if constexpr(std::is_same_v<T, ossia::impulse>)
    {
      std::get<ControlN>(this->control_input_timed).emplace(int64_t{0}, std::move(v));
    }
    else
    {
      std::get<ControlN>(this->control_input) = std::move(v);
    }
  }

  template <std::size_t ControlN>
  constexpr const auto& get_control_accessor() noexcept
  {
    using namespace boost::pfr;
    using namespace std;
    using control_member = std::tuple_element_t<ControlN, typename info::control_input_tuple>;
    using control_type = decltype(control_member::control);
    using control_value_type = typename control_type::type;

    // ControlN = 0: first control in this->controls, this->control_tuple, etc..

    // Used to index into the set of input_ports
    using port_index_t = typename info::template control_input_index<ControlN>;

    // Get the timed_vector<float>
    auto& vec = get<ControlN>(this->control_input_timed);
    // vec.clear();

    // Get the ossia::value_port data
    const auto& vp = get<port_index_t::value>(this->input_ports)->get_data();
    vec.container.reserve(vp.size() + 1);

    if constexpr(std::is_same_v<control_value_type, ossia::impulse>)
    {
      // copy all the values... values arrived later replace previous ones
      load_control_from_port<control_type::must_validate, ControlN, port_index_t::value>(vec, vp);
    }
    else
    {
      // in all cases, set the current value at t=0
      vec.insert(std::make_pair(int64_t{0}, get<ControlN>(this->control_input)));

      // copy all the values... values arrived later replace previous ones
      load_control_from_port<control_type::must_validate, ControlN, port_index_t::value>(vec, vp);

      // the last value will be the first for the next tick
      get<ControlN>(this->control_input) = vec.rbegin()->second;
    }

    return vec;
  }

  template <std::size_t ControlN>
  constexpr auto& get_control_outlet_accessor (const ossia::outlets& outl) noexcept
  {
    static_assert(info::control_out_count > 0);
    static_assert(ControlN < info::control_out_count);

    return std::get<ControlN>(this->control_outs_tuple);
  }

  template <bool Validate, std::size_t ControlN, std::size_t PortN, typename Vec, typename Vp>
  void load_control_from_port(Vec& vec, const Vp& vp) noexcept
  {
    using namespace boost::pfr;
    constexpr const auto ctrl = std::remove_reference_t<decltype(get<PortN>(state.inputs))>::control;
    if constexpr(Validate)
    {
      for (auto& v : vp)
      {
        if (auto res = ctrl.fromValue(v.value))
        {
          vec[int64_t{v.timestamp}] = *std::move(res);
          this->control_input_changed.set(ControlN);
        }
      }
    }
    else
    {
      for (auto& v : vp)
      {
        vec[int64_t{v.timestamp}] = ctrl.fromValue(v.value);
        this->control_input_changed.set(ControlN);
      }
    }
  }

  // copies all the controls to the state class
  template<typename... Controls, std::size_t... CI>
  void apply_controls_impl(const std::index_sequence<CI...>&, Controls&&... ctls)
  {
    using namespace boost::pfr;
    ((get<info::template control_input_index<CI>::value...>(state.inputs).value = ctls),
     ...);
  }

  constexpr auto get_policy() noexcept {

    if constexpr(requires { typename Node_T::control_policy {}; }) {
      return typename Node_T::control_policy{};
    }
    else {
      return typename ossia::safe_nodes::default_tick{};
    }
  }

  template <std::size_t... CI>
  void apply_all_impl(
      const std::index_sequence<CI...>&,
      ossia::token_request tk,
      ossia::exec_state_facade st) noexcept
  {
    static_assert(info::control_in_count > 0);

    get_policy()([&] (const ossia::token_request& sub_tk, auto&& ... ctls) {
                   apply_controls_impl(std::make_index_sequence<sizeof... (ctls)>{}, ctls...);
                   state.run(sub_tk, st);
                 }, tk, get_control_accessor<CI>()...);
  }

  void clear_controls_in()
  {
    ossia::for_each_in_tuple(control_input_timed, [] (auto& vec) { vec.clear(); });
  }

  void clear_controls_out()
  {
    ossia::for_each_in_tuple(control_output_timed, [] (auto& vec) { vec.clear(); });
  }

  void
  run(const ossia::token_request& tk, ossia::exec_state_facade st) noexcept override
  {
    if constexpr(info::control_out_count > 0)
    {
      clear_controls_out();
    }

    // If there are no controls
    if constexpr (info::control_in_count == 0)
    {
      state.run(tk, st);
    }
    else
    {
      using controls_indices = std::make_index_sequence<info::control_in_count>;

      apply_all_impl(controls_indices{}, tk, st);

      if (control_ins_queue.size_approx() < 1 && control_input_changed.any())
      {
        control_ins_queue.try_enqueue(control_input);
        control_input_changed.reset();
      }

      clear_controls_in();
    }

    if constexpr(info::control_out_count > 0)
    {
      std::size_t i = 0;
      bool ok = false;

      ossia::for_each_in_tuples_ref(
          this->control_output_timed,
          this->control_output,
          [&] (auto&& control_in, auto&& control_out) {
            if(!control_in.empty())
            {
              ok = true;
              control_out = std::move(control_in.container.back().second);
              control_in.clear();
            }

            i++;
      });

      if(ok)
      {
        this->control_outs_queue.enqueue(this->control_output);
      }
    }
  }

  void all_notes_off() noexcept override
  {
    if constexpr (info::midi_in_count > 0)
    {
      // TODO
    }
  }

  std::string label() const noexcept override
  {
    return "Control";
  }
};

/*
struct value_adder
{
  ossia::value_port& port;
  ossia::value v;
  void operator()()
  {
    // timestamp should be > all others so that it is always active ?
    port.write_value(std::move(v), 0);
  }
};

template <typename T>
struct control_updater
{
  T& control;
  T v;
  void operator()()
  {
    control = std::move(v);
  }
};
*/
}
