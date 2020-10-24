#ifndef APP_H
#define APP_H

#include "auditclientcpp.h"
#include "ui_auditclientcpp.h"

#include <boost/uuid/uuid.hpp>


// forward declarations
class AuditClientCPP;
namespace Ui {
class AuditClientCPP;
}
namespace boost {
    namespace uuids {
        struct uuid;
    }
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
    
    std::vector<boost::uuids::uuid> m_deviceids; 
    std::vector<std::string> m_products  = {"IA" , "KPI", "MP" , "PR" , "SSO", "VS" , "BAD"};
    std::vector<std::string> m_customers = {"AUD", "BMW", "KIA", "REN", "SEA", "VOL", "BAD"};
};

#endif // APP_H
