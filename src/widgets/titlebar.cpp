#include "titlebar.h"
#include "ui_titlebar.h"
#include "mainwindow.h"
#include <QDebug>


TitleBar::TitleBar(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::TitleBar)
{
    ui->setupUi(this);
    setFixedHeight(52);
}

TitleBar::~TitleBar()
{
    delete ui;
}

bool TitleBar::eventFilter(QObject *obj, QEvent *event) {
#ifndef Q_OS_OSX
    switch (event->type()) {
    case QEvent::WindowIconChange: {
        qWarning() << "WindowIconChanged";
        ui->iconButton->setIcon(static_cast<MainWindow*>(obj)->windowIcon());
        break;
    }
    case QEvent::WindowTitleChange: {
        qWarning() << "WindowTitleChanged";
        ui->titleLabel->setText( static_cast<MainWindow*>(obj)->windowTitle());
        break;
    }
    default:
        break;
    }
#endif
    return QWidget::eventFilter(obj, event);
}
