#ifndef AUDITCLIENTCPP_H
#define AUDITCLIENTCPP_H

#include <QMainWindow>
#include <QScopedPointer>

#include <boost/uuid/uuid.hpp>


namespace Ui {
class AuditClientCPP;
}

class AuditClientCPP : public QMainWindow
{
    Q_OBJECT

public:
    explicit AuditClientCPP(QWidget *parent = nullptr);
    ~AuditClientCPP() override;

    void generateDeviceIds();
    
private:
    QScopedPointer<Ui::AuditClientCPP> m_ui;
    
    bool sendAudit(std::string TrustName, std::string ProductName, std::string DeviceUID);
    
    std::vector<boost::uuids::uuid> deviceids; 
    std::vector<std::string> products = {"IA","KPI","MP","PR","SSO","VS"};
    std::vector<std::string> customers = {"AUD","BMW","KIA","REN","SEA","VOL"};
    
    
private slots:
    void on_simulateStampedeButton_clicked();
    void on_simulateQueueButton_clicked();

};

#endif // AUDITCLIENTCPP_H
