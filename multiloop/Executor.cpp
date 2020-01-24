#include "Executor.hpp"

#include <Process/ExecutionContext.hpp>
#include <Loop/LoopExecution.hpp>
#include <ossia/dataflow/port.hpp>

#include <multiloop/Process.hpp>

#include <Process/ExecutionContext.hpp>
#include <Scenario/Document/Event/EventExecution.hpp>
#include <Scenario/Document/Event/EventModel.hpp>
#include <Scenario/Document/Interval/IntervalModel.hpp>
#include <Scenario/Document/Interval/IntervalRawPtrExecution.hpp>
#include <Scenario/Document/State/StateExecution.hpp>
#include <Scenario/Document/State/StateModel.hpp>
#include <Scenario/Document/TimeSync/TimeSyncModel.hpp>
#include <Scenario/Document/TimeSync/TimeSyncRawPtrExecution.hpp>

#include <ossia/dataflow/graph/graph_interface.hpp>
#include <ossia/dataflow/graph_edge.hpp>
#include <ossia/editor/loop/loop.hpp>
#include <ossia/editor/scenario/time_event.hpp>
#include <ossia/editor/scenario/time_sync.hpp>
namespace multiloop
{
class node final : public ossia::nonowning_graph_node
{
public:
  node()
  {
  }

  void
  run(const ossia::token_request& tk, ossia::exec_state_facade s) noexcept override
  {
  }

  std::string label() const noexcept override
  {
    return "multiloop";
  }
};
class multiloop_process final  : public ossia::looping_process<multiloop_process>
{
public:

  multiloop_process(ossia::node_ptr n)
  {
    node = n;
  }
  void offset_impl(ossia::time_value date) { }

  void transport_impl(ossia::time_value date) { }

  void state_impl(const ossia::token_request& req)
  {
    struct {
      multiloop_process& self;
      void operator()(const StartClip& clip)
      {
        self.start_clip(clip.track, clip.clip);
      }
      void operator()(const StopClip& clip)
      {
        self.stop_clip(clip.track, clip.clip);
      }
    } vis{*this};
    for(const auto& cmd : commands)
    {
      std::visit(vis, cmd);
    }
    commands.clear();
    for(auto& track : loops)
    {
      for(auto& clip : track)
      {
        if(clip.active)
        {
          clip.loop->state(req);
        }
      }
    }
    node->request(req);
  }

  void start_clip(int track, int clip)
  {
    loops[track][clip].loop->start();
    loops[track][clip].active = true;
  }
  void stop_clip(int track, int clip)
  {
    loops[track][clip].loop->stop();
    loops[track][clip].active = false;
  }
  void start_scene(int clip)
  {
    for(int i = 0; i < loops.size(); i++)
      start_clip(i, clip);
  }
  void stop_scene(int clip)
  {
    for(int i = 0; i < loops.size(); i++)
      stop_clip(i, clip);
  }
  struct player {
    std::shared_ptr<ossia::loop> loop;
    bool active{};
  };

  std::vector<std::vector<player>> loops;

  struct StartClip {
    int track{};
    int clip{};
  };
  struct StopClip {
    int track{};
    int clip{};
  };
  using Command = std::variant<StartClip, StopClip>;

  std::vector<Command> commands;
};

class ClipExecutor final
{
public:
  ClipExecutor(
      Scenario::IntervalModel& interval,
      const Scenario::StateModel& ss,
      const Scenario::StateModel& es,
      const Scenario::EventModel& sev,
      const Scenario::EventModel& eev,
      const Scenario::TimeSyncModel& sts,
      const Scenario::TimeSyncModel& ets,
      const ::Execution::Context& ctx,
      QObject* parent);

  virtual ~ClipExecutor();

  void cleanup();

  void stop();


  std::shared_ptr<ossia::loop> loop;
private:
  void startIntervalExecution(const Id<Scenario::IntervalModel>&);
  void stopIntervalExecution(const Id<Scenario::IntervalModel>&);

  std::shared_ptr<Execution::IntervalRawPtrComponent> m_ossia_interval;

  std::shared_ptr<Execution::TimeSyncRawPtrComponent> m_ossia_startTimeSync;
  std::shared_ptr<Execution::TimeSyncRawPtrComponent> m_ossia_endTimeSync;

