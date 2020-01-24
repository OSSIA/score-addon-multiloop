#pragma once
#include <Process/Execution/ProcessComponent.hpp>

#include <ossia/dataflow/node_process.hpp>

namespace multiloop
{
class ClipExecutor;
class Model;
class ProcessExecutorComponent final
    : public Execution::ProcessComponent_T<
          multiloop::Model, ossia::node_process>
{
  COMPONENT_METADATA("54256c2b-d460-471b-afe8-07e21c893919")
public:
  ProcessExecutorComponent(
      Model& element, const Execution::Context& ctx,
      const Id<score::Component>& id, QObject* parent);
  std::vector<std::vector<ClipExecutor*>> executors;
};

using ProcessExecutorComponentFactory
    = Execution::ProcessComponentFactory_T<ProcessExecutorComponent>;
}
