#include "settingswidget.h"

#include "../subsonicapi.h"

#include <QEventLoop>
#include <QComboBox>
#include <QFileDialog>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QSpinBox>
#include <QTimer>
#include <QVBoxLayout>
#include <QDir>

namespace {
QString humanReadableSize(qint64 bytes)
{
    const double mb = bytes / (1024.0 * 1024.0);
    if (mb >= 1024.0) {
        return QString::number(mb / 1024.0, 'f', 1) + " GB";
    }
    return QString::number(mb, 'f', 1) + " MB";
}
}

SettingsWidget::SettingsWidget(QWidget *parent)
    : QWidget(parent)
    , m_serverUrlEdit(nullptr)
    , m_usernameEdit(nullptr)
    , m_passwordEdit(nullptr)
    , m_cacheSizeSpinBox(nullptr)
    , m_cacheTtlSpinBox(nullptr)
    , m_albumPageSizeComboBox(nullptr)
    , m_cacheDirectoryEdit(nullptr)
    , m_cacheSizeHintLabel(nullptr)
    , m_testButton(nullptr)
    , m_saveButton(nullptr)
    , m_browseCacheDirectoryButton(nullptr)
{
    setupUI();
}

void SettingsWidget::setupUI()
{
    setStyleSheet(R"(
        QWidget {
            background-color: #1e1e1e;
            color: #ffffff;
        }
        QLabel {
            color: #ffffff;
            font-size: 13px;
        }
        QLineEdit, QSpinBox {
            background-color: #2b2b2b;
            color: #ffffff;
            border: 1px solid #3d3d3d;
            border-radius: 4px;
            padding: 8px;
            font-size: 13px;
        }
        QPushButton {
            background-color: #1db954;
            color: #ffffff;
            border: none;
            border-radius: 4px;
            padding: 9px 16px;
            font-size: 13px;
            font-weight: bold;
        }
        QPushButton:hover {
            background-color: #1ed760;
        }
        QPushButton:pressed {
            background-color: #1aa34a;
        }
        QPushButton#secondaryButton {
            background-color: #3d3d3d;
        }
        QPushButton#secondaryButton:hover {
            background-color: #4d4d4d;
        }
    )");

    QVBoxLayout *rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(24, 24, 24, 24);
    rootLayout->setSpacing(16);

    QLabel *titleLabel = new QLabel("设置", this);
    QFont titleFont = titleLabel->font();
    titleFont.setPointSize(24);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);

    QLabel *subtitleLabel = new QLabel("配置 Subsonic 服务器、本地歌曲缓存和缓存TTL。", this);
    subtitleLabel->setStyleSheet("color: #bbbbbb;");

    QWidget *formCard = new QWidget(this);
    formCard->setStyleSheet("background-color: #252525; border-radius: 8px;");
    QVBoxLayout *cardLayout = new QVBoxLayout(formCard);
    cardLayout->setContentsMargins(20, 20, 20, 20);
    cardLayout->setSpacing(16);

    QFormLayout *formLayout = new QFormLayout();
    formLayout->setLabelAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    formLayout->setFormAlignment(Qt::AlignTop);
    formLayout->setHorizontalSpacing(16);
    formLayout->setVerticalSpacing(12);

    m_serverUrlEdit = new QLineEdit(formCard);
    m_serverUrlEdit->setPlaceholderText("http://localhost:4533");
    formLayout->addRow("服务器地址", m_serverUrlEdit);

    m_usernameEdit = new QLineEdit(formCard);
    formLayout->addRow("用户名", m_usernameEdit);

    m_passwordEdit = new QLineEdit(formCard);
    m_passwordEdit->setEchoMode(QLineEdit::Password);
    formLayout->addRow("密码", m_passwordEdit);

    m_cacheSizeSpinBox = new QSpinBox(formCard);
    m_cacheSizeSpinBox->setRange(0, 10240);
    m_cacheSizeSpinBox->setSuffix(" MB");
    m_cacheSizeSpinBox->setSingleStep(128);
    m_cacheSizeSpinBox->setToolTip("设置为 0 表示禁用本地歌曲缓存。");
    formLayout->addRow("歌曲缓存上限", m_cacheSizeSpinBox);

    m_cacheTtlSpinBox = new QSpinBox(formCard);
    m_cacheTtlSpinBox->setRange(1, 1440);
    m_cacheTtlSpinBox->setSuffix(" 分钟");
    m_cacheTtlSpinBox->setSingleStep(1);
    m_cacheTtlSpinBox->setValue(10);
    m_cacheTtlSpinBox->setToolTip("控制页面数据缓存的有效期；命中缓存时会优先秒开，再按缓存TTL在后台静默刷新。");
    formLayout->addRow("缓存TTL", m_cacheTtlSpinBox);

    m_albumPageSizeComboBox = new QComboBox(formCard);
    m_albumPageSizeComboBox->addItem("10 张", 10);
    m_albumPageSizeComboBox->addItem("20 张", 20);
    m_albumPageSizeComboBox->addItem("24 张", 24);
    m_albumPageSizeComboBox->addItem("30 张", 30);
    m_albumPageSizeComboBox->addItem("50 张", 50);
    m_albumPageSizeComboBox->setCurrentIndex(2);
    m_albumPageSizeComboBox->setStyleSheet(R"(
        QComboBox {
            background-color: #2b2b2b;
            color: #ffffff;
            border: 1px solid #3d3d3d;
            border-radius: 4px;
            padding: 8px;
            font-size: 13px;
        }
        QComboBox QAbstractItemView {
            background-color: #2b2b2b;
            color: #ffffff;
            selection-background-color: #3d3d3d;
        }
    )");
    m_albumPageSizeComboBox->setToolTip("控制专辑列表每页拉取与渲染的专辑数量，数量越少切页越快。");
    formLayout->addRow("每页展示专辑数量", m_albumPageSizeComboBox);

    QWidget *cachePathRow = new QWidget(formCard);
    QHBoxLayout *cachePathLayout = new QHBoxLayout(cachePathRow);
    cachePathLayout->setContentsMargins(0, 0, 0, 0);
    cachePathLayout->setSpacing(8);

    m_cacheDirectoryEdit = new QLineEdit(cachePathRow);
    m_cacheDirectoryEdit->setPlaceholderText("默认路径（建议留空）");
    m_browseCacheDirectoryButton = new QPushButton("选择...", cachePathRow);
    m_browseCacheDirectoryButton->setObjectName("secondaryButton");
    m_browseCacheDirectoryButton->setFixedWidth(84);

    cachePathLayout->addWidget(m_cacheDirectoryEdit);
    cachePathLayout->addWidget(m_browseCacheDirectoryButton);

    formLayout->addRow("歌曲缓存路径", cachePathRow);

    m_cacheSizeHintLabel = new QLabel("当前本地歌曲缓存占用: 0 MB", formCard);
    m_cacheSizeHintLabel->setStyleSheet("color: #aaaaaa;");

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    m_testButton = new QPushButton("测试连接", formCard);
    m_testButton->setObjectName("secondaryButton");
    m_saveButton = new QPushButton("保存设置", formCard);
    buttonLayout->addWidget(m_testButton);
    buttonLayout->addStretch();
    buttonLayout->addWidget(m_saveButton);

    connect(m_testButton, &QPushButton::clicked, this, &SettingsWidget::onTestConnectionClicked);
    connect(m_saveButton, &QPushButton::clicked, this, &SettingsWidget::onSaveClicked);
    connect(m_browseCacheDirectoryButton, &QPushButton::clicked, this, &SettingsWidget::onBrowseCacheDirectory);

    cardLayout->addLayout(formLayout);
    cardLayout->addWidget(m_cacheSizeHintLabel);
    cardLayout->addLayout(buttonLayout);

    rootLayout->addWidget(titleLabel);
    rootLayout->addWidget(subtitleLabel);
    rootLayout->addWidget(formCard);
    rootLayout->addStretch();
}

