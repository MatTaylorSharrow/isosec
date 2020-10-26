#include "app.h"

#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonParseError>
#include <QRandomGenerator>
#include <QElapsedTimer>

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

#include <memory>

#include "httpclient.h"

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
    m_httpclient->setup(this);
    
    generateDeviceIds();
}

/**
 * Helper method to pass text updates for debug purpose to the GUI's text areas
 */
void App::printToTextArea(std::string s) {
    m_qtapp->printToTextArea(s);
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
    QElapsedTimer timer;
    timer.start();
    
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
    
    m_qtapp->printToTextArea("We sent " + std::to_string(m_emitsize) + " audit logs, " + std::to_string(m_emitsize - fail) + " succeeded, " + std::to_string(fail) + " failed. \n");
    double seconds = double(timer.elapsed()) / 1000;
    m_qtapp->printToTextArea("It has take " + std::to_string(seconds) + " seconds \n");
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
    json_request.insert("event_timestamp", QJsonValue(QDateTime::currentDateTime().toString(Qt::ISODate)));
    json_request.insert("device_id", QJsonValue(QString::fromStdString(device_id)));
    QJsonDocument json_doc = QJsonDocument(json_request);

    m_httpclient->makeConnection(m_host, m_port);
    m_httpclient->sendRequest(m_target, json_doc.toJson().toStdString());
    std::pair<int, std::string> response = m_httpclient->getResponse();
    m_httpclient->closeConnection();

    // hmm we are processing each response in turn here, perhaps it would be
    // quicker / less memory intensive to store the failed responses and then 
    // process them afterwards to save on the creation of the json parser objects
    return processResponse(response.first, response.second);
}

/**
 * If the request failed then update the text area with an error message
 */
bool App::processResponse(int code, std::string body) {    
    bool result = false;
   
    switch (code) {
        case 204:
            // excellent - nothing todo as there is not response body to have to process.
#ifdef DEBUG
            m_qtapp->printToTextArea("Audit log message created ");
#endif
            result = true;
            break;
        case 200:
#ifdef DEBUG            
            m_qtapp->printToTextArea("Audit log message created "); // would only get this for a request to / by GET
#endif
            break;
        case 400:
            // Huston we have a problem.  Find out what type of error it was an let the user know.
            m_qtapp->printToTextArea("Audit log message NOT created because: ");
            m_qtapp->printToTextArea(App::getErrorMessageFromResponseJson(body));
            break;            
        case 405:
            break;
        default: // code received that's not part of the interface
            m_qtapp->printToTextArea("Unknown error has occurred. ");
    }
    
    return result;
}

/**
 * Get turn the body into a Json object and extract error information
 * 
 * Yuk - there must be an easier way to parse json objects
 */
std::string App::getErrorMessageFromResponseJson(std::string body) {
    
    auto json_error = std::make_shared<QJsonParseError>();
    
    QJsonDocument json_response = QJsonDocument::fromJson(QByteArray::fromStdString(body), json_error.get());
    if (json_response.isNull()) {
        // received json document failed to parse/validate
        std::string parser_error = json_error->errorString().toStdString();
    }

    if ( ! json_response.isObject()) {
        // The data transfer format says the that the top most document element 
        // must be an Object not an Array
        
        // @todo some error handling for the error handling
        return "";
    }

    std::string tmp = ""; // build up an error message

    // if validation_errors is defined (double negative ! undefined ;)
    if ( ! json_response.object().value(QString("validation_errors")).isUndefined() )  {
        if (json_response.object().value(QString("validation_errors")).isObject()) {
            
            QJsonObject verrors = json_response.object().value(QString("validation_errors")).toObject();
            QStringList vkeys = verrors.keys();
            
            std::for_each(vkeys.begin(), vkeys.end(), [&verrors,&tmp] (const QString & key) {
                
                tmp += key.toStdString() + " ::  failed validation : \n";
                
                if (verrors.value(key).isArray()) {
                    QJsonArray vkerrors = verrors.value(key).toArray();
                    
                    std::for_each(vkerrors.begin(), vkerrors.end(), [&tmp] (const QJsonValue & message) {
                        tmp += "\t - " + message.toString().toStdString() + "\n";
                    });
                }
            });
        }
    }

    // if database_errors is defined (double negative ! undefined ;)
    if ( ! json_response.object().value(QString("database_errors")).isUndefined()) {
        if (json_response.object().value(QString("database_errors")).isObject()) {
            
            tmp += "Database error : \n";
            
            // if database_errors.query is defined (double negative ! undefined ;)
            if ( ! json_response.object().value(QString("database_errors")).toObject().value(QString("query")).isUndefined() ) {
                tmp += "\t - The query failed to execute - The product or customer provided doesn't exist.  Check your device configuration. \n";
            }
            
            // if database_errors.statement is defined (double negative ! undefined ;)
            if ( ! json_response.object().value(QString("database_errors")).toObject().value(QString("statement")).isUndefined() ) { 
                tmp += "\t - There was a problem prior to executing your update.  The developers have gone an dropped the ball! \n";
            }
        }
    }
    
    return tmp;
}

