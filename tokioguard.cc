#include <iostream>
#include <boost/asio.hpp>
#include <boost/asio/steady_timer.hpp>
#include <unordered_map>
#include <string>
#include <thread>
#include <fstream>
#include <unordered_set>
#include <map>
#include <iomanip>

#include "sqlitebebra.hpp"

using namespace boost::asio;

std::unordered_map<std::string, uint32_t> request_per_ip;
std::unordered_set<std::string> banned_ip;

struct connection_t 
{
    connection_t(io_service& io) : timer(io), socket(io) 
    { }

    ip::tcp::socket socket;
    ip::address ip;
    uint16_t port;
    steady_timer timer;
    uint32_t request = 0;

    bool request_fully_received = false;
};

struct cfg_info {
    cfg_info() = default;

    cfg_info(const std::string& addr, const std::string& path_wait_page, uint16_t detected_value, 
        const std::string& bannedIpSavePath) :
       guard_address(addr), guard_wait_page(path_wait_page), ddos_attack_detected_value(detected_value), 
       bannedIpsSaveFile(bannedIpSavePath) 
       { 
         bannedIpFile.open(bannedIpsSaveFile);
       }

    std::string guard_address;
    std::string guard_wait_page;
    std::string bannedIpsSaveFile;
    uint16_t ddos_attack_detected_value;
    std::ofstream bannedIpFile;
    sqlite3* db = nullptr;
};

cfg_info start_cfg{};

enum class notify_color {
    green, red, blue
};

std::map<notify_color, std::string> colorsIds;

void filled_color_map() noexcept
{
    colorsIds[notify_color::green] = "\033[0;32m";
    colorsIds[notify_color::red] = "\033[0;31m";
    colorsIds[notify_color::blue] = "\033[0;34m";
}

void notify_print(const std::string& msg, notify_color ncolor) noexcept
{
    std::cout << colorsIds[ncolor] << msg << colorsIds[ncolor] << std::endl; 
    std::cout << "\033[0m";
}

std::string get_current_time()
{
    std::chrono::system_clock::time_point now = 
       std::chrono::system_clock::now();

    std::time_t time = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time), "%Y-%m-%d %X");

    return ss.str();
}

void failed() 
{
    notify_print("failed operation", notify_color::red);
}

void load_banned_ip(const std::string& path)
{
    std::ifstream bannedIpsFile(path);
    if(!bannedIpsFile) {
        return failed();
    }
    std::string line;
    while(std::getline(bannedIpsFile, line)) {
        banned_ip.emplace(line);
    }

    std::cout << "banned ip load" << std::endl;
}

std::string html_page_load(const std::string& path)
{
    std::string output_page{};
    std::ifstream file(path);
    if(!file) {
        output_page = 
        "<html>"
        "<body>"  
        "<h1>DDoS scaning</h1>"
        "<p>Wait...</p>"
        "</body>"
        "</html>";
        return output_page;
    }
    std::string line;
    while(std::getline(file, line)) {
        output_page += line;
    }
    return output_page;
}

void handle_connection(std::shared_ptr<connection_t> conn) {
    std::string headers = 
       "HTTP/1.1 200 OK\r\n"
       "Content-Type: text/html\r\n"
       "\r\n";

    conn->socket.send(buffer(headers));
    conn->socket.send(buffer(html_page_load(start_cfg.guard_wait_page)));

    conn->timer.expires_after(std::chrono::seconds(30));
    conn->timer.async_wait([conn](const boost::system::error_code& ec) {
        if(!ec) {
            if(!conn->request_fully_received) {
                conn->socket.close();
            }
        }
    });
    streambuf buf;
    read_until(conn->socket, buf, "\r\n\r\n");

    std::string request;
    bool valid_user_agent = false;

    std::string ip = conn->ip.to_string();
    request_per_ip[ip]++;
    std::cout << "Temp: " << request_per_ip[ip] 
       << " ddos attack detected: " << start_cfg.ddos_attack_detected_value << std::endl;

    if(request_per_ip[ip] > start_cfg.ddos_attack_detected_value) {
        notify_print(get_current_time() + " detected ddos attack", notify_color::red);
        insertIP(ip);
        conn->socket.close();
        return;
    }
    else if(existsInDb(ip)) {
        notify_print(get_current_time() + " detected banned ip", notify_color::red);
        conn->socket.close();
        return;
    }
    else {
        valid_user_agent = true;
    }
    
    if(valid_user_agent) {
        notify_print(get_current_time() + " client - " + ip + " is not ddos attack", notify_color::green);
        std::this_thread::sleep_for(std::chrono::seconds(2)); 
        std::string html = "<script>window.location = ";
        html += "'" + start_cfg.guard_address + "'" + ";</script>";
        
        write(conn->socket, buffer(html));
    }
    else {
        conn->socket.close();
    }
}

void accept_connection(io_service& io_service) {
    ip::tcp::acceptor acceptor(io_service, ip::tcp::endpoint(ip::tcp::v4(), 4141));
    while(true) {
        auto conn = std::make_shared<connection_t>(io_service);
        acceptor.accept(conn->socket);
        conn->ip = conn->socket.remote_endpoint().address();
        std::cout << get_current_time() << " connected user" << std::endl;
        std::thread t(&handle_connection, conn);
        t.detach();
    }
}

int main(int argc, char* argv[])
{
    filled_color_map();

    if(argc != 4) {
        notify_print("Usage: <guard_address> <wait_page.html> <banned_ip_db_name>", notify_color::blue);
        return -1;
    }
    else {
        start_cfg.guard_address = std::string(argv[1]);
        start_cfg.guard_wait_page = std::string(argv[2]);
        start_cfg.ddos_attack_detected_value = 10;
        data_base_init(std::string(argv[3]));
        insertIP("12.12");
        insertIP("12.12");
    }

    io_service io_service;
    std::thread acceptThread(&accept_connection, std::ref(io_service));
    std::thread ioThread([&io_service]() {
        io_service.run();
    });

    acceptThread.join();
    ioThread.join();
}