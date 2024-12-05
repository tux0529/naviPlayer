#include "tracktableview.h"

TrackTableView::TrackTableView(QWidget *parent) :
    AnchorTableView(parent){}

void TrackTableView::resizeEvent(QResizeEvent *event)
{

    Q_UNUSED(event)

    int w = this->width();

    int col1 = 40;
    int sCol = w*0.1;
    //width of albums title column
    int trkCol = ( w - sCol -col1 - 60)/2;

    this->setColumnWidth(0, col1);
    this->setColumnWidth(1, trkCol);
    this->setColumnWidth(2, trkCol);
    this->setColumnWidth(3, sCol);
}
