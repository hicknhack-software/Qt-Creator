// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0+ OR GPL-3.0 WITH Qt-GPL-exception-1.0

#include "quicktransitionitem.h"
#include "scxmleditortr.h"

#include <QGraphicsSceneMouseEvent>
#include <QPainter>

using namespace ScxmlEditor::PluginInterface;

const qreal CAPX = 4;
const qreal CAPY = 3;

QuickTransitionItem::QuickTransitionItem(int index, ItemType connectionType, QGraphicsItem *parent)
    : QGraphicsObject(parent)
    , m_connectionType(connectionType)
{
    setParentItem(parent);
    installSceneEventFilter(parent);
    setZValue(501);

    m_rect = QRectF(index * 25, -30, 20, 20);
    m_drawingRect = m_rect.adjusted(4, 4, -4, -4);
    m_stateRect = m_rect.adjusted(3, 4, -3, -4);
    m_brush.setStyle(Qt::SolidPattern);
    m_brush.setColor(QColor(0xe8, 0xe8, 0xe8));

    m_pen.setColor(QColor(0x12, 0x12, 0x12));
    m_pen.setCapStyle(Qt::RoundCap);
    setAcceptHoverEvents(true);
}

void QuickTransitionItem::setConnectionType(ItemType connectionType)
{
    m_connectionType = connectionType;
}

void QuickTransitionItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *e)
{
    Q_UNUSED(e)
    m_brush.setColor(QColor(0xe8, 0xe8, 0xe8));
    update();
}

void QuickTransitionItem::hoverEnterEvent(QGraphicsSceneHoverEvent *e)
{
    Q_UNUSED(e)
    m_brush.setColor(QColor(0xff, 0xc4, 0xff));
    update();
}

void QuickTransitionItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)

    painter->save();

    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->setPen(m_pen);
    painter->setBrush(m_brush);
    painter->drawRect(m_rect);

    if (m_connectionType == UnknownType) {
        QPointF endPoint = m_drawingRect.topRight();
        painter->drawLine(m_drawingRect.bottomLeft(), endPoint);
        painter->drawLine(endPoint, endPoint + QPointF(-5, 0));
        painter->drawLine(endPoint, endPoint + QPointF(0, 5));
    } else if (m_connectionType >= FinalStateType) {
        switch (m_connectionType) {
        case StateType:
            painter->setPen(m_pen);
            painter->setBrush(Qt::white);
            painter->drawRoundedRect(m_stateRect, 2, 2);
            break;

        case ParallelType:
            painter->setPen(m_pen);
            painter->setBrush(Qt::white);
            painter->drawRoundedRect(m_stateRect, 2, 2);

            painter->setPen(m_pen);
            painter->drawLine(QPointF(m_stateRect.left() + CAPX, m_stateRect.center().y()), QPointF(m_stateRect.right() - CAPX, m_stateRect.center().y()));
            painter->drawLine(QPointF(m_stateRect.center().x(), m_stateRect.top() + CAPY), QPointF(m_stateRect.center().x(), m_stateRect.bottom() - CAPY));
            painter->drawLine(QPointF(m_stateRect.right() - CAPX, m_stateRect.top() + CAPY), QPointF(m_stateRect.center().x(), m_stateRect.top() + CAPY));
            painter->drawLine(QPointF(m_stateRect.right() - CAPX, m_stateRect.bottom() - CAPY), QPointF(m_stateRect.center().x(), m_stateRect.bottom() - CAPY));
            break;

        case FinalStateType:
            painter->setPen(m_pen);
            painter->setBrush(Qt::white);
            painter->drawEllipse(m_stateRect.center(), 7, 7);

            painter->setPen(Qt::NoPen);
            painter->setBrush(Qt::black);
            painter->drawEllipse(m_stateRect.center(), 5, 5);
            break;

        case HistoryType:
            painter->setFont(QFont("Arial", 6));
            painter->setPen(m_pen);
            painter->setBrush(Qt::white);
            painter->drawEllipse(m_stateRect.center(), 7, 7);
            painter->drawText(m_stateRect, Qt::AlignCenter, Tr::tr("H"));
            break;
        default:
            break;
        }
    }

    painter->restore();
}

#include "moc_quicktransitionitem.cpp"
