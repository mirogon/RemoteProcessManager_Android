#include "remote_process_manager.h"


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    m1::remote_process_manager w;
    w.show();

    return a.exec();
}
