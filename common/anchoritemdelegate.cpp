#include "anchoritemdelegate.h"
//#include "config.h"

#include <QPainter>
#include <QTextDocument>
#include <QAbstractTextDocumentLayout>
#include <QApplication>
#include <QTableView>
#include <QDebug>
#include <QFlags>


AnchorItemDelegate::AnchorItemDelegate(QObject *parent) :
    QStyledItemDelegate(parent)
{
    m_hoverRow = -1;
}

QString AnchorItemDelegate::anchorAt(QString html, const QPoint &point, const QRect &rect, const QModelIndex &index) const
{
    QTextDocument doc;
    doc.setHtml(html);
    auto textLayout = doc.documentLayout();

    QPoint pos = point - rect.topLeft();
    Qt::Alignment flags = QFlags<Qt::AlignmentFlag>(index.data(Qt::TextAlignmentRole).toInt());
    if (flags.testFlag(Qt::AlignVCenter) ){
        pos -=QPoint(0, (rect.height()-doc.size().height())/2);
    }
    if (index.column() == 1){
        pos -= QPoint(rect.height() +15,0);
    }
    Q_ASSERT(textLayout != 0);
    return textLayout->anchorAt(pos);
}

void AnchorItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    auto options = option;
    initStyleOption(&options, index);

    painter->save();

    QTextDocument doc;
    doc.setHtml(options.text);

    options.text = "";
    const QWidget *widget = options.widget;
    QStyle *style = widget ? widget->style() : QApplication::style();
    style->drawControl(QStyle::CE_ItemViewItem, &options, painter, widget);

    if (m_hoverRow == index.row() ){
        painter->fillRect(options.rect, QColor(Qt::gray));
    }

    if (index.column() == 1){
        painter->translate(options.rect.left(), options.rect.top());
        QIcon icon = options.icon;
        if (icon.isNull()){
            painter->translate(0, (options.rect.height() - doc.size().height())/2);
            doc.drawContents(painter);
        }
        else{
            //Config::G_Debug("AnchorItemDelegate:Height:", options.rect.height());

            //QPixmap img = icon.pixmap(options.rect.height() - ICON_MARGIN * 2, options.rect.height()- ICON_MARGIN*2);
            //painter->drawPixmap(ICON_MARGIN, ICON_MARGIN, img);
            icon.paint(painter, ICON_MARGIN, ICON_MARGIN,
                       options.rect.height() - ICON_MARGIN * 2, options.rect.height()- ICON_MARGIN*2);

            painter->translate(options.rect.left() + 15, (options.rect.height() - doc.size().height())/2);
            QRect clip(0, 0, options.rect.width()-options.rect.height() -10, options.rect.height());
            doc.drawContents(painter, clip);
        }
    }
    else {
        painter->translate(options.rect.left(), options.rect.top() + (options.rect.height() - doc.size().height())/2);
        QRect clip(0, 0, options.rect.width() -10 , options.rect.height());
        doc.drawContents(painter, clip);
    }

    painter->restore();
}

void AnchorItemDelegate::setHoverRow(int newHoverRow)
{
    m_hoverRow = newHoverRow;
}

QSize AnchorItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionViewItem options = option;
    initStyleOption(&options, index);

    QTextDocument doc;
    doc.setHtml(options.text);
    doc.setTextWidth(options.rect.width());
    return QSize(doc.idealWidth(), doc.size().height());
}

