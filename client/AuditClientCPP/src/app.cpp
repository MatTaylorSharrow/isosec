#include "app.h"

#include <QJsonObject>
#include <QJsonDocument>
#include <QRandomGenerator>

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


App::App()
{
}

App::~App()
{
    m_qtapp = nullptr; // release pointer
}


void App::setupApp(AuditClientCPP * qtapp){
    m_qtapp = qtapp; 
    
    //@todo load config 
//     m_host = "api.isosec.com";
//     m_port = "80";
//     m_target = "/AuditLog";
//     m_deviceids_to_generate = 200;
//     m_emitsize = 1000;
    
    m_httpclient = std::make_shared<HttpClient>( /*host, port */ );
    
    generateDeviceIds();
}

/**
 * Generate 16 byte (128bit) uuid
 * 
 * Called from main()
 */
void App::generateDeviceIds() 
{    
    boost::uuids::random_generator gen;
    boost::uuids::uuid id;

#ifdef DEBUG    
    m_qtapp->printToTextArea("Generating UUIDs ... ");
#endif
    
    // allocate upfront to stop costly allocations.
    m_deviceids.reserve(m_deviceids_to_generate);
    for (int i = 0; i < m_deviceids_to_generate; i++) {
        id = gen();
        m_deviceids.emplace_back(id);
#ifdef DEBUG
        m_qtapp->printToTextArea(to_string(id));
#endif
    }
}

/**
 * Send a large number of audit log messages to the server.  Each message will
 * have a randomly selected customer, product and device id.
 * 
 */
void App::emitStampede() {
    QVector<quint32> dev_id_rand, cust_rand, prod_rand; // to hold m_emitsize random numbers
    
    // generate random numbers 3 * emitsize 
    // quicker to generate in advance prior to loop as only requires 3 trips to 
    // the system RNG
    populateRandoms(dev_id_rand, m_emitsize, 0, m_deviceids.size());
    populateRandoms(cust_rand,   m_emitsize, 0, m_customers.size());
    populateRandoms(prod_rand,   m_emitsize, 0, m_products.size());
    
    bool success;
    int fail = 0;
    
    // loop emitsize times 
    for (int i = 0; i < m_emitsize; i++) {
        success = sendAudit(
            m_customers[cust_rand[i]],
            m_products[prod_rand[i]], 
            to_string(m_deviceids[dev_id_rand[i]])
        );
        
        if (!success) {
            fail++;
        }
    }
    
    m_qtapp->printToTextArea("We sent " + std::to_string(m_emitsize) + " audit logs, " + std::to_string(m_emitsize - fail) + " succeeded, " + std::to_string(fail) + " failed. ");
}

/**
 * Helper method to populate random numbers into vectors
 */
void App::populateRandoms(QVector<quint32> &vec, int qty, quint32 lo, quint32 hi) 
{
    vec.resize(qty);
    std::generate(vec.begin(), vec.end(), [this,lo,hi]() {
        return QRandomGenerator::global()->bounded(lo, hi);
    });
}

/**
 * Connect to a web server and send an audit log message
 */
bool App::sendAudit(std::string customer, std::string product, std::string device_id) 
{
    QJsonObject json_request = QJsonObject();
    json_request.insert("customer", QJsonValue(QString::fromStdString(customer)));
    json_request.insert("product", QJsonValue(QString::fromStdString(product)));
    json_request.insert("event_timestamp", QJsonValue( QDateTime::currentDateTime().toString(Qt::ISODate) ));
    json_request.insert("device_id", QJsonValue(QString::fromStdString(device_id)));
    QJsonDocument json_doc = QJsonDocument(json_request);

#ifdef DEBUG    
    m_qtapp->printToTextArea(customer + " " + product + " " + device_id);
#endif

    m_httpclient->makeConnection(m_host, m_port);
    m_httpclient->sendRequest(m_target, json_doc.toJson().toStdString());
    std::pair<int, std::string> response = m_httpclient->getResponse();
    m_httpclient->closeConnection();

    bool result = false;
    switch (response.first) {
        case 204:
#ifdef DEBUG
            m_qtapp->printToTextArea("Audit log message created ");
#endif
            result = true;
            break;
        case 200:
            m_qtapp->printToTextArea("Audit log message created "); // would only get this for a request to / by GET
            break;            
        case 400:
            m_qtapp->printToTextArea("Audit log message NOT created because: ");
            m_qtapp->printToTextArea(" ");
            break;            
        case 405:
            break;
        default: // code received that's not part of the interface
            m_qtapp->printToTextArea("Unknown error has occurred ");
    }
    
    return result;
}
