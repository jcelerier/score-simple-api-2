#pragma once
#include <boost/pfr.hpp>
#include <SimpleApi2/Concepts.hpp>
#include <SimpleApi2/Metadatas.hpp>

#include <Process/ProcessMetadata.hpp>
#include <Process/Process.hpp>
#include <Process/ProcessFactory.hpp>
#include <Process/GenericProcessFactory.hpp>
#include <Process/Dataflow/Port.hpp>
#include <Process/Dataflow/PortFactory.hpp>
#include <score_plugin_engine.hpp>
#include <ossia/detail/typelist.hpp>

/**
 * This file instantiates the classes that are provided by this plug-in.
 */

////////// METADATA ////////////
namespace SimpleApi2
{
struct is_control
{
};
template <typename Info, typename = is_control>
class ProcessModel;
}
template <typename Info>
struct Metadata<PrettyName_k, SimpleApi2::ProcessModel<Info>>
{
  static Q_DECL_RELAXED_CONSTEXPR auto get()
  {
    return Info::Metadata::prettyName;
  }
};
template <typename Info>
struct Metadata<Category_k, SimpleApi2::ProcessModel<Info>>
{
  static Q_DECL_RELAXED_CONSTEXPR auto get()
  {
    return Info::Metadata::category;
  }
};
template <typename Info>
struct Metadata<Tags_k, SimpleApi2::ProcessModel<Info>>
{
  static QStringList get()
  {
    QStringList lst;
    for (auto str : Info::Metadata::tags)
      lst.append(str);
    return lst;
  }
};

template <typename Info>
struct Metadata<Process::Descriptor_k, SimpleApi2::ProcessModel<Info>>
{
  using info = SimpleApi2::info_functions_2<Info>;
  static std::vector<Process::PortType> inletDescription()
  {
    std::vector<Process::PortType> port;
    for (std::size_t i = 0; i < info::audio_in_count; i++)
      port.push_back(Process::PortType::Audio);
    for (std::size_t i = 0; i < info::midi_in_count; i++)
      port.push_back(Process::PortType::Midi);
    for (std::size_t i = 0; i < info::value_in_count; i++)
      port.push_back(Process::PortType::Message);
    for (std::size_t i = 0; i < info::control_in_count; i++)
      port.push_back(Process::PortType::Message);
    return port;
  }
  static std::vector<Process::PortType> outletDescription()
  {
    std::vector<Process::PortType> port;
    for (std::size_t i = 0; i < info::audio_out_count; i++)
      port.push_back(Process::PortType::Audio);
    for (std::size_t i = 0; i < info::midi_out_count; i++)
      port.push_back(Process::PortType::Midi);
    for (std::size_t i = 0; i < info::value_out_count; i++)
      port.push_back(Process::PortType::Message);
    for (std::size_t i = 0; i < info::control_out_count; i++)
      port.push_back(Process::PortType::Message);
    return port;
  }
  static Process::Descriptor get()
  {
    static Process::Descriptor desc{
        Info::Metadata::prettyName,
        Info::Metadata::kind,
        Info::Metadata::category,
        Info::Metadata::description,
        Info::Metadata::author,
        Metadata<Tags_k, SimpleApi2::ProcessModel<Info>>::get(),
        inletDescription(),
        outletDescription()};
    return desc;
  }
};
template <typename Info>
struct Metadata<Process::ProcessFlags_k, SimpleApi2::ProcessModel<Info>>
{
  static Process::ProcessFlags get() noexcept { return Info::Metadata::flags; }
};
template <typename Info>
struct Metadata<ObjectKey_k, SimpleApi2::ProcessModel<Info>>
{
  static Q_DECL_RELAXED_CONSTEXPR auto get() noexcept
  {
    return Info::Metadata::objectKey;
  }
};
template <typename Info>
struct Metadata<ConcreteKey_k, SimpleApi2::ProcessModel<Info>>
{
  static Q_DECL_RELAXED_CONSTEXPR UuidKey<Process::ProcessModel> get()
  {
    return Info::Metadata::uuid;
  }
};

