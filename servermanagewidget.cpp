#include "servermanagewidget.h"
#include "ui_servermanagewidget.h"
#include "iconhelper.h"
#include "config.h"
#include "mediahelper.h"

#include <QEvent>
#include <QMouseEvent>
#include <QListView>

#if (QT_VERSION >= QT_VERSION_CHECK(4,6,0))
#include <QGraphicsDropShadowEffect>
#endif

ServerManageWidget::ServerManageWidget(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ServerManageWidget)
{
    ui->setupUi(this);
    this->installEventFilter(this);
    initForm();
}

ServerManageWidget::~ServerManageWidget()
{
    delete ui;
}

bool ServerManageWidget::eventFilter(QObject *obj, QEvent *evt)
{
    static QPoint mousePoint;
    static bool mousePressed = false;

    switch (evt->type())
    {
    case QEvent::MouseButtonPress:
    {
        QMouseEvent *event = static_cast<QMouseEvent *>(evt);
        if (event->button() == Qt::LeftButton ){
            if (ui->titleWidget->geometry().contains( (event->pos() - static_cast<QWidget *>(ui->titleWidget->parent())->pos() ) )) {
                mousePressed = true;
                mousePoint = event->globalPos() - this->pos();
                return true;
            }
        }
        break;
    }
    case QEvent::MouseButtonRelease:
    {
        QMouseEvent *event = static_cast<QMouseEvent *>(evt);
        if (event->button() == Qt::LeftButton ){
            mousePressed = false;
        }
        break;
    }
    case QEvent::MouseMove:
    {
        QMouseEvent *event = static_cast<QMouseEvent *>(evt);
        if ((!this->isMaximized()) && mousePressed && (event->buttons() == Qt::LeftButton)) {
            this->move(event->globalPos() - mousePoint);
            return true;
        }
        break;
    }
    default:
    {

    }
    }
    return QDialog::eventFilter(obj, evt);
}

void ServerManageWidget::initForm()
{
    //Window
    this->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowSystemMenuHint | Qt::WindowMinMaxButtonsHint);

    setAttribute(Qt::WA_TranslucentBackground);

#if (QT_VERSION >= QT_VERSION_CHECK(4,6,0))
    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(this);
    shadow->setOffset(0,0);
    shadow->setColor(QColor(0,0,0));
    shadow->setBlurRadius(9);
    ui->shadowWidget->setGraphicsEffect(shadow);
#endif

    //Title bar
    ui->titleLabel->setFont(QFont("Microsoft Yahei", 12));

    IconHelper::Instance()->setIcon(ui->closeButton, QChar(0xF00d));

    ui->serverComboBox->setView(new QListView(ui->serverComboBox));

    Config::G_Debug( "Servers number: ", Config::Instance()->servers.size() );

    for (int i =0; i < Config::Instance()->servers.size(); ++i){
        ui->serverComboBox->addItem(Config::Instance()->servers.at(i).name);
    }

    connect( ui->closeButton, &QPushButton::clicked, this, &QDialog::close);

    connect( ui->addButton, &QPushButton::clicked, this, &ServerManageWidget::showAddFrame);

    connect( ui->okButton, &QPushButton::clicked, this, &ServerManageWidget::onAccept);

    connect( ui->cancelButton, &QPushButton::clicked, this, &ServerManageWidget::hideAddFrame);
    connect( ui->submitButton, &QPushButton::clicked, this, &ServerManageWidget::onAddServer);

    hideAddFrame();
}

void ServerManageWidget::showAddFrame()
{
    resize(400,540);
    ui->frame->show();
}

void ServerManageWidget::hideAddFrame()
{
    ui->frame->hide();
    resize(400,270);
}

void ServerManageWidget::onAddServer()
{
    if(ui->nameEdit->text().trimmed().isEmpty()){
        ui->resultLabel->setText(tr("Server Name must fill"));
        return;
    }
    if(ui->urlEdit->text().trimmed().isEmpty()){
        ui->resultLabel->setText(tr("Url must fill"));
        return;
    }
    if(ui->userEdit->text().trimmed().isEmpty()){
        ui->resultLabel->setText(tr("User Name must fill"));
        return;
    }
    if(ui->passwordEdit->text().trimmed().isEmpty()){
        ui->resultLabel->setText(tr("Password must fill"));
        return;
    }

    Config::Server srv;

    srv.useToken = false;
    srv.name = ui->nameEdit->text().trimmed();
    srv.url = ui->urlEdit->text().trimmed();
    srv.user = ui->userEdit->text().trimmed();
    srv.password = ui->passwordEdit->text().trimmed();

    bool ok = MediaHelper::Instance()->ping(srv);

    if (ok) {
        Config::G_Debug("useToken", srv.useToken);

        Config::Instance()->addServer(srv);

        ui->serverComboBox->clear();
        for (int i =0; i < Config::Instance()->servers.size(); ++i){
            ui->serverComboBox->addItem(Config::Instance()->servers.at(i).name);
        }
        ui->serverComboBox->setCurrentText(srv.name);

    }
    else{
        ui->resultLabel->setText(tr("Invalid Server"));
    }

}

void ServerManageWidget::onAccept()
{
    QString serverName = ui->serverComboBox->currentText();

    if(Config::Instance()->setCurrentSever(serverName)){
        Config::G_Debug("ServerManageWidget::onAccept:");
        accept();
    }
    else{
        reject();
    }
}
