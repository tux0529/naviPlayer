/**
 * @file    homepage.h
 * @brief   主页：最近添加 / 播放最多 / 最近播放 / 随机 四类专辑，每类 20 张
 *
 * 启动时优先显示本地缓存（页面缓存 + 本地播放历史），后台更新后刷新。
 */
#ifndef NAVIPLAYER_HOMEPAGE_H
#define NAVIPLAYER_HOMEPAGE_H

#include "widgets/pagebase.h"

#include <QVector>

#include "models/album.h"
#include "models/artist.h"

class QLabel;
class QVBoxLayout;
class QScrollArea;
class AlbumRowScroller;

class HomePage : public PageBase
{
    Q_OBJECT
public:
    explicit HomePage(QWidget *parent = nullptr);

    void refresh() override;
    void retranslate() override;

signals:
    void albumClicked(const Album &album);
    void artistClicked(const Artist &artist);
    void playAlbumRequested(const Album &album);

private slots:
    void onAlbumListReady(const QString &type, int offset, const QVector<Album> &albums);

private:
    struct HomeSection {
        QString key;            // newest / frequent / recent / random
        QString title;
        AlbumRowScroller *row = nullptr;
        QLabel *loadingLabel = nullptr;
    };

    void buildSection(const QString &key, const QString &title);
    /** 本地兜底数据（recent / frequent 优先用本地播放历史） */
    void showSectionFromLocal(const HomeSection &section);
    void fetchSection(const HomeSection &section);
    void applyTtlCache();

    QWidget *m_content = nullptr;
    QVBoxLayout *m_layout = nullptr;
    QVector<HomeSection> m_sections;
    int m_generation = 0;           // 请求代次，忽略过期响应
    int m_requestGeneration = -1;
};

#endif // NAVIPLAYER_HOMEPAGE_H
