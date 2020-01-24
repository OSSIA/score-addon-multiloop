#include "score_addon_multiloop.hpp"

#include <score/plugins/FactorySetup.hpp>

#include <multiloop/CommandFactory.hpp>
#include <multiloop/Executor.hpp>
#include <multiloop/Inspector.hpp>
#include <multiloop/Layer.hpp>
#include <multiloop/LocalTree.hpp>
#include <multiloop/Process.hpp>
#include <score_addon_multiloop_commands_files.hpp>

score_addon_multiloop::score_addon_multiloop()
{
}

score_addon_multiloop::~score_addon_multiloop()
{
}

std::vector<std::unique_ptr<score::InterfaceBase>>
score_addon_multiloop::factories(
    const score::ApplicationContext& ctx, const score::InterfaceKey& key) const
{
  return instantiate_factories<
      score::ApplicationContext,
      FW<Process::ProcessModelFactory, multiloop::ProcessFactory>,
      FW<Process::LayerFactory, multiloop::LayerFactory>,
      FW<Process::InspectorWidgetDelegateFactory, multiloop::InspectorFactory>,
      FW<Execution::ProcessComponentFactory,
         multiloop::ProcessExecutorComponentFactory>,
      FW<LocalTree::ProcessComponentFactory,
         multiloop::LocalTreeProcessComponentFactory>>(ctx, key);
}

std::pair<const CommandGroupKey, CommandGeneratorMap>
score_addon_multiloop::make_commands()
{
  using namespace multiloop;
  std::pair<const CommandGroupKey, CommandGeneratorMap> cmds{
      CommandFactoryName(), CommandGeneratorMap{}};

  ossia::for_each_type<
#include <score_addon_multiloop_commands.hpp>
      >(score::commands::FactoryInserter{cmds.second});

  return cmds;
}

#include <score/plugins/PluginInstances.hpp>
SCORE_EXPORT_PLUGIN(score_addon_multiloop)
