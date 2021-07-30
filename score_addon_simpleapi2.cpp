#include "score_addon_simpleapi2.hpp"

#include <SimpleApi2/ProcessModel.hpp>
#include <SimpleApi2/Executor.hpp>
#include <SimpleApi2/Layer.hpp>

#include <score/plugins/FactorySetup.hpp>

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
template <typename Node>
using ProcessFactory = Process::ProcessFactory_T<SimpleApi2::ProcessModel<Node>>;


template <typename Node>
struct ExecutorFactory final
    : public Execution::ProcessComponentFactory_T<SimpleApi2::Executor<Node>>
{
  using Execution::ProcessComponentFactory_T<
      SimpleApi2::Executor<Node>>::ProcessComponentFactory_T;
};

 //template <typename Node>
 //using LayerFactory = SimpleApi2::LayerFactory<Node>;


template <typename... Args>
struct create_types
{
  template <template <typename> typename GenericFactory>
  auto perform()
  {
    std::vector<std::unique_ptr<score::InterfaceBase>> vec;
    ossia::for_each_tagged(ossia::tl<Args...>{}, [&](auto t) {
                             using type = typename decltype(t)::type;
                             vec.emplace_back(std::make_unique<GenericFactory<type>>());
                           });
    return vec;
  }
};
template <typename... Nodes>
std::vector<std::unique_ptr<score::InterfaceBase>> instantiate_fx(
    const score::ApplicationContext& ctx,
    const score::InterfaceKey& key)
{
  if (key == Execution::ProcessComponentFactory::static_interfaceKey())
  {
    return create_types<Nodes...>{}
        .template perform<SimpleApi2::ExecutorFactory>();
  }
  else if (key == Process::ProcessModelFactory::static_interfaceKey())
  {
    return create_types<Nodes...>{}
        .template perform<SimpleApi2::ProcessFactory>();
  }
  //else if (key == Process::LayerFactory::static_interfaceKey())
  //{
  //  return create_types<Nodes...>{}.template perform<SimpleApi2::LayerFactory>();
  //}
  return {};
}
}

score_addon_simpleapi2::score_addon_simpleapi2() = default;
score_addon_simpleapi2::~score_addon_simpleapi2() = default;

std::vector<std::unique_ptr<score::InterfaceBase>>
score_addon_simpleapi2::factories(
    const score::ApplicationContext& ctx,
    const score::InterfaceKey& key) const
{
  return SimpleApi2::instantiate_fx<SimpleApi2::Distortion, SimpleApi2::CCC>(ctx, key);
}

std::vector<score::PluginKey> score_addon_simpleapi2::required() const
{
  return {score_plugin_engine::static_key()};
}

#include <score/plugins/PluginInstances.hpp>
SCORE_EXPORT_PLUGIN(score_addon_simpleapi2)
