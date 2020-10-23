#include "auditclientcpp.h"
#include "ui_auditclientcpp.h"

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
/*#include <boost/uuid/entropy_error.hpp> not in boost 1.66.0 but exists in 1.67
try {
    id = gen();
} catch(boost::uuids::entropy_error) {
    // an error can occur if the system doesn't have enough entropy, so
    // we're not too concerned so maybe just try again 
    error_count++;
    i--;
}
*/

AuditClientCPP::AuditClientCPP(QWidget *parent) :
    QMainWindow(parent),
    m_ui(new Ui::AuditClientCPP)
{
    m_ui->setupUi(this);
}

AuditClientCPP::~AuditClientCPP() = default;



void 
AuditClientCPP::generateDeviceIds() {
    boost::uuids::random_generator gen;
    boost::uuids::uuid id;
    
    m_ui->textEdit->insertPlainText("Generating UUIDs ... \n");
    
    // allocate upfront to stop costly allocations.
    deviceids.reserve(200);  // @todo - remove magic number
    
    for (int i = 0; i < 200; i++) {
        id = gen();
        deviceids.emplace_back(id);

//#ifdef DEBUG        
        m_ui->textEdit->insertPlainText(QString::fromStdString(to_string(id) + "\n"));

    }
}

bool 
AuditClientCPP::sendAudit(std::string customer, std::string product, std::string device_id) {
    
    
    
}

void 
AuditClientCPP::on_simulateStampedeButton_clicked() {
    m_ui->centralwidget->setStyleSheet("background-color:blue;");
}

void 
AuditClientCPP::on_simulateQueueButton_clicked() {
    m_ui->centralwidget->setStyleSheet("background-color:yellow;");
    
    for (int i = 0; i < 1000; i++) {  //@todo - remove magic number
        sendAudit("BMW","RIO","1234");
    }
}
