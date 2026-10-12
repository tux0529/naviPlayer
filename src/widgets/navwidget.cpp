#include "navwidget.h"
#include "ui_navwidget.h"

#include <QFile>
#include <QPushButton>

NavWidget::NavWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::NavWidget)
{
    ui->setupUi(this);

    //设置左侧导航按钮
    const QList<QPushButton *> btns = findChildren<QPushButton *>();
    for (QPushButton *btn : btns) {
        btn->setCheckable(true);
        setButtonIcon(btn);
        connect(btn, &QPushButton::clicked, this, &NavWidget::buttonClick);
    }
}

NavWidget::~NavWidget()
{
    delete ui;
}

void NavWidget::applyTheme(bool dark)
{
    if (m_dark == dark)
        return;
    m_dark = dark;
    const QList<QPushButton *> btns = findChildren<QPushButton *>();
    for (QPushButton *btn : btns)
        setButtonIcon(btn);
}

void NavWidget::setButtonIcon(QPushButton *btn)
{
    const QString theme = m_dark ? QStringLiteral("dark") : QStringLiteral("light");
    QString path = QStringLiteral(":/theme/%1/nav_%2.svg").arg(theme, btn->objectName());
    if (!QFile::exists(path))
        path = QStringLiteral(":/theme/dark/nav_%1.svg").arg(btn->objectName());

    QIcon icon;
    icon.addFile(path);
    btn->setIcon(icon);
}

void NavWidget::buttonClick()
{
    QPushButton *b = qobject_cast<QPushButton *>(sender());
    if (!b)
        return;

    const QList<QPushButton *> btns = findChildren<QPushButton *>();
    for (QPushButton *btn : btns)
        btn->setChecked(btn == b);

    Q_EMIT buttonClicked(b);
}
