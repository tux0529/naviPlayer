#include "anchortableview.h"
#include "anchoritemdelegate.h"

#include <QApplication>
#include <QCursor>
#include <QMouseEvent>
#include <QScrollBar>

#include <QDebug>
#include <QHeaderView>

int AnchorTableView::m_lastRow = -1;

AnchorTableView::AnchorTableView(QWidget *parent) :
    QTableView(parent)
{
    // needed for the hover functionality
    setMouseTracking(true);
    this->installEventFilter(this);

    this->setShowGrid(false);

    this->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    this->horizontalHeader()->setMinimumSectionSize(40);
    this->verticalHeader()->hide();
    this->verticalHeader()->setDefaultSectionSize(56);
    this->setAlternatingRowColors(false);

    this->setSelectionBehavior(QAbstractItemView::SelectRows);
    this->setSelectionMode(QAbstractItemView::ExtendedSelection);
}

bool AnchorTableView::eventFilter(QObject *object, QEvent *event)
{
    if (QEvent::Leave == event->type()){
        auto delegate = qobject_cast<AnchorItemDelegate *>(itemDelegate());
        AnchorTableView::m_lastRow = -1;
        delegate->setHoverRow(-1);
        QApplication::restoreOverrideCursor();
        update();
    }
//    else if (QEvent::Wheel == event->type()) {
//        QWheelEvent *evt = static_cast<QWheelEvent *>(event);
//        this->updateHoverRow(evt->pos() );
//        qDebug() << "QWheelEvent:delta:" << evt->delta();
//        qDebug() << "QWheelEvent:pixelDelta:" << evt->pixelDelta();
//        qDebug() << "QWheelEvent:pos:" << evt->pos();
//        qDebug() << "QWheelEvent:QObject:" << object;
//        //this->mapFromGlobal(QCursor::pos() )
//    }
    return QTableView::eventFilter(object, event);
}

void AnchorTableView::mousePressEvent(QMouseEvent *event)
{

    auto anchor = anchorAt(event->pos());
    m_mousePressAnchor = anchor;

    QTableView::mousePressEvent(event);
}

void AnchorTableView::mouseMoveEvent(QMouseEvent *event)
{
    this->updateHoverRow(event->pos());

    auto anchor = anchorAt(event->pos());

    if (m_mousePressAnchor != anchor) {
        m_mousePressAnchor.clear();
    }

    if (m_lastHoveredAnchor != anchor) {
        m_lastHoveredAnchor = anchor;
        if (!m_lastHoveredAnchor.isEmpty()) {
            QApplication::setOverrideCursor(QCursor(Qt::PointingHandCursor));
            emit linkHovered(m_lastHoveredAnchor);
        } else {
            QApplication::restoreOverrideCursor();
            emit linkUnhovered();
        }
    }

    QTableView::mouseMoveEvent(event);
}

void AnchorTableView::mouseReleaseEvent(QMouseEvent *event)
{
    if (!m_mousePressAnchor.isEmpty()) {
        auto anchor = anchorAt(event->pos());

        if (anchor == m_mousePressAnchor) {
            emit linkActivated(m_mousePressAnchor);
        }

        m_mousePressAnchor.clear();
    }

    QTableView::mouseReleaseEvent(event);
}

QString AnchorTableView::anchorAt(const QPoint &pos) const
{
    auto index = indexAt(pos);
    if (index.isValid()) {
        auto delegate = qobject_cast<AnchorItemDelegate *>(itemDelegate(index));
        if (delegate != 0) {
            auto itemRect = visualRect(index);

            auto html = model()->data(index, Qt::DisplayRole).toString();

            return delegate->anchorAt(html, pos, itemRect, index);
        }
    }

    return QString();
}

void AnchorTableView::updateHoverRow(const QPoint &pos)
{
    auto index = indexAt(pos);
    if (index.isValid()) {
        auto delegate = qobject_cast<AnchorItemDelegate *>(itemDelegate());
        if(AnchorTableView::m_lastRow != index.row()){
            AnchorTableView::m_lastRow = index.row();
            delegate->setHoverRow(index.row());
            //qDebug() << "AnchorTableView::updateHoverRow:" << index.row();
            update();
        }
    }

}
