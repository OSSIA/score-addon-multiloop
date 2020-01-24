#pragma once
#include <Process/Inspector/ProcessInspectorWidgetDelegate.hpp>
#include <Process/Inspector/ProcessInspectorWidgetDelegateFactory.hpp>

#include <score/command/Dispatchers/CommandDispatcher.hpp>

#include <multiloop/Process.hpp>

namespace multiloop
{
class InspectorWidget final
    : public Process::InspectorWidgetDelegate_T<multiloop::Model>
{
public:
  explicit InspectorWidget(
      const multiloop::Model& object, const score::DocumentContext& context,
      QWidget* parent);
  ~InspectorWidget() override;

private:
  CommandDispatcher<> m_dispatcher;
};

class InspectorFactory final
    : public Process::InspectorWidgetDelegateFactory_T<Model, InspectorWidget>
{
  SCORE_CONCRETE("1d9dd0ea-dc05-4f79-ab07-f849960912ab")
};
}