namespace SimpleApi2
{

struct PortInitFunc
{
  Process::ProcessModel& self;
  Process::Inlets& ins;
  Process::Outlets& outs;
  int inlet = 0;
  int outlet = 0;
  void operator()(const AudioInput auto& in)
  {
    auto p = new Process::AudioInlet(Id<Process::Port>(inlet++), &self);
    p->setName(QString::fromUtf8(in.name));
    ins.push_back(p);
  }
  void operator()(const AudioOutput auto& out)
  {
    auto p = new Process::AudioOutlet(Id<Process::Port>(outlet++), &self);
    p->setName(QString::fromUtf8(out.name));
    if (outlet == 1)
      p->setPropagate(true);
    outs.push_back(p);
  }
  void operator()(const ValueInput auto& in)
  {
    auto p = new Process::ValueInlet(Id<Process::Port>(inlet++), &self);
    p->setName(QString::fromUtf8(in.name));
    ins.push_back(p);
  }
  void operator()(const ValueOutput auto& out)
  {
    auto p = new Process::ValueOutlet(Id<Process::Port>(outlet++), &self);
    p->setName(QString::fromUtf8(out.name));
    outs.push_back(p);
  }
  void operator()(const MidiInput auto& in)
  {
    auto p = new Process::MidiInlet(Id<Process::Port>(inlet++), &self);
    p->setName(QString::fromUtf8(in.name));
    ins.push_back(p);
  }
  void operator()(const MidiOutput auto& out)
  {
    auto p = new Process::MidiOutlet(Id<Process::Port>(outlet++), &self);
    p->setName(QString::fromUtf8(out.name));
    outs.push_back(p);
  }
  void operator()(const ControlInput auto& ctrl)
  {
    if (auto p = ctrl.control.create_inlet(Id<Process::Port>(inlet++), &self))
    {
      p->hidden = true;
      ins.push_back(p);
    }
  }
  void operator()(const ControlOutput auto& ctrl)
  {
    if (auto p = ctrl.control.create_outlet(Id<Process::Port>(outlet++), &self))
    {
      p->hidden = true;
      outs.push_back(p);
    }
  }
};

template<typename Node, typename Func>
void for_each_port(Node& node, Func&& func)
{
  if constexpr(requires { node.audio_inputs; })
  boost::pfr::for_each_field_ref(node.audio_inputs, func);
  if constexpr(requires { node.midi_inputs; })
  boost::pfr::for_each_field_ref(node.midi_inputs, func);
  if constexpr(requires { node.value_inputs; })
  boost::pfr::for_each_field_ref(node.value_inputs, func);
  if constexpr(requires { node.control_inputs; })
  boost::pfr::for_each_field_ref(node.control_inputs, func);

  if constexpr(requires { node.audio_outputs; })
  boost::pfr::for_each_field_ref(node.audio_outputs, func);
  if constexpr(requires { node.midi_outputs; })
  boost::pfr::for_each_field_ref(node.midi_outputs, func);
  if constexpr(requires { node.value_outputs; })
  boost::pfr::for_each_field_ref(node.value_outputs, func);
  if constexpr(requires { node.control_outputs; })
  boost::pfr::for_each_field_ref(node.control_outputs, func);
}

struct PortSetup
{
  template <typename Node_T, typename T>
  static void load(DataStream::Deserializer& s, T& self)
  {
    Node_T node; // TODO

    auto& ins = self.m_inlets;
    auto& outs = self.m_outlets;
    for (std::size_t k = 0; k < info_functions_2<Node_T>::audio_in_count; k++)
    {
      ins.push_back(
          deserialize_known_interface<Process::AudioInlet>(s, &self));
    }
    for (std::size_t k = 0; k < info_functions_2<Node_T>::midi_in_count; k++)
    {
      ins.push_back(deserialize_known_interface<Process::MidiInlet>(s, &self));
    }
    for (std::size_t k = 0; k < info_functions_2<Node_T>::value_in_count; k++)
    {
      ins.push_back(
          deserialize_known_interface<Process::ValueInlet>(s, &self));
    }
    boost::pfr::for_each_field_ref(node.control_inputs, [&] (const auto& ctrl) {
                                     if (auto p = ctrl.control.create_inlet(s, &self))
                                     {
                                       p->hidden = true;
                                       ins.push_back(p);
                                     }
                                   });

    for (std::size_t k = 0; k < info_functions_2<Node_T>::audio_out_count; k++)
    {
      outs.push_back(
          deserialize_known_interface<Process::AudioOutlet>(s, &self));
    }
    for (std::size_t k = 0; k < info_functions_2<Node_T>::midi_out_count; k++)
    {
      outs.push_back(
          deserialize_known_interface<Process::MidiOutlet>(s, &self));
    }
    for (std::size_t k = 0; k < info_functions_2<Node_T>::value_out_count; k++)
    {
      outs.push_back(
          deserialize_known_interface<Process::ValueOutlet>(s, &self));
    }
    boost::pfr::for_each_field_ref(node.control_outputs, [&] (const auto& ctrl) {
                                     if (auto p = ctrl.control.create_outlet(s, &self))
                                     {
                                       p->hidden = true;
                                       outs.push_back(p);
                                     }
                                   });
  }

