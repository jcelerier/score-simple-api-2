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


template <typename info, std::size_t N>
static constexpr auto& get_inlet_accessor(const ossia::inlets& inl) noexcept
{
  constexpr auto cat = info::categorize_inlet(N);
  if constexpr (cat == ossia::safe_nodes::inlet_kind::audio_in)
    return *inl[N]->target<ossia::audio_port>();
  else if constexpr (cat == ossia::safe_nodes::inlet_kind::midi_in)
    return *inl[N]->target<ossia::midi_port>();
  else if constexpr (cat == ossia::safe_nodes::inlet_kind::value_in)
    return *inl[N]->target<ossia::value_port>();
  else if constexpr (cat == ossia::safe_nodes::inlet_kind::address_in)
    return inl[N]->address;
  else
    throw;
}

template <typename info, std::size_t N>
static constexpr auto& get_outlet_accessor(const ossia::outlets& outl) noexcept
{
  constexpr auto cat = info::categorize_outlet(N);
  if constexpr (cat == ossia::safe_nodes::outlet_kind::audio_out)
    return *outl[N]->target<ossia::audio_port>();
  else if constexpr (cat == ossia::safe_nodes::outlet_kind::midi_out)
    return *outl[N]->target<ossia::midi_port>();
  else if constexpr (cat == ossia::safe_nodes::outlet_kind::value_out)
    return *outl[N]->target<ossia::value_port>();
  else
    throw;
}

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
  int ctrl_i = 0;

  void operator()(AudioInput auto& in, ossia::audio_inlet& port)
  {
    inlets.push_back(std::addressof(port));

    in.port = std::addressof(*port);
  }

  void operator()(AudioOutput auto& out, ossia::audio_outlet& port)
  {
    outlets.push_back(std::addressof(port));

    out.port = std::addressof(*port);
  }

  void operator()(ValueInput auto& in, ossia::value_inlet& port)
  {
    inlets.push_back(std::addressof(port));

    if constexpr(requires { in.is_event; }) {
      port->is_event = in.is_event;
    }
    in.port = std::addressof(*port);
  }

  void operator()(ValueOutput auto& out, ossia::value_outlet& port)
  {
    outlets.push_back(std::addressof(port));

    out.port = std::addressof(*port);

    if constexpr(requires { out.type; }) {
      if(!out.type.empty())
        port->type = ossia::parse_dataspace(out.type);
    }
  }

  void operator()(MidiInput auto& in, ossia::midi_inlet& port)
  {
    inlets.push_back(std::addressof(port));

    in.port = std::addressof(*port);
  }

  void operator()(MidiOutput auto& out, ossia::midi_outlet& port)
  {
    outlets.push_back(std::addressof(port));

    out.port = std::addressof(*port);
  }

  void operator()(ControlInput auto& ctrl, ossia::value_inlet& port)
  {
    inlets.push_back(std::addressof(port));

    port->is_event = true;
    //ctrl.port = std::addressof(*port);

    ctrl.control.setup_exec(port);
    ctrl_i++;
  }

  void operator()(ControlOutput auto& ctrl, ossia::value_outlet& port)
  {
    outlets.push_back(std::addressof(port));

    ctrl.port = std::addressof(*port);
  }
};

/*
template<typename T, typename = void> struct to_ossia_port;
template<AudioInput T> struct to_ossia_port<T, void>  { using type = ossia::audio_inlet; };
template<AudioOutput T> struct to_ossia_port<T, void> { using type = ossia::audio_outlet; };
template<MidiInput T> struct to_ossia_port<T, void>   { using type = ossia::midi_inlet; };
template<MidiOutput T> struct to_ossia_port<T, void>  { using type = ossia::midi_outlet; };
template<ValueInput T> struct to_ossia_port<T, void>   { using type = ossia::value_inlet; };
template<ValueOutput T> struct to_ossia_port<T, void>  { using type = ossia::value_outlet; };
template<ControlInput T> struct to_ossia_port<T, void>   { using type = ossia::value_inlet; };
template<ControlOutput T> struct to_ossia_port<T, void>  { using type = ossia::value_outlet; };

template<typename T>
using convert_to_ossia_port = typename to_ossia_port<std::remove_reference_t<T>>::type;
template<typename PortSpecTuple>
using ossia_port_tuple = boost::mp11::mp_transform<convert_to_ossia_port, PortSpecTuple>;

*/
template <typename Node_T>
class safe_node final : public ossia::nonowning_graph_node
{
public:
  Node_T state;

