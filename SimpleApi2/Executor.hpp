#pragma once
#include <SimpleApi2/ProcessModel.hpp>
#include <SimpleApi2/ExecutorNode.hpp>

#include <Engine/Node/TickPolicy.hpp>
#include <Explorer/DeviceList.hpp>
#include <Explorer/DocumentPlugin/DeviceDocumentPlugin.hpp>
#include <Process/Execution/ProcessComponent.hpp>
#include <Process/ExecutionContext.hpp>

#include <score/tools/Bind.hpp>

#include <QTimer>

#include <Scenario/Execution/score2OSSIA.hpp>

namespace SimpleApi2
{

template <typename Node_T, typename T, std::size_t ControlN>
struct control_updater
{
  std::weak_ptr<Node_T> weak_node;
  T v;
  void operator()()
  {
    if(auto n = weak_node.lock())
    {
      n->template control_updated_from_ui<T, ControlN>(std::move(v));
    }
  }
};

template <typename Info_T, typename Node_T, typename Element>
struct setup_Impl0
{
  Element& element;
  const Execution::Context& ctx;
  const std::shared_ptr<Node_T>& node_ptr;
  QObject* parent;

  template <typename ControlIndexT>
  struct con_validated
  {
    const Execution::Context& ctx;
    std::weak_ptr<Node_T> weak_node;
    void operator()(const ossia::value& val)
    {
      using namespace boost::pfr;
      using namespace ossia::safe_nodes;
      using Info = Info_T;
      constexpr auto control_index = ControlIndexT::value;
      using port_index_t = typename info_functions_2<Info>::template control_input_index<ControlIndexT::value>;

      constexpr const auto control_spec = tuple_element_t<port_index_t::value, decltype(Node_T::state.inputs)>::control();
      using control_type = decltype(control_spec);

      using control_value_type = typename control_type::type;

      if (auto node = weak_node.lock())
      {
        if (auto v = control_spec.fromValue(val))
          ctx.executionQueue.enqueue(control_updater<Node_T, control_value_type, control_index>{weak_node, std::move(*v)});
      }
    }
  };

  template <typename ControlIndexT>
  struct con_unvalidated
  {
    const Execution::Context& ctx;
    std::weak_ptr<Node_T> weak_node;
    void operator()(const ossia::value& val)
    {
      using namespace boost::pfr;
      using namespace ossia::safe_nodes;
      using Info = Info_T;
      constexpr auto control_index = ControlIndexT::value;
      using port_index_t = typename info_functions_2<Info>::template control_input_index<ControlIndexT::value>;

      constexpr const auto control_spec = tuple_element_t<port_index_t::value, decltype(Node_T::state.inputs)>::control();
      using control_type = decltype(control_spec);

      using control_value_type = typename control_type::type;

      if (auto node = weak_node.lock())
      {
        ctx.executionQueue.enqueue(control_updater<Node_T, control_value_type, control_index>{weak_node, control_spec.fromValue(val)});
      }
    }
  };

  template <typename ControlIndexT>
  constexpr void operator()(ControlIndexT)
  {
    using namespace boost::pfr;
    using namespace ossia::safe_nodes;
    using Info = Info_T;
    constexpr int control_index = ControlIndexT::value;
    using port_index_t = typename info_functions_2<Info>::template control_input_index<ControlIndexT::value>;

    constexpr const auto control_spec = tuple_element_t<port_index_t::value, decltype(Node_T::state.inputs)>::control();
    using control_type = decltype(control_spec);

    auto inlet = static_cast<Process::ControlInlet*>(element.inlets()[port_index_t::value]);

    auto& node = *node_ptr;
    std::weak_ptr<Node_T> weak_node = node_ptr;

    if constexpr (control_type::must_validate)
    {
      if (auto res = control_spec.fromValue(inlet->value()))
        get<control_index>(node.controls) = *res;

      QObject::connect(
          inlet,
          &Process::ControlInlet::valueChanged,
          parent,
          con_validated<ControlIndexT>{ctx, weak_node});
    }
    else
    {
      get<control_index>(node.control_input) = control_spec.fromValue(inlet->value());

      QObject::connect(
          inlet,
          &Process::ControlInlet::valueChanged,
          parent,
          con_unvalidated<ControlIndexT>{ctx, weak_node});
    }
  }
};

template <typename Info, typename Element, typename Node_T>
struct setup_Impl1
{
  typename Node_T::control_input_values_type& arr;
  Element& element;

  template <typename ControlIndexT>
  void operator()(ControlIndexT)
  {
    using namespace boost::pfr;
    using namespace ossia::safe_nodes;

    constexpr int control_index = ControlIndexT::value;
    using port_index_t = typename info_functions_2<Info>::template control_input_index<ControlIndexT::value>;

    constexpr const auto control_spec = tuple_element_t<port_index_t::value, decltype(Node_T::state.inputs)>::control();

    auto inlet = static_cast<Process::ControlInlet*>(element.inlets()[port_index_t::value]);

    inlet->setValue(control_spec.toValue(get<control_index>(arr)));
  }
};

template <typename Info, typename Element, typename Node_T>
struct setup_Impl1_Out
{
  typename Node_T::control_output_values_type& arr;
  Element& element;

