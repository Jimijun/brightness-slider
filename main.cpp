#include "MainWindow.h"

#include <QSharedMemory>
#include <QApplication>

int main(int argc, char *argv[])
{
    QSharedMemory running_flag("brightness-slider_running");
    QScopeGuard cleanup(
        [&running_flag]() {
            if (running_flag.isAttached()) {
                running_flag.unlock();
                running_flag.detach();
            }
        }
    );
    if (!running_flag.create(1))
        return 1;
    running_flag.lock();

    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
