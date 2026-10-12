/**
 * @file    songdetailsdialog.h
 * @brief   歌曲详情弹窗
 */
#ifndef NAVIPLAYER_SONGDETAILSDIALOG_H
#define NAVIPLAYER_SONGDETAILSDIALOG_H

#include <QDialog>

#include "models/song.h"

class SongDetailsDialog : public QDialog
{
    Q_OBJECT
public:
    explicit SongDetailsDialog(const Song &song, QWidget *parent = nullptr);
};

#endif // NAVIPLAYER_SONGDETAILSDIALOG_H
