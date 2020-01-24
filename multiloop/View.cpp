#include "View.hpp"

#include <Process/Style/ScenarioStyle.hpp>
#include <score/selection/SelectionStack.hpp>
#include <score/selection/SelectionDispatcher.hpp>
#include <score/document/DocumentContext.hpp>
#include <QPainter>
#include <Process/LayerPresenter.hpp>
#include <Process/ProcessList.hpp>
#include <Process/Style/Pixmaps.hpp>
#include <score/graphics/GraphicWidgets.hpp>
namespace multiloop
{
constexpr double x_padding = 15;
constexpr double y_padding = 15;
constexpr double cell_header = 15;
constexpr double cell_w = 60;
constexpr double cell_h = cell_w / 2.187;
constexpr double cell_padding = 2;
class CellItem : public QObject, public QGraphicsItem, public Nano::Observer
{
public:
  int track{};
  int clip{};
  double cell_w{};
  double cell_h{};

  Process::LayerView* m_fx{};
  Process::LayerPresenter* m_presenter{};
  score::QGraphicsPixmapToggle* m_play{};
  CellItem(
      int track, int clip,
      double cell_w, double cell_h,
      const Cell& cell,
      const Model& multiloop,
      const Process::Context& context,
      QGraphicsItem* parent): QGraphicsItem{parent}
  , track{track}
  , clip{clip}
  , cell_w{cell_w}
  , cell_h{cell_h}
  , multiloop{multiloop}
  , context{context}
  , cell{cell}
  {
    auto& p = Process::Pixmaps::instance();
    m_play = new score::QGraphicsPixmapToggle{p.play, p.stop, this};
    m_play->setPos(1, 1);
    connect(m_play, &score::QGraphicsPixmapToggle::toggled,
            this, [this] (bool b) {
      if(b)
        this->multiloop.startClip(this->track, this->clip);
      else
        this->multiloop.stopClip(this->track, this->clip);
    });
    auto& itv = multiloop.intervals.at(cell.interval);
    itv.processes.added.connect<&CellItem::updateFX2>(*this);
    itv.processes.removing.connect<&CellItem::updateFX2>(*this);
    updateFX();
  }

  void updateFX2(const Process::ProcessModel&)
  {
    updateFX();
  }

  void updateFX()
  {
    delete m_presenter;
    m_presenter = nullptr;
    m_fx = nullptr;
    auto& itv = multiloop.intervals.at(cell.interval);
    if(!itv.processes.empty())
    {
      auto& child_proc = *itv.processes.begin();
      auto& fact = context.app.interfaces<Process::LayerFactoryList>();
      if (auto factory = fact.findDefaultFactory(child_proc))
      {
        if (auto fx = factory->makeLayerView(child_proc, context, this))
        {
          m_fx = fx;
          m_fx->setY(cell_header);
          m_presenter = factory->makeLayerPresenter(child_proc, fx, context, this);
          m_presenter->setWidth(cell_w, cell_w);
          m_presenter->setHeight(cell_h);
          m_presenter->on_zoomRatioChanged(itv.duration.defaultDuration().msec() / cell_w);
          m_presenter->parentGeometryChanged();
        }
      }
    }
  }
  const Model& multiloop;
  const Process::Context& context;

  ~CellItem()
  {
    delete m_presenter;
  }

  Cell cell;
  QRectF boundingRect() const override
  {
    return {0, 0, cell_w, cell_header + cell_h};
  }

  void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override
  {
    auto& skin = Process::Style::instance();
    painter->setPen(skin.skin.Base1.lighter180.pen1_solid_flat_miter);
    painter->setBrush(skin.skin.Base1.darker300.brush);
    painter->drawRect(QRectF{1, 1, cell_w - 2, cell_header - 1});
    painter->setBrush(skin.skin.TransparentBrush);
    painter->drawRect(QRectF{1, cell_header, cell_w - 2, cell_h - 1});
  }

  void mousePressEvent(QGraphicsSceneMouseEvent* ev)
  {
    score::SelectionDispatcher disp{context.selectionStack};
    auto& itv = multiloop.intervals.at(cell.interval);
    if(itv.processes.empty())
      disp.setAndCommit({&itv});
    else
      disp.setAndCommit({&*itv.processes.begin()});
  }
};

View::View(
    const Model& p,
    const Process::Context& ctx,
    QGraphicsItem* parent)
  : LayerView{parent}
  , m_model{p}
  , m_ctx{ctx}
{
}

void View::heightChanged(qreal)
{
  recreate();
}

void View::widthChanged(qreal)
{
  recreate();
}

void View::recreate()
{
  for(auto it : items)
    delete it;
  items.clear();

  double cell_w = (width() - x_padding* 2) / m_model.grid().tracks.size() - cell_padding  - 2;
  cell_w = std::max(10., cell_w);
  double cell_h = (height() - y_padding * 2) / m_model.grid().tracks[0].cells.size() - cell_padding - cell_header - 2;
  cell_h = std::max(10., cell_h);
  qreal x = x_padding;
  int t = 0;
  for(const Track& track : m_model.grid().tracks)
  {
    int c = 0;
    qreal y = y_padding;
    for(const Cell& clip : track.cells)
    {
      auto item = new CellItem{t, c, cell_w, cell_h, clip, m_model, m_ctx, this};
      items.push_back(item);

      item->setPos(x, y);
      y += cell_h + cell_header+ cell_padding;
      c++;
    }
    t++;
    x += cell_w + cell_padding;
  }
}

View::~View()
{
}

void View::paint_impl(QPainter* painter) const
{
}
}
