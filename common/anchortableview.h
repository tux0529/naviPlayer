#ifndef ANCHORTABLEVIEW_H
#define ANCHORTABLEVIEW_H

#include <QTableView>

class AnchorTableView : public QTableView
{
    Q_OBJECT
public:
    explicit AnchorTableView(QWidget *parent = nullptr);

signals:
    void linkActivated(const QString &link);
    void linkHovered(const QString &link);
    void linkUnhovered();

protected:
    bool eventFilter(QObject *object, QEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    QString anchorAt(const QPoint &pos) const;
    void updateHoverRow(const QPoint &pos);

private:
    QString m_mousePressAnchor;
    QString m_lastHoveredAnchor;
    static int m_lastRow;
};

#endif // ANCHORTABLEVIEW_H