  template <typename ControlIndexT>
  void operator()(ControlIndexT)
  {
    using namespace boost::pfr;
    using namespace ossia::safe_nodes;
    constexpr int control_index = ControlIndexT::value;
    using port_index_t = typename info_functions_2<Info>::template control_output_index<ControlIndexT::value>;

    constexpr const auto control_spec = tuple_element_t<port_index_t::value, decltype(Node_T::state.outputs)>::display();

    auto outlet = static_cast<Process::ControlOutlet*>(element.outlets()[port_index_t::value]);

    outlet->setValue(control_spec.toValue(get<control_index>(arr)));
  }
};

template <typename Info, typename Node_T, typename Element_T>
struct ExecutorGuiUpdate
{
  std::weak_ptr<Node_T> weak_node;
  Element_T& element;

  void handle_controls(Node_T& node) const noexcept
  {
    using namespace ossia::safe_nodes;
    // TODO disconnect the connection ? it will be disconnected shortly
    // after...

    typename Node_T::control_input_values_type arr;
    bool ok = false;
    while (node.control_ins_queue.try_dequeue(arr))
    {
      ok = true;
    }
    if (ok)
    {
      constexpr const auto control_count = info_functions_2<Info>::control_in_count;

      ossia::for_each_in_range<control_count>(
          setup_Impl1<Info, Element_T, Node_T>{arr, element});
    }
  }

  void handle_control_outs(Node_T& node) const noexcept
  {
    using namespace ossia::safe_nodes;
    // TODO disconnect the connection ? it will be disconnected shortly
    // after...
    typename Node_T::control_output_values_type arr;
    bool ok = false;
    while (node.control_outs_queue.try_dequeue(arr))
    {
      ok = true;
    }
    if (ok)
    {
      constexpr const auto control_out_count
          = info_functions_2<Info>::control_out_count;

      ossia::for_each_in_range<control_out_count>(
          setup_Impl1_Out<Info, Element_T, Node_T>{arr, element});
    }
  }

  void operator()() const noexcept
  {
    using namespace ossia::safe_nodes;
    if (auto node = weak_node.lock())
    {
      if constexpr (info_functions_2<Info>::control_in_count > 0)
        handle_controls(*node);

      if constexpr (info_functions_2<Info>::control_out_count > 0)
        handle_control_outs(*node);
    }
  }
};

template <typename Info, typename Node_T, typename Element_T>
void setup_node(
    const std::shared_ptr<Node_T>& node_ptr,
    Element_T& element,
    const Execution::Context& ctx,
    QObject* parent)
{
  using namespace ossia::safe_nodes;

  (void)parent;
  if constexpr (info_functions_2<Info>::control_in_count > 0)
  {
    // Initialize all the controls in the node with the current value.
    //
    // And update the node when the UI changes
    ossia::for_each_in_range<info_functions_2<Info>::control_in_count>(
        setup_Impl0<Info, Node_T, Element_T>{element, ctx, node_ptr, parent});
  }

  if constexpr (
      info_functions_2<Info>::control_in_count > 0
      || info_functions_2<Info>::control_out_count > 0)
  {
    // Update the value in the UI
    std::weak_ptr<Node_T> weak_node = node_ptr;
    con(ctx.doc.coarseUpdateTimer,
        &QTimer::timeout,
        parent,
        ExecutorGuiUpdate<Info, Node_T, Element_T>{weak_node, element},
        Qt::QueuedConnection);
  }
}

template <DataflowNode Info>
class Executor final
    : public Execution::
          ProcessComponent_T<ProcessModel<Info>, ossia::node_process>
{
public:
  static Q_DECL_RELAXED_CONSTEXPR UuidKey<score::Component>
  static_key() noexcept
  {
    return Info::uuid();
  }

  UuidKey<score::Component> key() const noexcept final override
  {
    return static_key();
  }

  bool key_match(UuidKey<score::Component> other) const noexcept final override
  {
    return static_key() == other
           || Execution::ProcessComponent::base_key_match(other);
  }

  Executor(
      ProcessModel<Info>& element,
      const ::Execution::Context& ctx,
      QObject* parent)
      : Execution::
          ProcessComponent_T<ProcessModel<Info>, ossia::node_process>{
              element,
              ctx,
              "Executor::ProcessModel<Info>",
              parent}
  {
    auto node = std::make_shared<safe_node<Info>>(ossia::exec_state_facade{ctx.execState.get()});
    this->node = node;
    this->m_ossia_process = std::make_shared<ossia::node_process>(this->node);

    setup_node<Info>(node, element, ctx, this);
  }

  ~Executor() { }
};
}
