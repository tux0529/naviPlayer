#include "settingspage.h"
#include "ui_settingspage.h"

#include "core/appcontext.h"
#include "core/appsettings.h"
#include "core/subsonicapi.h"

#include <QFileDialog>
#include <QColorDialog>
#include <QMessageBox>
#include <QSpinBox>
#include <QLineEdit>
#include <QCheckBox>
#include <QComboBox>
#include <QToolButton>

SettingsPage::SettingsPage(QWidget *parent)
    : PageBase(parent)
    , ui(new Ui::SettingsPage)
{
    ui->setupUi(this);

    // 主题
    ui->themeCombo->addItem(QStringLiteral("深色"), static_cast<int>(AppSettings::Theme::Dark));
    ui->themeCombo->addItem(QStringLiteral("浅色"), static_cast<int>(AppSettings::Theme::Light));

    // 语言
    ui->languageCombo->addItem(QStringLiteral("跟随系统"), QStringLiteral("system"));
    ui->languageCombo->addItem(QStringLiteral("简体中文"), QStringLiteral("zh_CN"));
    ui->languageCombo->addItem(QStringLiteral("English"), QStringLiteral("en_US"));

    ui->passwordEdit->setEchoMode(QLineEdit::Password);

    loadFromSettings();
    connectAutoSave();

    connect(ui->testConnectionButton, &QPushButton::clicked, this, &SettingsPage::onTestConnection);
    connect(ui->lyricsDirButton, &QToolButton::clicked, this, &SettingsPage::onBrowseLyricsDir);
    connect(ui->lyricsPositionButton, &QPushButton::toggled, this,
            &SettingsPage::onLyricsPositionToggled);

    AppContext *ctx = AppContext::instance();
    if (ctx && ctx->api()) {
        connect(ctx->api(), &SubsonicApi::pingResult, this, [this](bool ok, const QString &message) {
            if (ok) {
                ui->serverTestInfo->setText(QStringLiteral("连接成功（API %1）").arg(message));
                ui->serverTestInfo->setStyleSheet(QStringLiteral("color: #1DB954;"));
            } else {
                ui->serverTestInfo->setText(QStringLiteral("连接失败：%1").arg(message));
                ui->serverTestInfo->setStyleSheet(QStringLiteral("color: #E0245E;"));
            }
        });
        connect(ctx->api(), &SubsonicApi::errorOccurred, this, [this](const QString &message) {
            ui->serverTestInfo->setText(QStringLiteral("错误：%1").arg(message));
            ui->serverTestInfo->setStyleSheet(QStringLiteral("color: #E0245E;"));
        });
    }
}

SettingsPage::~SettingsPage()
{
    delete ui;
}

void SettingsPage::connectAutoSave()
{
    auto onChanged = [this]() {
        if (!m_loading)
            saveToSettings();
    };

    connect(ui->serverEdit, &QLineEdit::editingFinished, this, onChanged);
    connect(ui->usernameEdit, &QLineEdit::editingFinished, this, onChanged);
    connect(ui->passwordEdit, &QLineEdit::editingFinished, this, onChanged);
    connect(ui->ignoreSslCheck, &QCheckBox::toggled, this, onChanged);
    connect(ui->lyricsDirEdit, &QLineEdit::editingFinished, this, onChanged);
    connect(ui->lyricsFontEdit, &QLineEdit::editingFinished, this, onChanged);
    connect(ui->lyricsHighlightEdit, &QLineEdit::editingFinished, this, onChanged);
    connect(ui->lyricsNormalEdit, &QLineEdit::editingFinished, this, onChanged);
    connect(ui->lyricsEnabledCheck, &QCheckBox::toggled, this, onChanged);
    connect(ui->songCacheEdit, &QLineEdit::editingFinished, this, onChanged);
    connect(ui->pageTtlEdit, &QLineEdit::editingFinished, this, onChanged);
    connect(ui->albumsPerPageEdit, &QLineEdit::editingFinished, this, onChanged);
    connect(ui->songsPerPageEdit, &QLineEdit::editingFinished, this, onChanged);

    connect(ui->lyricsFontSizeEdit, &QLineEdit::editingFinished, this, onChanged);

    connect(ui->themeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            [this](int) {
                if (m_loading)
                    return;
                saveToSettings();
            });
    connect(ui->languageCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            [this](int) {
                if (m_loading)
                    return;
                saveToSettings();
            });
}

