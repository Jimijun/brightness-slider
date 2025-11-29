#pragma once

#include <QSlider>
#include <QWidget>

#include <memory>
#include <vector>

class SliderWidget;

class MainWindow: public QWidget
{
    Q_OBJECT
public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow() {}

protected:
    virtual void mousePressEvent(QMouseEvent *event);

private:
    std::vector<SliderWidget *> m_sliders;
};
