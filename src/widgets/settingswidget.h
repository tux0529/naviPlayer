#ifndef SETTINGSWIDGET_H
#define SETTINGSWIDGET_H

#include <QWidget>

class QLineEdit;
class QPushButton;
class QLabel;
class QSpinBox;
class QComboBox;
class QHBoxLayout;

class SettingsWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SettingsWidget(QWidget *parent = nullptr);

    void setServerUrl(const QString &url);
    void setUsername(const QString &username);
    void setPassword(const QString &password);
    void setCacheSizeMb(int sizeMb);
    void setCacheTtlMinutes(int minutes);
    void setAlbumPageSize(int pageSize);
    void setCurrentCacheSize(qint64 bytes);
    void setSongCacheDirectory(const QString &directory);

    QString serverUrl() const;
    QString username() const;
    QString password() const;
    int cacheSizeMb() const;
    int cacheTtlMinutes() const;
    int albumPageSize() const;
    QString songCacheDirectory() const;

signals:
    void saveRequested(const QString &serverUrl, const QString &username, const QString &password,
                       int cacheSizeMb, int cacheTtlMinutes, int albumPageSize, const QString &cacheDirectory);

private slots:
    void onSaveClicked();
    void onTestConnectionClicked();
    void onBrowseCacheDirectory();

private:
    void setupUI();

    QLineEdit *m_serverUrlEdit;
    QLineEdit *m_usernameEdit;
    QLineEdit *m_passwordEdit;
    QSpinBox *m_cacheSizeSpinBox;
    QSpinBox *m_cacheTtlSpinBox;
    QComboBox *m_albumPageSizeComboBox;
    QLineEdit *m_cacheDirectoryEdit;
    QLabel *m_cacheSizeHintLabel;
    QPushButton *m_testButton;
    QPushButton *m_saveButton;
    QPushButton *m_browseCacheDirectoryButton;
};

#endif // SETTINGSWIDGET_H
