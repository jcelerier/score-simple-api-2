#pragma once
#include <boost/pfr.hpp>
#include <oscr/Concepts.hpp>
#include <oscr/Metadatas.hpp>

#include <Process/ProcessMetadata.hpp>
#include <Process/Process.hpp>
#include <Process/ProcessFactory.hpp>
#include <Process/GenericProcessFactory.hpp>
#include <Process/Dataflow/Port.hpp>
#include <Process/Dataflow/PortFactory.hpp>
#include <score_plugin_engine.hpp>
#include <ossia/detail/typelist.hpp>
#include <Gfx/TexturePort.hpp>

/**
 * This file instantiates the classes that are provided by this plug-in.
 */

////////// METADATA ////////////
namespace oscr
{
struct is_control
{
};
template <DataflowNode Info, typename = is_control>
class ProcessModel;
}
template <oscr::DataflowNode Info>
struct Metadata<PrettyName_k, oscr::ProcessModel<Info>>
{
  static Q_DECL_RELAXED_CONSTEXPR auto get()
  {
    return Info::prettyName();
  }
};
template <oscr::DataflowNode Info>
struct Metadata<Category_k, oscr::ProcessModel<Info>>
{
  static Q_DECL_RELAXED_CONSTEXPR auto get()
  {
    return Info::category();
  }
};
template <oscr::DataflowNode Info>
struct Metadata<Tags_k, oscr::ProcessModel<Info>>
{
  static QStringList get()
  {
    QStringList lst;
    if constexpr(requires { Info::Metadata::tags(); })
      for (auto str : Info::Metadata::tags())
        lst.append(str);
    return lst;
  }
};

template <oscr::DataflowNode Info>
struct Metadata<Process::Descriptor_k, oscr::ProcessModel<Info>>
{
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
        Info::prettyName(),
        Info::kind(),
        Info::category(),
        Info::description(),
        Info::author(),
        Metadata<Tags_k, oscr::ProcessModel<Info>>::get(),
        inletDescription(),
        outletDescription()};
    return desc;
  }
};
template <typename Info>
struct Metadata<Process::ProcessFlags_k, oscr::ProcessModel<Info>>
{
  static Process::ProcessFlags get() noexcept {
    if constexpr(requires { Info::flags(); }) {
      return Info::flags();
    } else {
      return Process::ProcessFlags(Process::ProcessFlags::SupportsLasting | Process::ProcessFlags::ControlSurface);
    }
  }
};
template <typename Info>
struct Metadata<ObjectKey_k, oscr::ProcessModel<Info>>
{
  static Q_DECL_RELAXED_CONSTEXPR auto get() noexcept
  {
    return Info::objectKey();
  }
};
template <typename Info>
struct Metadata<ConcreteKey_k, oscr::ProcessModel<Info>>
{
  static Q_DECL_RELAXED_CONSTEXPR UuidKey<Process::ProcessModel> get()
  {
    return Info::uuid();
  }
};

namespace oscr
{

struct InletInitFunc
{
  Process::ProcessModel& self;
  Process::Inlets& ins;
  int inlet = 0;
  void operator()(const AudioInput auto& in)
  {
    auto p = new Process::AudioInlet(Id<Process::Port>(inlet++), &self);
    p->setName(QString::fromUtf8(in.name()));
    ins.push_back(p);
  }
  void operator()(const ValueInput auto& in)
  {
    auto p = new Process::ValueInlet(Id<Process::Port>(inlet++), &self);
    p->setName(QString::fromUtf8(in.name()));
    ins.push_back(p);
  }
  void operator()(const MidiInput auto& in)
  {
    auto p = new Process::MidiInlet(Id<Process::Port>(inlet++), &self);
    p->setName(QString::fromUtf8(in.name()));
    ins.push_back(p);
  }
  void operator()(const TextureInput auto& in)
  {
    auto p = new Gfx::TextureInlet(Id<Process::Port>(inlet++), &self);
    p->setName(QString::fromUtf8(in.name()));
    ins.push_back(p);
  }
  void operator()(const ControlInput auto& ctrl)
  {
    if (auto p = ctrl.control().create_inlet(Id<Process::Port>(inlet++), &self))
    {
      p->hidden = true;
      ins.push_back(p);
    }
  }
};

struct OutletInitFunc
{
  Process::ProcessModel& self;
  Process::Outlets& outs;
  int outlet = 0;
  void operator()(const AudioOutput auto& out)
  {
    auto p = new Process::AudioOutlet(Id<Process::Port>(outlet++), &self);
    p->setName(QString::fromUtf8(out.name()));
    if (outlet == 1)
      p->setPropagate(true);
    outs.push_back(p);
  }
  void operator()(const ValueOutput auto& out)
  {
    auto p = new Process::ValueOutlet(Id<Process::Port>(outlet++), &self);
    p->setName(QString::fromUtf8(out.name()));
    outs.push_back(p);
  }
  void operator()(const MidiOutput auto& out)
  {
    auto p = new Process::MidiOutlet(Id<Process::Port>(outlet++), &self);
    p->setName(QString::fromUtf8(out.name()));
    outs.push_back(p);
  }
  void operator()(const TextureOutput auto& out)
  {
    auto p = new Gfx::TextureOutlet(Id<Process::Port>(outlet++), &self);
    p->setName(QString::fromUtf8(out.name()));
    outs.push_back(p);
  }
  void operator()(const ControlOutput auto& ctrl)
  {
    if (auto p = ctrl.display().create_outlet(Id<Process::Port>(outlet++), &self))
    {
      p->hidden = true;
      outs.push_back(p);
    }
  }
};
/*
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
*/
template <oscr::DataflowNode Info, typename>
class ProcessModel final : public Process::ProcessModel
{
  SCORE_SERIALIZE_FRIENDS
  PROCESS_METADATA_IMPL(ProcessModel<Info>)
  friend struct TSerializer<DataStream, oscr::ProcessModel<Info>>;
  friend struct TSerializer<JSONObject, oscr::ProcessModel<Info>>;

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

    if constexpr(requires { Info::inputs; })
    {
      decltype(Info::inputs) ins;
      boost::pfr::for_each_field_ref(ins, InletInitFunc{*this, m_inlets});
    }
    if constexpr(requires { Info::outputs; })
    {
      decltype(Info::outputs) outs;
      boost::pfr::for_each_field_ref(outs, OutletInitFunc{*this, m_outlets});
    }
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
struct is_custom_serialized<oscr::ProcessModel<Info>> : std::true_type
{
};

template <template <typename, typename> class Model, typename Info>
struct TSerializer<DataStream, Model<Info, oscr::is_control>>
{
  using model_type = Model<Info, oscr::is_control>;
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
struct TSerializer<JSONObject, Model<Info, oscr::is_control>>
{
  using model_type = Model<Info, oscr::is_control>;
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
void serialize_dyn_impl(Vis& v, const oscr::ProcessModel<Info>& t)
{
  TSerializer<typename Vis::type, oscr::ProcessModel<Info>>::readFrom(
      v, t);
}
}

