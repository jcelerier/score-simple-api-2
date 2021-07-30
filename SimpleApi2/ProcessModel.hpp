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
    /*
    for (std::size_t i = 0; i < info::audio_in_count; i++)
      port.push_back(Process::PortType::Audio);
    for (std::size_t i = 0; i < info::midi_in_count; i++)
      port.push_back(Process::PortType::Midi);
    for (std::size_t i = 0; i < info::value_in_count; i++)
      port.push_back(Process::PortType::Message);
    for (std::size_t i = 0; i < info::control_in_count; i++)
      port.push_back(Process::PortType::Message);
    */
    return port;
  }
  static std::vector<Process::PortType> outletDescription()
  {
    std::vector<Process::PortType> port;
    /*
    for (std::size_t i = 0; i < info::audio_out_count; i++)
      port.push_back(Process::PortType::Audio);
    for (std::size_t i = 0; i < info::midi_out_count; i++)
      port.push_back(Process::PortType::Midi);
    for (std::size_t i = 0; i < info::value_out_count; i++)
      port.push_back(Process::PortType::Message);
    for (std::size_t i = 0; i < info::control_out_count; i++)
      port.push_back(Process::PortType::Message);
    */
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
  static Process::ProcessFlags get() noexcept {
    if constexpr(requires { Info::Metadata::flags; }) {
      return Info::Metadata::flags;
    } else {
      return Process::ProcessFlags(Process::ProcessFlags::SupportsLasting | Process::ProcessFlags::ControlSurface);
    }
  }
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
struct PortLoadFunc
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
  if constexpr(requires { node.inputs; })
    boost::pfr::for_each_field_ref(node.inputs, func);
  if constexpr(requires { node.outputs; })
    boost::pfr::for_each_field_ref(node.outputs, func);
}

template <typename Info, typename>
class ProcessModel final : public Process::ProcessModel
{
  SCORE_SERIALIZE_FRIENDS
  PROCESS_METADATA_IMPL(ProcessModel<Info>)
  friend struct TSerializer<DataStream, SimpleApi2::ProcessModel<Info>>;
  friend struct TSerializer<JSONObject, SimpleApi2::ProcessModel<Info>>;

public:
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
    Process::readPorts(s, obj.m_inlets, obj.m_outlets);
    s.insertDelimiter();
  }

  static void writeTo(DataStream::Deserializer& s, model_type& obj)
  {
    Process::writePorts(s, s.components.interfaces<Process::PortFactoryList>(), obj.m_inlets, obj.m_outlets, &obj);
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

    Process::readPorts(s, obj.m_inlets, obj.m_outlets);
  }

  static void writeTo(JSONObject::Deserializer& s, model_type& obj)
  {
    using namespace Control;

    Process::writePorts(s, s.components.interfaces<Process::PortFactoryList>(), obj.m_inlets, obj.m_outlets, &obj);
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

