#include "songdetailsdialog.h"

#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

namespace {
QString valueOrUnknown(const QString &value)
{
    return value.isEmpty() ? QStringLiteral("未知") : value;
}

QString valueOrUnknown(int value, const QString &unit = QString())
{
    if (value <= 0)
        return QStringLiteral("未知");
    return unit.isEmpty() ? QString::number(value) : QStringLiteral("%1 %2").arg(value).arg(unit);
}
}

SongDetailsDialog::SongDetailsDialog(const Song &song, QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(QStringLiteral("歌曲详情"));
    setModal(true);
    resize(420, 380);

    auto *layout = new QVBoxLayout(this);
    auto *form = new QFormLayout();
    form->setLabelAlignment(Qt::AlignRight);
    form->setHorizontalSpacing(16);
    form->setVerticalSpacing(8);

    form->addRow(QStringLiteral("标题"), new QLabel(valueOrUnknown(song.title()), this));
    form->addRow(QStringLiteral("艺术家"), new QLabel(valueOrUnknown(song.artist()), this));
    form->addRow(QStringLiteral("专辑"), new QLabel(valueOrUnknown(song.album()), this));
    form->addRow(QStringLiteral("风格"), new QLabel(valueOrUnknown(song.genre()), this));
    form->addRow(QStringLiteral("码率"), new QLabel(valueOrUnknown(song.bitRate(), QStringLiteral("kbps")), this));
    form->addRow(QStringLiteral("采样率"), new QLabel(valueOrUnknown(song.sampleRate(), QStringLiteral("Hz")), this));
    form->addRow(QStringLiteral("声道数"), new QLabel(valueOrUnknown(song.channels()), this));
    form->addRow(QStringLiteral("文件大小"), new QLabel(valueOrUnknown(song.sizeText()), this));
    form->addRow(QStringLiteral("时长"), new QLabel(valueOrUnknown(song.durationText()), this));
    form->addRow(QStringLiteral("播放次数"), new QLabel(QString::number(song.playCount()), this));
    form->addRow(QStringLiteral("文件格式"), new QLabel(valueOrUnknown(song.suffix().toUpper()), this));
    form->addRow(QStringLiteral("路径"), new QLabel(valueOrUnknown(song.path()), this));

    layout->addLayout(form);
    layout->addSpacing(8);

    auto *buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    auto *closeButton = new QPushButton(QStringLiteral("关闭"), this);
    connect(closeButton, &QPushButton::clicked, this, &QDialog::accept);
    buttonLayout->addWidget(closeButton);
    layout->addLayout(buttonLayout);
}
