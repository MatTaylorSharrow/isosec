#ifndef AUDITCLIENTCPP_H
#define AUDITCLIENTCPP_H

#include <QMainWindow>
#include <QScopedPointer>

#include "app.h"

// foreward declarations
namespace Ui {
class AuditClientCPP;
}
class App;

class AuditClientCPP : public QMainWindow
{
    Q_OBJECT

public:
    explicit AuditClientCPP(QWidget *parent = nullptr);
    ~AuditClientCPP() override;

    void printToTextArea(std::string s);
    
private:
    QScopedPointer<Ui::AuditClientCPP> m_ui;
    QScopedPointer<App> m_app;

private slots:
    void on_simulateStampedeButton_clicked();
    void on_simulateQueueButton_clicked();

};

#endif // AUDITCLIENTCPP_H
