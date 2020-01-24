#pragma once
#include <Process/GenericProcessFactory.hpp>

#include <multiloop/Presenter.hpp>
#include <multiloop/Process.hpp>
#include <multiloop/View.hpp>

namespace multiloop
{
using LayerFactory = Process::LayerFactory_T<
    multiloop::Model, multiloop::Presenter, multiloop::View>;
}
