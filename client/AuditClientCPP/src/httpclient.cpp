#include "httpclient.h"

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/beast/core/buffers_to_string.hpp>
#include <boost/beast/core/error.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>

using tcp = boost::asio::ip::tcp;       // type alias



HttpClient::HttpClient() :
    m_resolver(tcp::resolver(m_ioc)),
    m_socket(tcp::socket(m_ioc))
{
}

HttpClient::HttpClient(const std::string & host, const std::string & port) :
    m_resolver(tcp::resolver(m_ioc)),
    m_socket(tcp::socket(m_ioc))
{  
    makeConnection(host,port);
}


HttpClient::~HttpClient()
{
    m_app = nullptr;
    closeConnection();
}

void HttpClient::setup(App * app){
    m_app = app; 
}

/**
 * Make a connection to a remote server through a socket
 */
void HttpClient::makeConnection(const std::string & host, const std::string & port) 
{
    m_host = host;
    
    // Look up the domain name and make the connection on the IP address we get from the lookup
    boost::asio::connect(m_socket, m_resolver.resolve(host, port));
}

/**
 * Close the socket connection
 */
void HttpClient::closeConnection()
{
    // Gracefully close the socket
    boost::system::error_code ec;
    m_socket.shutdown(tcp::socket::shutdown_both, ec);

    // not_connected happens sometimes so don't bother reporting it.
    if (ec && ec != boost::system::errc::not_connected) {
        throw boost::system::system_error{ec};
    }
}

/**
 * Send a http request to an already open connection
 * 
 */
void HttpClient::sendRequest(const std::string & target, const std::string & payload)
{
    auto const http_version = 11;
    
    // Set up an HTTP GET request message
    boost::beast::http::request<boost::beast::http::string_body> req{boost::beast::http::verb::post, target, http_version};
    
    req.set(boost::beast::http::field::host, m_host);
    req.set(boost::beast::http::field::user_agent, BOOST_BEAST_VERSION_STRING);
    req.set(boost::beast::http::field::content_type, "application/json");
    
    req.body() = payload;
    req.prepare_payload();
#ifdef DEBUG
    m_app->printToTextArea(payload);
#endif

    boost::beast::http::write(m_socket, req);                                   // Send the HTTP request to the remote host
}

/**
 * Return the status code and the body document
 * 
 * @return std::pair<int code, std::string body> 
 */
std::pair<int, std::string> HttpClient::getResponse()
{
    boost::beast::flat_buffer buffer;                                           // This buffer is used for reading and must be persisted
    boost::beast::http::response<boost::beast::http::dynamic_body> resp;        // Declare a container to hold the response
    boost::beast::http::read(m_socket, buffer, resp);                           // Receive the HTTP response

    std::string res_string = boost::beast::buffers_to_string(resp.body().data());

#ifdef DEBUG
    m_app->printToTextArea(res_string);
#endif
    
    return std::make_pair(resp.result_int(), res_string);
}