void SettingsPage::loadFromSettings()
{
    AppSettings *settings = AppContext::instance() ? AppContext::instance()->settings() : nullptr;
    if (!settings)
        return;

    m_loading = true;

    ui->serverEdit->setText(settings->serverUrl());
    ui->usernameEdit->setText(settings->username());
    ui->passwordEdit->setText(settings->password());
    ui->ignoreSslCheck->setChecked(settings->ignoreSslErrors());

    ui->themeCombo->setCurrentIndex(settings->theme() == AppSettings::Theme::Light ? 1 : 0);
    const QString language = settings->language();
    const int langIndex = ui->languageCombo->findData(language);
    ui->languageCombo->setCurrentIndex(langIndex >= 0 ? langIndex : 0);

    ui->lyricsDirEdit->setText(settings->lyricsDirectory());
    ui->lyricsFontEdit->setText(settings->lyricsFontFamily());
    ui->lyricsFontSizeEdit->setText(QString::number(settings->lyricsFontSize()));
    ui->lyricsHighlightEdit->setText(settings->lyricsHighlightColor());
    ui->lyricsNormalEdit->setText(settings->lyricsNormalColor());
    ui->lyricsEnabledCheck->setChecked(settings->lyricsEnabled());

    ui->songCacheEdit->setText(QString::number(settings->maxSongCacheMb()));
    ui->pageTtlEdit->setText(QString::number(settings->pageCacheTtlMinutes()));
    ui->albumsPerPageEdit->setText(QString::number(settings->albumsPerPage()));
    ui->songsPerPageEdit->setText(QString::number(settings->songsPerPage()));

    m_loading = false;
}

void SettingsPage::saveToSettings()
{
    AppSettings *settings = AppContext::instance() ? AppContext::instance()->settings() : nullptr;
    if (!settings)
        return;

    settings->setServerUrl(ui->serverEdit->text().trimmed());
    settings->setUsername(ui->usernameEdit->text().trimmed());
    settings->setPassword(ui->passwordEdit->text());
    settings->setIgnoreSslErrors(ui->ignoreSslCheck->isChecked());

    settings->setTheme(ui->themeCombo->currentIndex() == 1 ? AppSettings::Theme::Light
                                                           : AppSettings::Theme::Dark);
    settings->setLanguage(ui->languageCombo->currentData().toString());

    settings->setLyricsDirectory(ui->lyricsDirEdit->text().trimmed());
    settings->setLyricsFontFamily(ui->lyricsFontEdit->text().trimmed());
    settings->setLyricsFontSize(ui->lyricsFontSizeEdit->text().toInt());
    settings->setLyricsHighlightColor(ui->lyricsHighlightEdit->text().trimmed());
    settings->setLyricsNormalColor(ui->lyricsNormalEdit->text().trimmed());
    settings->setLyricsEnabled(ui->lyricsEnabledCheck->isChecked());

    settings->setMaxSongCacheMb(ui->songCacheEdit->text().toInt());
    settings->setPageCacheTtlMinutes(ui->pageTtlEdit->text().toInt());
    settings->setAlbumsPerPage(ui->albumsPerPageEdit->text().toInt());
    settings->setSongsPerPage(ui->songsPerPageEdit->text().toInt());

    Q_EMIT themeChanged(settings->theme() == AppSettings::Theme::Light ? false : true);
    Q_EMIT languageChanged(settings->language());
}

void SettingsPage::onTestConnection()
{
    saveToSettings();

    AppContext *ctx = AppContext::instance();
    if (!ctx || !ctx->api())
        return;

    ui->serverTestInfo->setText(QStringLiteral("正在连接…"));
    ui->serverTestInfo->setStyleSheet(QString());
    ctx->applyServerSettings();
    ctx->api()->ping();
}

void SettingsPage::onLyricsPositionToggled(bool checked)
{
    Q_EMIT lyricsPositionAdjustRequested(checked);
}

void SettingsPage::setLyricsPositionAdjusting(bool enabled)
{
    if (ui->lyricsPositionButton->isChecked() == enabled)
        return;
    ui->lyricsPositionButton->blockSignals(true);   // 避免回环触发请求
    ui->lyricsPositionButton->setChecked(enabled);
    ui->lyricsPositionButton->blockSignals(false);
}

void SettingsPage::onBrowseLyricsDir()
{
    const QString dir = QFileDialog::getExistingDirectory(this, QStringLiteral("选择本地歌词目录"),
                                                          ui->lyricsDirEdit->text());
    if (dir.isEmpty())
        return;
    ui->lyricsDirEdit->setText(dir);
    saveToSettings();
}

void SettingsPage::onSaveClicked()
{
    saveToSettings();
}

void SettingsPage::refresh()
{
    loadFromSettings();
}

void SettingsPage::retranslate()
{
    loadFromSettings();
}