  using info = info_functions_2<Node_T>;


  using controls_changed_list = std::bitset<info::control_in_count>;
  using controls_type = typename info::controls_type;
  using controls_values_type = typename info::controls_values_type;
  using control_tuple_t = typename ossia::apply_type<controls_values_type, ossia::safe_nodes::timed_vec>::type;

  // std::tuple<float, int...> : current running values of the controls
  controls_values_type controls;

  // bitset : 1 if the control has changed since the last tick, 0 else
  controls_changed_list controls_changed;


  // holds the std::tuple<timed_vec<float>, ...>
  control_tuple_t control_tuple;



  using control_outs_changed_list = std::bitset<info::control_out_count>;
  using control_outs_type = typename info::control_outs_type;
  using control_outs_values_type = typename info::control_outs_values_type;
  using control_out_tuple_t = typename ossia::apply_type<control_outs_values_type, ossia::safe_nodes::timed_vec>::type;


  control_outs_values_type control_outs;
  control_outs_changed_list control_outs_changed;


  control_out_tuple_t control_outs_tuple;



  // used to communicate control changes from / to the ui
  ossia::spsc_queue<controls_values_type> cqueue;
  ossia::spsc_queue<control_outs_values_type> control_outs_queue;

  std::array<ossia::audio_inlet, info::audio_in_count> audio_in_ports;
  std::array<ossia::midi_inlet, info::midi_in_count> midi_in_ports;
  std::array<ossia::value_inlet, info::value_in_count> value_in_ports;
  std::array<ossia::value_inlet, info::control_in_count> control_in_ports;

  std::array<ossia::audio_outlet, info::audio_out_count> audio_out_ports;
  std::array<ossia::midi_outlet, info::midi_out_count> midi_out_ports;
  std::array<ossia::value_outlet, info::value_out_count> value_out_ports;
  std::array<ossia::value_outlet, info::control_out_count> control_out_ports;

  safe_node() noexcept
  {
    m_inlets.reserve(info::inlet_size);
    m_outlets.reserve(info::outlet_size);

    InitPorts<safe_node> port_init_func{*this, this->m_inlets, this->m_outlets};
    if constexpr(requires { state.audio_inputs; })
    ossia::tuple_array_func(boost::pfr::detail::tie_as_tuple(state.audio_inputs), this->audio_in_ports, port_init_func);
    if constexpr(requires { state.midi_inputs; })
    ossia::tuple_array_func(boost::pfr::detail::tie_as_tuple(state.midi_inputs), this->midi_in_ports, port_init_func);
    if constexpr(requires { state.value_inputs; })
    ossia::tuple_array_func(boost::pfr::detail::tie_as_tuple(state.value_inputs), this->value_in_ports, port_init_func);
    if constexpr(requires { state.control_inputs; })
    ossia::tuple_array_func(boost::pfr::detail::tie_as_tuple(state.control_inputs), this->control_in_ports, port_init_func);

    if constexpr(requires { state.audio_outputs; })
    ossia::tuple_array_func(boost::pfr::detail::tie_as_tuple(state.audio_outputs), this->audio_out_ports, port_init_func);
    if constexpr(requires { state.midi_outputs; })
    ossia::tuple_array_func(boost::pfr::detail::tie_as_tuple(state.midi_outputs), this->midi_out_ports, port_init_func);
    if constexpr(requires { state.value_outputs; })
    ossia::tuple_array_func(boost::pfr::detail::tie_as_tuple(state.value_outputs), this->value_out_ports, port_init_func);
    if constexpr(requires { state.control_outputs; })
    ossia::tuple_array_func(boost::pfr::detail::tie_as_tuple(state.control_outputs), this->control_out_ports, port_init_func);
  }

