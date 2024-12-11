#ifndef ANCHORITEMDELEGATE_H
#define ANCHORITEMDELEGATE_H

#include <QStyledItemDelegate>

#define ICON_MARGIN  4

class AnchorItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:

    explicit AnchorItemDelegate(QObject *parent = nullptr);

    QString anchorAt(QString html, const QPoint &point, const QRect &rect, const QModelIndex &index) const;

    void setHoverRow(int newHoverRow);

protected:
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;
    //void initStyleOption(QStyleOptionViewItem *option, const QModelIndex &index) const;
private:
    int m_hoverRow;
};

#endif // ANCHORITEMDELEGATE_H
