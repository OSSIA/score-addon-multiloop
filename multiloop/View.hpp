#pragma once
#include <Process/LayerView.hpp>
#include <multiloop/Process.hpp>
namespace multiloop
{
class CellItem;
class View final : public Process::LayerView
{
public:
  explicit View(
      const Model& p,
      const Process::Context& ctx,
      QGraphicsItem* parent);
  ~View() override;

private:
  void paint_impl(QPainter*) const override;

  const Model& m_model;
  const Process::Context& m_ctx;
  void recreate();
  void heightChanged(qreal);
  void widthChanged(qreal);
  std::vector<CellItem*> items;
};
}
