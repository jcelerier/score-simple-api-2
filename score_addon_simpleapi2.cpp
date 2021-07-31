#include "score_addon_simpleapi2.hpp"

#include <SimpleApi2/ProcessModel.hpp>
#include <SimpleApi2/Executor.hpp>
#include <SimpleApi2/Layer.hpp>

#include <score/plugins/FactorySetup.hpp>

#include <Examples/AudioEffect.hpp>
#include <Examples/AudioEffectWithSidechains.hpp>
#include <Examples/Empty.hpp>
#include <Examples/SampleAccurateGenerator.hpp>
#include <Examples/SampleAccurateFilter.hpp>
#include <Examples/TrivialGenerator.hpp>
#include <Examples/TrivialFilter.hpp>
#include <Examples/RawPorts.hpp>

#include <Examples/Distortion.hpp>
#include <Examples/CCC.hpp>

#include <boost/pfr.hpp>
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



namespace SimpleApi2
{
template <SimpleApi2::DataflowNode Node>
using ProcessFactory = Process::ProcessFactory_T<SimpleApi2::ProcessModel<Node>>;


template <SimpleApi2::DataflowNode Node>
struct ExecutorFactory final
    : public Execution::ProcessComponentFactory_T<SimpleApi2::Executor<Node>>
{
  using Execution::ProcessComponentFactory_T<
      SimpleApi2::Executor<Node>>::ProcessComponentFactory_T;
};

 //template <typename Node>
 //using LayerFactory = SimpleApi2::LayerFactory<Node>;


template <SimpleApi2::DataflowNode... Nodes>
std::vector<std::unique_ptr<score::InterfaceBase>> instantiate_fx(
    const score::ApplicationContext& ctx,
    const score::InterfaceKey& key)
{
  std::vector<std::unique_ptr<score::InterfaceBase>> v;
  if (key == Execution::ProcessComponentFactory::static_interfaceKey())
  {
    //static_assert((requires { std::declval<Nodes>().run({}, {}); } && ...));
    (v.push_back(std::make_unique<SimpleApi2::ExecutorFactory<Nodes>>()), ...);
  }
  else if (key == Process::ProcessModelFactory::static_interfaceKey())
  {
    (v.push_back(std::make_unique<SimpleApi2::ProcessFactory<Nodes>>()), ...);
  }
  //else if (key == Process::LayerFactory::static_interfaceKey())
  //{
  //  (v.push_back(make_interface<SimpleApi2::LayerFactory<Nodes>>()), ...);
  //}
  return v;
}
}

score_addon_simpleapi2::score_addon_simpleapi2() = default;
score_addon_simpleapi2::~score_addon_simpleapi2() = default;

std::vector<std::unique_ptr<score::InterfaceBase>>
score_addon_simpleapi2::factories(
    const score::ApplicationContext& ctx,
    const score::InterfaceKey& key) const
{
  using namespace SimpleApi2;
  return SimpleApi2::instantiate_fx<
      SimpleApi2::Distortion
    , SimpleApi2::CCC
    , SimpleApi2::RawPortsExample
    , SimpleApi2::EmptyExample

    , SimpleApi2::SampleAccurateGeneratorExample,
      SimpleApi2::SampleAccurateFilterExample,
      SimpleApi2::TrivialGeneratorExample,
      SimpleApi2::TrivialFilterExample,
      SimpleApi2::AudioEffectExample,
      SimpleApi2::AudioSidechainExample>(ctx, key);
}

std::vector<score::PluginKey> score_addon_simpleapi2::required() const
{
  return {score_plugin_engine::static_key()};
}

#include <score/plugins/PluginInstances.hpp>
SCORE_EXPORT_PLUGIN(score_addon_simpleapi2)
