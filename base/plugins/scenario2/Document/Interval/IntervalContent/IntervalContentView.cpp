#include "IntervalContentView.hpp"
#include <QPainter>
#include <QGraphicsScene>
#include <QDebug>
IntervalContentView::IntervalContentView(QGraphicsObject* parent):
	QNamedGraphicsObject{parent, "IntervalContentView"}
{
	this->setParentItem(parent);
	//parentItem()->scene()->addItem(this);
}

QRectF IntervalContentView::boundingRect() const
{
	return {parentItem()->boundingRect().x() + DEMO_PIXEL_SPACING_TEST,
			parentItem()->boundingRect().y() + 50 + DEMO_PIXEL_SPACING_TEST,
			parentItem()->boundingRect().width()  - 2 * DEMO_PIXEL_SPACING_TEST,
			parentItem()->boundingRect().height() - 50 - 2 * DEMO_PIXEL_SPACING_TEST};
	// TODO : plutôt prendre le height de chaque storey
}

void IntervalContentView::paint(QPainter* painter,
								const QStyleOptionGraphicsItem* option,
								QWidget* widget)
{
	painter->drawText(boundingRect(), "content model");
	painter->drawRect(boundingRect());
}
