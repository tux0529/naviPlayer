/**
 * @file    pagebase.h
 * @brief   页面基类：MainWindow 切换页面时调用 refresh()（先显示本地缓存，再后台更新）；
 *          语言切换时调用 retranslate()。
 */
#ifndef PAGEBASE_H
#define PAGEBASE_H

#include <QWidget>

class PageBase : public QWidget
{
    Q_OBJECT
public:
    explicit PageBase(QWidget *parent = nullptr)
        : QWidget(parent)
    {
        // 允许 QSS 为页面绘制背景色
        setAttribute(Qt::WA_StyledBackground);
    }

    /** 页面显示时刷新数据 */
    virtual void refresh() {}

    /** 界面语言切换后重新翻译静态文本 */
    virtual void retranslate() {}
};


#endif // PAGEBASE_H