  template <typename Node_T, typename T>
  static void load(
      const rapidjson::Value::ConstArray& inlets,
      const rapidjson::Value::ConstArray& outlets,
      T& self)
  {
    Node_T node; // TODO

    auto& ins = self.m_inlets;
    auto& outs = self.m_outlets;
    std::size_t inlet = 0;
    for (std::size_t k = 0; k < info_functions_2<Node_T>::audio_in_count; k++)
    {
      ins.push_back(deserialize_known_interface<Process::AudioInlet>(
          JSONWriter{inlets[inlet++]}, &self));
    }
    for (std::size_t k = 0; k < info_functions_2<Node_T>::midi_in_count; k++)
    {
      ins.push_back(deserialize_known_interface<Process::MidiInlet>(
          JSONWriter{inlets[inlet++]}, &self));
    }
    for (std::size_t k = 0; k < info_functions_2<Node_T>::value_in_count; k++)
    {
      ins.push_back(deserialize_known_interface<Process::ValueInlet>(
          JSONWriter{inlets[inlet++]}, &self));
    }

    boost::pfr::for_each_field_ref(node.control_inputs, [&] (const auto& ctrl) {
                                     if (auto p = ctrl.control.create_inlet(JSONWriter{inlets[inlet++]}, &self))
                                     {
                                       p->hidden = true;
                                       ins.push_back(p);
                                     }
                                   });
    int outlet = 0;
    for (std::size_t k = 0; k < info_functions_2<Node_T>::audio_out_count; k++)
    {
      outs.push_back(deserialize_known_interface<Process::AudioOutlet>(
          JSONWriter{outlets[outlet++]}, &self));
    }
    for (std::size_t k = 0; k < info_functions_2<Node_T>::midi_out_count; k++)
    {
      outs.push_back(deserialize_known_interface<Process::MidiOutlet>(
          JSONWriter{outlets[outlet++]}, &self));
    }
    for (std::size_t k = 0; k < info_functions_2<Node_T>::value_out_count; k++)
    {
      outs.push_back(deserialize_known_interface<Process::ValueOutlet>(
          JSONWriter{outlets[outlet++]}, &self));
    }
    boost::pfr::for_each_field_ref(node.control_outputs, [&] (const auto& ctrl) {
                                     if (auto p = ctrl.control.create_outlet(JSONWriter{outlets[outlet++]}, &self))
                                     {
                                       p->hidden = true;
                                       outs.push_back(p);
                                     }
                                   });
  }
};

template <typename Info, typename>
class ProcessModel final : public Process::ProcessModel
{
  SCORE_SERIALIZE_FRIENDS
  PROCESS_METADATA_IMPL(ProcessModel<Info>)
  friend struct TSerializer<DataStream, SimpleApi2::ProcessModel<Info>>;
  friend struct TSerializer<JSONObject, SimpleApi2::ProcessModel<Info>>;
  friend struct SimpleApi2::PortSetup;

public:
  ossia::value control(std::size_t i) const
  {
    static_assert(SimpleApi2::info_functions_2<Info>::control_in_count != 0);
    constexpr auto start
        = SimpleApi2::info_functions_2<Info>::control_start;

    return static_cast<Process::ControlInlet*>(m_inlets[start + i])->value();
  }

