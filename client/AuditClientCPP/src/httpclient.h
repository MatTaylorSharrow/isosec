#ifndef HTTPCLIENT_H
#define HTTPCLIENT_H

#include <boost/asio/ip/tcp.hpp>
using tcp = boost::asio::ip::tcp;       // type alias

// forward declarations
class App;

/**
 * This is a simple wrapper around the boost::beast classes and calls to provide
 * the most basic of http clients to allow sending requests and processing 
 * responses
 */
class HttpClient
{
public:    
    /**
     * Default constructor
     */
    HttpClient();
    
    /**
     * Create an HttpClient and make a connection through the socket
     */
    HttpClient(const std::string & host, const std::string & port);


    /**
     * Destructor
     */
    ~HttpClient();

    void setup(App * app);
    void makeConnection(const std::string & host, const std::string & port);
    void closeConnection();
    void sendRequest(const std::string & target, const std::string & payload);
    std::pair<int, std::string> getResponse();
    
private:
    // I only wanted m_socket as a member but had to make m_ioc and m_resolver members too
    boost::asio::io_context m_ioc;
    tcp::resolver m_resolver;
    tcp::socket m_socket;
    
    App * m_app;
    
    std::string m_host;
};

#endif // HTTPCLIENT_H
