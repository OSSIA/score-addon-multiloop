#pragma once
#include <Process/GenericProcessFactory.hpp>
#include <Process/Process.hpp>

#include <multiloop/Metadata.hpp>
#include <Scenario/Document/Interval/IntervalModel.hpp>
#include <Scenario/Document/Event/EventModel.hpp>
#include <Scenario/Document/TimeSync/TimeSyncModel.hpp>
#include <Scenario/Document/State/StateModel.hpp>
#include <Scenario/Process/ScenarioInterface.hpp>


namespace multiloop
{
class Cell// : public IdentifiedObject<Cell>
{
public:
  //using IdentifiedObject<Cell>::IdentifiedObject;
  Id<Scenario::IntervalModel> interval;
  Id<Scenario::StateModel> startState;
  Id<Scenario::StateModel> endState;
  Id<Scenario::EventModel> startEvent;
  Id<Scenario::EventModel> endEvent;
  Id<Scenario::TimeSyncModel> startSync;
  Id<Scenario::TimeSyncModel> endSync;
};
class Track
{
public:
  std::vector<Cell> cells;
};

class Grid
{
public:
  std::vector<Track> tracks;
};

class Model final
    : public Process::ProcessModel
    , public Scenario::ScenarioInterface
{
  SCORE_SERIALIZE_FRIENDS
  PROCESS_METADATA_IMPL(multiloop::Model)
  W_OBJECT(Model)

public:
    using IntervalModel = Scenario::IntervalModel;
    using EventModel = Scenario::EventModel;
    using StateModel = Scenario::StateModel;
    using TimeSyncModel = Scenario::TimeSyncModel;

    std::unique_ptr<Process::AudioInlet> inlet;
    std::unique_ptr<Process::AudioOutlet> outlet;

  Model(
      const TimeVal& duration, const Id<Process::ProcessModel>& id,
      QObject* parent);

  template <typename Impl>
  Model(Impl& vis, QObject* parent) : Process::ProcessModel{vis, parent}
  {
    vis.writeTo(*this);
    init();
  }

  ~Model() override;

  void init();
  const Grid& grid() const noexcept { return m_grid; }
  score::EntityMap<Scenario::IntervalModel> intervals;
  score::EntityMap<Scenario::EventModel> events;
  score::EntityMap<Scenario::TimeSyncModel> timeSyncs;
  score::EntityMap<Scenario::StateModel> states;

  // Accessors
  score::IndirectContainer<IntervalModel> getIntervals() const final override
  {
    return intervals.map().as_indirect_vec();
  }

  score::IndirectContainer<StateModel> getStates() const final override
  {
    return states.map().as_indirect_vec();
  }

  score::IndirectContainer<EventModel> getEvents() const final override
  {
    return events.map().as_indirect_vec();
  }

  score::IndirectContainer<TimeSyncModel> getTimeSyncs() const final override
  {
    return timeSyncs.map().as_indirect_vec();
  }

  IntervalModel* findInterval(const Id<IntervalModel>& id) const final override
  {
    return ossia::ptr_find(intervals, id);
  }
  EventModel* findEvent(const Id<EventModel>& id) const final override
  {
    return ossia::ptr_find(events, id);
  }
  TimeSyncModel* findTimeSync(const Id<TimeSyncModel>& id) const final override
  {
    return ossia::ptr_find(timeSyncs, id);
  }
  StateModel* findState(const Id<StateModel>& id) const final override
  {
    return ossia::ptr_find(states, id);
  }

  IntervalModel&
  interval(const Id<IntervalModel>& intervalId) const final override
  {
    return intervals.at(intervalId);
  }
  EventModel& event(const Id<EventModel>& eventId) const final override
  {
    return events.at(eventId);
  }
  TimeSyncModel&
  timeSync(const Id<TimeSyncModel>& timeSyncId) const final override
  {
    return timeSyncs.at(timeSyncId);
  }
  StateModel& state(const Id<StateModel>& stId) const final override
  {
    return states.at(stId);
  }

  void startClip(int track, int clip) const W_SIGNAL(startClip, track, clip);
  void stopClip(int track, int clip) const W_SIGNAL(stopClip, track, clip);
  void startScene(int clip) const W_SIGNAL(startScene, clip);
  void stopScene(int clip) const W_SIGNAL(stopScene, clip);
private:
  Cell generateCell() const;
  void createCell(const Cell& c);
  Grid m_grid;
};

using ProcessFactory = Process::ProcessFactory_T<multiloop::Model>;
}