void SettingsWidget::setServerUrl(const QString &url)
{
    m_serverUrlEdit->setText(url);
}

void SettingsWidget::setUsername(const QString &username)
{
    m_usernameEdit->setText(username);
}

void SettingsWidget::setPassword(const QString &password)
{
    m_passwordEdit->setText(password);
}

void SettingsWidget::setCacheSizeMb(int sizeMb)
{
    m_cacheSizeSpinBox->setValue(sizeMb);
}

void SettingsWidget::setCacheTtlMinutes(int minutes)
{
    m_cacheTtlSpinBox->setValue(minutes);
}

void SettingsWidget::setAlbumPageSize(int pageSize)
{
    const int index = m_albumPageSizeComboBox->findData(pageSize);
    if (index >= 0) {
        m_albumPageSizeComboBox->setCurrentIndex(index);
    }
}

void SettingsWidget::setCurrentCacheSize(qint64 bytes)
{
    m_cacheSizeHintLabel->setText(QString("当前本地歌曲缓存占用: %1").arg(humanReadableSize(bytes)));
}

void SettingsWidget::setSongCacheDirectory(const QString &directory)
{
    m_cacheDirectoryEdit->setText(directory);
}

QString SettingsWidget::serverUrl() const
{
    return m_serverUrlEdit->text().trimmed();
}

