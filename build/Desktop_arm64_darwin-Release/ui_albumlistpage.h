/********************************************************************************
** Form generated from reading UI file 'albumlistpage.ui'
**
** Created by: Qt User Interface Compiler version 5.15.16
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_ALBUMLISTPAGE_H
#define UI_ALBUMLISTPAGE_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QStackedWidget>
#include <QtWidgets/QToolButton>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_AlbumListPage
{
public:
    QVBoxLayout *verticalLayout;
    QHBoxLayout *horizontalLayout_2;
    QLabel *albumListTitleLabel;
    QSpacerItem *horizontalSpacer;
    QComboBox *comboBox;
    QToolButton *toolButton;
    QStackedWidget *stackedWidget;
    QWidget *page_2;
    QHBoxLayout *horizontalLayout;
    QLabel *label;
    QSpacerItem *horizontalSpacer_2;
    QLabel *pagerLabel;

    void setupUi(QWidget *AlbumListPage)
    {
        if (AlbumListPage->objectName().isEmpty())
            AlbumListPage->setObjectName(QString::fromUtf8("AlbumListPage"));
        AlbumListPage->resize(939, 619);
        verticalLayout = new QVBoxLayout(AlbumListPage);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        albumListTitleLabel = new QLabel(AlbumListPage);
        albumListTitleLabel->setObjectName(QString::fromUtf8("albumListTitleLabel"));
        albumListTitleLabel->setStyleSheet(QString::fromUtf8("font: 36pt \"Arial\";\n"
"font: 36pt \"Hei\";\n"
"font: 36pt \"Heiti SC\";"));

        horizontalLayout_2->addWidget(albumListTitleLabel);

        horizontalSpacer = new QSpacerItem(623, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_2->addItem(horizontalSpacer);

        comboBox = new QComboBox(AlbumListPage);
        comboBox->addItem(QString());
        comboBox->addItem(QString());
        comboBox->addItem(QString());
        comboBox->addItem(QString());
        comboBox->setObjectName(QString::fromUtf8("comboBox"));
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(comboBox->sizePolicy().hasHeightForWidth());
        comboBox->setSizePolicy(sizePolicy);
        comboBox->setMinimumSize(QSize(180, 28));

        horizontalLayout_2->addWidget(comboBox);

        toolButton = new QToolButton(AlbumListPage);
        toolButton->setObjectName(QString::fromUtf8("toolButton"));

        horizontalLayout_2->addWidget(toolButton);


        verticalLayout->addLayout(horizontalLayout_2);

        stackedWidget = new QStackedWidget(AlbumListPage);
        stackedWidget->setObjectName(QString::fromUtf8("stackedWidget"));
        QFont font;
        font.setFamily(QString::fromUtf8(".AppleSystemUIFont"));
        stackedWidget->setFont(font);
        page_2 = new QWidget();
        page_2->setObjectName(QString::fromUtf8("page_2"));
        stackedWidget->addWidget(page_2);

        verticalLayout->addWidget(stackedWidget);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        label = new QLabel(AlbumListPage);
        label->setObjectName(QString::fromUtf8("label"));

        horizontalLayout->addWidget(label);

        horizontalSpacer_2 = new QSpacerItem(758, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer_2);

        pagerLabel = new QLabel(AlbumListPage);
        pagerLabel->setObjectName(QString::fromUtf8("pagerLabel"));

        horizontalLayout->addWidget(pagerLabel);


        verticalLayout->addLayout(horizontalLayout);


        retranslateUi(AlbumListPage);

        stackedWidget->setCurrentIndex(0);


        QMetaObject::connectSlotsByName(AlbumListPage);
    } // setupUi

    void retranslateUi(QWidget *AlbumListPage)
    {
        AlbumListPage->setWindowTitle(QCoreApplication::translate("AlbumListPage", "Form", nullptr));
        albumListTitleLabel->setText(QCoreApplication::translate("AlbumListPage", "Albums", nullptr));
        comboBox->setItemText(0, QCoreApplication::translate("AlbumListPage", "Recent Added", nullptr));
        comboBox->setItemText(1, QCoreApplication::translate("AlbumListPage", "Most Played", nullptr));
        comboBox->setItemText(2, QCoreApplication::translate("AlbumListPage", "Recent Played", nullptr));
        comboBox->setItemText(3, QCoreApplication::translate("AlbumListPage", "Random", nullptr));

        toolButton->setText(QCoreApplication::translate("AlbumListPage", "...", nullptr));
        label->setText(QCoreApplication::translate("AlbumListPage", "1-50", nullptr));
        pagerLabel->setText(QCoreApplication::translate("AlbumListPage", "1 2 3 4 5 ... 10", nullptr));
    } // retranslateUi

};

namespace Ui {
    class AlbumListPage: public Ui_AlbumListPage {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_ALBUMLISTPAGE_H
