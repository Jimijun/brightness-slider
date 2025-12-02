#pragma once

#include <QBoxLayout>
#include <QSlider>
#include <QWidget>

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

private slots:
    void refresh(bool force_update = false);

private:
    QVBoxLayout *m_layout;
    std::vector<SliderWidget *> m_sliders;
};
