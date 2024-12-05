#ifndef SERVERMANAGEWIDGET_H
#define SERVERMANAGEWIDGET_H

#include <QDialog>

namespace Ui {
class ServerManageWidget;
}

class ServerManageWidget : public QDialog
{
    Q_OBJECT

public:
    explicit ServerManageWidget(QWidget *parent = nullptr);
    ~ServerManageWidget();

protected:
    bool eventFilter(QObject *obj, QEvent *evt);

private slots:
    void showAddFrame();

    void onAddServer();

    void onAccept();

    void hideAddFrame();

private:
    Ui::ServerManageWidget *ui;
    void initForm();

};

#endif // SERVERMANAGEWIDGET_H