  template <std::size_t N>
  constexpr const auto& get_control_accessor() noexcept
  {
    using namespace boost::pfr;
    using namespace std;
    using control_member = boost::pfr::tuple_element_t<N, decltype(Node_T::control_inputs)>;
    using control_type = decltype(control_member::control);

    auto& vec = get<N>(this->control_tuple);
    vec.clear();

    const auto& vp = this->control_in_ports[N]->get_data();
    vec.container.reserve(vp.size() + 1);

    // in all cases, set the current value at t=0
    vec.insert(std::make_pair(int64_t{0}, get<N>(this->controls)));

    // copy all the values... values arrived later replace previous ones
    apply_control<control_type::must_validate, N>(vec, vp);

    // the last value will be the first for the next tick
    get<N>(this->controls) = vec.rbegin()->second;
    return vec;
  }

  template <std::size_t N>
  constexpr auto& get_control_outlet_accessor (const ossia::outlets& outl) noexcept
  {
    static_assert(info::control_out_count > 0);
    static_assert(N < info::control_out_count);

    return std::get<N>(this->control_outs_tuple);
  }

  template <bool Validate, std::size_t N, typename Vec, typename Vp>
  void apply_control(Vec& vec, const Vp& vp) noexcept
  {
    using namespace boost::pfr;
    constexpr const auto ctrl = std::remove_reference_t<decltype(get<N>(state.control_inputs))>::control;
    if constexpr(Validate)
    {
      for (auto& v : vp)
      {
        if (auto res = ctrl.fromValue(v.value))
        {
          vec[int64_t{v.timestamp}] = *std::move(res);
          this->controls_changed.set(N);
        }
      }
    }
    else
    {
      for (auto& v : vp)
      {
        vec[int64_t{v.timestamp}] = ctrl.fromValue(v.value);
        this->controls_changed.set(N);
      }
    }
  }

  template<typename... Controls, std::size_t... CI>
  void apply_controls_impl(const std::index_sequence<CI...>&, Controls&&... ctls)
  {
    // copies all the controls to the state classes
    using namespace boost::pfr;
    ((get<CI...>(state.control_inputs).value = ctls), ...);
  }


  template<typename... Controls>
  void apply_controls(Controls&&... ctls)
  {
    // copies all the controls to the state class
    apply_controls_impl(std::make_index_sequence<sizeof... (ctls)>{}, ctls...);
  }

  template <std::size_t... CI>
  void
  apply_all_impl(
      const std::index_sequence<CI...>&,
      ossia::token_request tk,
      ossia::exec_state_facade st) noexcept
  {
    static_assert(info::control_in_count > 0);
    using policy = typename Node_T::control_policy;
    policy{}([&] (const ossia::token_request& sub_tk, auto&& ... ctls) {
               apply_controls(ctls...);
               state.run(sub_tk, st);
    }, tk, get_control_accessor<CI>()...);
  }



  void
  run(const ossia::token_request& tk, ossia::exec_state_facade st) noexcept override
  {
    // If there are no controls
    if constexpr (info::control_in_count == 0)
    {
      state.run(tk, st);
    }
    else
    {
      using controls_indices = std::make_index_sequence<info::control_in_count>;

      apply_all_impl(controls_indices{}, tk, st);

      if (cqueue.size_approx() < 1 && controls_changed.any())
      {
        cqueue.try_enqueue(controls);
        controls_changed.reset();
      }
    }

    if constexpr(info::control_out_count > 0)
    {
      std::size_t i = 0;
      bool ok = false;

      ossia::for_each_in_tuples_ref(this->control_outs_tuple, this->control_outs, [&] (auto&& control_in, auto&& control_out) {
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
        this->control_outs_queue.enqueue(this->control_outs);
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
}
