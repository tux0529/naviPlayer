/**
 * @file    settingspage.h
 * @brief   设置页：服务器、外观、歌词、缓存、显示
 */
#ifndef NAVIPLAYER_SETTINGSPAGE_H
#define NAVIPLAYER_SETTINGSPAGE_H

#include "widgets/pagebase.h"

#include <QWidget>

namespace Ui {
class SettingsPage;
}

class SettingsPage : public PageBase
{
    Q_OBJECT

public:
    explicit SettingsPage(QWidget *parent = nullptr);
    ~SettingsPage();

    void refresh() override;
    void retranslate() override;

signals:
    void themeChanged(bool dark);
    void languageChanged(const QString &language);
    /** 请求进入/退出"拖动调整歌词窗口位置"模式 */
    void lyricsPositionAdjustRequested(bool enabled);

public slots:
    /** 拖动完成后由主窗口回调，把按钮恢复成未按下 */
    void setLyricsPositionAdjusting(bool enabled);

private slots:
    void onTestConnection();
    void onBrowseLyricsDir();
    void onSaveClicked();
    void onLyricsPositionToggled(bool checked);

private:
    void loadFromSettings();
    void saveToSettings();
    void connectAutoSave();

    Ui::SettingsPage *ui;
    bool m_loading = false;
};

#endif // NAVIPLAYER_SETTINGSPAGE_H