QString SettingsWidget::username() const
{
    return m_usernameEdit->text().trimmed();
}

QString SettingsWidget::password() const
{
    return m_passwordEdit->text();
}

int SettingsWidget::cacheSizeMb() const
{
    return m_cacheSizeSpinBox->value();
}

int SettingsWidget::cacheTtlMinutes() const
{
    return m_cacheTtlSpinBox->value();
}

int SettingsWidget::albumPageSize() const
{
    return m_albumPageSizeComboBox->currentData().toInt();
}

QString SettingsWidget::songCacheDirectory() const
{
    return m_cacheDirectoryEdit->text().trimmed();
}

void SettingsWidget::onSaveClicked()
{
    if (serverUrl().isEmpty() || username().isEmpty() || password().isEmpty()) {
        QMessageBox::warning(this, "提示", "请先填写服务器地址、用户名和密码。");
        return;
    }

    const QString cacheDir = songCacheDirectory();
    if (!cacheDir.isEmpty()) {
        QDir dir(cacheDir);
        if (!dir.exists()) {
            if (!dir.mkpath(".")) {
                QMessageBox::warning(this, "提示", "歌曲缓存路径不可用，无法创建目录。");
                return;
            }
        }
    }

    emit saveRequested(serverUrl(), username(), password(),
                       cacheSizeMb(), cacheTtlMinutes(), albumPageSize(), cacheDir);
}

void SettingsWidget::onTestConnectionClicked()
{
    if (serverUrl().isEmpty() || username().isEmpty() || password().isEmpty()) {
        QMessageBox::warning(this, "提示", "请先填写服务器地址、用户名和密码。");
        return;
    }

    m_testButton->setEnabled(false);
    m_testButton->setText("测试中...");

    SubsonicApi api;
    api.setServerUrl(serverUrl());
    api.setUsername(username());
    api.setPassword(password());

    QEventLoop loop;
    QTimer timeoutTimer;
    timeoutTimer.setSingleShot(true);

    bool success = false;
    QString errorMessage;

    connect(&api, &SubsonicApi::pingFinished, this, [&](bool ok, const QString &error) {
        success = ok;
        errorMessage = error;
        loop.quit();
    });
    connect(&api, &SubsonicApi::errorOccurred, this, [&](const QString &error) {
        success = false;
        errorMessage = error;
        loop.quit();
    });
    connect(&timeoutTimer, &QTimer::timeout, this, [&]() {
        success = false;
        errorMessage = "连接超时";
        loop.quit();
    });

    timeoutTimer.start(8000);
    api.ping();
    loop.exec();

    m_testButton->setEnabled(true);
    m_testButton->setText("测试连接");

    if (success) {
        QMessageBox::information(this, "连接成功", "服务器连接正常。");
    } else {
        QMessageBox::warning(this, "连接失败", QString("无法连接到服务器: %1").arg(errorMessage));
    }
}

void SettingsWidget::onBrowseCacheDirectory()
{
    const QString baseDir = songCacheDirectory().isEmpty() ? QDir::homePath() : songCacheDirectory();
    const QString directory = QFileDialog::getExistingDirectory(this, "选择歌曲缓存路径", baseDir);
    if (!directory.isEmpty()) {
        m_cacheDirectoryEdit->setText(QDir::cleanPath(directory));
    }
}
