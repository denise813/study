#include "httplib.h"
namespace httplib {



// HTTP client implementation
ClientImpl::ClientImpl(const std::string &host)
    : ClientImpl(host, 80, std::string(), std::string()) {}

ClientImpl::ClientImpl(const std::string &host, int port)
    : ClientImpl(host, port, std::string(), std::string()) {}

ClientImpl::ClientImpl(const std::string &host, int port,
                              const std::string &client_cert_path,
                              const std::string &client_key_path)
    : host_(host), port_(port),
      host_and_port_(adjust_host_string(host) + ":" + std::to_string(port)),
      client_cert_path_(client_cert_path), client_key_path_(client_key_path) {}

ClientImpl::~ClientImpl() {
  std::lock_guard<std::mutex> guard(socket_mutex_);
  shutdown_socket(socket_);
  close_socket(socket_);
}

bool ClientImpl::is_valid() const { return true; }

void ClientImpl::copy_settings(const ClientImpl &rhs) {
  client_cert_path_ = rhs.client_cert_path_;
  client_key_path_ = rhs.client_key_path_;
  connection_timeout_sec_ = rhs.connection_timeout_sec_;
  read_timeout_sec_ = rhs.read_timeout_sec_;
  read_timeout_usec_ = rhs.read_timeout_usec_;
  write_timeout_sec_ = rhs.write_timeout_sec_;
  write_timeout_usec_ = rhs.write_timeout_usec_;
  basic_auth_username_ = rhs.basic_auth_username_;
  basic_auth_password_ = rhs.basic_auth_password_;
  bearer_token_auth_token_ = rhs.bearer_token_auth_token_;
#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
  digest_auth_username_ = rhs.digest_auth_username_;
  digest_auth_password_ = rhs.digest_auth_password_;
#endif
  keep_alive_ = rhs.keep_alive_;
  follow_location_ = rhs.follow_location_;
  url_encode_ = rhs.url_encode_;
  address_family_ = rhs.address_family_;
  tcp_nodelay_ = rhs.tcp_nodelay_;
  socket_options_ = rhs.socket_options_;
  compress_ = rhs.compress_;
  decompress_ = rhs.decompress_;
  interface_ = rhs.interface_;
  proxy_host_ = rhs.proxy_host_;
  proxy_port_ = rhs.proxy_port_;
  proxy_basic_auth_username_ = rhs.proxy_basic_auth_username_;
  proxy_basic_auth_password_ = rhs.proxy_basic_auth_password_;
  proxy_bearer_token_auth_token_ = rhs.proxy_bearer_token_auth_token_;
#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
  proxy_digest_auth_username_ = rhs.proxy_digest_auth_username_;
  proxy_digest_auth_password_ = rhs.proxy_digest_auth_password_;
#endif
#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
  ca_cert_file_path_ = rhs.ca_cert_file_path_;
  ca_cert_dir_path_ = rhs.ca_cert_dir_path_;
  ca_cert_store_ = rhs.ca_cert_store_;
#endif
#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
  server_certificate_verification_ = rhs.server_certificate_verification_;
#endif
  logger_ = rhs.logger_;
}

socket_t ClientImpl::create_client_socket(Error &error) const {
  if (!proxy_host_.empty() && proxy_port_ != -1) {
    return detail::create_client_socket(
        proxy_host_, std::string(), proxy_port_, address_family_, tcp_nodelay_,
        socket_options_, connection_timeout_sec_, connection_timeout_usec_,
        read_timeout_sec_, read_timeout_usec_, write_timeout_sec_,
        write_timeout_usec_, interface_, error);
  }

  // Check is custom IP specified for host_
  std::string ip;
  auto it = addr_map_.find(host_);
  if (it != addr_map_.end()) { ip = it->second; }

  return detail::create_client_socket(
      host_, ip, port_, address_family_, tcp_nodelay_, socket_options_,
      connection_timeout_sec_, connection_timeout_usec_, read_timeout_sec_,
      read_timeout_usec_, write_timeout_sec_, write_timeout_usec_, interface_,
      error);
}

bool ClientImpl::create_and_connect_socket(Socket &socket,
                                                  Error &error) {
  auto sock = create_client_socket(error);
  if (sock == INVALID_SOCKET) { return false; }
  socket.sock = sock;
  return true;
}

void ClientImpl::shutdown_ssl(Socket & /*socket*/,
                                     bool /*shutdown_gracefully*/) {
  // If there are any requests in flight from threads other than us, then it's
  // a thread-unsafe race because individual ssl* objects are not thread-safe.
  assert(socket_requests_in_flight_ == 0 ||
         socket_requests_are_from_thread_ == std::this_thread::get_id());
}

void ClientImpl::shutdown_socket(Socket &socket) const {
  if (socket.sock == INVALID_SOCKET) { return; }
  detail::shutdown_socket(socket.sock);
}

void ClientImpl::close_socket(Socket &socket) {
  // If there are requests in flight in another thread, usually closing
  // the socket will be fine and they will simply receive an error when
  // using the closed socket, but it is still a bug since rarely the OS
  // may reassign the socket id to be used for a new socket, and then
  // suddenly they will be operating on a live socket that is different
  // than the one they intended!
  assert(socket_requests_in_flight_ == 0 ||
         socket_requests_are_from_thread_ == std::this_thread::get_id());

  // It is also a bug if this happens while SSL is still active
#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
  assert(socket.ssl == nullptr);
#endif
  if (socket.sock == INVALID_SOCKET) { return; }
  detail::close_socket(socket.sock);
  socket.sock = INVALID_SOCKET;
}

bool ClientImpl::read_response_line(Stream &strm, const Request &req,
                                           Response &res) const {
  std::array<char, 2048> buf{};

  detail::stream_line_reader line_reader(strm, buf.data(), buf.size());

  if (!line_reader.getline()) { return false; }

#ifdef CPPHTTPLIB_ALLOW_LF_AS_LINE_TERMINATOR
  const static std::regex re("(HTTP/1\\.[01]) (\\d{3})(?: (.*?))?\r?\n");
#else
  const static std::regex re("(HTTP/1\\.[01]) (\\d{3})(?: (.*?))?\r\n");
#endif

  std::cmatch m;
  if (!std::regex_match(line_reader.ptr(), m, re)) {
    return req.method == "CONNECT";
  }
  res.version = std::string(m[1]);
  res.status = std::stoi(std::string(m[2]));
  res.reason = std::string(m[3]);

  // Ignore '100 Continue'
  while (res.status == StatusCode::Continue_100) {
    if (!line_reader.getline()) { return false; } // CRLF
    if (!line_reader.getline()) { return false; } // next response line

    if (!std::regex_match(line_reader.ptr(), m, re)) { return false; }
    res.version = std::string(m[1]);
    res.status = std::stoi(std::string(m[2]));
    res.reason = std::string(m[3]);
  }

  return true;
}

bool ClientImpl::send(Request &req, Response &res, Error &error) {
  std::lock_guard<std::recursive_mutex> request_mutex_guard(request_mutex_);
  auto ret = send_(req, res, error);
  if (error == Error::SSLPeerCouldBeClosed_) {
    assert(!ret);
    ret = send_(req, res, error);
  }
  return ret;
}

bool ClientImpl::send_(Request &req, Response &res, Error &error) {
  {
    std::lock_guard<std::mutex> guard(socket_mutex_);

    // Set this to false immediately - if it ever gets set to true by the end of
    // the request, we know another thread instructed us to close the socket.
    socket_should_be_closed_when_request_is_done_ = false;

    auto is_alive = false;
    if (socket_.is_open()) {
      is_alive = detail::is_socket_alive(socket_.sock);
      if (!is_alive) {
        // Attempt to avoid sigpipe by shutting down nongracefully if it seems
        // like the other side has already closed the connection Also, there
        // cannot be any requests in flight from other threads since we locked
        // request_mutex_, so safe to close everything immediately
        const bool shutdown_gracefully = false;
        shutdown_ssl(socket_, shutdown_gracefully);
        shutdown_socket(socket_);
        close_socket(socket_);
      }
    }

    if (!is_alive) {
      if (!create_and_connect_socket(socket_, error)) { return false; }

#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
      // TODO: refactoring
      if (is_ssl()) {
        auto &scli = static_cast<SSLClient &>(*this);
        if (!proxy_host_.empty() && proxy_port_ != -1) {
          auto success = false;
          if (!scli.connect_with_proxy(socket_, res, success, error)) {
            return success;
          }
        }

        if (!scli.initialize_ssl(socket_, error)) { return false; }
      }
#endif
    }

    // Mark the current socket as being in use so that it cannot be closed by
    // anyone else while this request is ongoing, even though we will be
    // releasing the mutex.
    if (socket_requests_in_flight_ > 1) {
      assert(socket_requests_are_from_thread_ == std::this_thread::get_id());
    }
    socket_requests_in_flight_ += 1;
    socket_requests_are_from_thread_ = std::this_thread::get_id();
  }

  for (const auto &header : default_headers_) {
    if (req.headers.find(header.first) == req.headers.end()) {
      req.headers.insert(header);
    }
  }

  auto ret = false;
  auto close_connection = !keep_alive_;

  auto se = detail::scope_exit([&]() {
    // Briefly lock mutex in order to mark that a request is no longer ongoing
    std::lock_guard<std::mutex> guard(socket_mutex_);
    socket_requests_in_flight_ -= 1;
    if (socket_requests_in_flight_ <= 0) {
      assert(socket_requests_in_flight_ == 0);
      socket_requests_are_from_thread_ = std::thread::id();
    }

    if (socket_should_be_closed_when_request_is_done_ || close_connection ||
        !ret) {
      shutdown_ssl(socket_, true);
      shutdown_socket(socket_);
      close_socket(socket_);
    }
  });

  ret = process_socket(socket_, [&](Stream &strm) {
    return handle_request(strm, req, res, close_connection, error);
  });

  if (!ret) {
    if (error == Error::Success) { error = Error::Unknown; }
  }

  return ret;
}

Result ClientImpl::send(const Request &req) {
  auto req2 = req;
  return send_(std::move(req2));
}

Result ClientImpl::send_(Request &&req) {
  auto res = detail::make_unique<Response>();
  auto error = Error::Success;
  auto ret = send(req, *res, error);
  return Result{ret ? std::move(res) : nullptr, error, std::move(req.headers)};
}

bool ClientImpl::handle_request(Stream &strm, Request &req,
                                       Response &res, bool close_connection,
                                       Error &error) {
  if (req.path.empty()) {
    error = Error::Connection;
    return false;
  }

  auto req_save = req;

  bool ret;

  if (!is_ssl() && !proxy_host_.empty() && proxy_port_ != -1) {
    auto req2 = req;
    req2.path = "http://" + host_and_port_ + req.path;
    ret = process_request(strm, req2, res, close_connection, error);
    req = req2;
    req.path = req_save.path;
  } else {
    ret = process_request(strm, req, res, close_connection, error);
  }

  if (!ret) { return false; }

  if (res.get_header_value("Connection") == "close" ||
      (res.version == "HTTP/1.0" && res.reason != "Connection established")) {
    // TODO this requires a not-entirely-obvious chain of calls to be correct
    // for this to be safe.

    // This is safe to call because handle_request is only called by send_
    // which locks the request mutex during the process. It would be a bug
    // to call it from a different thread since it's a thread-safety issue
    // to do these things to the socket if another thread is using the socket.
    std::lock_guard<std::mutex> guard(socket_mutex_);
    shutdown_ssl(socket_, true);
    shutdown_socket(socket_);
    close_socket(socket_);
  }

  if (300 < res.status && res.status < 400 && follow_location_) {
    req = req_save;
    ret = redirect(req, res, error);
  }

#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
  if ((res.status == StatusCode::Unauthorized_401 ||
       res.status == StatusCode::ProxyAuthenticationRequired_407) &&
      req.authorization_count_ < 5) {
    auto is_proxy = res.status == StatusCode::ProxyAuthenticationRequired_407;
    const auto &username =
        is_proxy ? proxy_digest_auth_username_ : digest_auth_username_;
    const auto &password =
        is_proxy ? proxy_digest_auth_password_ : digest_auth_password_;

    if (!username.empty() && !password.empty()) {
      std::map<std::string, std::string> auth;
      if (detail::parse_www_authenticate(res, auth, is_proxy)) {
        Request new_req = req;
        new_req.authorization_count_ += 1;
        new_req.headers.erase(is_proxy ? "Proxy-Authorization"
                                       : "Authorization");
        new_req.headers.insert(detail::make_digest_authentication_header(
            req, auth, new_req.authorization_count_, detail::random_string(10),
            username, password, is_proxy));

        Response new_res;

        ret = send(new_req, new_res, error);
        if (ret) { res = new_res; }
      }
    }
  }
#endif

  return ret;
}

bool ClientImpl::redirect(Request &req, Response &res, Error &error) {
  if (req.redirect_count_ == 0) {
    error = Error::ExceedRedirectCount;
    return false;
  }

  auto location = res.get_header_value("location");
  if (location.empty()) { return false; }

  const static std::regex re(
      R"((?:(https?):)?(?://(?:\[([\d:]+)\]|([^:/?#]+))(?::(\d+))?)?([^?#]*)(\?[^#]*)?(?:#.*)?)");

  std::smatch m;
  if (!std::regex_match(location, m, re)) { return false; }

  auto scheme = is_ssl() ? "https" : "http";

  auto next_scheme = m[1].str();
  auto next_host = m[2].str();
  if (next_host.empty()) { next_host = m[3].str(); }
  auto port_str = m[4].str();
  auto next_path = m[5].str();
  auto next_query = m[6].str();

  auto next_port = port_;
  if (!port_str.empty()) {
    next_port = std::stoi(port_str);
  } else if (!next_scheme.empty()) {
    next_port = next_scheme == "https" ? 443 : 80;
  }

  if (next_scheme.empty()) { next_scheme = scheme; }
  if (next_host.empty()) { next_host = host_; }
  if (next_path.empty()) { next_path = "/"; }

  auto path = detail::decode_url(next_path, true) + next_query;

  if (next_scheme == scheme && next_host == host_ && next_port == port_) {
    return detail::redirect(*this, req, res, path, location, error);
  } else {
    if (next_scheme == "https") {
#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
      SSLClient cli(next_host, next_port);
      cli.copy_settings(*this);
      if (ca_cert_store_) { cli.set_ca_cert_store(ca_cert_store_); }
      return detail::redirect(cli, req, res, path, location, error);
#else
      return false;
#endif
    } else {
      ClientImpl cli(next_host, next_port);
      cli.copy_settings(*this);
      return detail::redirect(cli, req, res, path, location, error);
    }
  }
}

bool ClientImpl::write_content_with_provider(Stream &strm,
                                                    const Request &req,
                                                    Error &error) const {
  auto is_shutting_down = []() { return false; };

  if (req.is_chunked_content_provider_) {
    // TODO: Brotli support
    std::unique_ptr<detail::compressor> compressor;
#ifdef CPPHTTPLIB_ZLIB_SUPPORT
    if (compress_) {
      compressor = detail::make_unique<detail::gzip_compressor>();
    } else
#endif
    {
      compressor = detail::make_unique<detail::nocompressor>();
    }

    return detail::write_content_chunked(strm, req.content_provider_,
                                         is_shutting_down, *compressor, error);
  } else {
    return detail::write_content(strm, req.content_provider_, 0,
                                 req.content_length_, is_shutting_down, error);
  }
}

bool ClientImpl::write_request(Stream &strm, Request &req,
                                      bool close_connection, Error &error) {
  // Prepare additional headers
  if (close_connection) {
    if (!req.has_header("Connection")) {
      req.set_header("Connection", "close");
    }
  }

  if (!req.has_header("Host")) {
    if (is_ssl()) {
      if (port_ == 443) {
        req.set_header("Host", host_);
      } else {
        req.set_header("Host", host_and_port_);
      }
    } else {
      if (port_ == 80) {
        req.set_header("Host", host_);
      } else {
        req.set_header("Host", host_and_port_);
      }
    }
  }

  if (!req.has_header("Accept")) { req.set_header("Accept", "*/*"); }

#ifndef CPPHTTPLIB_NO_DEFAULT_USER_AGENT
  if (!req.has_header("User-Agent")) {
    auto agent = std::string("cpp-httplib/") + CPPHTTPLIB_VERSION;
    req.set_header("User-Agent", agent);
  }
#endif

  if (req.body.empty()) {
    if (req.content_provider_) {
      if (!req.is_chunked_content_provider_) {
        if (!req.has_header("Content-Length")) {
          auto length = std::to_string(req.content_length_);
          req.set_header("Content-Length", length);
        }
      }
    } else {
      if (req.method == "POST" || req.method == "PUT" ||
          req.method == "PATCH") {
        req.set_header("Content-Length", "0");
      }
    }
  } else {
    if (!req.has_header("Content-Type")) {
      req.set_header("Content-Type", "text/plain");
    }

    if (!req.has_header("Content-Length")) {
      auto length = std::to_string(req.body.size());
      req.set_header("Content-Length", length);
    }
  }

  if (!basic_auth_password_.empty() || !basic_auth_username_.empty()) {
    if (!req.has_header("Authorization")) {
      req.headers.insert(make_basic_authentication_header(
          basic_auth_username_, basic_auth_password_, false));
    }
  }

  if (!proxy_basic_auth_username_.empty() &&
      !proxy_basic_auth_password_.empty()) {
    if (!req.has_header("Proxy-Authorization")) {
      req.headers.insert(make_basic_authentication_header(
          proxy_basic_auth_username_, proxy_basic_auth_password_, true));
    }
  }

  if (!bearer_token_auth_token_.empty()) {
    if (!req.has_header("Authorization")) {
      req.headers.insert(make_bearer_token_authentication_header(
          bearer_token_auth_token_, false));
    }
  }

  if (!proxy_bearer_token_auth_token_.empty()) {
    if (!req.has_header("Proxy-Authorization")) {
      req.headers.insert(make_bearer_token_authentication_header(
          proxy_bearer_token_auth_token_, true));
    }
  }

  // Request line and headers
  {
    detail::BufferStream bstrm;

    const auto &path = url_encode_ ? detail::encode_url(req.path) : req.path;
    bstrm.write_format("%s %s HTTP/1.1\r\n", req.method.c_str(), path.c_str());

    header_writer_(bstrm, req.headers);

    // Flush buffer
    auto &data = bstrm.get_buffer();
    if (!detail::write_data(strm, data.data(), data.size())) {
      error = Error::Write;
      return false;
    }
  }

  // Body
  if (req.body.empty()) {
    return write_content_with_provider(strm, req, error);
  }

  if (!detail::write_data(strm, req.body.data(), req.body.size())) {
    error = Error::Write;
    return false;
  }

  return true;
}

std::unique_ptr<Response> ClientImpl::send_with_content_provider(
    Request &req, const char *body, size_t content_length,
    ContentProvider content_provider,
    ContentProviderWithoutLength content_provider_without_length,
    const std::string &content_type, Error &error) {
  if (!content_type.empty()) { req.set_header("Content-Type", content_type); }

#ifdef CPPHTTPLIB_ZLIB_SUPPORT
  if (compress_) { req.set_header("Content-Encoding", "gzip"); }
#endif

#ifdef CPPHTTPLIB_ZLIB_SUPPORT
  if (compress_ && !content_provider_without_length) {
    // TODO: Brotli support
    detail::gzip_compressor compressor;

    if (content_provider) {
      auto ok = true;
      size_t offset = 0;
      DataSink data_sink;

      data_sink.write = [&](const char *data, size_t data_len) -> bool {
        if (ok) {
          auto last = offset + data_len == content_length;

          auto ret = compressor.compress(
              data, data_len, last,
              [&](const char *compressed_data, size_t compressed_data_len) {
                req.body.append(compressed_data, compressed_data_len);
                return true;
              });

          if (ret) {
            offset += data_len;
          } else {
            ok = false;
          }
        }
        return ok;
      };

      while (ok && offset < content_length) {
        if (!content_provider(offset, content_length - offset, data_sink)) {
          error = Error::Canceled;
          return nullptr;
        }
      }
    } else {
      if (!compressor.compress(body, content_length, true,
                               [&](const char *data, size_t data_len) {
                                 req.body.append(data, data_len);
                                 return true;
                               })) {
        error = Error::Compression;
        return nullptr;
      }
    }
  } else
#endif
  {
    if (content_provider) {
      req.content_length_ = content_length;
      req.content_provider_ = std::move(content_provider);
      req.is_chunked_content_provider_ = false;
    } else if (content_provider_without_length) {
      req.content_length_ = 0;
      req.content_provider_ = detail::ContentProviderAdapter(
          std::move(content_provider_without_length));
      req.is_chunked_content_provider_ = true;
      req.set_header("Transfer-Encoding", "chunked");
    } else {
      req.body.assign(body, content_length);
    }
  }

  auto res = detail::make_unique<Response>();
  return send(req, *res, error) ? std::move(res) : nullptr;
}

Result ClientImpl::send_with_content_provider(
    const std::string &method, const std::string &path, const Headers &headers,
    const char *body, size_t content_length, ContentProvider content_provider,
    ContentProviderWithoutLength content_provider_without_length,
    const std::string &content_type) {
  Request req;
  req.method = method;
  req.headers = headers;
  req.path = path;

  auto error = Error::Success;

  auto res = send_with_content_provider(
      req, body, content_length, std::move(content_provider),
      std::move(content_provider_without_length), content_type, error);

  return Result{std::move(res), error, std::move(req.headers)};
}

std::string
ClientImpl::adjust_host_string(const std::string &host) const {
  if (host.find(':') != std::string::npos) { return "[" + host + "]"; }
  return host;
}

bool ClientImpl::process_request(Stream &strm, Request &req,
                                        Response &res, bool close_connection,
                                        Error &error) {
  // Send request
  if (!write_request(strm, req, close_connection, error)) { return false; }

#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
  if (is_ssl()) {
    auto is_proxy_enabled = !proxy_host_.empty() && proxy_port_ != -1;
    if (!is_proxy_enabled) {
      char buf[1];
      if (SSL_peek(socket_.ssl, buf, 1) == 0 &&
          SSL_get_error(socket_.ssl, 0) == SSL_ERROR_ZERO_RETURN) {
        error = Error::SSLPeerCouldBeClosed_;
        return false;
      }
    }
  }
#endif

  // Receive response and headers
  if (!read_response_line(strm, req, res) ||
      !detail::read_headers(strm, res.headers)) {
    error = Error::Read;
    return false;
  }

  // Body
  if ((res.status != StatusCode::NoContent_204) && req.method != "HEAD" &&
      req.method != "CONNECT") {
    auto redirect = 300 < res.status && res.status < 400 && follow_location_;

    if (req.response_handler && !redirect) {
      if (!req.response_handler(res)) {
        error = Error::Canceled;
        return false;
      }
    }

    auto out =
        req.content_receiver
            ? static_cast<ContentReceiverWithProgress>(
                  [&](const char *buf, size_t n, uint64_t off, uint64_t len) {
                    if (redirect) { return true; }
                    auto ret = req.content_receiver(buf, n, off, len);
                    if (!ret) { error = Error::Canceled; }
                    return ret;
                  })
            : static_cast<ContentReceiverWithProgress>(
                  [&](const char *buf, size_t n, uint64_t /*off*/,
                      uint64_t /*len*/) {
                    if (res.body.size() + n > res.body.max_size()) {
                      return false;
                    }
                    res.body.append(buf, n);
                    return true;
                  });

    auto progress = [&](uint64_t current, uint64_t total) {
      if (!req.progress || redirect) { return true; }
      auto ret = req.progress(current, total);
      if (!ret) { error = Error::Canceled; }
      return ret;
    };

    int dummy_status;
    if (!detail::read_content(strm, res, (std::numeric_limits<size_t>::max)(),
                              dummy_status, std::move(progress), std::move(out),
                              decompress_)) {
      if (error != Error::Canceled) { error = Error::Read; }
      return false;
    }
  }

  // Log
  if (logger_) { logger_(req, res); }

  return true;
}

ContentProviderWithoutLength ClientImpl::get_multipart_content_provider(
    const std::string &boundary, const MultipartFormDataItems &items,
    const MultipartFormDataProviderItems &provider_items) const {
  size_t cur_item = 0;
  size_t cur_start = 0;
  // cur_item and cur_start are copied to within the std::function and maintain
  // state between successive calls
  return [&, cur_item, cur_start](size_t offset,
                                  DataSink &sink) mutable -> bool {
    if (!offset && !items.empty()) {
      sink.os << detail::serialize_multipart_formdata(items, boundary, false);
      return true;
    } else if (cur_item < provider_items.size()) {
      if (!cur_start) {
        const auto &begin = detail::serialize_multipart_formdata_item_begin(
            provider_items[cur_item], boundary);
        offset += begin.size();
        cur_start = offset;
        sink.os << begin;
      }

      DataSink cur_sink;
      auto has_data = true;
      cur_sink.write = sink.write;
      cur_sink.done = [&]() { has_data = false; };

      if (!provider_items[cur_item].provider(offset - cur_start, cur_sink)) {
        return false;
      }

      if (!has_data) {
        sink.os << detail::serialize_multipart_formdata_item_end();
        cur_item++;
        cur_start = 0;
      }
      return true;
    } else {
      sink.os << detail::serialize_multipart_formdata_finish(boundary);
      sink.done();
      return true;
    }
  };
}

bool
ClientImpl::process_socket(const Socket &socket,
                           std::function<bool(Stream &strm)> callback) {
  return detail::process_client_socket(
      socket.sock, read_timeout_sec_, read_timeout_usec_, write_timeout_sec_,
      write_timeout_usec_, std::move(callback));
}

bool ClientImpl::is_ssl() const { return false; }

Result ClientImpl::Get(const std::string &path) {
  return Get(path, Headers(), Progress());
}

Result ClientImpl::Get(const std::string &path, Progress progress) {
  return Get(path, Headers(), std::move(progress));
}

Result ClientImpl::Get(const std::string &path, const Headers &headers) {
  return Get(path, headers, Progress());
}

Result ClientImpl::Get(const std::string &path, const Headers &headers,
                              Progress progress) {
  Request req;
  req.method = "GET";
  req.path = path;
  req.headers = headers;
  req.progress = std::move(progress);

  return send_(std::move(req));
}

Result ClientImpl::Get(const std::string &path,
                              ContentReceiver content_receiver) {
  return Get(path, Headers(), nullptr, std::move(content_receiver), nullptr);
}

Result ClientImpl::Get(const std::string &path,
                              ContentReceiver content_receiver,
                              Progress progress) {
  return Get(path, Headers(), nullptr, std::move(content_receiver),
             std::move(progress));
}

Result ClientImpl::Get(const std::string &path, const Headers &headers,
                              ContentReceiver content_receiver) {
  return Get(path, headers, nullptr, std::move(content_receiver), nullptr);
}

Result ClientImpl::Get(const std::string &path, const Headers &headers,
                              ContentReceiver content_receiver,
                              Progress progress) {
  return Get(path, headers, nullptr, std::move(content_receiver),
             std::move(progress));
}

Result ClientImpl::Get(const std::string &path,
                              ResponseHandler response_handler,
                              ContentReceiver content_receiver) {
  return Get(path, Headers(), std::move(response_handler),
             std::move(content_receiver), nullptr);
}

Result ClientImpl::Get(const std::string &path, const Headers &headers,
                              ResponseHandler response_handler,
                              ContentReceiver content_receiver) {
  return Get(path, headers, std::move(response_handler),
             std::move(content_receiver), nullptr);
}

Result ClientImpl::Get(const std::string &path,
                              ResponseHandler response_handler,
                              ContentReceiver content_receiver,
                              Progress progress) {
  return Get(path, Headers(), std::move(response_handler),
             std::move(content_receiver), std::move(progress));
}

Result ClientImpl::Get(const std::string &path, const Headers &headers,
                              ResponseHandler response_handler,
                              ContentReceiver content_receiver,
                              Progress progress) {
  Request req;
  req.method = "GET";
  req.path = path;
  req.headers = headers;
  req.response_handler = std::move(response_handler);
  req.content_receiver =
      [content_receiver](const char *data, size_t data_length,
                         uint64_t /*offset*/, uint64_t /*total_length*/) {
        return content_receiver(data, data_length);
      };
  req.progress = std::move(progress);

  return send_(std::move(req));
}

Result ClientImpl::Get(const std::string &path, const Params &params,
                              const Headers &headers, Progress progress) {
  if (params.empty()) { return Get(path, headers); }

  std::string path_with_query = append_query_params(path, params);
  return Get(path_with_query, headers, std::move(progress));
}

Result ClientImpl::Get(const std::string &path, const Params &params,
                              const Headers &headers,
                              ContentReceiver content_receiver,
                              Progress progress) {
  return Get(path, params, headers, nullptr, std::move(content_receiver),
             std::move(progress));
}

Result ClientImpl::Get(const std::string &path, const Params &params,
                              const Headers &headers,
                              ResponseHandler response_handler,
                              ContentReceiver content_receiver,
                              Progress progress) {
  if (params.empty()) {
    return Get(path, headers, std::move(response_handler),
               std::move(content_receiver), std::move(progress));
  }

  std::string path_with_query = append_query_params(path, params);
  return Get(path_with_query, headers, std::move(response_handler),
             std::move(content_receiver), std::move(progress));
}

Result ClientImpl::Head(const std::string &path) {
  return Head(path, Headers());
}

Result ClientImpl::Head(const std::string &path,
                               const Headers &headers) {
  Request req;
  req.method = "HEAD";
  req.headers = headers;
  req.path = path;

  return send_(std::move(req));
}

Result ClientImpl::Post(const std::string &path) {
  return Post(path, std::string(), std::string());
}

Result ClientImpl::Post(const std::string &path,
                               const Headers &headers) {
  return Post(path, headers, nullptr, 0, std::string());
}

Result ClientImpl::Post(const std::string &path, const char *body,
                               size_t content_length,
                               const std::string &content_type) {
  return Post(path, Headers(), body, content_length, content_type);
}

Result ClientImpl::Post(const std::string &path, const Headers &headers,
                               const char *body, size_t content_length,
                               const std::string &content_type) {
  return send_with_content_provider("POST", path, headers, body, content_length,
                                    nullptr, nullptr, content_type);
}

Result ClientImpl::Post(const std::string &path, const std::string &body,
                               const std::string &content_type) {
  return Post(path, Headers(), body, content_type);
}

Result ClientImpl::Post(const std::string &path, const Headers &headers,
                               const std::string &body,
                               const std::string &content_type) {
  return send_with_content_provider("POST", path, headers, body.data(),
                                    body.size(), nullptr, nullptr,
                                    content_type);
}

Result ClientImpl::Post(const std::string &path, const Params &params) {
  return Post(path, Headers(), params);
}

Result ClientImpl::Post(const std::string &path, size_t content_length,
                               ContentProvider content_provider,
                               const std::string &content_type) {
  return Post(path, Headers(), content_length, std::move(content_provider),
              content_type);
}

Result ClientImpl::Post(const std::string &path,
                               ContentProviderWithoutLength content_provider,
                               const std::string &content_type) {
  return Post(path, Headers(), std::move(content_provider), content_type);
}

Result ClientImpl::Post(const std::string &path, const Headers &headers,
                               size_t content_length,
                               ContentProvider content_provider,
                               const std::string &content_type) {
  return send_with_content_provider("POST", path, headers, nullptr,
                                    content_length, std::move(content_provider),
                                    nullptr, content_type);
}

Result ClientImpl::Post(const std::string &path, const Headers &headers,
                               ContentProviderWithoutLength content_provider,
                               const std::string &content_type) {
  return send_with_content_provider("POST", path, headers, nullptr, 0, nullptr,
                                    std::move(content_provider), content_type);
}

Result ClientImpl::Post(const std::string &path, const Headers &headers,
                               const Params &params) {
  auto query = detail::params_to_query_str(params);
  return Post(path, headers, query, "application/x-www-form-urlencoded");
}

Result ClientImpl::Post(const std::string &path,
                               const MultipartFormDataItems &items) {
  return Post(path, Headers(), items);
}

Result ClientImpl::Post(const std::string &path, const Headers &headers,
                               const MultipartFormDataItems &items) {
  const auto &boundary = detail::make_multipart_data_boundary();
  const auto &content_type =
      detail::serialize_multipart_formdata_get_content_type(boundary);
  const auto &body = detail::serialize_multipart_formdata(items, boundary);
  return Post(path, headers, body, content_type);
}

Result ClientImpl::Post(const std::string &path, const Headers &headers,
                               const MultipartFormDataItems &items,
                               const std::string &boundary) {
  if (!detail::is_multipart_boundary_chars_valid(boundary)) {
    return Result{nullptr, Error::UnsupportedMultipartBoundaryChars};
  }

  const auto &content_type =
      detail::serialize_multipart_formdata_get_content_type(boundary);
  const auto &body = detail::serialize_multipart_formdata(items, boundary);
  return Post(path, headers, body, content_type);
}

Result
ClientImpl::Post(const std::string &path, const Headers &headers,
                 const MultipartFormDataItems &items,
                 const MultipartFormDataProviderItems &provider_items) {
  const auto &boundary = detail::make_multipart_data_boundary();
  const auto &content_type =
      detail::serialize_multipart_formdata_get_content_type(boundary);
  return send_with_content_provider(
      "POST", path, headers, nullptr, 0, nullptr,
      get_multipart_content_provider(boundary, items, provider_items),
      content_type);
}

Result ClientImpl::Put(const std::string &path) {
  return Put(path, std::string(), std::string());
}

Result ClientImpl::Put(const std::string &path, const char *body,
                              size_t content_length,
                              const std::string &content_type) {
  return Put(path, Headers(), body, content_length, content_type);
}

Result ClientImpl::Put(const std::string &path, const Headers &headers,
                              const char *body, size_t content_length,
                              const std::string &content_type) {
  return send_with_content_provider("PUT", path, headers, body, content_length,
                                    nullptr, nullptr, content_type);
}

Result ClientImpl::Put(const std::string &path, const std::string &body,
                              const std::string &content_type) {
  return Put(path, Headers(), body, content_type);
}

Result ClientImpl::Put(const std::string &path, const Headers &headers,
                              const std::string &body,
                              const std::string &content_type) {
  return send_with_content_provider("PUT", path, headers, body.data(),
                                    body.size(), nullptr, nullptr,
                                    content_type);
}

Result ClientImpl::Put(const std::string &path, size_t content_length,
                              ContentProvider content_provider,
                              const std::string &content_type) {
  return Put(path, Headers(), content_length, std::move(content_provider),
             content_type);
}

Result ClientImpl::Put(const std::string &path,
                              ContentProviderWithoutLength content_provider,
                              const std::string &content_type) {
  return Put(path, Headers(), std::move(content_provider), content_type);
}

Result ClientImpl::Put(const std::string &path, const Headers &headers,
                              size_t content_length,
                              ContentProvider content_provider,
                              const std::string &content_type) {
  return send_with_content_provider("PUT", path, headers, nullptr,
                                    content_length, std::move(content_provider),
                                    nullptr, content_type);
}

Result ClientImpl::Put(const std::string &path, const Headers &headers,
                              ContentProviderWithoutLength content_provider,
                              const std::string &content_type) {
  return send_with_content_provider("PUT", path, headers, nullptr, 0, nullptr,
                                    std::move(content_provider), content_type);
}

Result ClientImpl::Put(const std::string &path, const Params &params) {
  return Put(path, Headers(), params);
}

Result ClientImpl::Put(const std::string &path, const Headers &headers,
                              const Params &params) {
  auto query = detail::params_to_query_str(params);
  return Put(path, headers, query, "application/x-www-form-urlencoded");
}

Result ClientImpl::Put(const std::string &path,
                              const MultipartFormDataItems &items) {
  return Put(path, Headers(), items);
}

Result ClientImpl::Put(const std::string &path, const Headers &headers,
                              const MultipartFormDataItems &items) {
  const auto &boundary = detail::make_multipart_data_boundary();
  const auto &content_type =
      detail::serialize_multipart_formdata_get_content_type(boundary);
  const auto &body = detail::serialize_multipart_formdata(items, boundary);
  return Put(path, headers, body, content_type);
}

Result ClientImpl::Put(const std::string &path, const Headers &headers,
                              const MultipartFormDataItems &items,
                              const std::string &boundary) {
  if (!detail::is_multipart_boundary_chars_valid(boundary)) {
    return Result{nullptr, Error::UnsupportedMultipartBoundaryChars};
  }

  const auto &content_type =
      detail::serialize_multipart_formdata_get_content_type(boundary);
  const auto &body = detail::serialize_multipart_formdata(items, boundary);
  return Put(path, headers, body, content_type);
}

Result
ClientImpl::Put(const std::string &path, const Headers &headers,
                const MultipartFormDataItems &items,
                const MultipartFormDataProviderItems &provider_items) {
  const auto &boundary = detail::make_multipart_data_boundary();
  const auto &content_type =
      detail::serialize_multipart_formdata_get_content_type(boundary);
  return send_with_content_provider(
      "PUT", path, headers, nullptr, 0, nullptr,
      get_multipart_content_provider(boundary, items, provider_items),
      content_type);
}
Result ClientImpl::Patch(const std::string &path) {
  return Patch(path, std::string(), std::string());
}

Result ClientImpl::Patch(const std::string &path, const char *body,
                                size_t content_length,
                                const std::string &content_type) {
  return Patch(path, Headers(), body, content_length, content_type);
}

Result ClientImpl::Patch(const std::string &path, const Headers &headers,
                                const char *body, size_t content_length,
                                const std::string &content_type) {
  return send_with_content_provider("PATCH", path, headers, body,
                                    content_length, nullptr, nullptr,
                                    content_type);
}

Result ClientImpl::Patch(const std::string &path,
                                const std::string &body,
                                const std::string &content_type) {
  return Patch(path, Headers(), body, content_type);
}

Result ClientImpl::Patch(const std::string &path, const Headers &headers,
                                const std::string &body,
                                const std::string &content_type) {
  return send_with_content_provider("PATCH", path, headers, body.data(),
                                    body.size(), nullptr, nullptr,
                                    content_type);
}

Result ClientImpl::Patch(const std::string &path, size_t content_length,
                                ContentProvider content_provider,
                                const std::string &content_type) {
  return Patch(path, Headers(), content_length, std::move(content_provider),
               content_type);
}

Result ClientImpl::Patch(const std::string &path,
                                ContentProviderWithoutLength content_provider,
                                const std::string &content_type) {
  return Patch(path, Headers(), std::move(content_provider), content_type);
}

Result ClientImpl::Patch(const std::string &path, const Headers &headers,
                                size_t content_length,
                                ContentProvider content_provider,
                                const std::string &content_type) {
  return send_with_content_provider("PATCH", path, headers, nullptr,
                                    content_length, std::move(content_provider),
                                    nullptr, content_type);
}

Result ClientImpl::Patch(const std::string &path, const Headers &headers,
                                ContentProviderWithoutLength content_provider,
                                const std::string &content_type) {
  return send_with_content_provider("PATCH", path, headers, nullptr, 0, nullptr,
                                    std::move(content_provider), content_type);
}

Result ClientImpl::Delete(const std::string &path) {
  return Delete(path, Headers(), std::string(), std::string());
}

Result ClientImpl::Delete(const std::string &path,
                                 const Headers &headers) {
  return Delete(path, headers, std::string(), std::string());
}

Result ClientImpl::Delete(const std::string &path, const char *body,
                                 size_t content_length,
                                 const std::string &content_type) {
  return Delete(path, Headers(), body, content_length, content_type);
}

Result ClientImpl::Delete(const std::string &path,
                                 const Headers &headers, const char *body,
                                 size_t content_length,
                                 const std::string &content_type) {
  Request req;
  req.method = "DELETE";
  req.headers = headers;
  req.path = path;

  if (!content_type.empty()) { req.set_header("Content-Type", content_type); }
  req.body.assign(body, content_length);

  return send_(std::move(req));
}

Result ClientImpl::Delete(const std::string &path,
                                 const std::string &body,
                                 const std::string &content_type) {
  return Delete(path, Headers(), body.data(), body.size(), content_type);
}

Result ClientImpl::Delete(const std::string &path,
                                 const Headers &headers,
                                 const std::string &body,
                                 const std::string &content_type) {
  return Delete(path, headers, body.data(), body.size(), content_type);
}

Result ClientImpl::Options(const std::string &path) {
  return Options(path, Headers());
}

Result ClientImpl::Options(const std::string &path,
                                  const Headers &headers) {
  Request req;
  req.method = "OPTIONS";
  req.headers = headers;
  req.path = path;

  return send_(std::move(req));
}

void ClientImpl::stop() {
  std::lock_guard<std::mutex> guard(socket_mutex_);

  // If there is anything ongoing right now, the ONLY thread-safe thing we can
  // do is to shutdown_socket, so that threads using this socket suddenly
  // discover they can't read/write any more and error out. Everything else
  // (closing the socket, shutting ssl down) is unsafe because these actions are
  // not thread-safe.
  if (socket_requests_in_flight_ > 0) {
    shutdown_socket(socket_);

    // Aside from that, we set a flag for the socket to be closed when we're
    // done.
    socket_should_be_closed_when_request_is_done_ = true;
    return;
  }

  // Otherwise, still holding the mutex, we can shut everything down ourselves
  shutdown_ssl(socket_, true);
  shutdown_socket(socket_);
  close_socket(socket_);
}

std::string ClientImpl::host() const { return host_; }

int ClientImpl::port() const { return port_; }

size_t ClientImpl::is_socket_open() const {
  std::lock_guard<std::mutex> guard(socket_mutex_);
  return socket_.is_open();
}

socket_t ClientImpl::socket() const { return socket_.sock; }

void ClientImpl::set_connection_timeout(time_t sec, time_t usec) {
  connection_timeout_sec_ = sec;
  connection_timeout_usec_ = usec;
}

void ClientImpl::set_read_timeout(time_t sec, time_t usec) {
  read_timeout_sec_ = sec;
  read_timeout_usec_ = usec;
}

void ClientImpl::set_write_timeout(time_t sec, time_t usec) {
  write_timeout_sec_ = sec;
  write_timeout_usec_ = usec;
}

void ClientImpl::set_basic_auth(const std::string &username,
                                       const std::string &password) {
  basic_auth_username_ = username;
  basic_auth_password_ = password;
}

void ClientImpl::set_bearer_token_auth(const std::string &token) {
  bearer_token_auth_token_ = token;
}

#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
void ClientImpl::set_digest_auth(const std::string &username,
                                        const std::string &password) {
  digest_auth_username_ = username;
  digest_auth_password_ = password;
}
#endif

void ClientImpl::set_keep_alive(bool on) { keep_alive_ = on; }

void ClientImpl::set_follow_location(bool on) { follow_location_ = on; }

void ClientImpl::set_url_encode(bool on) { url_encode_ = on; }

void
ClientImpl::set_hostname_addr_map(std::map<std::string, std::string> addr_map) {
  addr_map_ = std::move(addr_map);
}

void ClientImpl::set_default_headers(Headers headers) {
  default_headers_ = std::move(headers);
}

void ClientImpl::set_header_writer(
    std::function<ssize_t(Stream &, Headers &)> const &writer) {
  header_writer_ = writer;
}

void ClientImpl::set_address_family(int family) {
  address_family_ = family;
}

void ClientImpl::set_tcp_nodelay(bool on) { tcp_nodelay_ = on; }

void ClientImpl::set_socket_options(SocketOptions socket_options) {
  socket_options_ = std::move(socket_options);
}

void ClientImpl::set_compress(bool on) { compress_ = on; }

void ClientImpl::set_decompress(bool on) { decompress_ = on; }

void ClientImpl::set_interface(const std::string &intf) {
  interface_ = intf;
}

void ClientImpl::set_proxy(const std::string &host, int port) {
  proxy_host_ = host;
  proxy_port_ = port;
}

void ClientImpl::set_proxy_basic_auth(const std::string &username,
                                             const std::string &password) {
  proxy_basic_auth_username_ = username;
  proxy_basic_auth_password_ = password;
}

void ClientImpl::set_proxy_bearer_token_auth(const std::string &token) {
  proxy_bearer_token_auth_token_ = token;
}

#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
void ClientImpl::set_proxy_digest_auth(const std::string &username,
                                              const std::string &password) {
  proxy_digest_auth_username_ = username;
  proxy_digest_auth_password_ = password;
}

void ClientImpl::set_ca_cert_path(const std::string &ca_cert_file_path,
                                         const std::string &ca_cert_dir_path) {
  ca_cert_file_path_ = ca_cert_file_path;
  ca_cert_dir_path_ = ca_cert_dir_path;
}

void ClientImpl::set_ca_cert_store(X509_STORE *ca_cert_store) {
  if (ca_cert_store && ca_cert_store != ca_cert_store_) {
    ca_cert_store_ = ca_cert_store;
  }
}

X509_STORE *ClientImpl::create_ca_cert_store(const char *ca_cert,
                                                    std::size_t size) const {
  auto mem = BIO_new_mem_buf(ca_cert, static_cast<int>(size));
  if (!mem) { return nullptr; }

  auto inf = PEM_X509_INFO_read_bio(mem, nullptr, nullptr, nullptr);
  if (!inf) {
    BIO_free_all(mem);
    return nullptr;
  }

  auto cts = X509_STORE_new();
  if (cts) {
    for (auto i = 0; i < static_cast<int>(sk_X509_INFO_num(inf)); i++) {
      auto itmp = sk_X509_INFO_value(inf, i);
      if (!itmp) { continue; }

      if (itmp->x509) { X509_STORE_add_cert(cts, itmp->x509); }
      if (itmp->crl) { X509_STORE_add_crl(cts, itmp->crl); }
    }
  }

  sk_X509_INFO_pop_free(inf, X509_INFO_free);
  BIO_free_all(mem);
  return cts;
}

void ClientImpl::enable_server_certificate_verification(bool enabled) {
  server_certificate_verification_ = enabled;
}
#endif

void ClientImpl::set_logger(Logger logger) {
  logger_ = std::move(logger);
}


// SSL HTTP client implementation
SSLClient::SSLClient(const std::string &host)
    : SSLClient(host, 443, std::string(), std::string()) {}

SSLClient::SSLClient(const std::string &host, int port)
    : SSLClient(host, port, std::string(), std::string()) {}

SSLClient::SSLClient(const std::string &host, int port,
                            const std::string &client_cert_path,
                            const std::string &client_key_path,
                            const std::string &private_key_password)
    : ClientImpl(host, port, client_cert_path, client_key_path) {
  ctx_ = SSL_CTX_new(TLS_client_method());

  detail::split(&host_[0], &host_[host_.size()], '.',
                [&](const char *b, const char *e) {
                  host_components_.emplace_back(b, e);
                });

  if (!client_cert_path.empty() && !client_key_path.empty()) {
    if (!private_key_password.empty()) {
      SSL_CTX_set_default_passwd_cb_userdata(
          ctx_, reinterpret_cast<void *>(
                    const_cast<char *>(private_key_password.c_str())));
    }

    if (SSL_CTX_use_certificate_file(ctx_, client_cert_path.c_str(),
                                     SSL_FILETYPE_PEM) != 1 ||
        SSL_CTX_use_PrivateKey_file(ctx_, client_key_path.c_str(),
                                    SSL_FILETYPE_PEM) != 1) {
      SSL_CTX_free(ctx_);
      ctx_ = nullptr;
    }
  }
}

SSLClient::SSLClient(const std::string &host, int port,
                            X509 *client_cert, EVP_PKEY *client_key,
                            const std::string &private_key_password)
    : ClientImpl(host, port) {
  ctx_ = SSL_CTX_new(TLS_client_method());

  detail::split(&host_[0], &host_[host_.size()], '.',
                [&](const char *b, const char *e) {
                  host_components_.emplace_back(b, e);
                });

  if (client_cert != nullptr && client_key != nullptr) {
    if (!private_key_password.empty()) {
      SSL_CTX_set_default_passwd_cb_userdata(
          ctx_, reinterpret_cast<void *>(
                    const_cast<char *>(private_key_password.c_str())));
    }

    if (SSL_CTX_use_certificate(ctx_, client_cert) != 1 ||
        SSL_CTX_use_PrivateKey(ctx_, client_key) != 1) {
      SSL_CTX_free(ctx_);
      ctx_ = nullptr;
    }
  }
}

SSLClient::~SSLClient() {
  if (ctx_) { SSL_CTX_free(ctx_); }
  // Make sure to shut down SSL since shutdown_ssl will resolve to the
  // base function rather than the derived function once we get to the
  // base class destructor, and won't free the SSL (causing a leak).
  shutdown_ssl_impl(socket_, true);
}

bool SSLClient::is_valid() const { return ctx_; }

void SSLClient::set_ca_cert_store(X509_STORE *ca_cert_store) {
  if (ca_cert_store) {
    if (ctx_) {
      if (SSL_CTX_get_cert_store(ctx_) != ca_cert_store) {
        // Free memory allocated for old cert and use new store `ca_cert_store`
        SSL_CTX_set_cert_store(ctx_, ca_cert_store);
      }
    } else {
      X509_STORE_free(ca_cert_store);
    }
  }
}

void SSLClient::load_ca_cert_store(const char *ca_cert,
                                          std::size_t size) {
  set_ca_cert_store(ClientImpl::create_ca_cert_store(ca_cert, size));
}

long SSLClient::get_openssl_verify_result() const {
  return verify_result_;
}

SSL_CTX *SSLClient::ssl_context() const { return ctx_; }

bool SSLClient::create_and_connect_socket(Socket &socket, Error &error) {
  return is_valid() && ClientImpl::create_and_connect_socket(socket, error);
}

// Assumes that socket_mutex_ is locked and that there are no requests in flight
bool SSLClient::connect_with_proxy(Socket &socket, Response &res,
                                          bool &success, Error &error) {
  success = true;
  Response proxy_res;
  if (!detail::process_client_socket(
          socket.sock, read_timeout_sec_, read_timeout_usec_,
          write_timeout_sec_, write_timeout_usec_, [&](Stream &strm) {
            Request req2;
            req2.method = "CONNECT";
            req2.path = host_and_port_;
            return process_request(strm, req2, proxy_res, false, error);
          })) {
    // Thread-safe to close everything because we are assuming there are no
    // requests in flight
    shutdown_ssl(socket, true);
    shutdown_socket(socket);
    close_socket(socket);
    success = false;
    return false;
  }

  if (proxy_res.status == StatusCode::ProxyAuthenticationRequired_407) {
    if (!proxy_digest_auth_username_.empty() &&
        !proxy_digest_auth_password_.empty()) {
      std::map<std::string, std::string> auth;
      if (detail::parse_www_authenticate(proxy_res, auth, true)) {
        proxy_res = Response();
        if (!detail::process_client_socket(
                socket.sock, read_timeout_sec_, read_timeout_usec_,
                write_timeout_sec_, write_timeout_usec_, [&](Stream &strm) {
                  Request req3;
                  req3.method = "CONNECT";
                  req3.path = host_and_port_;
                  req3.headers.insert(detail::make_digest_authentication_header(
                      req3, auth, 1, detail::random_string(10),
                      proxy_digest_auth_username_, proxy_digest_auth_password_,
                      true));
                  return process_request(strm, req3, proxy_res, false, error);
                })) {
          // Thread-safe to close everything because we are assuming there are
          // no requests in flight
          shutdown_ssl(socket, true);
          shutdown_socket(socket);
          close_socket(socket);
          success = false;
          return false;
        }
      }
    }
  }

  // If status code is not 200, proxy request is failed.
  // Set error to ProxyConnection and return proxy response
  // as the response of the request
  if (proxy_res.status != StatusCode::OK_200) {
    error = Error::ProxyConnection;
    res = std::move(proxy_res);
    // Thread-safe to close everything because we are assuming there are
    // no requests in flight
    shutdown_ssl(socket, true);
    shutdown_socket(socket);
    close_socket(socket);
    return false;
  }

  return true;
}

bool SSLClient::load_certs() {
  auto ret = true;

  std::call_once(initialize_cert_, [&]() {
    std::lock_guard<std::mutex> guard(ctx_mutex_);
    if (!ca_cert_file_path_.empty()) {
      if (!SSL_CTX_load_verify_locations(ctx_, ca_cert_file_path_.c_str(),
                                         nullptr)) {
        ret = false;
      }
    } else if (!ca_cert_dir_path_.empty()) {
      if (!SSL_CTX_load_verify_locations(ctx_, nullptr,
                                         ca_cert_dir_path_.c_str())) {
        ret = false;
      }
    } else {
      auto loaded = false;
#ifdef _WIN32
      loaded =
          detail::load_system_certs_on_windows(SSL_CTX_get_cert_store(ctx_));
#elif defined(CPPHTTPLIB_USE_CERTS_FROM_MACOSX_KEYCHAIN) && defined(__APPLE__)
#if TARGET_OS_OSX
      loaded = detail::load_system_certs_on_macos(SSL_CTX_get_cert_store(ctx_));
#endif // TARGET_OS_OSX
#endif // _WIN32
      if (!loaded) { SSL_CTX_set_default_verify_paths(ctx_); }
    }
  });

  return ret;
}

bool SSLClient::initialize_ssl(Socket &socket, Error &error) {
  auto ssl = detail::ssl_new(
      socket.sock, ctx_, ctx_mutex_,
      [&](SSL *ssl2) {
        if (server_certificate_verification_) {
          if (!load_certs()) {
            error = Error::SSLLoadingCerts;
            return false;
          }
          SSL_set_verify(ssl2, SSL_VERIFY_NONE, nullptr);
        }

        if (!detail::ssl_connect_or_accept_nonblocking(
                socket.sock, ssl2, SSL_connect, connection_timeout_sec_,
                connection_timeout_usec_)) {
          error = Error::SSLConnection;
          return false;
        }

        if (server_certificate_verification_) {
          verify_result_ = SSL_get_verify_result(ssl2);

          if (verify_result_ != X509_V_OK) {
            error = Error::SSLServerVerification;
            return false;
          }

          auto server_cert = SSL_get1_peer_certificate(ssl2);

          if (server_cert == nullptr) {
            error = Error::SSLServerVerification;
            return false;
          }

          if (!verify_host(server_cert)) {
            X509_free(server_cert);
            error = Error::SSLServerVerification;
            return false;
          }
          X509_free(server_cert);
        }

        return true;
      },
      [&](SSL *ssl2) {
        // NOTE: Direct call instead of using the OpenSSL macro to suppress
        // -Wold-style-cast warning
        // SSL_set_tlsext_host_name(ssl2, host_.c_str());
        SSL_ctrl(ssl2, SSL_CTRL_SET_TLSEXT_HOSTNAME, TLSEXT_NAMETYPE_host_name,
                 static_cast<void *>(const_cast<char *>(host_.c_str())));
        return true;
      });

  if (ssl) {
    socket.ssl = ssl;
    return true;
  }

  shutdown_socket(socket);
  close_socket(socket);
  return false;
}

void SSLClient::shutdown_ssl(Socket &socket, bool shutdown_gracefully) {
  shutdown_ssl_impl(socket, shutdown_gracefully);
}

void SSLClient::shutdown_ssl_impl(Socket &socket,
                                         bool shutdown_gracefully) {
  if (socket.sock == INVALID_SOCKET) {
    assert(socket.ssl == nullptr);
    return;
  }
  if (socket.ssl) {
    detail::ssl_delete(ctx_mutex_, socket.ssl, shutdown_gracefully);
    socket.ssl = nullptr;
  }
  assert(socket.ssl == nullptr);
}

bool
SSLClient::process_socket(const Socket &socket,
                          std::function<bool(Stream &strm)> callback) {
  assert(socket.ssl);
  return detail::process_client_socket_ssl(
      socket.ssl, socket.sock, read_timeout_sec_, read_timeout_usec_,
      write_timeout_sec_, write_timeout_usec_, std::move(callback));
}

bool SSLClient::is_ssl() const { return true; }

bool SSLClient::verify_host(X509 *server_cert) const {
  /* Quote from RFC2818 section 3.1 "Server Identity"

     If a subjectAltName extension of type dNSName is present, that MUST
     be used as the identity. Otherwise, the (most specific) Common Name
     field in the Subject field of the certificate MUST be used. Although
     the use of the Common Name is existing practice, it is deprecated and
     Certification Authorities are encouraged to use the dNSName instead.

     Matching is performed using the matching rules specified by
     [RFC2459].  If more than one identity of a given type is present in
     the certificate (e.g., more than one dNSName name, a match in any one
     of the set is considered acceptable.) Names may contain the wildcard
     character * which is considered to match any single domain name
     component or component fragment. E.g., *.a.com matches foo.a.com but
     not bar.foo.a.com. f*.com matches foo.com but not bar.com.

     In some cases, the URI is specified as an IP address rather than a
     hostname. In this case, the iPAddress subjectAltName must be present
     in the certificate and must exactly match the IP in the URI.

  */
  return verify_host_with_subject_alt_name(server_cert) ||
         verify_host_with_common_name(server_cert);
}

bool
SSLClient::verify_host_with_subject_alt_name(X509 *server_cert) const {
  auto ret = false;

  auto type = GEN_DNS;

  struct in6_addr addr6 {};
  struct in_addr addr {};
  size_t addr_len = 0;

#ifndef __MINGW32__
  if (inet_pton(AF_INET6, host_.c_str(), &addr6)) {
    type = GEN_IPADD;
    addr_len = sizeof(struct in6_addr);
  } else if (inet_pton(AF_INET, host_.c_str(), &addr)) {
    type = GEN_IPADD;
    addr_len = sizeof(struct in_addr);
  }
#endif

  auto alt_names = static_cast<const struct stack_st_GENERAL_NAME *>(
      X509_get_ext_d2i(server_cert, NID_subject_alt_name, nullptr, nullptr));

  if (alt_names) {
    auto dsn_matched = false;
    auto ip_matched = false;

    auto count = sk_GENERAL_NAME_num(alt_names);

    for (decltype(count) i = 0; i < count && !dsn_matched; i++) {
      auto val = sk_GENERAL_NAME_value(alt_names, i);
      if (val->type == type) {
        auto name =
            reinterpret_cast<const char *>(ASN1_STRING_get0_data(val->d.ia5));
        auto name_len = static_cast<size_t>(ASN1_STRING_length(val->d.ia5));

        switch (type) {
        case GEN_DNS: dsn_matched = check_host_name(name, name_len); break;

        case GEN_IPADD:
          if (!memcmp(&addr6, name, addr_len) ||
              !memcmp(&addr, name, addr_len)) {
            ip_matched = true;
          }
          break;
        }
      }
    }

    if (dsn_matched || ip_matched) { ret = true; }
  }

  GENERAL_NAMES_free(const_cast<STACK_OF(GENERAL_NAME) *>(
      reinterpret_cast<const STACK_OF(GENERAL_NAME) *>(alt_names)));
  return ret;
}

bool SSLClient::verify_host_with_common_name(X509 *server_cert) const {
  const auto subject_name = X509_get_subject_name(server_cert);

  if (subject_name != nullptr) {
    char name[BUFSIZ];
    auto name_len = X509_NAME_get_text_by_NID(subject_name, NID_commonName,
                                              name, sizeof(name));

    if (name_len != -1) {
      return check_host_name(name, static_cast<size_t>(name_len));
    }
  }

  return false;
}

bool SSLClient::check_host_name(const char *pattern,
                                       size_t pattern_len) const {
  if (host_.size() == pattern_len && host_ == pattern) { return true; }

  // Wildcard match
  // https://bugs.launchpad.net/ubuntu/+source/firefox-3.0/+bug/376484
  std::vector<std::string> pattern_components;
  detail::split(&pattern[0], &pattern[pattern_len], '.',
                [&](const char *b, const char *e) {
                  pattern_components.emplace_back(b, e);
                });

  if (host_components_.size() != pattern_components.size()) { return false; }

  auto itr = pattern_components.begin();
  for (const auto &h : host_components_) {
    auto &p = *itr;
    if (p != h && p != "*") {
      auto partial_match = (p.size() > 0 && p[p.size() - 1] == '*' &&
                            !p.compare(0, p.size() - 1, h));
      if (!partial_match) { return false; }
    }
    ++itr;
  }

  return true;
}
#endif

// Universal client implementation
Client::Client(const std::string &scheme_host_port)
    : Client(scheme_host_port, std::string(), std::string()) {}

Client::Client(const std::string &scheme_host_port,
                      const std::string &client_cert_path,
                      const std::string &client_key_path) {
  const static std::regex re(
      R"((?:([a-z]+):\/\/)?(?:\[([\d:]+)\]|([^:/?#]+))(?::(\d+))?)");

  std::smatch m;
  if (std::regex_match(scheme_host_port, m, re)) {
    auto scheme = m[1].str();

#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
    if (!scheme.empty() && (scheme != "http" && scheme != "https")) {
#else
    if (!scheme.empty() && scheme != "http") {
#endif
#ifndef CPPHTTPLIB_NO_EXCEPTIONS
      std::string msg = "'" + scheme + "' scheme is not supported.";
      throw std::invalid_argument(msg);
#endif
      return;
    }

    auto is_ssl = scheme == "https";

    auto host = m[2].str();
    if (host.empty()) { host = m[3].str(); }

    auto port_str = m[4].str();
    auto port = !port_str.empty() ? std::stoi(port_str) : (is_ssl ? 443 : 80);

    if (is_ssl) {
#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
      cli_ = detail::make_unique<SSLClient>(host, port, client_cert_path,
                                            client_key_path);
      is_ssl_ = is_ssl;
#endif
    } else {
      cli_ = detail::make_unique<ClientImpl>(host, port, client_cert_path,
                                             client_key_path);
    }
  } else {
    cli_ = detail::make_unique<ClientImpl>(scheme_host_port, 80,
                                           client_cert_path, client_key_path);
  }
}

Client::Client(const std::string &host, int port)
    : cli_(detail::make_unique<ClientImpl>(host, port)) {}

Client::Client(const std::string &host, int port,
                      const std::string &client_cert_path,
                      const std::string &client_key_path)
    : cli_(detail::make_unique<ClientImpl>(host, port, client_cert_path,
                                           client_key_path)) {}

Client::~Client() = default;

bool Client::is_valid() const {
  return cli_ != nullptr && cli_->is_valid();
}

Result Client::Get(const std::string &path) { return cli_->Get(path); }
Result Client::Get(const std::string &path, const Headers &headers) {
  return cli_->Get(path, headers);
}
Result Client::Get(const std::string &path, Progress progress) {
  return cli_->Get(path, std::move(progress));
}
Result Client::Get(const std::string &path, const Headers &headers,
                          Progress progress) {
  return cli_->Get(path, headers, std::move(progress));
}
Result Client::Get(const std::string &path,
                          ContentReceiver content_receiver) {
  return cli_->Get(path, std::move(content_receiver));
}
Result Client::Get(const std::string &path, const Headers &headers,
                          ContentReceiver content_receiver) {
  return cli_->Get(path, headers, std::move(content_receiver));
}
Result Client::Get(const std::string &path,
                          ContentReceiver content_receiver, Progress progress) {
  return cli_->Get(path, std::move(content_receiver), std::move(progress));
}
Result Client::Get(const std::string &path, const Headers &headers,
                          ContentReceiver content_receiver, Progress progress) {
  return cli_->Get(path, headers, std::move(content_receiver),
                   std::move(progress));
}
Result Client::Get(const std::string &path,
                          ResponseHandler response_handler,
                          ContentReceiver content_receiver) {
  return cli_->Get(path, std::move(response_handler),
                   std::move(content_receiver));
}
Result Client::Get(const std::string &path, const Headers &headers,
                          ResponseHandler response_handler,
                          ContentReceiver content_receiver) {
  return cli_->Get(path, headers, std::move(response_handler),
                   std::move(content_receiver));
}
Result Client::Get(const std::string &path,
                          ResponseHandler response_handler,
                          ContentReceiver content_receiver, Progress progress) {
  return cli_->Get(path, std::move(response_handler),
                   std::move(content_receiver), std::move(progress));
}
Result Client::Get(const std::string &path, const Headers &headers,
                          ResponseHandler response_handler,
                          ContentReceiver content_receiver, Progress progress) {
  return cli_->Get(path, headers, std::move(response_handler),
                   std::move(content_receiver), std::move(progress));
}
Result Client::Get(const std::string &path, const Params &params,
                          const Headers &headers, Progress progress) {
  return cli_->Get(path, params, headers, std::move(progress));
}
Result Client::Get(const std::string &path, const Params &params,
                          const Headers &headers,
                          ContentReceiver content_receiver, Progress progress) {
  return cli_->Get(path, params, headers, std::move(content_receiver),
                   std::move(progress));
}
Result Client::Get(const std::string &path, const Params &params,
                          const Headers &headers,
                          ResponseHandler response_handler,
                          ContentReceiver content_receiver, Progress progress) {
  return cli_->Get(path, params, headers, std::move(response_handler),
                   std::move(content_receiver), std::move(progress));
}

Result Client::Head(const std::string &path) { return cli_->Head(path); }
Result Client::Head(const std::string &path, const Headers &headers) {
  return cli_->Head(path, headers);
}

Result Client::Post(const std::string &path) { return cli_->Post(path); }
Result Client::Post(const std::string &path, const Headers &headers) {
  return cli_->Post(path, headers);
}
Result Client::Post(const std::string &path, const char *body,
                           size_t content_length,
                           const std::string &content_type) {
  return cli_->Post(path, body, content_length, content_type);
}
Result Client::Post(const std::string &path, const Headers &headers,
                           const char *body, size_t content_length,
                           const std::string &content_type) {
  return cli_->Post(path, headers, body, content_length, content_type);
}
Result Client::Post(const std::string &path, const std::string &body,
                           const std::string &content_type) {
  return cli_->Post(path, body, content_type);
}
Result Client::Post(const std::string &path, const Headers &headers,
                           const std::string &body,
                           const std::string &content_type) {
  return cli_->Post(path, headers, body, content_type);
}
Result Client::Post(const std::string &path, size_t content_length,
                           ContentProvider content_provider,
                           const std::string &content_type) {
  return cli_->Post(path, content_length, std::move(content_provider),
                    content_type);
}
Result Client::Post(const std::string &path,
                           ContentProviderWithoutLength content_provider,
                           const std::string &content_type) {
  return cli_->Post(path, std::move(content_provider), content_type);
}
Result Client::Post(const std::string &path, const Headers &headers,
                           size_t content_length,
                           ContentProvider content_provider,
                           const std::string &content_type) {
  return cli_->Post(path, headers, content_length, std::move(content_provider),
                    content_type);
}
Result Client::Post(const std::string &path, const Headers &headers,
                           ContentProviderWithoutLength content_provider,
                           const std::string &content_type) {
  return cli_->Post(path, headers, std::move(content_provider), content_type);
}
Result Client::Post(const std::string &path, const Params &params) {
  return cli_->Post(path, params);
}
Result Client::Post(const std::string &path, const Headers &headers,
                           const Params &params) {
  return cli_->Post(path, headers, params);
}
Result Client::Post(const std::string &path,
                           const MultipartFormDataItems &items) {
  return cli_->Post(path, items);
}
Result Client::Post(const std::string &path, const Headers &headers,
                           const MultipartFormDataItems &items) {
  return cli_->Post(path, headers, items);
}
Result Client::Post(const std::string &path, const Headers &headers,
                           const MultipartFormDataItems &items,
                           const std::string &boundary) {
  return cli_->Post(path, headers, items, boundary);
}
Result
Client::Post(const std::string &path, const Headers &headers,
             const MultipartFormDataItems &items,
             const MultipartFormDataProviderItems &provider_items) {
  return cli_->Post(path, headers, items, provider_items);
}
Result Client::Put(const std::string &path) { return cli_->Put(path); }
Result Client::Put(const std::string &path, const char *body,
                          size_t content_length,
                          const std::string &content_type) {
  return cli_->Put(path, body, content_length, content_type);
}
Result Client::Put(const std::string &path, const Headers &headers,
                          const char *body, size_t content_length,
                          const std::string &content_type) {
  return cli_->Put(path, headers, body, content_length, content_type);
}
Result Client::Put(const std::string &path, const std::string &body,
                          const std::string &content_type) {
  return cli_->Put(path, body, content_type);
}
Result Client::Put(const std::string &path, const Headers &headers,
                          const std::string &body,
                          const std::string &content_type) {
  return cli_->Put(path, headers, body, content_type);
}
Result Client::Put(const std::string &path, size_t content_length,
                          ContentProvider content_provider,
                          const std::string &content_type) {
  return cli_->Put(path, content_length, std::move(content_provider),
                   content_type);
}
Result Client::Put(const std::string &path,
                          ContentProviderWithoutLength content_provider,
                          const std::string &content_type) {
  return cli_->Put(path, std::move(content_provider), content_type);
}
Result Client::Put(const std::string &path, const Headers &headers,
                          size_t content_length,
                          ContentProvider content_provider,
                          const std::string &content_type) {
  return cli_->Put(path, headers, content_length, std::move(content_provider),
                   content_type);
}
Result Client::Put(const std::string &path, const Headers &headers,
                          ContentProviderWithoutLength content_provider,
                          const std::string &content_type) {
  return cli_->Put(path, headers, std::move(content_provider), content_type);
}
Result Client::Put(const std::string &path, const Params &params) {
  return cli_->Put(path, params);
}
Result Client::Put(const std::string &path, const Headers &headers,
                          const Params &params) {
  return cli_->Put(path, headers, params);
}
Result Client::Put(const std::string &path,
                          const MultipartFormDataItems &items) {
  return cli_->Put(path, items);
}
Result Client::Put(const std::string &path, const Headers &headers,
                          const MultipartFormDataItems &items) {
  return cli_->Put(path, headers, items);
}
Result Client::Put(const std::string &path, const Headers &headers,
                          const MultipartFormDataItems &items,
                          const std::string &boundary) {
  return cli_->Put(path, headers, items, boundary);
}
Result
Client::Put(const std::string &path, const Headers &headers,
            const MultipartFormDataItems &items,
            const MultipartFormDataProviderItems &provider_items) {
  return cli_->Put(path, headers, items, provider_items);
}
Result Client::Patch(const std::string &path) {
  return cli_->Patch(path);
}
Result Client::Patch(const std::string &path, const char *body,
                            size_t content_length,
                            const std::string &content_type) {
  return cli_->Patch(path, body, content_length, content_type);
}
Result Client::Patch(const std::string &path, const Headers &headers,
                            const char *body, size_t content_length,
                            const std::string &content_type) {
  return cli_->Patch(path, headers, body, content_length, content_type);
}
Result Client::Patch(const std::string &path, const std::string &body,
                            const std::string &content_type) {
  return cli_->Patch(path, body, content_type);
}
Result Client::Patch(const std::string &path, const Headers &headers,
                            const std::string &body,
                            const std::string &content_type) {
  return cli_->Patch(path, headers, body, content_type);
}
Result Client::Patch(const std::string &path, size_t content_length,
                            ContentProvider content_provider,
                            const std::string &content_type) {
  return cli_->Patch(path, content_length, std::move(content_provider),
                     content_type);
}
Result Client::Patch(const std::string &path,
                            ContentProviderWithoutLength content_provider,
                            const std::string &content_type) {
  return cli_->Patch(path, std::move(content_provider), content_type);
}
Result Client::Patch(const std::string &path, const Headers &headers,
                            size_t content_length,
                            ContentProvider content_provider,
                            const std::string &content_type) {
  return cli_->Patch(path, headers, content_length, std::move(content_provider),
                     content_type);
}
Result Client::Patch(const std::string &path, const Headers &headers,
                            ContentProviderWithoutLength content_provider,
                            const std::string &content_type) {
  return cli_->Patch(path, headers, std::move(content_provider), content_type);
}
Result Client::Delete(const std::string &path) {
  return cli_->Delete(path);
}
Result Client::Delete(const std::string &path, const Headers &headers) {
  return cli_->Delete(path, headers);
}
Result Client::Delete(const std::string &path, const char *body,
                             size_t content_length,
                             const std::string &content_type) {
  return cli_->Delete(path, body, content_length, content_type);
}
Result Client::Delete(const std::string &path, const Headers &headers,
                             const char *body, size_t content_length,
                             const std::string &content_type) {
  return cli_->Delete(path, headers, body, content_length, content_type);
}
Result Client::Delete(const std::string &path, const std::string &body,
                             const std::string &content_type) {
  return cli_->Delete(path, body, content_type);
}
Result Client::Delete(const std::string &path, const Headers &headers,
                             const std::string &body,
                             const std::string &content_type) {
  return cli_->Delete(path, headers, body, content_type);
}
Result Client::Options(const std::string &path) {
  return cli_->Options(path);
}
Result Client::Options(const std::string &path, const Headers &headers) {
  return cli_->Options(path, headers);
}

bool Client::send(Request &req, Response &res, Error &error) {
  return cli_->send(req, res, error);
}

Result Client::send(const Request &req) { return cli_->send(req); }

void Client::stop() { cli_->stop(); }

std::string Client::host() const { return cli_->host(); }

int Client::port() const { return cli_->port(); }

size_t Client::is_socket_open() const { return cli_->is_socket_open(); }

socket_t Client::socket() const { return cli_->socket(); }

void
Client::set_hostname_addr_map(std::map<std::string, std::string> addr_map) {
  cli_->set_hostname_addr_map(std::move(addr_map));
}

void Client::set_default_headers(Headers headers) {
  cli_->set_default_headers(std::move(headers));
}

void Client::set_header_writer(
    std::function<ssize_t(Stream &, Headers &)> const &writer) {
  cli_->set_header_writer(writer);
}

void Client::set_address_family(int family) {
  cli_->set_address_family(family);
}

void Client::set_tcp_nodelay(bool on) { cli_->set_tcp_nodelay(on); }

void Client::set_socket_options(SocketOptions socket_options) {
  cli_->set_socket_options(std::move(socket_options));
}

void Client::set_connection_timeout(time_t sec, time_t usec) {
  cli_->set_connection_timeout(sec, usec);
}

void Client::set_read_timeout(time_t sec, time_t usec) {
  cli_->set_read_timeout(sec, usec);
}

void Client::set_write_timeout(time_t sec, time_t usec) {
  cli_->set_write_timeout(sec, usec);
}

void Client::set_basic_auth(const std::string &username,
                                   const std::string &password) {
  cli_->set_basic_auth(username, password);
}
void Client::set_bearer_token_auth(const std::string &token) {
  cli_->set_bearer_token_auth(token);
}
#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
void Client::set_digest_auth(const std::string &username,
                                    const std::string &password) {
  cli_->set_digest_auth(username, password);
}
#endif

void Client::set_keep_alive(bool on) { cli_->set_keep_alive(on); }
void Client::set_follow_location(bool on) {
  cli_->set_follow_location(on);
}

void Client::set_url_encode(bool on) { cli_->set_url_encode(on); }

void Client::set_compress(bool on) { cli_->set_compress(on); }

void Client::set_decompress(bool on) { cli_->set_decompress(on); }

void Client::set_interface(const std::string &intf) {
  cli_->set_interface(intf);
}

void Client::set_proxy(const std::string &host, int port) {
  cli_->set_proxy(host, port);
}
void Client::set_proxy_basic_auth(const std::string &username,
                                         const std::string &password) {
  cli_->set_proxy_basic_auth(username, password);
}
void Client::set_proxy_bearer_token_auth(const std::string &token) {
  cli_->set_proxy_bearer_token_auth(token);
}
#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
void Client::set_proxy_digest_auth(const std::string &username,
                                          const std::string &password) {
  cli_->set_proxy_digest_auth(username, password);
}
#endif

#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
void Client::enable_server_certificate_verification(bool enabled) {
  cli_->enable_server_certificate_verification(enabled);
}
#endif

void Client::set_logger(Logger logger) {
  cli_->set_logger(std::move(logger));
}

#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
void Client::set_ca_cert_path(const std::string &ca_cert_file_path,
                                     const std::string &ca_cert_dir_path) {
  cli_->set_ca_cert_path(ca_cert_file_path, ca_cert_dir_path);
}

void Client::set_ca_cert_store(X509_STORE *ca_cert_store) {
  if (is_ssl_) {
    static_cast<SSLClient &>(*cli_).set_ca_cert_store(ca_cert_store);
  } else {
    cli_->set_ca_cert_store(ca_cert_store);
  }
}

void Client::load_ca_cert_store(const char *ca_cert, std::size_t size) {
  set_ca_cert_store(cli_->create_ca_cert_store(ca_cert, size));
}

long Client::get_openssl_verify_result() const {
  if (is_ssl_) {
    return static_cast<SSLClient &>(*cli_).get_openssl_verify_result();
  }
  return -1; // NOTE: -1 doesn't match any of X509_V_ERR_???
}

SSL_CTX *Client::ssl_context() const {
  if (is_ssl_) { return static_cast<SSLClient &>(*cli_).ssl_context(); }
  return nullptr;
}
#endif

} // namespace httplib
