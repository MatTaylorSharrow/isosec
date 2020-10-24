#include "auditclientcpp.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    AuditClientCPP w;
    w.show();

    return app.exec();
}

