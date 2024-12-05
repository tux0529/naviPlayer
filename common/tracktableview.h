#ifndef TRACKTABLEVIEW_H
#define TRACKTABLEVIEW_H

#include "anchortableview.h"

class TrackTableView : public AnchorTableView
{
    Q_OBJECT
public:
    TrackTableView(QWidget *parent = nullptr);

protected:
    void resizeEvent(QResizeEvent *event);
};

#endif // TRACKTABLEVIEW_H
