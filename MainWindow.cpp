#include "MainWindow.h"
#include "DisplayInfo.h"

#include <QApplication>
#include <QLabel>
#include <QMouseEvent>
#include <QPushButton>

class SliderWidget: public QWidget
{
    Q_OBJECT
public:
    SliderWidget(const DisplayInfo *info, QWidget *parent);

    void setDisplayInfo(const DisplayInfo *info);

public:
    const DisplayInfo *m_display_info;
    QLabel *m_name, *m_value;
    QSlider *m_slider;
};

SliderWidget::SliderWidget(const DisplayInfo *display_info, QWidget *parent)
    : QWidget(parent), m_name(nullptr), m_value(nullptr), m_slider(nullptr)
{
    if (!display_info)
        return;

    setDisplayInfo(display_info);

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->addWidget(m_name);
    layout->addWidget(m_slider);
    layout->addWidget(m_value);

    connect(m_slider, &QSlider::sliderMoved, this,
        [this](int value) { m_value->setText(QString::number(value)); }
    );
    connect(m_slider, &QSlider::valueChanged, this,
        [this](int value) {
            m_value->setText(QString::number(value));
            m_display_info->updateBrightness(value);
        }
    );
}

void SliderWidget::setDisplayInfo(const DisplayInfo *display_info)
{
    if (!display_info)
        return;

    m_display_info = display_info;
    const DisplayInfo::InfoStruct &info = m_display_info->info();
    if (!m_name) {
        m_name = new QLabel(this);
        m_name->setMinimumWidth(100);
    }
    m_name->setText(info.name);
    if (!m_slider) {
        m_slider = new QSlider(Qt::Horizontal, this);
        m_slider->setFixedWidth(100);
        m_slider->setMinimum(0);
        m_slider->setSingleStep(5);
        m_slider->setPageStep(5);
        m_slider->setTracking(false);
    }
    m_slider->setMaximum(info.max);
    m_slider->setValue(info.current);
    if (!m_value)
        m_value = new QLabel(this);
    m_value->setText(QString::number(info.current));
}

MainWindow::MainWindow(QWidget *parent): QWidget(parent), m_layout(new QVBoxLayout(this))
{
    setWindowFlag(Qt::FramelessWindowHint);

    QPushButton *button = new QPushButton("Refresh", this);
    m_layout->addWidget(button);
    connect(button, &QPushButton::clicked, this, [this]() { refresh(true); });

    refresh(false);
}

void MainWindow::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::RightButton) {
        DisplayInfo::updateCache();
        QApplication::exit(0);
    }
}

void MainWindow::refresh(bool force_update)
{
    const std::list<DisplayInfo> &list = DisplayInfo::displayInfoList(force_update);
    size_t list_i = 0;
    for (const DisplayInfo &display : list) {
        if (list_i < m_sliders.size()) {
            m_sliders[list_i]->setDisplayInfo(&display);
        } else {
            m_sliders.push_back(new SliderWidget(&display, this));
            m_layout->addWidget(m_sliders.back());
        }
        ++list_i;
    }
    while (m_sliders.size() > list_i) {
        m_layout->removeWidget(m_sliders.back());
        delete m_sliders.back();
        m_sliders.pop_back();
    }
    adjustSize();
}

#include "MainWindow.moc"
