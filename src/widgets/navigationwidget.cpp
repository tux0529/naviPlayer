#include "navigationwidget.h"
#include <QVBoxLayout>
#include <QListWidgetItem>
#include <QIcon>

NavigationWidget::NavigationWidget(QWidget *parent)
    : QWidget(parent)
{
    setupUI();
}

void NavigationWidget::setupUI()
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    m_listWidget = new QListWidget(this);
    m_listWidget->setFrameShape(QFrame::NoFrame);
    m_listWidget->setStyleSheet(R"(
        QListWidget {
            background-color: #2b2b2b;
            color: #ffffff;
            border: none;
            font-size: 14px;
        }
        QListWidget::item {
            padding: 12px 20px;
            border: none;
        }
        QListWidget::item:selected {
            background-color: #3d3d3d;
            color: #ffffff;
        }
        QListWidget::item:hover {
            background-color: #353535;
        }
    )");

    m_listWidget->addItem("🏠  主页");
    m_listWidget->addItem("♥  我的收藏");
    m_listWidget->addItem("📀  专辑");
    m_listWidget->addItem("👤  艺术家");
    m_listWidget->addItem("🔍  搜索");
    m_listWidget->addItem("📋  播放队列");
    m_listWidget->addItem("⚙  设置");

    m_listWidget->item(0)->setData(Qt::UserRole, static_cast<int>(Home));
    m_listWidget->item(1)->setData(Qt::UserRole, static_cast<int>(Favorites));
    m_listWidget->item(2)->setData(Qt::UserRole, static_cast<int>(Albums));
    m_listWidget->item(3)->setData(Qt::UserRole, static_cast<int>(Artists));
    m_listWidget->item(4)->setData(Qt::UserRole, static_cast<int>(Search));
    m_listWidget->item(5)->setData(Qt::UserRole, static_cast<int>(Playlist));
    m_listWidget->item(6)->setData(Qt::UserRole, static_cast<int>(Settings));

    m_listWidget->setCurrentRow(0);

    connect(m_listWidget, &QListWidget::itemClicked,
            this, &NavigationWidget::onItemClicked);

    layout->addWidget(m_listWidget);
}

void NavigationWidget::onItemClicked(QListWidgetItem *item)
{
    NavigationItem navItem = static_cast<NavigationItem>(item->data(Qt::UserRole).toInt());
    emit itemSelected(navItem);
}

void NavigationWidget::setCurrentItem(NavigationItem item)
{
    for (int row = 0; row < m_listWidget->count(); ++row) {
        QListWidgetItem *listItem = m_listWidget->item(row);
        if (!listItem) {
            continue;
        }

        if (listItem->data(Qt::UserRole).toInt() == static_cast<int>(item)) {
            m_listWidget->setCurrentRow(row);
            return;
        }
    }
}
