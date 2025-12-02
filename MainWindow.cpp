#include "MainWindow.h"
#include "DisplayInfo.h"

#include <QApplication>
#include <QBoxLayout>
#include <QMouseEvent>
#include <QLabel>

class SliderWidget: public QWidget
{
    Q_OBJECT
public:
    SliderWidget(const DisplayInfo *info, QWidget *parent);

public:
    const DisplayInfo *m_display_info;
    QLabel *m_name, *m_value;
    QSlider *m_slider;
};

SliderWidget::SliderWidget(const DisplayInfo *display_info, QWidget *parent)
    : m_display_info(display_info)
    , QWidget(parent)
{
    const DisplayInfo::InfoStruct &info = m_display_info->info();

    QHBoxLayout *layout = new QHBoxLayout(this);
    m_name = new QLabel(info.name, this);
    m_name->setMinimumWidth(100);
    layout->addWidget(m_name);
    m_slider = new QSlider(Qt::Horizontal, this);
    m_slider->setFixedWidth(100);
    layout->addWidget(m_slider);
    m_value = new QLabel(QString::number(info.current), this);
    layout->addWidget(m_value);

    m_slider->setMinimum(0);
    m_slider->setMaximum(info.max);
    m_slider->setValue(info.current);
    m_slider->setSingleStep(5);
    m_slider->setPageStep(5);
    m_slider->setTracking(false);

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

MainWindow::MainWindow(QWidget *parent): QWidget(parent)
{
    setWindowFlag(Qt::FramelessWindowHint);

    const std::list<DisplayInfo> &list = DisplayInfo::displayInfoList();
    const size_t size = list.size();
    if (size == 0)
        return;

    QVBoxLayout *layout = new QVBoxLayout(this);
    m_sliders.reserve(size);
    for (const DisplayInfo &info : list) {
        m_sliders.push_back(new SliderWidget(&info, this));
        SliderWidget *slider = m_sliders.back();
        layout->addWidget(slider);
    }
}

void MainWindow::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::RightButton) {
        DisplayInfo::updateCache();
        QApplication::exit(0);
    }
}

#include "MainWindow.moc"
