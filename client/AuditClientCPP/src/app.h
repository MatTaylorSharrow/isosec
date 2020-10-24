#ifndef APP_H
#define APP_H

#include "auditclientcpp.h"
#include "ui_auditclientcpp.h"

#include <boost/uuid/uuid.hpp>

#include "httpclient.h"

// forward declarations
class AuditClientCPP;
namespace Ui {
class AuditClientCPP;
}


/**
 * The App class is kindof an AppController to sepearate out logic from UI
 * 
 */
class App
{
public:
    /**
     * Default constructor
     */
    App();

    /**
     * Destructor
     */
    ~App();
    
    void setupApp(AuditClientCPP * qtapp);
    void emitStampede();
    void generateDeviceIds();
    
private:
    void populateRandoms(QVector<quint32> &vec, int qty, quint32 lo, quint32 hi);
    bool sendAudit(std::string customer, std::string product, std::string device_id);
    
    AuditClientCPP * m_qtapp;
    std::shared_ptr<HttpClient> m_httpclient;
    
    std::vector<boost::uuids::uuid> m_deviceids; 
    std::vector<std::string> m_products  = {"IA" , "KPI", "MP" , "PR" , "SSO", "VS" , "BAD"};
    std::vector<std::string> m_customers = {"AUD", "BMW", "KIA", "REN", "SEA", "VOL", "BAD"};
    
    
    std::string const m_host = "api.isosec.com";
    std::string const m_port = "80";
    std::string const m_target = "/AuditLog";
    int const m_deviceids_to_generate = 200;
    int const m_emitsize = 1000;
    
};

#endif // APP_H
