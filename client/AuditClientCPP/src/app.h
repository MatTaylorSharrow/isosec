#ifndef APP_H
#define APP_H

#include "auditclientcpp.h"
#include "ui_auditclientcpp.h"


// forward declarations
namespace boost {
    namespace uuids {
        struct uuid;
    }
}
class AuditClientCPP;
class HttpClient;
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
    
    void printToTextArea(std::string s);
    
private:
    void populateRandoms(QVector<quint32> &vec, int qty, quint32 lo, quint32 hi);
    bool sendAudit(std::string customer, std::string product, std::string device_id);
    bool processResponse(int code, std::string body);
    std::string getErrorMessageFromResponseJson(std::string body);
    
    AuditClientCPP * m_qtapp;
    std::shared_ptr<HttpClient> m_httpclient;
    
    std::vector<boost::uuids::uuid> m_deviceids; 
    std::vector<std::string> m_products  = {"IA" , "KPI", "MP" , "PR" , "SSO", "VS" , "BAD"   , "BADER"};
    std::vector<std::string> m_customers = {"AUD", "BMW", "KIA", "REN", "SEA", "VOL", "BADEST", "DAB"};
    
    
    std::string const m_host = "api.isosec.com";
    std::string const m_port = "80";
    std::string const m_target = "/AuditLog";
    int const m_deviceids_to_generate = 200;
    int const m_emitsize = 1000;
    
};

#endif // APP_H
