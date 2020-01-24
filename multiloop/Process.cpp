#include "Process.hpp"

#include <wobjectimpl.h>
#include <score/tools/IdentifierGeneration.hpp>
#include <score/selection/SelectionStack.hpp>
#include <score/document/DocumentInterface.hpp>
#include <score/document/DocumentContext.hpp>
#include <score/model/EntitySerialization.hpp>
#include <score/model/path/PathSerialization.hpp>
W_OBJECT_IMPL(multiloop::Model)
namespace multiloop
{
  using namespace Scenario;

Model::Model(
    const TimeVal& duration, const Id<Process::ProcessModel>& id,
    QObject* parent)
    : Process::ProcessModel{duration, id, "multiloopProcess", parent}
    , inlet{Process::make_audio_inlet(Id<Process::Port>(0), this)}
    , outlet{Process::make_audio_outlet(Id<Process::Port>(0), this)}
{
  metadata().setInstanceName(*this);
  outlet->setPropagate(true);

  for(int track = 0; track < 4; track++)
  {
    Track t;
    for(int cell = 0; cell < 2; cell++)
    {
      auto c = generateCell();
      createCell(c);
      t.cells.push_back(c);
    }
    m_grid.tracks.push_back(t);
  }
  init();
}

Model::~Model()
{
  try
  {
    score::IDocument::documentContext(*parent()).selectionStack.clear();
  }
  catch (...)
  {
    // Sometimes the scenario isn't in the hierarchy, e.G. in
    // ScenarioPasteElements
  }
  intervals.clear();
  states.clear();
  events.clear();
  timeSyncs.clear();

  identified_object_destroying(this);
}

void Model::init()
{
  inlet->setCustomData("In");
  outlet->setCustomData("Out");
  m_inlets.push_back(inlet.get());
  m_outlets.push_back(outlet.get());
}

Cell Model::generateCell() const
{
  Cell c;
  c.interval = getStrongId(intervals);
  auto states = getStrongIdRange<StateModel>(2, this->states);
  auto events = getStrongIdRange<EventModel>(2, this->events);
  auto syncs = getStrongIdRange<TimeSyncModel>(2, this->timeSyncs);
  c.startState = states[0];
  c.endState = states[1];
  c.startEvent = events[0];
  c.endEvent = events[1];
  c.startSync = syncs[0];
  c.endSync = syncs[1];
  return c;
}

void Model::createCell(const Cell& c)
{
  auto itv = new IntervalModel{c.interval, 0., this};
  Scenario::IntervalDurations::Algorithms::fixAllDurations(*itv, this->duration());
  auto ss = new StateModel{c.startState, c.startEvent, 0., this};
  auto es = new StateModel{c.endState, c.endEvent, 0., this};
  auto sev = new EventModel{c.startEvent, c.startSync, TimeVal{}, this};
  auto eev = new EventModel{c.endEvent, c.endSync, TimeVal{}, this};
  auto sts = new TimeSyncModel{c.startSync, TimeVal{}, this};
  auto ets = new TimeSyncModel{c.endSync, TimeVal{}, this};

  timeSyncs.add(sts);
  timeSyncs.add(ets);

  events.add(sev);
  events.add(eev);

  states.add(ss);
  states.add(es);

  intervals.add(itv);

  sts->addEvent(c.startEvent);
  ets->addEvent(c.endEvent);


  sev->addState(c.startState);
  eev->addState(c.endState);


  itv->setStartState(c.startState);
  itv->setEndState(c.endState);

  ss->setNextInterval(c.interval);
  es->setPreviousInterval(c.interval);

}
/*
QString Model::prettyName() const noexcept
{
  return tr("multiloop Process");
}

void Model::startExecution()
{
  // TODO this is called for each process!!
  // But it should be done only once at the global level.
  for (IntervalModel& interval : intervals)
  {
    interval.startExecution();
  }
}

void Model::stopExecution()
{
  for (IntervalModel& interval : intervals)
  {
    interval.stopExecution();
  }
  for (EventModel& ev : events)
  {
    ev.setStatus(ExecutionStatus::Editing, *this);
  }
}

void Model::reset()
{
  for (auto& interval : intervals)
  {
    interval.reset();
  }

  for (auto& event : events)
  {
    event.setStatus(Scenario::ExecutionStatus::Editing, *this);
  }
}
*/
}
template <>
void DataStreamReader::read(const multiloop::Model& proc)
{
  insertDelimiter();
}

template <>
void DataStreamWriter::write(multiloop::Model& proc)
{
  checkDelimiter();
}

template <>
void JSONObjectReader::read(const multiloop::Model& proc)
{
}

template <>
void JSONObjectWriter::write(multiloop::Model& proc)
{
}
