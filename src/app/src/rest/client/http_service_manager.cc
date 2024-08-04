#include "cJSON/cJSON.h"
#include "../include/http_service_manager.h"


HTTPServiceManager::HTTPServiceManager(std::string ip, int port)
{
    m_ip = ip;
    m_port = port;
}

HTTPServiceManager::~HTTPServiceManager()
{
}

int HTTPServiceManager::init()
{
    
}

int HTTPServiceManager::exit()
{
    return 0;
}

#if 0
int HTTPServiceManager::accept()
{
    std::unique_ptr<TaskQueue> task_queue(new_task_queue());

    while (m_socket != INVALID_SOCKET) {
        auto val = select_read(m_socket);
        if (val == 0) { // Timeout
          task_queue->on_idle();
          continue;
        }
        socket_t client_sock = accept(m_socket, nullptr, nullptr);
        if (client_sock < 0) {
            if (errno == EMFILE) {
              std::this_thread::sleep_for(std::chrono::milliseconds(1));
              continue;
            } else if (errno == EINTR || errno == EAGAIN) {
              continue;
            }
            if (m_socket != INVALID_SOCKET) {
              close(m_socket);
            }
            break;
        }
        timeval tv;
        tv.tv_sec = static_cast<long>(read_timeout_sec_);
        tv.tv_usec = static_cast<decltype(tv.tv_usec)>(read_timeout_usec_);
        setsockopt(client_sock, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv, sizeof(tv));
        timeval tv;
        tv.tv_sec = static_cast<long>(write_timeout_sec_);
        tv.tv_usec = static_cast<decltype(tv.tv_usec)>(write_timeout_usec_);
        setsockopt(client_sock, SOL_SOCKET, SO_SNDTIMEO, (char *)&tv, sizeof(tv));
       task_queue->enqueue([this, client_sock]() { process_and_close_socket(client_sock); });
    }

    task_queue->shutdown();
    return 0;
}

ssize_t HTTPServiceManager::read(socket_t sock, time_t sec, time_t usec) {
#ifdef CPPHTTPLIB_USE_POLL
  struct pollfd pfd_read;
  pfd_read.fd = sock;
  pfd_read.events = POLLIN;

  auto timeout = static_cast<int>(sec * 1000 + usec / 1000);

  return handle_EINTR([&]() { return poll(&pfd_read, 1, timeout); });
#else
  fd_set fds;
  FD_ZERO(&fds);
  FD_SET(sock, &fds);

  timeval tv;
  tv.tv_sec = static_cast<long>(sec);
  tv.tv_usec = static_cast<decltype(tv.tv_usec)>(usec);

  return handle_EINTR([&]() {
    return select(static_cast<int>(sock + 1), &fds, nullptr, nullptr, &tv);
  });
#endif
}

ssize_t HTTPServiceManager::select_write(socket_t sock, time_t sec, time_t usec) {
#ifdef CPPHTTPLIB_USE_POLL
  struct pollfd pfd_read;
  pfd_read.fd = sock;
  pfd_read.events = POLLOUT;

  auto timeout = static_cast<int>(sec * 1000 + usec / 1000);

  return handle_EINTR([&]() { return poll(&pfd_read, 1, timeout); });
#else
  fd_set fds;
  FD_ZERO(&fds);
  FD_SET(sock, &fds);

  timeval tv;
  tv.tv_sec = static_cast<long>(sec);
  tv.tv_usec = static_cast<decltype(tv.tv_usec)>(usec);

  return handle_EINTR([&]() {
    return select(static_cast<int>(sock + 1), nullptr, &fds, nullptr, &tv);
  });
#endif
}

int HTTPServiceManager::recvMessage()
{
    return 0;
}

int HTTPServiceManager::sendMessage()
{
    return 0;
}

socket_t HTTPServiceManager::create_socket()
{
    // Get address info
    int rc = 0;
    const char *node = m_ip.c_str();
    struct addrinfo hints;

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = 0;
    // Ask getaddrinfo to convert IP in c-string to address
    hints.ai_family = AF_INET;

    //hints->ai_family == AF_INET6
    //hints.ai_flags = AI_NUMERICHOST;
    auto service = std::to_string(m_port);
    auto sock = socket(hints->ai_family, hints->ai_socktype,
                    hints->ai_protocol);
    if (sock == INVALID_SOCKET) {
        rc = -errno;
        return rc;
    }
    int yes = 1;
    setsockopt(sock, IPPROTO_TCP, TCP_NODELAY,
                    reinterpret_cast<char *>(&yes),
                    sizeof(yes));
#if 0
    int no = 0;
    setsockopt(sock, IPPROTO_IPV6, IPV6_V6ONLY,
                    reinterpret_cast<char *>(&no),
                    sizeof(no));
 #endif

    rc = bind(sock, ai.ai_addr, static_cast<socklen_t>(ai.ai_addrlen)));
    if (rc < 0){
        rc = -errno;
        return rc;
    }
    rc = listen(sock, CPPHTTPLIB_LISTEN_BACKLOG));
    if (rc < 0){
        rc = -errno;
        close(sock);
        return rc;
    }

    m_socket = sock;
    return 0;
}
#endif

int HTTPServiceManager::custom_dispather(Request & req, Response & res)
{
    int rc = 0;
    std::string uri = req.path;
    std::string params = req.params;
    std::string method = req.method;

    std::shared_ptr<cJSON> repBodyPtr;
    std::shared_ptr<cJSON> reqBodyPtr = std::shared_ptr<cJSON>(
                    cJSON_ParseWithLength(req.body.c_str(), req.body.size()),
                    [](cJSON * root){
                        if (root != nullptr) {
                            cJSON_Delete(root);
                        }
                    }
    );


    rc = m_rootControllerPtr->dispather(uri, params, method, reqBodyPtr, repBodyPtr);
    if (rc < 0) {
        return rc;
    }
    return 0;
}

int HTTPServiceManager::custom_dispather_for_content_reader(
                Request & req,
                Response & res,
                ContentReader content_reader)
{
    int rc = 0;
    std::string uri = req.path;
    std::string params = req.params;
    std::shared_ptr<cJSON> rootPtr = std::shared_ptr<cJSON>(
                    cJSON_ParseWithLength(req.body.c_str(), req.body.size()),
                    [](cJSON * root){
                        if (root != nullptr) {
                            cJSON_Delete(root);
                        }
                    }
    );

    rc = m_rootControllerPtr->dispather(uri, params, content_reader);
    if (rc < 0) {
        return rc;
    }
    return 0;
}

int HTTPServiceManager::run()
{
    listen(0.0.0.0, 8888);
    return 0;
}