  void setControl(std::size_t i, ossia::value v)
  {
    static_assert(SimpleApi2::info_functions_2<Info>::control_in_count != 0);
    constexpr auto start
        = SimpleApi2::info_functions_2<Info>::control_start;

    static_cast<Process::ControlInlet*>(m_inlets[start + i])
        ->setValue(std::move(v));
  }

  ossia::value controlOut(std::size_t i) const
  {
    static_assert(
        SimpleApi2::info_functions_2<Info>::control_out_count != 0);
    constexpr auto start
        = SimpleApi2::info_functions_2<Info>::control_out_start;

    return static_cast<Process::ControlOutlet*>(m_outlets[start + i])->value();
  }

  void setControlOut(std::size_t i, ossia::value v)
  {
    static_assert(
        SimpleApi2::info_functions_2<Info>::control_out_count != 0);
    constexpr auto start
        = SimpleApi2::info_functions_2<Info>::control_out_start;

    static_cast<Process::ControlOutlet*>(m_outlets[start + i])
        ->setValue(std::move(v));
  }

  ProcessModel(
      const TimeVal& duration,
      const Id<Process::ProcessModel>& id,
      QObject* parent)
      : Process::ProcessModel{
          duration,
          id,
          Metadata<ObjectKey_k, ProcessModel>::get(),
          parent}
  {
    metadata().setInstanceName(*this);

    Info node; // TODO

    for_each_port(node, PortInitFunc{*this, m_inlets, m_outlets});
  }

  template <typename Impl>
  explicit ProcessModel(Impl& vis, QObject* parent)
      : Process::ProcessModel{vis, parent}
  {
    vis.writeTo(*this);
  }

  ~ProcessModel() override { }
};
}

template <typename Info>
struct is_custom_serialized<SimpleApi2::ProcessModel<Info>> : std::true_type
{
};

template <template <typename, typename> class Model, typename Info>
struct TSerializer<DataStream, Model<Info, SimpleApi2::is_control>>
{
  using model_type = Model<Info, SimpleApi2::is_control>;
  static void readFrom(DataStream::Serializer& s, const model_type& obj)
  {
    using namespace Control;
    for (auto obj : obj.inlets())
    {
      s.stream() << *obj;
    }

    for (auto obj : obj.outlets())
    {
      s.stream() << *obj;
    }
    s.insertDelimiter();
  }

  static void writeTo(DataStream::Deserializer& s, model_type& obj)
  {
    using namespace Control;

    SimpleApi2::PortSetup::load<Info>(s, obj);

    s.checkDelimiter();
  }
};

template <template <typename, typename> class Model, typename Info>
struct TSerializer<JSONObject, Model<Info, SimpleApi2::is_control>>
{
  using model_type = Model<Info, SimpleApi2::is_control>;
  static void readFrom(JSONObject::Serializer& s, const model_type& obj)
  {
    using namespace Control;

    Process::readPorts(s, obj.inlets(), obj.outlets());

  }

  static void writeTo(JSONObject::Deserializer& s, model_type& obj)
  {
    using namespace Control;

    const auto& inlets = s.obj["Inlets"].toArray();
    const auto& outlets = s.obj["Outlets"].toArray();

    SimpleApi2::PortSetup::load<Info>(inlets, outlets, obj);

  }
};

namespace score
{
template <typename Vis, typename Info>
void serialize_dyn_impl(Vis& v, const SimpleApi2::ProcessModel<Info>& t)
{
  TSerializer<typename Vis::type, SimpleApi2::ProcessModel<Info>>::readFrom(
      v, t);
}
}

