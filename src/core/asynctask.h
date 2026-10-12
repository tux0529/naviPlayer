/**
 * @file    asynctask.h
 * @brief   后台任务统一入口：Qt Concurrent + 结果回投主线程
 *
 * 所有阻塞操作（SQLite 批量写、JSON 解析、ffmpeg 探测、缓存淘汰、图片解码）
 * 都应通过 Async::run() 执行，回调保证在 context 所在线程（通常为主线程）执行。
 */
#ifndef NAVIPLAYER_ASYNCTASK_H
#define NAVIPLAYER_ASYNCTASK_H

#include <QtConcurrent/QtConcurrentRun>
#include <QFuture>
#include <QFutureWatcher>
#include <QObject>
#include <utility>

namespace Async {

/**
 * @brief 在线程池执行 work()，完成后把结果投递到 context 所在线程执行 callback(result)
 *
 * @param work    无参可调用对象，必须返回非 void 值
 * @param context 回调上下文；若在其回调前被销毁，回调自动取消
 * @param cb      形如 void(R) 的可调用对象
 */
template <typename Work, typename Callback>
void run(QObject *context, Work &&work, Callback &&cb)
{
    using Result = decltype(work());

    QFuture<Result> future = QtConcurrent::run(std::forward<Work>(work));
    auto *watcher = new QFutureWatcher<Result>();

    QObject::connect(watcher, &QFutureWatcherBase::finished, context,
                     [watcher, cb = std::forward<Callback>(cb)]() {
                         cb(watcher->result());
                         watcher->deleteLater();
                     });

    if (context) {
        QObject::connect(context, &QObject::destroyed, watcher, [watcher]() {
            watcher->deleteLater();
        });
    }

    watcher->setFuture(future);
}

/**
 * @brief 仅后台执行，不关心结果
 */
template <typename Work>
void run(Work &&work)
{
    QtConcurrent::run(std::forward<Work>(work));
}

} // namespace Async

#endif // NAVIPLAYER_ASYNCTASK_H
