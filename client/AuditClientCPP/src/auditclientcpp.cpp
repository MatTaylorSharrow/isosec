#include "auditclientcpp.h"
#include "ui_auditclientcpp.h"

#include <QJsonObject>
#include <QJsonDocument>

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

using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>


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

#ifdef DEBUG    
    m_ui->textEdit->insertPlainText("Generating UUIDs ... \n");
#endif
    
    // allocate upfront to stop costly allocations.
    deviceids.reserve(200);  // @todo - remove magic number
    
    for (int i = 0; i < 200; i++) {
        id = gen();
        deviceids.emplace_back(id);
#ifdef DEBUG
        m_ui->textEdit->insertPlainText(QString::fromStdString(to_string(id) + "\n"));
#endif
    }
}


bool 
AuditClientCPP::sendAudit(std::string customer, std::string product, std::string device_id) {
    
    QJsonObject json_request = QJsonObject();
    json_request.insert("customer", QJsonValue(QString::fromStdString(customer)));
    json_request.insert("product", QJsonValue(QString::fromStdString(product)));
    json_request.insert("event_timestamp", QJsonValue(QString::fromStdString("2020-10-23 10:09:23")));
    json_request.insert("device_id", QJsonValue(QString::fromStdString(device_id)));
    QJsonDocument json_doc = QJsonDocument(json_request);

#ifdef DEBUG    
m_ui->textEdit->insertPlainText(QString::fromStdString(customer + " " + product + " " + device_id + "\n"));
#endif

    auto const host = "api.isosec.com";
    auto const port = "80";
    auto const target = "/AuditLog";
    auto const http_version = 11;
    
    // The io_context is required for all I/O
    boost::asio::io_context ioc;

    // These objects perform our I/O
    tcp::resolver resolver{ioc};
    tcp::socket socket{ioc};

    // Look up the domain name
    auto const results = resolver.resolve(host, port);

    // Make the connection on the IP address we get from a lookup
    boost::asio::connect(socket, results.begin(), results.end());

    // Set up an HTTP GET request message
    boost::beast::http::request<boost::beast::http::string_body> req{boost::beast::http::verb::post, target, http_version};
    req.set(boost::beast::http::field::host, host);
    req.set(boost::beast::http::field::user_agent, BOOST_BEAST_VERSION_STRING);
    req.set(boost::beast::http::field::content_type, "application/json");
    req.body() = json_doc.toJson().toStdString();
    req.prepare_payload();
    
#ifdef DEBUG
m_ui->textEdit->insertPlainText(QString::fromStdString(json_doc.toJson().toStdString() + "\n"));
#endif

    // Send the HTTP request to the remote host
    boost::beast::http::write(socket, req);
    
    // This buffer is used for reading and must be persisted
    boost::beast::flat_buffer buffer;

    // Declare a container to hold the response
    boost::beast::http::response<boost::beast::http::dynamic_body> res;

    // Receive the HTTP response
    boost::beast::http::read(socket, buffer, res);

#ifdef DEBUG
    std::string res_string = boost::beast::buffers_to_string(res.body().data());
    m_ui->textEdit->insertPlainText(QString::fromStdString(res_string + "\n"));
#endif
    
    // Gracefully close the socket
    boost::system::error_code ec;
    socket.shutdown(tcp::socket::shutdown_both, ec);

    // not_connected happens sometimes so don't bother reporting it.
    if (ec && ec != boost::system::errc::not_connected) {
        throw boost::system::system_error{ec};
    }
}

void 
AuditClientCPP::on_simulateStampedeButton_clicked() {
    //m_ui->centralwidget->setStyleSheet("background-color:blue;");
    
    for (int i = 0; i < 1000; i++) {  //@todo - remove magic number
        
        sendAudit("BMW","RIO","abcdefghijklmnopqrstuvwxyz");
    }
}

void 
AuditClientCPP::on_simulateQueueButton_clicked() {
    m_ui->centralwidget->setStyleSheet("background-color:yellow;");
    
    
}

