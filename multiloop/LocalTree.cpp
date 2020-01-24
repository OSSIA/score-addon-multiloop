#include "LocalTree.hpp"

#include <LocalTree/Property.hpp>
#include <multiloop/Process.hpp>

namespace multiloop
{
LocalTreeProcessComponent::LocalTreeProcessComponent(
    const Id<score::Component>& id, ossia::net::node_base& parent,
    multiloop::Model& proc, const score::DocumentContext& sys, QObject* parent_obj)
    : LocalTree::ProcessComponent_T<multiloop::Model>{
          parent, proc, sys, id, "multiloopComponent", parent_obj}
{
}

LocalTreeProcessComponent::~LocalTreeProcessComponent()
{
}
}
