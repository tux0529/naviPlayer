#include "volumepopup.h"

#include <QLabel>
#include <QSlider>
#include <QVBoxLayout>

VolumePopup::VolumePopup(QWidget *parent)
    : QFrame(parent, Qt::Popup)
{
    setObjectName(QStringLiteral("volumePopup"));
    setFrameShape(QFrame::StyledPanel);

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(12, 12, 12, 12);
    layout->setSpacing(6);

    m_label = new QLabel(this);
    m_label->setAlignment(Qt::AlignCenter);

    m_slider = new QSlider(Qt::Vertical, this);
    m_slider->setRange(0, 100);
    m_slider->setFixedHeight(120);
    m_slider->setValue(80);

    layout->addWidget(m_label);
    layout->addWidget(m_slider);

    connect(m_slider, &QSlider::valueChanged, this, [this](int value) {
        m_label->setText(QStringLiteral("%1%").arg(value));
        Q_EMIT volumeChanged(value);
    });

    m_label->setText(QStringLiteral("%1%").arg(m_slider->value()));
}

int VolumePopup::volume() const
{
    return m_slider->value();
}

void VolumePopup::setVolume(int volume)
{
    m_slider->setValue(volume);
    m_label->setText(QStringLiteral("%1%").arg(volume));
}
