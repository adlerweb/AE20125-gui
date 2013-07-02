#include "ae20125gui.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    AE20125gui w;
    w.show();
    
    return a.exec();
}