  std::shared_ptr<Execution::EventComponent> m_ossia_startEvent;
  std::shared_ptr<Execution::EventComponent> m_ossia_endEvent;

  std::shared_ptr<Execution::StateComponent> m_ossia_startState;
  std::shared_ptr<Execution::StateComponent> m_ossia_endState;
};
ClipExecutor::ClipExecutor(
    Scenario::IntervalModel& interval,
    const Scenario::StateModel& ss,
    const Scenario::StateModel& es,
    const Scenario::EventModel& sev,
    const Scenario::EventModel& eev,
    const Scenario::TimeSyncModel& sts,
    const Scenario::TimeSyncModel& ets,
    const ::Execution::Context& ctx,
    QObject* parent)

{
  ossia::time_value main_duration(
      ctx.time(interval.duration.defaultDuration()));

  this->loop = std::make_shared<ossia::loop>(
      main_duration,
      ossia::time_interval::exec_callback{},
      [this, &sev](ossia::time_event::status newStatus) {
        // sev.setStatus(
        //     static_cast<Scenario::ExecutionStatus>(newStatus), process());
        switch (newStatus)
        {
          case ossia::time_event::status::NONE:
            break;
          case ossia::time_event::status::PENDING:
            break;
          case ossia::time_event::status::HAPPENED:
            startIntervalExecution(m_ossia_interval->scoreInterval().id());
            break;
          case ossia::time_event::status::DISPOSED:
            break;
          default:
            SCORE_TODO;
            break;
        }
      },
      [this, &eev](ossia::time_event::status newStatus) {
        // eev.setStatus(
        //     static_cast<Scenario::ExecutionStatus>(newStatus), process());
        switch (newStatus)
        {
          case ossia::time_event::status::NONE:
            break;
          case ossia::time_event::status::PENDING:
            break;
          case ossia::time_event::status::HAPPENED:
            stopIntervalExecution(m_ossia_interval->scoreInterval().id());
            break;
          case ossia::time_event::status::DISPOSED:
            break;
          default:
            SCORE_TODO;
            break;
        }
      });

  //m_ossia_process = loop;
  //node = loop->node;

  // TODO also states in BasEelement
  // TODO put graphical settings somewhere.
  auto& main_start_node = loop->get_start_timesync();
  auto& main_end_node = loop->get_end_timesync();
  auto main_start_event = *main_start_node.get_time_events().begin();
  auto main_end_event = *main_end_node.get_time_events().begin();

  using namespace Execution;
  m_ossia_startTimeSync = std::make_shared<TimeSyncRawPtrComponent>(
      sts,
      ctx,
      score::newId(sts),
      parent);
  m_ossia_endTimeSync = std::make_shared<TimeSyncRawPtrComponent>(
      ets,
      ctx,
      score::newId(ets),
      parent);

  m_ossia_startEvent = std::make_shared<EventComponent>(
      sev,
      ctx,
      score::newId(sev),
      parent);
  m_ossia_endEvent = std::make_shared<EventComponent>(
      eev, ctx, score::newId(eev), parent);

  m_ossia_startState = std::make_shared<StateComponent>(
      ss,
      ctx,
      score::newId(ss),
      parent);
  m_ossia_endState = std::make_shared<StateComponent>(
      es, ctx, score::newId(es), parent);

  m_ossia_interval = std::make_shared<IntervalRawPtrComponent>(
      interval, ctx, score::newId(interval), parent);

  m_ossia_startTimeSync->onSetup(
      &main_start_node, m_ossia_startTimeSync->makeTrigger());
  m_ossia_endTimeSync->onSetup(
      &main_end_node, m_ossia_endTimeSync->makeTrigger());
  m_ossia_startEvent->onSetup(
      main_start_event,
      m_ossia_startEvent->makeExpression(),
      (ossia::time_event::offset_behavior)sev
          .offsetBehavior());
  m_ossia_endEvent->onSetup(
      main_end_event,
      m_ossia_endEvent->makeExpression(),
      (ossia::time_event::offset_behavior)eev.offsetBehavior());
  m_ossia_startState->onSetup(main_start_event);
  m_ossia_endState->onSetup(main_end_event);
  m_ossia_interval->onSetup(
      m_ossia_interval,
      &loop->get_time_interval(),
      m_ossia_interval->makeDurations());

  auto cable = ossia::make_edge(
      ossia::immediate_glutton_connection{},
      m_ossia_interval->OSSIAInterval()->node->outputs()[0],
      loop->node->inputs()[0],
      m_ossia_interval->OSSIAInterval()->node,
      loop->node);

  ctx.executionQueue.enqueue([g = ctx.execGraph, cable] { g->connect(cable); });
}

ClipExecutor::~ClipExecutor() {}

void ClipExecutor::cleanup()
{
  if (m_ossia_interval)
  {
    m_ossia_interval->cleanup(m_ossia_interval);
  }
  if (m_ossia_startState)
  {
    m_ossia_startState->cleanup(m_ossia_startState);
  }
  if (m_ossia_endState)
  {
    m_ossia_endState->cleanup(m_ossia_endState);
  }
  if (m_ossia_startEvent)
  {
    m_ossia_startEvent->cleanup();
  }
  if (m_ossia_endEvent)
  {
    m_ossia_endEvent->cleanup();
  }
  if (m_ossia_startTimeSync)
  {
    m_ossia_startTimeSync->cleanup();
  }
  if (m_ossia_endTimeSync)
  {
    m_ossia_endTimeSync->cleanup();
  }

  m_ossia_interval = nullptr;
  m_ossia_startState = nullptr;
  m_ossia_endState = nullptr;
  m_ossia_startEvent = nullptr;
  m_ossia_endEvent = nullptr;
  m_ossia_startTimeSync = nullptr;
  m_ossia_endTimeSync = nullptr;

  // ProcessComponent::cleanup();
}

void ClipExecutor::stop()
{
  /*
  ProcessComponent::stop();
  process().interval().duration.setPlayPercentage(0);
  process().startEvent().setStatus(
      Scenario::ExecutionStatus::Editing, process());
  process().endEvent().setStatus(
      Scenario::ExecutionStatus::Editing, process());
      */
}

void ClipExecutor::startIntervalExecution(const Id<Scenario::IntervalModel>&)
{
  m_ossia_interval->executionStarted();
}

void ClipExecutor::stopIntervalExecution(const Id<Scenario::IntervalModel>&)
{
  m_ossia_interval->executionStopped();
}

ProcessExecutorComponent::ProcessExecutorComponent(
    multiloop::Model& element, const Execution::Context& ctx,
    const Id<score::Component>& id, QObject* parent)
    : ProcessComponent_T{element, ctx, id, "multiloopExecutorComponent", parent}
{
  auto n = std::make_shared<multiloop::node>();
  this->node = n;
  auto ossia_process = std::make_shared<multiloop_process>(n);
  m_ossia_process = ossia_process;
  for(auto& track : element.grid().tracks)
  {
    std::vector<ClipExecutor*> exec;
    std::vector<multiloop_process::player> loops;
    for(auto& clip : track.cells)
    {
      auto lp = new ClipExecutor{
          element.interval(clip.interval),
          element.state(clip.startState),
          element.state(clip.endState),
          element.event(clip.startEvent),
          element.event(clip.endEvent),
          element.timeSync(clip.startSync),
          element.timeSync(clip.endSync),
          ctx, this
      };
      loops.push_back({lp->loop, false});
      exec.push_back(lp);
    }
    this->executors.push_back(exec);
    ossia_process->loops.push_back(loops);
  }

  connect(&element, &multiloop::Model::startClip,
      this, [=] (int track, int clip){
    in_exec([ossia_process, track, clip] {
      ossia_process->commands.push_back(multiloop_process::StartClip{track, clip});
    });
  });
  connect(&element, &multiloop::Model::stopClip,
          this, [=] (int track, int clip){
    in_exec([ossia_process, track, clip] {
      ossia_process->commands.push_back(multiloop_process::StopClip{track, clip});
    });
  });


  /** Don't forget that the node executes in another thread.
   * -> handle live updates with the in_exec function, e.g. :
   *
   * connect(&element.metadata(), &score::ModelMetadata::ColorChanged,
   *         this, [=] (const QColor& c) {
   *
   *   in_exec([c,n=std::dynamic_pointer_cast<multiloop::node>(this->node)] {
   *     n->set_color(c);
   *   });
   *
   * });
   */
}
}
