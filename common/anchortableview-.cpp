#include "anchortableview.h"
#include "anchoritemdelegate.h"

#include <QApplication>
#include <QCursor>
#include <QMouseEvent>
#include <QDebug>

int AnchorTableView::m_lastRow = -1;

AnchorTableView::AnchorTableView(QWidget *parent) :
    QTableView(parent)
{
    // needed for the hover functionality
    setMouseTracking(true);
    this->installEventFilter(this);
}

bool AnchorTableView::eventFilter(QObject *object, QEvent *event)
{
    if (QEvent::Leave == event->type()){
        qDebug() << "Leave!";
    }
    return QTableView::eventFilter(object, event);
}

void AnchorTableView::mousePressEvent(QMouseEvent *event)
{
    QTableView::mousePressEvent(event);

    auto anchor = anchorAt(event->pos());
    m_mousePressAnchor = anchor;
}

void AnchorTableView::mouseMoveEvent(QMouseEvent *event)
{
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

void AnchorTableView::wheelEvent(QWheelEvent *event)
{
    //auto anchor = anchorAt(event->position().toPoint());

    QTableView::wheelEvent(event);
}

QString AnchorTableView::anchorAt(const QPoint &pos) const
{
    auto index = indexAt(pos);
    if (index.isValid()) {
        auto delegate = qobject_cast<AnchorItemDelegate *>(itemDelegate(index));
        if (delegate != 0) {
            auto itemRect = visualRect(index);

            auto html = model()->data(index, Qt::DisplayRole).toString();

            if(AnchorTableView::m_lastRow != index.row()){
                AnchorTableView::m_lastRow = index.row();
                delegate->setHoverRow(index.row());
                qDebug() << "AnchorTableView::anchorAt:" << index.row();
            }

            return delegate->anchorAt(html, pos, itemRect, index);
        }
    }

    return QString();
}
