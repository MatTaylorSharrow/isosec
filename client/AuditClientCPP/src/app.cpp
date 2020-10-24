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


#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/beast/core/buffers_to_string.hpp>
#include <boost/beast/core/error.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>

using tcp = boost::asio::ip::tcp;       // type alias

App::App()
{
}

App::~App()
{
}

void App::setupApp(AuditClientCPP * qtapp){
    m_qtapp = qtapp; 
    generateDeviceIds();
}


/**
 * Generate 16 byte (128bit) uuid
 * 
 * Called from main()
 */
void App::generateDeviceIds() 
{
    auto const deviceids_to_generate = 200; // @todo - remove magic number
    
    boost::uuids::random_generator gen;
    boost::uuids::uuid id;

//#ifdef DEBUG    
    m_qtapp->printToTextArea("Generating UUIDs ... ");
//#endif
    
    // allocate upfront to stop costly allocations.
    m_deviceids.reserve(deviceids_to_generate);
    for (int i = 0; i < deviceids_to_generate; i++) {
        id = gen();
        m_deviceids.emplace_back(id);
        
//#ifdef DEBUG
        m_qtapp->printToTextArea(to_string(id));
//#endif
    }
}


void App::emitStampede() {
    auto const emitsize = 1000; // @todo - remove magic number
    QVector<quint32> dev_id_rand, cust_rand, prod_rand; // to hold emitsize random numbers
    
    // generate random numbers 3 * emitsize 
    // quicker to generate in advance prior to loop as only requires 3 trips to 
    // the system RNG
    populateRandoms(dev_id_rand, emitsize, 0, m_deviceids.size());
    populateRandoms(cust_rand,   emitsize, 0, m_customers.size());
    populateRandoms(prod_rand,   emitsize, 0, m_products.size());
    
    // loop emitsize times 
    for (int i = 0; i < emitsize; i++) {
        sendAudit(
            m_customers[cust_rand[i]],
            m_products[prod_rand[i]], 
            to_string(m_deviceids[dev_id_rand[i]])
        );
    }
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

//#ifdef DEBUG    
    m_qtapp->printToTextArea(customer + " " + product + " " + device_id);
//#endif

    auto const host = "api.isosec.com";
    auto const port = "80";
    auto const target = "/AuditLog";
    auto const http_version = 11;
    
    boost::asio::io_context ioc;                                                // The io_context is required for all I/O

    // These objects perform our I/O
    tcp::resolver resolver{ioc};
    tcp::socket socket{ioc};
    
    auto const results = resolver.resolve(host, port);                          // Look up the domain name
    boost::asio::connect(socket, results.begin(), results.end());               // Make the connection on the IP address we get from the lookup

    // Set up an HTTP GET request message
    boost::beast::http::request<boost::beast::http::string_body> req{boost::beast::http::verb::post, target, http_version};
    req.set(boost::beast::http::field::host, host);
    req.set(boost::beast::http::field::user_agent, BOOST_BEAST_VERSION_STRING);
    req.set(boost::beast::http::field::content_type, "application/json");
    req.body() = json_doc.toJson().toStdString();
    req.prepare_payload();
//#ifdef DEBUG
    m_qtapp->printToTextArea(json_doc.toJson().toStdString());
//#endif

    boost::beast::http::write(socket, req);                                     // Send the HTTP request to the remote host
    boost::beast::flat_buffer buffer;                                           // This buffer is used for reading and must be persisted
    boost::beast::http::response<boost::beast::http::dynamic_body> resp;        // Declare a container to hold the response
    boost::beast::http::read(socket, buffer, resp);                             // Receive the HTTP response
    
//#ifdef DEBUG
    std::string res_string = boost::beast::buffers_to_string(resp.body().data());
    m_qtapp->printToTextArea(res_string);
//#endif
    
    // Gracefully close the socket
    boost::system::error_code ec;
    socket.shutdown(tcp::socket::shutdown_both, ec);

    // not_connected happens sometimes so don't bother reporting it.
    if (ec && ec != boost::system::errc::not_connected) {
        throw boost::system::system_error{ec};
    }
    
    switch (resp.result()) {
        case boost::beast::http::status::no_content: //204
//#ifdef DEBUG
            m_qtapp->printToTextArea("Audit log message created ");
//#endif
            break;
        case boost::beast::http::status::ok: //200
            m_qtapp->printToTextArea("Audit log message created "); // would only get this for a request to / by GET
            break;
        case boost::beast::http::status::bad_request: //400
            m_qtapp->printToTextArea("Audit log message NOT created because: ");
            m_qtapp->printToTextArea(" ");
            break;            
        case boost::beast::http::status::method_not_allowed: //405
            break;
        default: // code received that's not part of the interface
            m_qtapp->printToTextArea("Unknown error has occurred ");
    }
}
