#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QDialogButtonBox>
#include <QPushButton>

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget *parent = nullptr);

    void setServerUrl(const QString &url);
    void setUsername(const QString &username);
    void setPassword(const QString &password);

    QString serverUrl() const;
    QString username() const;
    QString password() const;

private slots:
    void onTestConnectionClicked();

private:
    void setupUI();

    QLineEdit *m_serverUrlEdit;
    QLineEdit *m_usernameEdit;
    QLineEdit *m_passwordEdit;
    QPushButton *m_testButton;
    QDialogButtonBox *m_buttonBox;
};

#endif // SETTINGSDIALOG_H
