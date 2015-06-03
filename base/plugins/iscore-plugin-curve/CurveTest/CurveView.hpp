#pragma once
#include <QGraphicsItem>

class CurveView : public QGraphicsObject
{
        Q_OBJECT
    public:
        CurveView(QGraphicsItem* parent);

        void setRect(const QRectF& theRect);

        QRectF boundingRect() const;
        void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

        void setSelectionArea(const QRectF&);
    signals:
        void pressed(const QPointF&);
        void moved(const QPointF&);
        void released(const QPointF&);

        void escPressed();

    protected:
        void mousePressEvent(QGraphicsSceneMouseEvent* event) override;

        void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;

        void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;


    private:
        QRectF m_rect; // The rect in which the whole curve must fit.
        QRectF m_selectArea;
};

