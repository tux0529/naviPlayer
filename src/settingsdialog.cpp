#include "settingsdialog.h"
#include "subsonicapi.h"
#include <QEventLoop>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QMessageBox>
#include <QTimer>

SettingsDialog::SettingsDialog(QWidget *parent)
    : QDialog(parent)
{
    setupUI();
}

void SettingsDialog::setupUI()
{
    setWindowTitle("设置 - Subsonic 服务器");
    setMinimumSize(400, 250);
    setStyleSheet(R"(
        QDialog {
            background-color: #1e1e1e;
        }
        QLabel {
            color: #ffffff;
            font-size: 13px;
        }
        QLineEdit {
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
            padding: 8px 16px;
            font-size: 13px;
            font-weight: bold;
        }
        QPushButton:hover {
            background-color: #1ed760;
        }
        QPushButton:pressed {
            background-color: #1aa34a;
        }
        QPushButton#cancelButton {
            background-color: #3d3d3d;
        }
        QPushButton#cancelButton:hover {
            background-color: #4d4d4d;
        }
        QDialogButtonBox {
            button-layout: QDialogButtonBox::Cancel | QDialogButtonBox::Ok;
        }
    )");

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setSpacing(12);
    layout->setContentsMargins(24, 24, 24, 24);

    // Server URL
    QLabel *serverUrlLabel = new QLabel("服务器地址", this);
    m_serverUrlEdit = new QLineEdit(this);
    m_serverUrlEdit->setPlaceholderText("http://localhost:4533");

    // Username
    QLabel *usernameLabel = new QLabel("用户名", this);
    m_usernameEdit = new QLineEdit(this);

    // Password
    QLabel *passwordLabel = new QLabel("密码", this);
    m_passwordEdit = new QLineEdit(this);
    m_passwordEdit->setEchoMode(QLineEdit::Password);

    // Test button
    m_testButton = new QPushButton("测试连接", this);
    connect(m_testButton, &QPushButton::clicked,
            this, &SettingsDialog::onTestConnectionClicked);

    // Buttons
    m_buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    m_buttonBox->button(QDialogButtonBox::Ok)->setText("保存");
    m_buttonBox->button(QDialogButtonBox::Cancel)->setText("取消");
    m_buttonBox->button(QDialogButtonBox::Cancel)->setObjectName("cancelButton");

    connect(m_buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(m_buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    layout->addWidget(serverUrlLabel);
    layout->addWidget(m_serverUrlEdit);
    layout->addWidget(usernameLabel);
    layout->addWidget(m_usernameEdit);
    layout->addWidget(passwordLabel);
    layout->addWidget(m_passwordEdit);
    layout->addWidget(m_testButton);
    layout->addStretch();
    layout->addWidget(m_buttonBox);
}

void SettingsDialog::setServerUrl(const QString &url)
{
    m_serverUrlEdit->setText(url);
}

void SettingsDialog::setUsername(const QString &username)
{
    m_usernameEdit->setText(username);
}

void SettingsDialog::setPassword(const QString &password)
{
    m_passwordEdit->setText(password);
}

QString SettingsDialog::serverUrl() const
{
    return m_serverUrlEdit->text().trimmed();
}

QString SettingsDialog::username() const
{
    return m_usernameEdit->text().trimmed();
}

QString SettingsDialog::password() const
{
    return m_passwordEdit->text();
}

void SettingsDialog::onTestConnectionClicked()
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

    bool finished = false;
    bool success = false;
    QString errorMessage;

    connect(&api, &SubsonicApi::pingFinished, this, [&](bool ok, const QString &error) {
        finished = true;
        success = ok;
        errorMessage = error;
        loop.quit();
    });
    connect(&api, &SubsonicApi::errorOccurred, this, [&](const QString &error) {
        finished = true;
        success = false;
        errorMessage = error;
        loop.quit();
    });
    connect(&timeoutTimer, &QTimer::timeout, this, [&]() {
        finished = true;
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
        if (!finished) {
            errorMessage = "连接失败";
        }
        QMessageBox::warning(this, "连接失败", QString("无法连接到服务器: %1").arg(errorMessage));
    }
}
