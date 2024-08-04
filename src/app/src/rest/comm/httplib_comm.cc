#include "httplib.h"
namespace httplib {

/*
 * Implementation that will be part of the .cc file if split into .h + .cc.
 */

namespace detail {

bool is_hex(char c, int &v) {
  if (0x20 <= c && isdigit(c)) {
    v = c - '0';
    return true;
  } else if ('A' <= c && c <= 'F') {
    v = c - 'A' + 10;
    return true;
  } else if ('a' <= c && c <= 'f') {
    v = c - 'a' + 10;
    return true;
  }
  return false;
}

bool from_hex_to_i(const std::string &s, size_t i, size_t cnt,
                          int &val) {
  if (i >= s.size()) { return false; }

  val = 0;
  for (; cnt; i++, cnt--) {
    if (!s[i]) { return false; }
    auto v = 0;
    if (is_hex(s[i], v)) {
      val = val * 16 + v;
    } else {
      return false;
    }
  }
  return true;
}

std::string from_i_to_hex(size_t n) {
  static const auto charset = "0123456789abcdef";
  std::string ret;
  do {
    ret = charset[n & 15] + ret;
    n >>= 4;
  } while (n > 0);
  return ret;
}

size_t to_utf8(int code, char *buff) {
  if (code < 0x0080) {
    buff[0] = static_cast<char>(code & 0x7F);
    return 1;
  } else if (code < 0x0800) {
    buff[0] = static_cast<char>(0xC0 | ((code >> 6) & 0x1F));
    buff[1] = static_cast<char>(0x80 | (code & 0x3F));
    return 2;
  } else if (code < 0xD800) {
    buff[0] = static_cast<char>(0xE0 | ((code >> 12) & 0xF));
    buff[1] = static_cast<char>(0x80 | ((code >> 6) & 0x3F));
    buff[2] = static_cast<char>(0x80 | (code & 0x3F));
    return 3;
  } else if (code < 0xE000) { // D800 - DFFF is invalid...
    return 0;
  } else if (code < 0x10000) {
    buff[0] = static_cast<char>(0xE0 | ((code >> 12) & 0xF));
    buff[1] = static_cast<char>(0x80 | ((code >> 6) & 0x3F));
    buff[2] = static_cast<char>(0x80 | (code & 0x3F));
    return 3;
  } else if (code < 0x110000) {
    buff[0] = static_cast<char>(0xF0 | ((code >> 18) & 0x7));
    buff[1] = static_cast<char>(0x80 | ((code >> 12) & 0x3F));
    buff[2] = static_cast<char>(0x80 | ((code >> 6) & 0x3F));
    buff[3] = static_cast<char>(0x80 | (code & 0x3F));
    return 4;
  }

  // NOTREACHED
  return 0;
}

// NOTE: This code came up with the following stackoverflow post:
// https://stackoverflow.com/questions/180947/base64-decode-snippet-in-c
std::string base64_encode(const std::string &in) {
  static const auto lookup =
      "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

  std::string out;
  out.reserve(in.size());

  auto val = 0;
  auto valb = -6;

  for (auto c : in) {
    val = (val << 8) + static_cast<uint8_t>(c);
    valb += 8;
    while (valb >= 0) {
      out.push_back(lookup[(val >> valb) & 0x3F]);
      valb -= 6;
    }
  }

  if (valb > -6) { out.push_back(lookup[((val << 8) >> (valb + 8)) & 0x3F]); }

  while (out.size() % 4) {
    out.push_back('=');
  }

  return out;
}

bool is_file(const std::string &path) {
#ifdef _WIN32
  return _access_s(path.c_str(), 0) == 0;
#else
  struct stat st;
  return stat(path.c_str(), &st) >= 0 && S_ISREG(st.st_mode);
#endif
}

bool is_dir(const std::string &path) {
  struct stat st;
  return stat(path.c_str(), &st) >= 0 && S_ISDIR(st.st_mode);
}

bool is_valid_path(const std::string &path) {
  size_t level = 0;
  size_t i = 0;

  // Skip slash
  while (i < path.size() && path[i] == '/') {
    i++;
  }

  while (i < path.size()) {
    // Read component
    auto beg = i;
    while (i < path.size() && path[i] != '/') {
      if (path[i] == '\0') {
        return false;
      } else if (path[i] == '\\') {
        return false;
      }
      i++;
    }

    auto len = i - beg;
    assert(len > 0);

    if (!path.compare(beg, len, ".")) {
      ;
    } else if (!path.compare(beg, len, "..")) {
      if (level == 0) { return false; }
      level--;
    } else {
      level++;
    }

    // Skip slash
    while (i < path.size() && path[i] == '/') {
      i++;
    }
  }

  return true;
}

std::string encode_query_param(const std::string &value) {
  std::ostringstream escaped;
  escaped.fill('0');
  escaped << std::hex;

  for (auto c : value) {
    if (std::isalnum(static_cast<uint8_t>(c)) || c == '-' || c == '_' ||
        c == '.' || c == '!' || c == '~' || c == '*' || c == '\'' || c == '(' ||
        c == ')') {
      escaped << c;
    } else {
      escaped << std::uppercase;
      escaped << '%' << std::setw(2)
              << static_cast<int>(static_cast<unsigned char>(c));
      escaped << std::nouppercase;
    }
  }

  return escaped.str();
}

std::string encode_url(const std::string &s) {
  std::string result;
  result.reserve(s.size());

  for (size_t i = 0; s[i]; i++) {
    switch (s[i]) {
    case ' ': result += "%20"; break;
    case '+': result += "%2B"; break;
    case '\r': result += "%0D"; break;
    case '\n': result += "%0A"; break;
    case '\'': result += "%27"; break;
    case ',': result += "%2C"; break;
    // case ':': result += "%3A"; break; // ok? probably...
    case ';': result += "%3B"; break;
    default:
      auto c = static_cast<uint8_t>(s[i]);
      if (c >= 0x80) {
        result += '%';
        char hex[4];
        auto len = snprintf(hex, sizeof(hex) - 1, "%02X", c);
        assert(len == 2);
        result.append(hex, static_cast<size_t>(len));
      } else {
        result += s[i];
      }
      break;
    }
  }

  return result;
}

std::string decode_url(const std::string &s,
                              bool convert_plus_to_space) {
  std::string result;

  for (size_t i = 0; i < s.size(); i++) {
    if (s[i] == '%' && i + 1 < s.size()) {
      if (s[i + 1] == 'u') {
        auto val = 0;
        if (from_hex_to_i(s, i + 2, 4, val)) {
          // 4 digits Unicode codes
          char buff[4];
          size_t len = to_utf8(val, buff);
          if (len > 0) { result.append(buff, len); }
          i += 5; // 'u0000'
        } else {
          result += s[i];
        }
      } else {
        auto val = 0;
        if (from_hex_to_i(s, i + 1, 2, val)) {
          // 2 digits hex codes
          result += static_cast<char>(val);
          i += 2; // '00'
        } else {
          result += s[i];
        }
      }
    } else if (convert_plus_to_space && s[i] == '+') {
      result += ' ';
    } else {
      result += s[i];
    }
  }

  return result;
}

void read_file(const std::string &path, std::string &out) {
  std::ifstream fs(path, std::ios_base::binary);
  fs.seekg(0, std::ios_base::end);
  auto size = fs.tellg();
  fs.seekg(0);
  out.resize(static_cast<size_t>(size));
  fs.read(&out[0], static_cast<std::streamsize>(size));
}

std::string file_extension(const std::string &path) {
  std::smatch m;
  static auto re = std::regex("\\.([a-zA-Z0-9]+)$");
  if (std::regex_search(path, m, re)) { return m[1].str(); }
  return std::string();
}

bool is_space_or_tab(char c) { return c == ' ' || c == '\t'; }

std::pair<size_t, size_t> trim(const char *b, const char *e, size_t left,
                                      size_t right) {
  while (b + left < e && is_space_or_tab(b[left])) {
    left++;
  }
  while (right > 0 && is_space_or_tab(b[right - 1])) {
    right--;
  }
  return std::make_pair(left, right);
}

std::string trim_copy(const std::string &s) {
  auto r = trim(s.data(), s.data() + s.size(), 0, s.size());
  return s.substr(r.first, r.second - r.first);
}

std::string trim_double_quotes_copy(const std::string &s) {
  if (s.length() >= 2 && s.front() == '"' && s.back() == '"') {
    return s.substr(1, s.size() - 2);
  }
  return s;
}

void split(const char *b, const char *e, char d,
                  std::function<void(const char *, const char *)> fn) {
  return split(b, e, d, (std::numeric_limits<size_t>::max)(), std::move(fn));
}

void split(const char *b, const char *e, char d, size_t m,
                  std::function<void(const char *, const char *)> fn) {
  size_t i = 0;
  size_t beg = 0;
  size_t count = 1;

  while (e ? (b + i < e) : (b[i] != '\0')) {
    if (b[i] == d && count < m) {
      auto r = trim(b, e, beg, i);
      if (r.first < r.second) { fn(&b[r.first], &b[r.second]); }
      beg = i + 1;
      count++;
    }
    i++;
  }

  if (i) {
    auto r = trim(b, e, beg, i);
    if (r.first < r.second) { fn(&b[r.first], &b[r.second]); }
  }
}

stream_line_reader::stream_line_reader(Stream &strm, char *fixed_buffer,
                                              size_t fixed_buffer_size)
    : strm_(strm), fixed_buffer_(fixed_buffer),
      fixed_buffer_size_(fixed_buffer_size) {}

const char *stream_line_reader::ptr() const {
  if (glowable_buffer_.empty()) {
    return fixed_buffer_;
  } else {
    return glowable_buffer_.data();
  }
}

size_t stream_line_reader::size() const {
  if (glowable_buffer_.empty()) {
    return fixed_buffer_used_size_;
  } else {
    return glowable_buffer_.size();
  }
}

bool stream_line_reader::end_with_crlf() const {
  auto end = ptr() + size();
  return size() >= 2 && end[-2] == '\r' && end[-1] == '\n';
}

bool stream_line_reader::getline() {
  fixed_buffer_used_size_ = 0;
  glowable_buffer_.clear();

  for (size_t i = 0;; i++) {
    char byte;
    auto n = strm_.read(&byte, 1);

    if (n < 0) {
      return false;
    } else if (n == 0) {
      if (i == 0) {
        return false;
      } else {
        break;
      }
    }

    append(byte);

    if (byte == '\n') { break; }
  }

  return true;
}

void stream_line_reader::append(char c) {
  if (fixed_buffer_used_size_ < fixed_buffer_size_ - 1) {
    fixed_buffer_[fixed_buffer_used_size_++] = c;
    fixed_buffer_[fixed_buffer_used_size_] = '\0';
  } else {
    if (glowable_buffer_.empty()) {
      assert(fixed_buffer_[fixed_buffer_used_size_] == '\0');
      glowable_buffer_.assign(fixed_buffer_, fixed_buffer_used_size_);
    }
    glowable_buffer_ += c;
  }
}

mmap::mmap(const char *path)
#if defined(_WIN32)
    : hFile_(NULL), hMapping_(NULL)
#else
    : fd_(-1)
#endif
      ,
      size_(0), addr_(nullptr) {
  open(path);
}

mmap::~mmap() { close(); }

bool mmap::open(const char *path) {
  close();

#if defined(_WIN32)
  std::wstring wpath;
  for (size_t i = 0; i < strlen(path); i++) {
    wpath += path[i];
  }

  hFile_ = ::CreateFile2(wpath.c_str(), GENERIC_READ, FILE_SHARE_READ,
                         OPEN_EXISTING, NULL);

  if (hFile_ == INVALID_HANDLE_VALUE) { return false; }

  LARGE_INTEGER size{};
  if (!::GetFileSizeEx(hFile_, &size)) { return false; }
  size_ = static_cast<size_t>(size.QuadPart);

  hMapping_ =
      ::CreateFileMappingFromApp(hFile_, NULL, PAGE_READONLY, size_, NULL);

  if (hMapping_ == NULL) {
    close();
    return false;
  }

  addr_ = ::MapViewOfFileFromApp(hMapping_, FILE_MAP_READ, 0, 0);
#else
  fd_ = ::open(path, O_RDONLY);
  if (fd_ == -1) { return false; }

  struct stat sb;
  if (fstat(fd_, &sb) == -1) {
    close();
    return false;
  }
  size_ = static_cast<size_t>(sb.st_size);

  addr_ = ::mmap(NULL, size_, PROT_READ, MAP_PRIVATE, fd_, 0);
#endif

  if (addr_ == nullptr) {
    close();
    return false;
  }

  return true;
}

bool mmap::is_open() const { return addr_ != nullptr; }

size_t mmap::size() const { return size_; }

const char *mmap::data() const {
  return static_cast<const char *>(addr_);
}

void mmap::close() {
#if defined(_WIN32)
  if (addr_) {
    ::UnmapViewOfFile(addr_);
    addr_ = nullptr;
  }

  if (hMapping_) {
    ::CloseHandle(hMapping_);
    hMapping_ = NULL;
  }

  if (hFile_ != INVALID_HANDLE_VALUE) {
    ::CloseHandle(hFile_);
    hFile_ = INVALID_HANDLE_VALUE;
  }
#else
  if (addr_ != nullptr) {
    munmap(addr_, size_);
    addr_ = nullptr;
  }

  if (fd_ != -1) {
    ::close(fd_);
    fd_ = -1;
  }
#endif
  size_ = 0;
}
int close_socket(socket_t sock) {
#ifdef _WIN32
  return closesocket(sock);
#else
  return close(sock);
#endif
}

template <typename T> ssize_t handle_EINTR(T fn) {
  ssize_t res = 0;
  while (true) {
    res = fn();
    if (res < 0 && errno == EINTR) { continue; }
    break;
  }
  return res;
}

ssize_t read_socket(socket_t sock, void *ptr, size_t size, int flags) {
  return handle_EINTR([&]() {
    return recv(sock,
#ifdef _WIN32
                static_cast<char *>(ptr), static_cast<int>(size),
#else
                ptr, size,
#endif
                flags);
  });
}

ssize_t send_socket(socket_t sock, const void *ptr, size_t size,
                           int flags) {
  return handle_EINTR([&]() {
    return send(sock,
#ifdef _WIN32
                static_cast<const char *>(ptr), static_cast<int>(size),
#else
                ptr, size,
#endif
                flags);
  });
}

ssize_t select_read(socket_t sock, time_t sec, time_t usec) {
#ifdef CPPHTTPLIB_USE_POLL
  struct pollfd pfd_read;
  pfd_read.fd = sock;
  pfd_read.events = POLLIN;

  auto timeout = static_cast<int>(sec * 1000 + usec / 1000);

  return handle_EINTR([&]() { return poll(&pfd_read, 1, timeout); });
#else
#ifndef _WIN32
  if (sock >= FD_SETSIZE) { return -1; }
#endif

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

ssize_t select_write(socket_t sock, time_t sec, time_t usec) {
#ifdef CPPHTTPLIB_USE_POLL
  struct pollfd pfd_read;
  pfd_read.fd = sock;
  pfd_read.events = POLLOUT;

  auto timeout = static_cast<int>(sec * 1000 + usec / 1000);

  return handle_EINTR([&]() { return poll(&pfd_read, 1, timeout); });
#else
#ifndef _WIN32
  if (sock >= FD_SETSIZE) { return -1; }
#endif

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

Error wait_until_socket_is_ready(socket_t sock, time_t sec,
                                        time_t usec) {
#ifdef CPPHTTPLIB_USE_POLL
  struct pollfd pfd_read;
  pfd_read.fd = sock;
  pfd_read.events = POLLIN | POLLOUT;

  auto timeout = static_cast<int>(sec * 1000 + usec / 1000);

  auto poll_res = handle_EINTR([&]() { return poll(&pfd_read, 1, timeout); });

  if (poll_res == 0) { return Error::ConnectionTimeout; }

  if (poll_res > 0 && pfd_read.revents & (POLLIN | POLLOUT)) {
    auto error = 0;
    socklen_t len = sizeof(error);
    auto res = getsockopt(sock, SOL_SOCKET, SO_ERROR,
                          reinterpret_cast<char *>(&error), &len);
    auto successful = res >= 0 && !error;
    return successful ? Error::Success : Error::Connection;
  }

  return Error::Connection;
#else
#ifndef _WIN32
  if (sock >= FD_SETSIZE) { return Error::Connection; }
#endif

  fd_set fdsr;
  FD_ZERO(&fdsr);
  FD_SET(sock, &fdsr);

  auto fdsw = fdsr;
  auto fdse = fdsr;

  timeval tv;
  tv.tv_sec = static_cast<long>(sec);
  tv.tv_usec = static_cast<decltype(tv.tv_usec)>(usec);

  auto ret = handle_EINTR([&]() {
    return select(static_cast<int>(sock + 1), &fdsr, &fdsw, &fdse, &tv);
  });

  if (ret == 0) { return Error::ConnectionTimeout; }

  if (ret > 0 && (FD_ISSET(sock, &fdsr) || FD_ISSET(sock, &fdsw))) {
    auto error = 0;
    socklen_t len = sizeof(error);
    auto res = getsockopt(sock, SOL_SOCKET, SO_ERROR,
                          reinterpret_cast<char *>(&error), &len);
    auto successful = res >= 0 && !error;
    return successful ? Error::Success : Error::Connection;
  }
  return Error::Connection;
#endif
}

bool is_socket_alive(socket_t sock) {
  const auto val = detail::select_read(sock, 0, 0);
  if (val == 0) {
    return true;
  } else if (val < 0 && errno == EBADF) {
    return false;
  }
  char buf[1];
  return detail::read_socket(sock, &buf[0], sizeof(buf), MSG_PEEK) > 0;
}

class SocketStream final : public Stream {
public:
  SocketStream(socket_t sock, time_t read_timeout_sec, time_t read_timeout_usec,
               time_t write_timeout_sec, time_t write_timeout_usec);
  ~SocketStream() override;

  bool is_readable() const override;
  bool is_writable() const override;
  ssize_t read(char *ptr, size_t size) override;
  ssize_t write(const char *ptr, size_t size) override;
  void get_remote_ip_and_port(std::string &ip, int &port) const override;
  void get_local_ip_and_port(std::string &ip, int &port) const override;
  socket_t socket() const override;

private:
  socket_t sock_;
  time_t read_timeout_sec_;
  time_t read_timeout_usec_;
  time_t write_timeout_sec_;
  time_t write_timeout_usec_;

  std::vector<char> read_buff_;
  size_t read_buff_off_ = 0;
  size_t read_buff_content_size_ = 0;

  static const size_t read_buff_size_ = 1024l * 4;
};

#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
class SSLSocketStream final : public Stream {
public:
  SSLSocketStream(socket_t sock, SSL *ssl, time_t read_timeout_sec,
                  time_t read_timeout_usec, time_t write_timeout_sec,
                  time_t write_timeout_usec);
  ~SSLSocketStream() override;

  bool is_readable() const override;
  bool is_writable() const override;
  ssize_t read(char *ptr, size_t size) override;
  ssize_t write(const char *ptr, size_t size) override;
  void get_remote_ip_and_port(std::string &ip, int &port) const override;
  void get_local_ip_and_port(std::string &ip, int &port) const override;
  socket_t socket() const override;

private:
  socket_t sock_;
  SSL *ssl_;
  time_t read_timeout_sec_;
  time_t read_timeout_usec_;
  time_t write_timeout_sec_;
  time_t write_timeout_usec_;
};
#endif

bool keep_alive(socket_t sock, time_t keep_alive_timeout_sec) {
  using namespace std::chrono;
  auto start = steady_clock::now();
  while (true) {
    auto val = select_read(sock, 0, 10000);
    if (val < 0) {
      return false;
    } else if (val == 0) {
      auto current = steady_clock::now();
      auto duration = duration_cast<milliseconds>(current - start);
      auto timeout = keep_alive_timeout_sec * 1000;
      if (duration.count() > timeout) { return false; }
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
    } else {
      return true;
    }
  }
}

template <typename T>
bool
process_server_socket_core(const std::atomic<socket_t> &svr_sock, socket_t sock,
                           size_t keep_alive_max_count,
                           time_t keep_alive_timeout_sec, T callback) {
  assert(keep_alive_max_count > 0);
  auto ret = false;
  auto count = keep_alive_max_count;
  while (svr_sock != INVALID_SOCKET && count > 0 &&
         keep_alive(sock, keep_alive_timeout_sec)) {
    auto close_connection = count == 1;
    auto connection_closed = false;
    ret = callback(close_connection, connection_closed);
    if (!ret || connection_closed) { break; }
    count--;
  }
  return ret;
}

template <typename T>
bool
process_server_socket(const std::atomic<socket_t> &svr_sock, socket_t sock,
                      size_t keep_alive_max_count,
                      time_t keep_alive_timeout_sec, time_t read_timeout_sec,
                      time_t read_timeout_usec, time_t write_timeout_sec,
                      time_t write_timeout_usec, T callback) {
  return process_server_socket_core(
      svr_sock, sock, keep_alive_max_count, keep_alive_timeout_sec,
      [&](bool close_connection, bool &connection_closed) {
        SocketStream strm(sock, read_timeout_sec, read_timeout_usec,
                          write_timeout_sec, write_timeout_usec);
        return callback(strm, close_connection, connection_closed);
      });
}

bool process_client_socket(socket_t sock, time_t read_timeout_sec,
                                  time_t read_timeout_usec,
                                  time_t write_timeout_sec,
                                  time_t write_timeout_usec,
                                  std::function<bool(Stream &)> callback) {
  SocketStream strm(sock, read_timeout_sec, read_timeout_usec,
                    write_timeout_sec, write_timeout_usec);
  return callback(strm);
}

int shutdown_socket(socket_t sock) {
#ifdef _WIN32
  return shutdown(sock, SD_BOTH);
#else
  return shutdown(sock, SHUT_RDWR);
#endif
}

template <typename BindOrConnect>
socket_t create_socket(const std::string &host, const std::string &ip, int port,
                       int address_family, int socket_flags, bool tcp_nodelay,
                       SocketOptions socket_options,
                       BindOrConnect bind_or_connect) {
  // Get address info
  const char *node = nullptr;
  struct addrinfo hints;
  struct addrinfo *result;

  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_protocol = 0;

  if (!ip.empty()) {
    node = ip.c_str();
    // Ask getaddrinfo to convert IP in c-string to address
    hints.ai_family = AF_UNSPEC;
    hints.ai_flags = AI_NUMERICHOST;
  } else {
    if (!host.empty()) { node = host.c_str(); }
    hints.ai_family = address_family;
    hints.ai_flags = socket_flags;
  }

#ifndef _WIN32
  if (hints.ai_family == AF_UNIX) {
    const auto addrlen = host.length();
    if (addrlen > sizeof(sockaddr_un::sun_path)) { return INVALID_SOCKET; }

    auto sock = socket(hints.ai_family, hints.ai_socktype, hints.ai_protocol);
    if (sock != INVALID_SOCKET) {
      sockaddr_un addr{};
      addr.sun_family = AF_UNIX;
      std::copy(host.begin(), host.end(), addr.sun_path);

      hints.ai_addr = reinterpret_cast<sockaddr *>(&addr);
      hints.ai_addrlen = static_cast<socklen_t>(
          sizeof(addr) - sizeof(addr.sun_path) + addrlen);

      fcntl(sock, F_SETFD, FD_CLOEXEC);
      if (socket_options) { socket_options(sock); }

      if (!bind_or_connect(sock, hints)) {
        close_socket(sock);
        sock = INVALID_SOCKET;
      }
    }
    return sock;
  }
#endif

  auto service = std::to_string(port);

  if (getaddrinfo(node, service.c_str(), &hints, &result)) {
#if defined __linux__ && !defined __ANDROID__
    res_init();
#endif
    return INVALID_SOCKET;
  }

  for (auto rp = result; rp; rp = rp->ai_next) {
    // Create a socket
#ifdef _WIN32
    auto sock =
        WSASocketW(rp->ai_family, rp->ai_socktype, rp->ai_protocol, nullptr, 0,
                   WSA_FLAG_NO_HANDLE_INHERIT | WSA_FLAG_OVERLAPPED);
    /**
     * Since the WSA_FLAG_NO_HANDLE_INHERIT is only supported on Windows 7 SP1
     * and above the socket creation fails on older Windows Systems.
     *
     * Let's try to create a socket the old way in this case.
     *
     * Reference:
     * https://docs.microsoft.com/en-us/windows/win32/api/winsock2/nf-winsock2-wsasocketa
     *
     * WSA_FLAG_NO_HANDLE_INHERIT:
     * This flag is supported on Windows 7 with SP1, Windows Server 2008 R2 with
     * SP1, and later
     *
     */
    if (sock == INVALID_SOCKET) {
      sock = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
    }
#else
    auto sock = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
#endif
    if (sock == INVALID_SOCKET) { continue; }

#ifndef _WIN32
    if (fcntl(sock, F_SETFD, FD_CLOEXEC) == -1) {
      close_socket(sock);
      continue;
    }
#endif

    if (tcp_nodelay) {
      auto yes = 1;
#ifdef _WIN32
      setsockopt(sock, IPPROTO_TCP, TCP_NODELAY,
                 reinterpret_cast<const char *>(&yes), sizeof(yes));
#else
      setsockopt(sock, IPPROTO_TCP, TCP_NODELAY,
                 reinterpret_cast<const void *>(&yes), sizeof(yes));
#endif
    }

    if (socket_options) { socket_options(sock); }

    if (rp->ai_family == AF_INET6) {
      auto no = 0;
#ifdef _WIN32
      setsockopt(sock, IPPROTO_IPV6, IPV6_V6ONLY,
                 reinterpret_cast<const char *>(&no), sizeof(no));
#else
      setsockopt(sock, IPPROTO_IPV6, IPV6_V6ONLY,
                 reinterpret_cast<const void *>(&no), sizeof(no));
#endif
    }

    // bind or connect
    if (bind_or_connect(sock, *rp)) {
      freeaddrinfo(result);
      return sock;
    }

    close_socket(sock);
  }

  freeaddrinfo(result);
  return INVALID_SOCKET;
}

void set_nonblocking(socket_t sock, bool nonblocking) {
#ifdef _WIN32
  auto flags = nonblocking ? 1UL : 0UL;
  ioctlsocket(sock, FIONBIO, &flags);
#else
  auto flags = fcntl(sock, F_GETFL, 0);
  fcntl(sock, F_SETFL,
        nonblocking ? (flags | O_NONBLOCK) : (flags & (~O_NONBLOCK)));
#endif
}

bool is_connection_error() {
#ifdef _WIN32
  return WSAGetLastError() != WSAEWOULDBLOCK;
#else
  return errno != EINPROGRESS;
#endif
}

bool bind_ip_address(socket_t sock, const std::string &host) {
  struct addrinfo hints;
  struct addrinfo *result;

  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_protocol = 0;

  if (getaddrinfo(host.c_str(), "0", &hints, &result)) { return false; }

  auto ret = false;
  for (auto rp = result; rp; rp = rp->ai_next) {
    const auto &ai = *rp;
    if (!::bind(sock, ai.ai_addr, static_cast<socklen_t>(ai.ai_addrlen))) {
      ret = true;
      break;
    }
  }

  freeaddrinfo(result);
  return ret;
}

#if !defined _WIN32 && !defined ANDROID && !defined _AIX && !defined __MVS__
#define USE_IF2IP
#endif

#ifdef USE_IF2IP
std::string if2ip(int address_family, const std::string &ifn) {
  struct ifaddrs *ifap;
  getifaddrs(&ifap);
  std::string addr_candidate;
  for (auto ifa = ifap; ifa; ifa = ifa->ifa_next) {
    if (ifa->ifa_addr && ifn == ifa->ifa_name &&
        (AF_UNSPEC == address_family ||
         ifa->ifa_addr->sa_family == address_family)) {
      if (ifa->ifa_addr->sa_family == AF_INET) {
        auto sa = reinterpret_cast<struct sockaddr_in *>(ifa->ifa_addr);
        char buf[INET_ADDRSTRLEN];
        if (inet_ntop(AF_INET, &sa->sin_addr, buf, INET_ADDRSTRLEN)) {
          freeifaddrs(ifap);
          return std::string(buf, INET_ADDRSTRLEN);
        }
      } else if (ifa->ifa_addr->sa_family == AF_INET6) {
        auto sa = reinterpret_cast<struct sockaddr_in6 *>(ifa->ifa_addr);
        if (!IN6_IS_ADDR_LINKLOCAL(&sa->sin6_addr)) {
          char buf[INET6_ADDRSTRLEN] = {};
          if (inet_ntop(AF_INET6, &sa->sin6_addr, buf, INET6_ADDRSTRLEN)) {
            // equivalent to mac's IN6_IS_ADDR_UNIQUE_LOCAL
            auto s6_addr_head = sa->sin6_addr.s6_addr[0];
            if (s6_addr_head == 0xfc || s6_addr_head == 0xfd) {
              addr_candidate = std::string(buf, INET6_ADDRSTRLEN);
            } else {
              freeifaddrs(ifap);
              return std::string(buf, INET6_ADDRSTRLEN);
            }
          }
        }
      }
    }
  }
  freeifaddrs(ifap);
  return addr_candidate;
}
#endif

socket_t create_client_socket(
    const std::string &host, const std::string &ip, int port,
    int address_family, bool tcp_nodelay, SocketOptions socket_options,
    time_t connection_timeout_sec, time_t connection_timeout_usec,
    time_t read_timeout_sec, time_t read_timeout_usec, time_t write_timeout_sec,
    time_t write_timeout_usec, const std::string &intf, Error &error) {
  auto sock = create_socket(
      host, ip, port, address_family, 0, tcp_nodelay, std::move(socket_options),
      [&](socket_t sock2, struct addrinfo &ai) -> bool {
        if (!intf.empty()) {
#ifdef USE_IF2IP
          auto ip_from_if = if2ip(address_family, intf);
          if (ip_from_if.empty()) { ip_from_if = intf; }
          if (!bind_ip_address(sock2, ip_from_if)) {
            error = Error::BindIPAddress;
            return false;
          }
#endif
        }

        set_nonblocking(sock2, true);

        auto ret =
            ::connect(sock2, ai.ai_addr, static_cast<socklen_t>(ai.ai_addrlen));

        if (ret < 0) {
          if (is_connection_error()) {
            error = Error::Connection;
            return false;
          }
          error = wait_until_socket_is_ready(sock2, connection_timeout_sec,
                                             connection_timeout_usec);
          if (error != Error::Success) { return false; }
        }

        set_nonblocking(sock2, false);

        {
#ifdef _WIN32
          auto timeout = static_cast<uint32_t>(read_timeout_sec * 1000 +
                                               read_timeout_usec / 1000);
          setsockopt(sock2, SOL_SOCKET, SO_RCVTIMEO,
                     reinterpret_cast<const char *>(&timeout), sizeof(timeout));
#else
          timeval tv;
          tv.tv_sec = static_cast<long>(read_timeout_sec);
          tv.tv_usec = static_cast<decltype(tv.tv_usec)>(read_timeout_usec);
          setsockopt(sock2, SOL_SOCKET, SO_RCVTIMEO,
                     reinterpret_cast<const void *>(&tv), sizeof(tv));
#endif
        }
        {

#ifdef _WIN32
          auto timeout = static_cast<uint32_t>(write_timeout_sec * 1000 +
                                               write_timeout_usec / 1000);
          setsockopt(sock2, SOL_SOCKET, SO_SNDTIMEO,
                     reinterpret_cast<const char *>(&timeout), sizeof(timeout));
#else
          timeval tv;
          tv.tv_sec = static_cast<long>(write_timeout_sec);
          tv.tv_usec = static_cast<decltype(tv.tv_usec)>(write_timeout_usec);
          setsockopt(sock2, SOL_SOCKET, SO_SNDTIMEO,
                     reinterpret_cast<const void *>(&tv), sizeof(tv));
#endif
        }

        error = Error::Success;
        return true;
      });

  if (sock != INVALID_SOCKET) {
    error = Error::Success;
  } else {
    if (error == Error::Success) { error = Error::Connection; }
  }

  return sock;
}

bool get_ip_and_port(const struct sockaddr_storage &addr,
                            socklen_t addr_len, std::string &ip, int &port) {
  if (addr.ss_family == AF_INET) {
    port = ntohs(reinterpret_cast<const struct sockaddr_in *>(&addr)->sin_port);
  } else if (addr.ss_family == AF_INET6) {
    port =
        ntohs(reinterpret_cast<const struct sockaddr_in6 *>(&addr)->sin6_port);
  } else {
    return false;
  }

  std::array<char, NI_MAXHOST> ipstr{};
  if (getnameinfo(reinterpret_cast<const struct sockaddr *>(&addr), addr_len,
                  ipstr.data(), static_cast<socklen_t>(ipstr.size()), nullptr,
                  0, NI_NUMERICHOST)) {
    return false;
  }

  ip = ipstr.data();
  return true;
}

void get_local_ip_and_port(socket_t sock, std::string &ip, int &port) {
  struct sockaddr_storage addr;
  socklen_t addr_len = sizeof(addr);
  if (!getsockname(sock, reinterpret_cast<struct sockaddr *>(&addr),
                   &addr_len)) {
    get_ip_and_port(addr, addr_len, ip, port);
  }
}

void get_remote_ip_and_port(socket_t sock, std::string &ip, int &port) {
  struct sockaddr_storage addr;
  socklen_t addr_len = sizeof(addr);

  if (!getpeername(sock, reinterpret_cast<struct sockaddr *>(&addr),
                   &addr_len)) {
#ifndef _WIN32
    if (addr.ss_family == AF_UNIX) {
#if defined(__linux__)
      struct ucred ucred;
      socklen_t len = sizeof(ucred);
      if (getsockopt(sock, SOL_SOCKET, SO_PEERCRED, &ucred, &len) == 0) {
        port = ucred.pid;
      }
#elif defined(SOL_LOCAL) && defined(SO_PEERPID) // __APPLE__
      pid_t pid;
      socklen_t len = sizeof(pid);
      if (getsockopt(sock, SOL_LOCAL, SO_PEERPID, &pid, &len) == 0) {
        port = pid;
      }
#endif
      return;
    }
#endif
    get_ip_and_port(addr, addr_len, ip, port);
  }
}

constexpr unsigned int str2tag_core(const char *s, size_t l,
                                           unsigned int h) {
  return (l == 0)
             ? h
             : str2tag_core(
                   s + 1, l - 1,
                   // Unsets the 6 high bits of h, therefore no overflow happens
                   (((std::numeric_limits<unsigned int>::max)() >> 6) &
                    h * 33) ^
                       static_cast<unsigned char>(*s));
}

unsigned int str2tag(const std::string &s) {
  return str2tag_core(s.data(), s.size(), 0);
}

namespace udl {

constexpr unsigned int operator"" _t(const char *s, size_t l) {
  return str2tag_core(s, l, 0);
}

} // namespace udl

std::string
find_content_type(const std::string &path,
                  const std::map<std::string, std::string> &user_data,
                  const std::string &default_content_type) {
  auto ext = file_extension(path);

  auto it = user_data.find(ext);
  if (it != user_data.end()) { return it->second; }

  using udl::operator""_t;

  switch (str2tag(ext)) {
  default: return default_content_type;

  case "css"_t: return "text/css";
  case "csv"_t: return "text/csv";
  case "htm"_t:
  case "html"_t: return "text/html";
  case "js"_t:
  case "mjs"_t: return "text/javascript";
  case "txt"_t: return "text/plain";
  case "vtt"_t: return "text/vtt";

  case "apng"_t: return "image/apng";
  case "avif"_t: return "image/avif";
  case "bmp"_t: return "image/bmp";
  case "gif"_t: return "image/gif";
  case "png"_t: return "image/png";
  case "svg"_t: return "image/svg+xml";
  case "webp"_t: return "image/webp";
  case "ico"_t: return "image/x-icon";
  case "tif"_t: return "image/tiff";
  case "tiff"_t: return "image/tiff";
  case "jpg"_t:
  case "jpeg"_t: return "image/jpeg";

  case "mp4"_t: return "video/mp4";
  case "mpeg"_t: return "video/mpeg";
  case "webm"_t: return "video/webm";

  case "mp3"_t: return "audio/mp3";
  case "mpga"_t: return "audio/mpeg";
  case "weba"_t: return "audio/webm";
  case "wav"_t: return "audio/wave";

  case "otf"_t: return "font/otf";
  case "ttf"_t: return "font/ttf";
  case "woff"_t: return "font/woff";
  case "woff2"_t: return "font/woff2";

  case "7z"_t: return "application/x-7z-compressed";
  case "atom"_t: return "application/atom+xml";
  case "pdf"_t: return "application/pdf";
  case "json"_t: return "application/json";
  case "rss"_t: return "application/rss+xml";
  case "tar"_t: return "application/x-tar";
  case "xht"_t:
  case "xhtml"_t: return "application/xhtml+xml";
  case "xslt"_t: return "application/xslt+xml";
  case "xml"_t: return "application/xml";
  case "gz"_t: return "application/gzip";
  case "zip"_t: return "application/zip";
  case "wasm"_t: return "application/wasm";
  }
}

bool can_compress_content_type(const std::string &content_type) {
  using udl::operator""_t;

  auto tag = str2tag(content_type);

  switch (tag) {
  case "image/svg+xml"_t:
  case "application/javascript"_t:
  case "application/json"_t:
  case "application/xml"_t:
  case "application/protobuf"_t:
  case "application/xhtml+xml"_t: return true;

  default:
    return !content_type.rfind("text/", 0) && tag != "text/event-stream"_t;
  }
}

EncodingType encoding_type(const Request &req, const Response &res) {
  auto ret =
      detail::can_compress_content_type(res.get_header_value("Content-Type"));
  if (!ret) { return EncodingType::None; }

  const auto &s = req.get_header_value("Accept-Encoding");
  (void)(s);

#ifdef CPPHTTPLIB_BROTLI_SUPPORT
  // TODO: 'Accept-Encoding' has br, not br;q=0
  ret = s.find("br") != std::string::npos;
  if (ret) { return EncodingType::Brotli; }
#endif

#ifdef CPPHTTPLIB_ZLIB_SUPPORT
  // TODO: 'Accept-Encoding' has gzip, not gzip;q=0
  ret = s.find("gzip") != std::string::npos;
  if (ret) { return EncodingType::Gzip; }
#endif

  return EncodingType::None;
}

bool nocompressor::compress(const char *data, size_t data_length,
                                   bool /*last*/, Callback callback) {
  if (!data_length) { return true; }
  return callback(data, data_length);
}

#ifdef CPPHTTPLIB_ZLIB_SUPPORT
gzip_compressor::gzip_compressor() {
  std::memset(&strm_, 0, sizeof(strm_));
  strm_.zalloc = Z_NULL;
  strm_.zfree = Z_NULL;
  strm_.opaque = Z_NULL;

  is_valid_ = deflateInit2(&strm_, Z_DEFAULT_COMPRESSION, Z_DEFLATED, 31, 8,
                           Z_DEFAULT_STRATEGY) == Z_OK;
}

gzip_compressor::~gzip_compressor() { deflateEnd(&strm_); }

bool gzip_compressor::compress(const char *data, size_t data_length,
                                      bool last, Callback callback) {
  assert(is_valid_);

  do {
    constexpr size_t max_avail_in =
        (std::numeric_limits<decltype(strm_.avail_in)>::max)();

    strm_.avail_in = static_cast<decltype(strm_.avail_in)>(
        (std::min)(data_length, max_avail_in));
    strm_.next_in = const_cast<Bytef *>(reinterpret_cast<const Bytef *>(data));

    data_length -= strm_.avail_in;
    data += strm_.avail_in;

    auto flush = (last && data_length == 0) ? Z_FINISH : Z_NO_FLUSH;
    auto ret = Z_OK;

    std::array<char, CPPHTTPLIB_COMPRESSION_BUFSIZ> buff{};
    do {
      strm_.avail_out = static_cast<uInt>(buff.size());
      strm_.next_out = reinterpret_cast<Bytef *>(buff.data());

      ret = deflate(&strm_, flush);
      if (ret == Z_STREAM_ERROR) { return false; }

      if (!callback(buff.data(), buff.size() - strm_.avail_out)) {
        return false;
      }
    } while (strm_.avail_out == 0);

    assert((flush == Z_FINISH && ret == Z_STREAM_END) ||
           (flush == Z_NO_FLUSH && ret == Z_OK));
    assert(strm_.avail_in == 0);
  } while (data_length > 0);

  return true;
}

gzip_decompressor::gzip_decompressor() {
  std::memset(&strm_, 0, sizeof(strm_));
  strm_.zalloc = Z_NULL;
  strm_.zfree = Z_NULL;
  strm_.opaque = Z_NULL;

  // 15 is the value of wbits, which should be at the maximum possible value
  // to ensure that any gzip stream can be decoded. The offset of 32 specifies
  // that the stream type should be automatically detected either gzip or
  // deflate.
  is_valid_ = inflateInit2(&strm_, 32 + 15) == Z_OK;
}

gzip_decompressor::~gzip_decompressor() { inflateEnd(&strm_); }

bool gzip_decompressor::is_valid() const { return is_valid_; }

bool gzip_decompressor::decompress(const char *data, size_t data_length,
                                          Callback callback) {
  assert(is_valid_);

  auto ret = Z_OK;

  do {
    constexpr size_t max_avail_in =
        (std::numeric_limits<decltype(strm_.avail_in)>::max)();

    strm_.avail_in = static_cast<decltype(strm_.avail_in)>(
        (std::min)(data_length, max_avail_in));
    strm_.next_in = const_cast<Bytef *>(reinterpret_cast<const Bytef *>(data));

    data_length -= strm_.avail_in;
    data += strm_.avail_in;

    std::array<char, CPPHTTPLIB_COMPRESSION_BUFSIZ> buff{};
    while (strm_.avail_in > 0 && ret == Z_OK) {
      strm_.avail_out = static_cast<uInt>(buff.size());
      strm_.next_out = reinterpret_cast<Bytef *>(buff.data());

      ret = inflate(&strm_, Z_NO_FLUSH);

      assert(ret != Z_STREAM_ERROR);
      switch (ret) {
      case Z_NEED_DICT:
      case Z_DATA_ERROR:
      case Z_MEM_ERROR: inflateEnd(&strm_); return false;
      }

      if (!callback(buff.data(), buff.size() - strm_.avail_out)) {
        return false;
      }
    }

    if (ret != Z_OK && ret != Z_STREAM_END) { return false; }

  } while (data_length > 0);

  return true;
}
#endif

#ifdef CPPHTTPLIB_BROTLI_SUPPORT
brotli_compressor::brotli_compressor() {
  state_ = BrotliEncoderCreateInstance(nullptr, nullptr, nullptr);
}

brotli_compressor::~brotli_compressor() {
  BrotliEncoderDestroyInstance(state_);
}

bool brotli_compressor::compress(const char *data, size_t data_length,
                                        bool last, Callback callback) {
  std::array<uint8_t, CPPHTTPLIB_COMPRESSION_BUFSIZ> buff{};

  auto operation = last ? BROTLI_OPERATION_FINISH : BROTLI_OPERATION_PROCESS;
  auto available_in = data_length;
  auto next_in = reinterpret_cast<const uint8_t *>(data);

  for (;;) {
    if (last) {
      if (BrotliEncoderIsFinished(state_)) { break; }
    } else {
      if (!available_in) { break; }
    }

    auto available_out = buff.size();
    auto next_out = buff.data();

    if (!BrotliEncoderCompressStream(state_, operation, &available_in, &next_in,
                                     &available_out, &next_out, nullptr)) {
      return false;
    }

    auto output_bytes = buff.size() - available_out;
    if (output_bytes) {
      callback(reinterpret_cast<const char *>(buff.data()), output_bytes);
    }
  }

  return true;
}

brotli_decompressor::brotli_decompressor() {
  decoder_s = BrotliDecoderCreateInstance(0, 0, 0);
  decoder_r = decoder_s ? BROTLI_DECODER_RESULT_NEEDS_MORE_INPUT
                        : BROTLI_DECODER_RESULT_ERROR;
}

brotli_decompressor::~brotli_decompressor() {
  if (decoder_s) { BrotliDecoderDestroyInstance(decoder_s); }
}

bool brotli_decompressor::is_valid() const { return decoder_s; }

bool brotli_decompressor::decompress(const char *data,
                                            size_t data_length,
                                            Callback callback) {
  if (decoder_r == BROTLI_DECODER_RESULT_SUCCESS ||
      decoder_r == BROTLI_DECODER_RESULT_ERROR) {
    return 0;
  }

  auto next_in = reinterpret_cast<const uint8_t *>(data);
  size_t avail_in = data_length;
  size_t total_out;

  decoder_r = BROTLI_DECODER_RESULT_NEEDS_MORE_OUTPUT;

  std::array<char, CPPHTTPLIB_COMPRESSION_BUFSIZ> buff{};
  while (decoder_r == BROTLI_DECODER_RESULT_NEEDS_MORE_OUTPUT) {
    char *next_out = buff.data();
    size_t avail_out = buff.size();

    decoder_r = BrotliDecoderDecompressStream(
        decoder_s, &avail_in, &next_in, &avail_out,
        reinterpret_cast<uint8_t **>(&next_out), &total_out);

    if (decoder_r == BROTLI_DECODER_RESULT_ERROR) { return false; }

    if (!callback(buff.data(), buff.size() - avail_out)) { return false; }
  }

  return decoder_r == BROTLI_DECODER_RESULT_SUCCESS ||
         decoder_r == BROTLI_DECODER_RESULT_NEEDS_MORE_INPUT;
}
#endif

bool has_header(const Headers &headers, const std::string &key) {
  return headers.find(key) != headers.end();
}

const char *get_header_value(const Headers &headers,
                                    const std::string &key, size_t id,
                                    const char *def) {
  auto rng = headers.equal_range(key);
  auto it = rng.first;
  std::advance(it, static_cast<ssize_t>(id));
  if (it != rng.second) { return it->second.c_str(); }
  return def;
}

bool compare_case_ignore(const std::string &a, const std::string &b) {
  if (a.size() != b.size()) { return false; }
  for (size_t i = 0; i < b.size(); i++) {
    if (::tolower(a[i]) != ::tolower(b[i])) { return false; }
  }
  return true;
}

template <typename T>
bool parse_header(const char *beg, const char *end, T fn) {
  // Skip trailing spaces and tabs.
  while (beg < end && is_space_or_tab(end[-1])) {
    end--;
  }

  auto p = beg;
  while (p < end && *p != ':') {
    p++;
  }

  if (p == end) { return false; }

  auto key_end = p;

  if (*p++ != ':') { return false; }

  while (p < end && is_space_or_tab(*p)) {
    p++;
  }

  if (p < end) {
    auto key_len = key_end - beg;
    if (!key_len) { return false; }

    auto key = std::string(beg, key_end);
    auto val = compare_case_ignore(key, "Location")
                   ? std::string(p, end)
                   : decode_url(std::string(p, end), false);
    fn(std::move(key), std::move(val));
    return true;
  }

  return false;
}

bool read_headers(Stream &strm, Headers &headers) {
  const auto bufsiz = 2048;
  char buf[bufsiz];
  stream_line_reader line_reader(strm, buf, bufsiz);

  for (;;) {
    if (!line_reader.getline()) { return false; }

    // Check if the line ends with CRLF.
    auto line_terminator_len = 2;
    if (line_reader.end_with_crlf()) {
      // Blank line indicates end of headers.
      if (line_reader.size() == 2) { break; }
#ifdef CPPHTTPLIB_ALLOW_LF_AS_LINE_TERMINATOR
    } else {
      // Blank line indicates end of headers.
      if (line_reader.size() == 1) { break; }
      line_terminator_len = 1;
    }
#else
    } else {
      continue; // Skip invalid line.
    }
#endif

    if (line_reader.size() > CPPHTTPLIB_HEADER_MAX_LENGTH) { return false; }

    // Exclude line terminator
    auto end = line_reader.ptr() + line_reader.size() - line_terminator_len;

    parse_header(line_reader.ptr(), end,
                 [&](std::string &&key, std::string &&val) {
                   headers.emplace(std::move(key), std::move(val));
                 });
  }

  return true;
}

bool read_content_with_length(Stream &strm, uint64_t len,
                                     Progress progress,
                                     ContentReceiverWithProgress out) {
  char buf[CPPHTTPLIB_RECV_BUFSIZ];

  uint64_t r = 0;
  while (r < len) {
    auto read_len = static_cast<size_t>(len - r);
    auto n = strm.read(buf, (std::min)(read_len, CPPHTTPLIB_RECV_BUFSIZ));
    if (n <= 0) { return false; }

    if (!out(buf, static_cast<size_t>(n), r, len)) { return false; }
    r += static_cast<uint64_t>(n);

    if (progress) {
      if (!progress(r, len)) { return false; }
    }
  }

  return true;
}

void skip_content_with_length(Stream &strm, uint64_t len) {
  char buf[CPPHTTPLIB_RECV_BUFSIZ];
  uint64_t r = 0;
  while (r < len) {
    auto read_len = static_cast<size_t>(len - r);
    auto n = strm.read(buf, (std::min)(read_len, CPPHTTPLIB_RECV_BUFSIZ));
    if (n <= 0) { return; }
    r += static_cast<uint64_t>(n);
  }
}

bool read_content_without_length(Stream &strm,
                                        ContentReceiverWithProgress out) {
  char buf[CPPHTTPLIB_RECV_BUFSIZ];
  uint64_t r = 0;
  for (;;) {
    auto n = strm.read(buf, CPPHTTPLIB_RECV_BUFSIZ);
    if (n <= 0) { return true; }

    if (!out(buf, static_cast<size_t>(n), r, 0)) { return false; }
    r += static_cast<uint64_t>(n);
  }

  return true;
}

template <typename T>
bool read_content_chunked(Stream &strm, T &x,
                                 ContentReceiverWithProgress out) {
  const auto bufsiz = 16;
  char buf[bufsiz];

  stream_line_reader line_reader(strm, buf, bufsiz);

  if (!line_reader.getline()) { return false; }

  unsigned long chunk_len;
  while (true) {
    char *end_ptr;

    chunk_len = std::strtoul(line_reader.ptr(), &end_ptr, 16);

    if (end_ptr == line_reader.ptr()) { return false; }
    if (chunk_len == ULONG_MAX) { return false; }

    if (chunk_len == 0) { break; }

    if (!read_content_with_length(strm, chunk_len, nullptr, out)) {
      return false;
    }

    if (!line_reader.getline()) { return false; }

    if (strcmp(line_reader.ptr(), "\r\n") != 0) { return false; }

    if (!line_reader.getline()) { return false; }
  }

  assert(chunk_len == 0);

  // Trailer
  if (!line_reader.getline()) { return false; }

  while (strcmp(line_reader.ptr(), "\r\n") != 0) {
    if (line_reader.size() > CPPHTTPLIB_HEADER_MAX_LENGTH) { return false; }

    // Exclude line terminator
    constexpr auto line_terminator_len = 2;
    auto end = line_reader.ptr() + line_reader.size() - line_terminator_len;

    parse_header(line_reader.ptr(), end,
                 [&](std::string &&key, std::string &&val) {
                   x.headers.emplace(std::move(key), std::move(val));
                 });

    if (!line_reader.getline()) { return false; }
  }

  return true;
}

bool is_chunked_transfer_encoding(const Headers &headers) {
  return compare_case_ignore(
      get_header_value(headers, "Transfer-Encoding", 0, ""), "chunked");
}

template <typename T, typename U>
bool prepare_content_receiver(T &x, int &status,
                              ContentReceiverWithProgress receiver,
                              bool decompress, U callback) {
  if (decompress) {
    std::string encoding = x.get_header_value("Content-Encoding");
    std::unique_ptr<decompressor> decompressor;

    if (encoding == "gzip" || encoding == "deflate") {
#ifdef CPPHTTPLIB_ZLIB_SUPPORT
      decompressor = detail::make_unique<gzip_decompressor>();
#else
      status = StatusCode::UnsupportedMediaType_415;
      return false;
#endif
    } else if (encoding.find("br") != std::string::npos) {
#ifdef CPPHTTPLIB_BROTLI_SUPPORT
      decompressor = detail::make_unique<brotli_decompressor>();
#else
      status = StatusCode::UnsupportedMediaType_415;
      return false;
#endif
    }

    if (decompressor) {
      if (decompressor->is_valid()) {
        ContentReceiverWithProgress out = [&](const char *buf, size_t n,
                                              uint64_t off, uint64_t len) {
          return decompressor->decompress(buf, n,
                                          [&](const char *buf2, size_t n2) {
                                            return receiver(buf2, n2, off, len);
                                          });
        };
        return callback(std::move(out));
      } else {
        status = StatusCode::InternalServerError_500;
        return false;
      }
    }
  }

  ContentReceiverWithProgress out = [&](const char *buf, size_t n, uint64_t off,
                                        uint64_t len) {
    return receiver(buf, n, off, len);
  };
  return callback(std::move(out));
}

template <typename T>
bool read_content(Stream &strm, T &x, size_t payload_max_length, int &status,
                  Progress progress, ContentReceiverWithProgress receiver,
                  bool decompress) {
  return prepare_content_receiver(
      x, status, std::move(receiver), decompress,
      [&](const ContentReceiverWithProgress &out) {
        auto ret = true;
        auto exceed_payload_max_length = false;

        if (is_chunked_transfer_encoding(x.headers)) {
          ret = read_content_chunked(strm, x, out);
        } else if (!has_header(x.headers, "Content-Length")) {
          ret = read_content_without_length(strm, out);
        } else {
          auto len = get_header_value_u64(x.headers, "Content-Length", 0, 0);
          if (len > payload_max_length) {
            exceed_payload_max_length = true;
            skip_content_with_length(strm, len);
            ret = false;
          } else if (len > 0) {
            ret = read_content_with_length(strm, len, std::move(progress), out);
          }
        }

        if (!ret) {
          status = exceed_payload_max_length ? StatusCode::PayloadTooLarge_413
                                             : StatusCode::BadRequest_400;
        }
        return ret;
      });
} // namespace detail

ssize_t write_headers(Stream &strm, const Headers &headers) {
  ssize_t write_len = 0;
  for (const auto &x : headers) {
    auto len =
        strm.write_format("%s: %s\r\n", x.first.c_str(), x.second.c_str());
    if (len < 0) { return len; }
    write_len += len;
  }
  auto len = strm.write("\r\n");
  if (len < 0) { return len; }
  write_len += len;
  return write_len;
}

bool write_data(Stream &strm, const char *d, size_t l) {
  size_t offset = 0;
  while (offset < l) {
    auto length = strm.write(d + offset, l - offset);
    if (length < 0) { return false; }
    offset += static_cast<size_t>(length);
  }
  return true;
}

template <typename T>
bool write_content(Stream &strm, const ContentProvider &content_provider,
                          size_t offset, size_t length, T is_shutting_down,
                          Error &error) {
  size_t end_offset = offset + length;
  auto ok = true;
  DataSink data_sink;

  data_sink.write = [&](const char *d, size_t l) -> bool {
    if (ok) {
      if (strm.is_writable() && write_data(strm, d, l)) {
        offset += l;
      } else {
        ok = false;
      }
    }
    return ok;
  };

  data_sink.is_writable = [&]() -> bool { return strm.is_writable(); };

  while (offset < end_offset && !is_shutting_down()) {
    if (!strm.is_writable()) {
      error = Error::Write;
      return false;
    } else if (!content_provider(offset, end_offset - offset, data_sink)) {
      error = Error::Canceled;
      return false;
    } else if (!ok) {
      error = Error::Write;
      return false;
    }
  }

  error = Error::Success;
  return true;
}

template <typename T>
bool write_content(Stream &strm, const ContentProvider &content_provider,
                          size_t offset, size_t length,
                          const T &is_shutting_down) {
  auto error = Error::Success;
  return write_content(strm, content_provider, offset, length, is_shutting_down,
                       error);
}

template <typename T>
bool
write_content_without_length(Stream &strm,
                             const ContentProvider &content_provider,
                             const T &is_shutting_down) {
  size_t offset = 0;
  auto data_available = true;
  auto ok = true;
  DataSink data_sink;

  data_sink.write = [&](const char *d, size_t l) -> bool {
    if (ok) {
      offset += l;
      if (!strm.is_writable() || !write_data(strm, d, l)) { ok = false; }
    }
    return ok;
  };

  data_sink.is_writable = [&]() -> bool { return strm.is_writable(); };

  data_sink.done = [&](void) { data_available = false; };

  while (data_available && !is_shutting_down()) {
    if (!strm.is_writable()) {
      return false;
    } else if (!content_provider(offset, 0, data_sink)) {
      return false;
    } else if (!ok) {
      return false;
    }
  }
  return true;
}

template <typename T, typename U>
bool
write_content_chunked(Stream &strm, const ContentProvider &content_provider,
                      const T &is_shutting_down, U &compressor, Error &error) {
  size_t offset = 0;
  auto data_available = true;
  auto ok = true;
  DataSink data_sink;

  data_sink.write = [&](const char *d, size_t l) -> bool {
    if (ok) {
      data_available = l > 0;
      offset += l;

      std::string payload;
      if (compressor.compress(d, l, false,
                              [&](const char *data, size_t data_len) {
                                payload.append(data, data_len);
                                return true;
                              })) {
        if (!payload.empty()) {
          // Emit chunked response header and footer for each chunk
          auto chunk =
              from_i_to_hex(payload.size()) + "\r\n" + payload + "\r\n";
          if (!strm.is_writable() ||
              !write_data(strm, chunk.data(), chunk.size())) {
            ok = false;
          }
        }
      } else {
        ok = false;
      }
    }
    return ok;
  };

  data_sink.is_writable = [&]() -> bool { return strm.is_writable(); };

  auto done_with_trailer = [&](const Headers *trailer) {
    if (!ok) { return; }

    data_available = false;

    std::string payload;
    if (!compressor.compress(nullptr, 0, true,
                             [&](const char *data, size_t data_len) {
                               payload.append(data, data_len);
                               return true;
                             })) {
      ok = false;
      return;
    }

    if (!payload.empty()) {
      // Emit chunked response header and footer for each chunk
      auto chunk = from_i_to_hex(payload.size()) + "\r\n" + payload + "\r\n";
      if (!strm.is_writable() ||
          !write_data(strm, chunk.data(), chunk.size())) {
        ok = false;
        return;
      }
    }

    static const std::string done_marker("0\r\n");
    if (!write_data(strm, done_marker.data(), done_marker.size())) {
      ok = false;
    }

    // Trailer
    if (trailer) {
      for (const auto &kv : *trailer) {
        std::string field_line = kv.first + ": " + kv.second + "\r\n";
        if (!write_data(strm, field_line.data(), field_line.size())) {
          ok = false;
        }
      }
    }

    static const std::string crlf("\r\n");
    if (!write_data(strm, crlf.data(), crlf.size())) { ok = false; }
  };

  data_sink.done = [&](void) { done_with_trailer(nullptr); };

  data_sink.done_with_trailer = [&](const Headers &trailer) {
    done_with_trailer(&trailer);
  };

  while (data_available && !is_shutting_down()) {
    if (!strm.is_writable()) {
      error = Error::Write;
      return false;
    } else if (!content_provider(offset, 0, data_sink)) {
      error = Error::Canceled;
      return false;
    } else if (!ok) {
      error = Error::Write;
      return false;
    }
  }

  error = Error::Success;
  return true;
}

template <typename T, typename U>
bool write_content_chunked(Stream &strm,
                                  const ContentProvider &content_provider,
                                  const T &is_shutting_down, U &compressor) {
  auto error = Error::Success;
  return write_content_chunked(strm, content_provider, is_shutting_down,
                               compressor, error);
}

template <typename T>
bool redirect(T &cli, Request &req, Response &res,
                     const std::string &path, const std::string &location,
                     Error &error) {
  Request new_req = req;
  new_req.path = path;
  new_req.redirect_count_ -= 1;

  if (res.status == StatusCode::SeeOther_303 &&
      (req.method != "GET" && req.method != "HEAD")) {
    new_req.method = "GET";
    new_req.body.clear();
    new_req.headers.clear();
  }

  Response new_res;

  auto ret = cli.send(new_req, new_res, error);
  if (ret) {
    req = new_req;
    res = new_res;

    if (res.location.empty()) { res.location = location; }
  }
  return ret;
}

std::string params_to_query_str(const Params &params) {
  std::string query;

  for (auto it = params.begin(); it != params.end(); ++it) {
    if (it != params.begin()) { query += "&"; }
    query += it->first;
    query += "=";
    query += encode_query_param(it->second);
  }
  return query;
}

void parse_query_text(const std::string &s, Params &params) {
  std::set<std::string> cache;
  split(s.data(), s.data() + s.size(), '&', [&](const char *b, const char *e) {
    std::string kv(b, e);
    if (cache.find(kv) != cache.end()) { return; }
    cache.insert(kv);

    std::string key;
    std::string val;
    split(b, e, '=', [&](const char *b2, const char *e2) {
      if (key.empty()) {
        key.assign(b2, e2);
      } else {
        val.assign(b2, e2);
      }
    });

    if (!key.empty()) {
      params.emplace(decode_url(key, true), decode_url(val, true));
    }
  });
}

bool parse_multipart_boundary(const std::string &content_type,
                                     std::string &boundary) {
  auto boundary_keyword = "boundary=";
  auto pos = content_type.find(boundary_keyword);
  if (pos == std::string::npos) { return false; }
  auto end = content_type.find(';', pos);
  auto beg = pos + strlen(boundary_keyword);
  boundary = trim_double_quotes_copy(content_type.substr(beg, end - beg));
  return !boundary.empty();
}

void parse_disposition_params(const std::string &s, Params &params) {
  std::set<std::string> cache;
  split(s.data(), s.data() + s.size(), ';', [&](const char *b, const char *e) {
    std::string kv(b, e);
    if (cache.find(kv) != cache.end()) { return; }
    cache.insert(kv);

    std::string key;
    std::string val;
    split(b, e, '=', [&](const char *b2, const char *e2) {
      if (key.empty()) {
        key.assign(b2, e2);
      } else {
        val.assign(b2, e2);
      }
    });

    if (!key.empty()) {
      params.emplace(trim_double_quotes_copy((key)),
                     trim_double_quotes_copy((val)));
    }
  });
}

#ifdef CPPHTTPLIB_NO_EXCEPTIONS
bool parse_range_header(const std::string &s, Ranges &ranges) {
#else
bool parse_range_header(const std::string &s, Ranges &ranges) try {
#endif
  static auto re_first_range = std::regex(R"(bytes=(\d*-\d*(?:,\s*\d*-\d*)*))");
  std::smatch m;
  if (std::regex_match(s, m, re_first_range)) {
    auto pos = static_cast<size_t>(m.position(1));
    auto len = static_cast<size_t>(m.length(1));
    auto all_valid_ranges = true;
    split(&s[pos], &s[pos + len], ',', [&](const char *b, const char *e) {
      if (!all_valid_ranges) { return; }
      static auto re_another_range = std::regex(R"(\s*(\d*)-(\d*))");
      std::cmatch cm;
      if (std::regex_match(b, e, cm, re_another_range)) {
        ssize_t first = -1;
        if (!cm.str(1).empty()) {
          first = static_cast<ssize_t>(std::stoll(cm.str(1)));
        }

        ssize_t last = -1;
        if (!cm.str(2).empty()) {
          last = static_cast<ssize_t>(std::stoll(cm.str(2)));
        }

        if (first != -1 && last != -1 && first > last) {
          all_valid_ranges = false;
          return;
        }
        ranges.emplace_back(std::make_pair(first, last));
      }
    });
    return all_valid_ranges;
  }
  return false;
#ifdef CPPHTTPLIB_NO_EXCEPTIONS
}
#else
} catch (...) { return false; }
#endif

class MultipartFormDataParser {
public:
  MultipartFormDataParser() = default;

  void set_boundary(std::string &&boundary) {
    boundary_ = boundary;
    dash_boundary_crlf_ = dash_ + boundary_ + crlf_;
    crlf_dash_boundary_ = crlf_ + dash_ + boundary_;
  }

  bool is_valid() const { return is_valid_; }

  bool parse(const char *buf, size_t n, const ContentReceiver &content_callback,
             const MultipartContentHeader &header_callback) {

    buf_append(buf, n);

    while (buf_size() > 0) {
      switch (state_) {
      case 0: { // Initial boundary
        buf_erase(buf_find(dash_boundary_crlf_));
        if (dash_boundary_crlf_.size() > buf_size()) { return true; }
        if (!buf_start_with(dash_boundary_crlf_)) { return false; }
        buf_erase(dash_boundary_crlf_.size());
        state_ = 1;
        break;
      }
      case 1: { // New entry
        clear_file_info();
        state_ = 2;
        break;
      }
      case 2: { // Headers
        auto pos = buf_find(crlf_);
        if (pos > CPPHTTPLIB_HEADER_MAX_LENGTH) { return false; }
        while (pos < buf_size()) {
          // Empty line
          if (pos == 0) {
            if (!header_callback(file_)) {
              is_valid_ = false;
              return false;
            }
            buf_erase(crlf_.size());
            state_ = 3;
            break;
          }

          const auto header = buf_head(pos);

          if (!parse_header(header.data(), header.data() + header.size(),
                            [&](std::string &&, std::string &&) {})) {
            is_valid_ = false;
            return false;
          }

          static const std::string header_content_type = "Content-Type:";

          if (start_with_case_ignore(header, header_content_type)) {
            file_.content_type =
                trim_copy(header.substr(header_content_type.size()));
          } else {
            static const std::regex re_content_disposition(
                R"~(^Content-Disposition:\s*form-data;\s*(.*)$)~",
                std::regex_constants::icase);

            std::smatch m;
            if (std::regex_match(header, m, re_content_disposition)) {
              Params params;
              parse_disposition_params(m[1], params);

              auto it = params.find("name");
              if (it != params.end()) {
                file_.name = it->second;
              } else {
                is_valid_ = false;
                return false;
              }

              it = params.find("filename");
              if (it != params.end()) { file_.filename = it->second; }

              it = params.find("filename*");
              if (it != params.end()) {
                // Only allow UTF-8 enconnding...
                static const std::regex re_rfc5987_encoding(
                    R"~(^UTF-8''(.+?)$)~", std::regex_constants::icase);

                std::smatch m2;
                if (std::regex_match(it->second, m2, re_rfc5987_encoding)) {
                  file_.filename = decode_url(m2[1], false); // override...
                } else {
                  is_valid_ = false;
                  return false;
                }
              }
            }
          }
          buf_erase(pos + crlf_.size());
          pos = buf_find(crlf_);
        }
        if (state_ != 3) { return true; }
        break;
      }
      case 3: { // Body
        if (crlf_dash_boundary_.size() > buf_size()) { return true; }
        auto pos = buf_find(crlf_dash_boundary_);
        if (pos < buf_size()) {
          if (!content_callback(buf_data(), pos)) {
            is_valid_ = false;
            return false;
          }
          buf_erase(pos + crlf_dash_boundary_.size());
          state_ = 4;
        } else {
          auto len = buf_size() - crlf_dash_boundary_.size();
          if (len > 0) {
            if (!content_callback(buf_data(), len)) {
              is_valid_ = false;
              return false;
            }
            buf_erase(len);
          }
          return true;
        }
        break;
      }
      case 4: { // Boundary
        if (crlf_.size() > buf_size()) { return true; }
        if (buf_start_with(crlf_)) {
          buf_erase(crlf_.size());
          state_ = 1;
        } else {
          if (dash_.size() > buf_size()) { return true; }
          if (buf_start_with(dash_)) {
            buf_erase(dash_.size());
            is_valid_ = true;
            buf_erase(buf_size()); // Remove epilogue
          } else {
            return true;
          }
        }
        break;
      }
      }
    }

    return true;
  }

private:
  void clear_file_info() {
    file_.name.clear();
    file_.filename.clear();
    file_.content_type.clear();
  }

  bool start_with_case_ignore(const std::string &a,
                              const std::string &b) const {
    if (a.size() < b.size()) { return false; }
    for (size_t i = 0; i < b.size(); i++) {
      if (::tolower(a[i]) != ::tolower(b[i])) { return false; }
    }
    return true;
  }

  const std::string dash_ = "--";
  const std::string crlf_ = "\r\n";
  std::string boundary_;
  std::string dash_boundary_crlf_;
  std::string crlf_dash_boundary_;

  size_t state_ = 0;
  bool is_valid_ = false;
  MultipartFormData file_;

  // Buffer
  bool start_with(const std::string &a, size_t spos, size_t epos,
                  const std::string &b) const {
    if (epos - spos < b.size()) { return false; }
    for (size_t i = 0; i < b.size(); i++) {
      if (a[i + spos] != b[i]) { return false; }
    }
    return true;
  }

  size_t buf_size() const { return buf_epos_ - buf_spos_; }

  const char *buf_data() const { return &buf_[buf_spos_]; }

  std::string buf_head(size_t l) const { return buf_.substr(buf_spos_, l); }

  bool buf_start_with(const std::string &s) const {
    return start_with(buf_, buf_spos_, buf_epos_, s);
  }

  size_t buf_find(const std::string &s) const {
    auto c = s.front();

    size_t off = buf_spos_;
    while (off < buf_epos_) {
      auto pos = off;
      while (true) {
        if (pos == buf_epos_) { return buf_size(); }
        if (buf_[pos] == c) { break; }
        pos++;
      }

      auto remaining_size = buf_epos_ - pos;
      if (s.size() > remaining_size) { return buf_size(); }

      if (start_with(buf_, pos, buf_epos_, s)) { return pos - buf_spos_; }

      off = pos + 1;
    }

    return buf_size();
  }

  void buf_append(const char *data, size_t n) {
    auto remaining_size = buf_size();
    if (remaining_size > 0 && buf_spos_ > 0) {
      for (size_t i = 0; i < remaining_size; i++) {
        buf_[i] = buf_[buf_spos_ + i];
      }
    }
    buf_spos_ = 0;
    buf_epos_ = remaining_size;

    if (remaining_size + n > buf_.size()) { buf_.resize(remaining_size + n); }

    for (size_t i = 0; i < n; i++) {
      buf_[buf_epos_ + i] = data[i];
    }
    buf_epos_ += n;
  }

  void buf_erase(size_t size) { buf_spos_ += size; }

  std::string buf_;
  size_t buf_spos_ = 0;
  size_t buf_epos_ = 0;
};

std::string to_lower(const char *beg, const char *end) {
  std::string out;
  auto it = beg;
  while (it != end) {
    out += static_cast<char>(::tolower(*it));
    it++;
  }
  return out;
}

std::string random_string(size_t length) {
  static const char data[] =
      "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

  // std::random_device might actually be deterministic on some
  // platforms, but due to lack of support in the c++ standard library,
  // doing better requires either some ugly hacks or breaking portability.
  static std::random_device seed_gen;

  // Request 128 bits of entropy for initialization
  static std::seed_seq seed_sequence{seed_gen(), seed_gen(), seed_gen(),
                                     seed_gen()};

  static std::mt19937 engine(seed_sequence);

  std::string result;
  for (size_t i = 0; i < length; i++) {
    result += data[engine() % (sizeof(data) - 1)];
  }
  return result;
}

std::string make_multipart_data_boundary() {
  return "--cpp-httplib-multipart-data-" + detail::random_string(16);
}

bool is_multipart_boundary_chars_valid(const std::string &boundary) {
  auto valid = true;
  for (size_t i = 0; i < boundary.size(); i++) {
    auto c = boundary[i];
    if (!std::isalnum(c) && c != '-' && c != '_') {
      valid = false;
      break;
    }
  }
  return valid;
}

template <typename T>
std::string
serialize_multipart_formdata_item_begin(const T &item,
                                        const std::string &boundary) {
  std::string body = "--" + boundary + "\r\n";
  body += "Content-Disposition: form-data; name=\"" + item.name + "\"";
  if (!item.filename.empty()) {
    body += "; filename=\"" + item.filename + "\"";
  }
  body += "\r\n";
  if (!item.content_type.empty()) {
    body += "Content-Type: " + item.content_type + "\r\n";
  }
  body += "\r\n";

  return body;
}

std::string serialize_multipart_formdata_item_end() { return "\r\n"; }

std::string
serialize_multipart_formdata_finish(const std::string &boundary) {
  return "--" + boundary + "--\r\n";
}

std::string
serialize_multipart_formdata_get_content_type(const std::string &boundary) {
  return "multipart/form-data; boundary=" + boundary;
}

std::string
serialize_multipart_formdata(const MultipartFormDataItems &items,
                             const std::string &boundary, bool finish = true) {
  std::string body;

  for (const auto &item : items) {
    body += serialize_multipart_formdata_item_begin(item, boundary);
    body += item.content + serialize_multipart_formdata_item_end();
  }

  if (finish) { body += serialize_multipart_formdata_finish(boundary); }

  return body;
}

bool range_error(Request &req, Response &res) {
  if (!req.ranges.empty() && 200 <= res.status && res.status < 300) {
    ssize_t contant_len = static_cast<ssize_t>(
        res.content_length_ ? res.content_length_ : res.body.size());

    ssize_t prev_first_pos = -1;
    ssize_t prev_last_pos = -1;
    size_t overwrapping_count = 0;

    // NOTE: The following Range check is based on '14.2. Range' in RFC 9110
    // 'HTTP Semantics' to avoid potential denial-of-service attacks.
    // https://www.rfc-editor.org/rfc/rfc9110#section-14.2

    // Too many ranges
    if (req.ranges.size() > CPPHTTPLIB_RANGE_MAX_COUNT) { return true; }

    for (auto &r : req.ranges) {
      auto &first_pos = r.first;
      auto &last_pos = r.second;

      if (first_pos == -1 && last_pos == -1) {
        first_pos = 0;
        last_pos = contant_len;
      }

      if (first_pos == -1) {
        first_pos = contant_len - last_pos;
        last_pos = contant_len - 1;
      }

      if (last_pos == -1) { last_pos = contant_len - 1; }

      // Range must be within content length
      if (!(0 <= first_pos && first_pos <= last_pos &&
            last_pos <= contant_len - 1)) {
        return true;
      }

      // Ranges must be in ascending order
      if (first_pos <= prev_first_pos) { return true; }

      // Request must not have more than two overlapping ranges
      if (first_pos <= prev_last_pos) {
        overwrapping_count++;
        if (overwrapping_count > 2) { return true; }
      }

      prev_first_pos = (std::max)(prev_first_pos, first_pos);
      prev_last_pos = (std::max)(prev_last_pos, last_pos);
    }
  }

  return false;
}

std::pair<size_t, size_t>
get_range_offset_and_length(Range r, size_t content_length) {
  assert(r.first != -1 && r.second != -1);
  assert(0 <= r.first && r.first < static_cast<ssize_t>(content_length));
  assert(r.first <= r.second &&
         r.second < static_cast<ssize_t>(content_length));

  return std::make_pair(r.first, static_cast<size_t>(r.second - r.first) + 1);
}

std::string make_content_range_header_field(
    const std::pair<size_t, size_t> &offset_and_length, size_t content_length) {
  auto st = offset_and_length.first;
  auto ed = st + offset_and_length.second - 1;

  std::string field = "bytes ";
  field += std::to_string(st);
  field += "-";
  field += std::to_string(ed);
  field += "/";
  field += std::to_string(content_length);
  return field;
}

template <typename SToken, typename CToken, typename Content>
bool process_multipart_ranges_data(const Request &req,
                                   const std::string &boundary,
                                   const std::string &content_type,
                                   size_t content_length, SToken stoken,
                                   CToken ctoken, Content content) {
  for (size_t i = 0; i < req.ranges.size(); i++) {
    ctoken("--");
    stoken(boundary);
    ctoken("\r\n");
    if (!content_type.empty()) {
      ctoken("Content-Type: ");
      stoken(content_type);
      ctoken("\r\n");
    }

    auto offset_and_length =
        get_range_offset_and_length(req.ranges[i], content_length);

    ctoken("Content-Range: ");
    stoken(make_content_range_header_field(offset_and_length, content_length));
    ctoken("\r\n");
    ctoken("\r\n");

    if (!content(offset_and_length.first, offset_and_length.second)) {
      return false;
    }
    ctoken("\r\n");
  }

  ctoken("--");
  stoken(boundary);
  ctoken("--");

  return true;
}

void make_multipart_ranges_data(const Request &req, Response &res,
                                       const std::string &boundary,
                                       const std::string &content_type,
                                       size_t content_length,
                                       std::string &data) {
  process_multipart_ranges_data(
      req, boundary, content_type, content_length,
      [&](const std::string &token) { data += token; },
      [&](const std::string &token) { data += token; },
      [&](size_t offset, size_t length) {
        assert(offset + length <= content_length);
        data += res.body.substr(offset, length);
        return true;
      });
}

size_t get_multipart_ranges_data_length(const Request &req,
                                               const std::string &boundary,
                                               const std::string &content_type,
                                               size_t content_length) {
  size_t data_length = 0;

  process_multipart_ranges_data(
      req, boundary, content_type, content_length,
      [&](const std::string &token) { data_length += token.size(); },
      [&](const std::string &token) { data_length += token.size(); },
      [&](size_t /*offset*/, size_t length) {
        data_length += length;
        return true;
      });

  return data_length;
}

template <typename T>
bool
write_multipart_ranges_data(Stream &strm, const Request &req, Response &res,
                            const std::string &boundary,
                            const std::string &content_type,
                            size_t content_length, const T &is_shutting_down) {
  return process_multipart_ranges_data(
      req, boundary, content_type, content_length,
      [&](const std::string &token) { strm.write(token); },
      [&](const std::string &token) { strm.write(token); },
      [&](size_t offset, size_t length) {
        return write_content(strm, res.content_provider_, offset, length,
                             is_shutting_down);
      });
}

bool expect_content(const Request &req) {
  if (req.method == "POST" || req.method == "PUT" || req.method == "PATCH" ||
      req.method == "PRI" || req.method == "DELETE") {
    return true;
  }
  // TODO: check if Content-Length is set
  return false;
}

bool has_crlf(const std::string &s) {
  auto p = s.c_str();
  while (*p) {
    if (*p == '\r' || *p == '\n') { return true; }
    p++;
  }
  return false;
}

#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
std::string message_digest(const std::string &s, const EVP_MD *algo) {
  auto context = std::unique_ptr<EVP_MD_CTX, decltype(&EVP_MD_CTX_free)>(
      EVP_MD_CTX_new(), EVP_MD_CTX_free);

  unsigned int hash_length = 0;
  unsigned char hash[EVP_MAX_MD_SIZE];

  EVP_DigestInit_ex(context.get(), algo, nullptr);
  EVP_DigestUpdate(context.get(), s.c_str(), s.size());
  EVP_DigestFinal_ex(context.get(), hash, &hash_length);

  std::stringstream ss;
  for (auto i = 0u; i < hash_length; ++i) {
    ss << std::hex << std::setw(2) << std::setfill('0')
       << static_cast<unsigned int>(hash[i]);
  }

  return ss.str();
}

std::string MD5(const std::string &s) {
  return message_digest(s, EVP_md5());
}

std::string SHA_256(const std::string &s) {
  return message_digest(s, EVP_sha256());
}

std::string SHA_512(const std::string &s) {
  return message_digest(s, EVP_sha512());
}
#endif

#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
#ifdef _WIN32
// NOTE: This code came up with the following stackoverflow post:
// https://stackoverflow.com/questions/9507184/can-openssl-on-windows-use-the-system-certificate-store
bool load_system_certs_on_windows(X509_STORE *store) {
  auto hStore = CertOpenSystemStoreW((HCRYPTPROV_LEGACY)NULL, L"ROOT");
  if (!hStore) { return false; }

  auto result = false;
  PCCERT_CONTEXT pContext = NULL;
  while ((pContext = CertEnumCertificatesInStore(hStore, pContext)) !=
         nullptr) {
    auto encoded_cert =
        static_cast<const unsigned char *>(pContext->pbCertEncoded);

    auto x509 = d2i_X509(NULL, &encoded_cert, pContext->cbCertEncoded);
    if (x509) {
      X509_STORE_add_cert(store, x509);
      X509_free(x509);
      result = true;
    }
  }

  CertFreeCertificateContext(pContext);
  CertCloseStore(hStore, 0);

  return result;
}
#elif defined(CPPHTTPLIB_USE_CERTS_FROM_MACOSX_KEYCHAIN) && defined(__APPLE__)
#if TARGET_OS_OSX
template <typename T>
using CFObjectPtr =
    std::unique_ptr<typename std::remove_pointer<T>::type, void (*)(CFTypeRef)>;

void cf_object_ptr_deleter(CFTypeRef obj) {
  if (obj) { CFRelease(obj); }
}

bool retrieve_certs_from_keychain(CFObjectPtr<CFArrayRef> &certs) {
  CFStringRef keys[] = {kSecClass, kSecMatchLimit, kSecReturnRef};
  CFTypeRef values[] = {kSecClassCertificate, kSecMatchLimitAll,
                        kCFBooleanTrue};

  CFObjectPtr<CFDictionaryRef> query(
      CFDictionaryCreate(nullptr, reinterpret_cast<const void **>(keys), values,
                         sizeof(keys) / sizeof(keys[0]),
                         &kCFTypeDictionaryKeyCallBacks,
                         &kCFTypeDictionaryValueCallBacks),
      cf_object_ptr_deleter);

  if (!query) { return false; }

  CFTypeRef security_items = nullptr;
  if (SecItemCopyMatching(query.get(), &security_items) != errSecSuccess ||
      CFArrayGetTypeID() != CFGetTypeID(security_items)) {
    return false;
  }

  certs.reset(reinterpret_cast<CFArrayRef>(security_items));
  return true;
}

bool retrieve_root_certs_from_keychain(CFObjectPtr<CFArrayRef> &certs) {
  CFArrayRef root_security_items = nullptr;
  if (SecTrustCopyAnchorCertificates(&root_security_items) != errSecSuccess) {
    return false;
  }

  certs.reset(root_security_items);
  return true;
}

bool add_certs_to_x509_store(CFArrayRef certs, X509_STORE *store) {
  auto result = false;
  for (auto i = 0; i < CFArrayGetCount(certs); ++i) {
    const auto cert = reinterpret_cast<const __SecCertificate *>(
        CFArrayGetValueAtIndex(certs, i));

    if (SecCertificateGetTypeID() != CFGetTypeID(cert)) { continue; }

    CFDataRef cert_data = nullptr;
    if (SecItemExport(cert, kSecFormatX509Cert, 0, nullptr, &cert_data) !=
        errSecSuccess) {
      continue;
    }

    CFObjectPtr<CFDataRef> cert_data_ptr(cert_data, cf_object_ptr_deleter);

    auto encoded_cert = static_cast<const unsigned char *>(
        CFDataGetBytePtr(cert_data_ptr.get()));

    auto x509 =
        d2i_X509(NULL, &encoded_cert, CFDataGetLength(cert_data_ptr.get()));

    if (x509) {
      X509_STORE_add_cert(store, x509);
      X509_free(x509);
      result = true;
    }
  }

  return result;
}

bool load_system_certs_on_macos(X509_STORE *store) {
  auto result = false;
  CFObjectPtr<CFArrayRef> certs(nullptr, cf_object_ptr_deleter);
  if (retrieve_certs_from_keychain(certs) && certs) {
    result = add_certs_to_x509_store(certs.get(), store);
  }

  if (retrieve_root_certs_from_keychain(certs) && certs) {
    result = add_certs_to_x509_store(certs.get(), store) || result;
  }

  return result;
}
#endif // TARGET_OS_OSX
#endif // _WIN32
#endif // CPPHTTPLIB_OPENSSL_SUPPORT

#ifdef _WIN32
class WSInit {
public:
  WSInit() {
    WSADATA wsaData;
    if (WSAStartup(0x0002, &wsaData) == 0) is_valid_ = true;
  }

  ~WSInit() {
    if (is_valid_) WSACleanup();
  }

  bool is_valid_ = false;
};

static WSInit wsinit_;
#endif

#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
std::pair<std::string, std::string> make_digest_authentication_header(
    const Request &req, const std::map<std::string, std::string> &auth,
    size_t cnonce_count, const std::string &cnonce, const std::string &username,
    const std::string &password, bool is_proxy = false) {
  std::string nc;
  {
    std::stringstream ss;
    ss << std::setfill('0') << std::setw(8) << std::hex << cnonce_count;
    nc = ss.str();
  }

  std::string qop;
  if (auth.find("qop") != auth.end()) {
    qop = auth.at("qop");
    if (qop.find("auth-int") != std::string::npos) {
      qop = "auth-int";
    } else if (qop.find("auth") != std::string::npos) {
      qop = "auth";
    } else {
      qop.clear();
    }
  }

  std::string algo = "MD5";
  if (auth.find("algorithm") != auth.end()) { algo = auth.at("algorithm"); }

  std::string response;
  {
    auto H = algo == "SHA-256"   ? detail::SHA_256
             : algo == "SHA-512" ? detail::SHA_512
                                 : detail::MD5;

    auto A1 = username + ":" + auth.at("realm") + ":" + password;

    auto A2 = req.method + ":" + req.path;
    if (qop == "auth-int") { A2 += ":" + H(req.body); }

    if (qop.empty()) {
      response = H(H(A1) + ":" + auth.at("nonce") + ":" + H(A2));
    } else {
      response = H(H(A1) + ":" + auth.at("nonce") + ":" + nc + ":" + cnonce +
                   ":" + qop + ":" + H(A2));
    }
  }

  auto opaque = (auth.find("opaque") != auth.end()) ? auth.at("opaque") : "";

  auto field = "Digest username=\"" + username + "\", realm=\"" +
               auth.at("realm") + "\", nonce=\"" + auth.at("nonce") +
               "\", uri=\"" + req.path + "\", algorithm=" + algo +
               (qop.empty() ? ", response=\""
                            : ", qop=" + qop + ", nc=" + nc + ", cnonce=\"" +
                                  cnonce + "\", response=\"") +
               response + "\"" +
               (opaque.empty() ? "" : ", opaque=\"" + opaque + "\"");

  auto key = is_proxy ? "Proxy-Authorization" : "Authorization";
  return std::make_pair(key, field);
}
#endif

bool parse_www_authenticate(const Response &res,
                                   std::map<std::string, std::string> &auth,
                                   bool is_proxy) {
  auto auth_key = is_proxy ? "Proxy-Authenticate" : "WWW-Authenticate";
  if (res.has_header(auth_key)) {
    static auto re = std::regex(R"~((?:(?:,\s*)?(.+?)=(?:"(.*?)"|([^,]*))))~");
    auto s = res.get_header_value(auth_key);
    auto pos = s.find(' ');
    if (pos != std::string::npos) {
      auto type = s.substr(0, pos);
      if (type == "Basic") {
        return false;
      } else if (type == "Digest") {
        s = s.substr(pos + 1);
        auto beg = std::sregex_iterator(s.begin(), s.end(), re);
        for (auto i = beg; i != std::sregex_iterator(); ++i) {
          const auto &m = *i;
          auto key = s.substr(static_cast<size_t>(m.position(1)),
                              static_cast<size_t>(m.length(1)));
          auto val = m.length(2) > 0
                         ? s.substr(static_cast<size_t>(m.position(2)),
                                    static_cast<size_t>(m.length(2)))
                         : s.substr(static_cast<size_t>(m.position(3)),
                                    static_cast<size_t>(m.length(3)));
          auth[key] = val;
        }
        return true;
      }
    }
  }
  return false;
}

class ContentProviderAdapter {
public:
  explicit ContentProviderAdapter(
      ContentProviderWithoutLength &&content_provider)
      : content_provider_(content_provider) {}

  bool operator()(size_t offset, size_t, DataSink &sink) {
    return content_provider_(offset, sink);
  }

private:
  ContentProviderWithoutLength content_provider_;
};

} // namespace detail

std::string hosted_at(const std::string &hostname) {
  std::vector<std::string> addrs;
  hosted_at(hostname, addrs);
  if (addrs.empty()) { return std::string(); }
  return addrs[0];
}

void hosted_at(const std::string &hostname,
                      std::vector<std::string> &addrs) {
  struct addrinfo hints;
  struct addrinfo *result;

  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_protocol = 0;

  if (getaddrinfo(hostname.c_str(), nullptr, &hints, &result)) {
#if defined __linux__ && !defined __ANDROID__
    res_init();
#endif
    return;
  }

  for (auto rp = result; rp; rp = rp->ai_next) {
    const auto &addr =
        *reinterpret_cast<struct sockaddr_storage *>(rp->ai_addr);
    std::string ip;
    auto dummy = -1;
    if (detail::get_ip_and_port(addr, sizeof(struct sockaddr_storage), ip,
                                dummy)) {
      addrs.push_back(ip);
    }
  }

  freeaddrinfo(result);
}

std::string append_query_params(const std::string &path,
                                       const Params &params) {
  std::string path_with_query = path;
  const static std::regex re("[^?]+\\?.*");
  auto delm = std::regex_match(path, re) ? '&' : '?';
  path_with_query += delm + detail::params_to_query_str(params);
  return path_with_query;
}

// Header utilities
std::pair<std::string, std::string>
make_range_header(const Ranges &ranges) {
  std::string field = "bytes=";
  auto i = 0;
  for (const auto &r : ranges) {
    if (i != 0) { field += ", "; }
    if (r.first != -1) { field += std::to_string(r.first); }
    field += '-';
    if (r.second != -1) { field += std::to_string(r.second); }
    i++;
  }
  return std::make_pair("Range", std::move(field));
}

std::pair<std::string, std::string>
make_basic_authentication_header(const std::string &username,
                                 const std::string &password, bool is_proxy) {
  auto field = "Basic " + detail::base64_encode(username + ":" + password);
  auto key = is_proxy ? "Proxy-Authorization" : "Authorization";
  return std::make_pair(key, std::move(field));
}

std::pair<std::string, std::string>
make_bearer_token_authentication_header(const std::string &token,
                                        bool is_proxy = false) {
  auto field = "Bearer " + token;
  auto key = is_proxy ? "Proxy-Authorization" : "Authorization";
  return std::make_pair(key, std::move(field));
}

// Request implementation
bool Request::has_header(const std::string &key) const {
  return detail::has_header(headers, key);
}

std::string Request::get_header_value(const std::string &key,
                                             size_t id) const {
  return detail::get_header_value(headers, key, id, "");
}

size_t Request::get_header_value_count(const std::string &key) const {
  auto r = headers.equal_range(key);
  return static_cast<size_t>(std::distance(r.first, r.second));
}

void Request::set_header(const std::string &key,
                                const std::string &val) {
  if (!detail::has_crlf(key) && !detail::has_crlf(val)) {
    headers.emplace(key, val);
  }
}

bool Request::has_param(const std::string &key) const {
  return params.find(key) != params.end();
}

std::string Request::get_param_value(const std::string &key,
                                            size_t id) const {
  auto rng = params.equal_range(key);
  auto it = rng.first;
  std::advance(it, static_cast<ssize_t>(id));
  if (it != rng.second) { return it->second; }
  return std::string();
}

size_t Request::get_param_value_count(const std::string &key) const {
  auto r = params.equal_range(key);
  return static_cast<size_t>(std::distance(r.first, r.second));
}

bool Request::is_multipart_form_data() const {
  const auto &content_type = get_header_value("Content-Type");
  return !content_type.rfind("multipart/form-data", 0);
}

bool Request::has_file(const std::string &key) const {
  return files.find(key) != files.end();
}

MultipartFormData Request::get_file_value(const std::string &key) const {
  auto it = files.find(key);
  if (it != files.end()) { return it->second; }
  return MultipartFormData();
}

std::vector<MultipartFormData>
Request::get_file_values(const std::string &key) const {
  std::vector<MultipartFormData> values;
  auto rng = files.equal_range(key);
  for (auto it = rng.first; it != rng.second; it++) {
    values.push_back(it->second);
  }
  return values;
}

// Response implementation
bool Response::has_header(const std::string &key) const {
  return headers.find(key) != headers.end();
}

std::string Response::get_header_value(const std::string &key,
                                              size_t id) const {
  return detail::get_header_value(headers, key, id, "");
}

size_t Response::get_header_value_count(const std::string &key) const {
  auto r = headers.equal_range(key);
  return static_cast<size_t>(std::distance(r.first, r.second));
}

void Response::set_header(const std::string &key,
                                 const std::string &val) {
  if (!detail::has_crlf(key) && !detail::has_crlf(val)) {
    headers.emplace(key, val);
  }
}

void Response::set_redirect(const std::string &url, int stat) {
  if (!detail::has_crlf(url)) {
    set_header("Location", url);
    if (300 <= stat && stat < 400) {
      this->status = stat;
    } else {
      this->status = StatusCode::Found_302;
    }
  }
}

void Response::set_content(const char *s, size_t n,
                                  const std::string &content_type) {
  body.assign(s, n);

  auto rng = headers.equal_range("Content-Type");
  headers.erase(rng.first, rng.second);
  set_header("Content-Type", content_type);
}

void Response::set_content(const std::string &s,
                                  const std::string &content_type) {
  set_content(s.data(), s.size(), content_type);
}

void Response::set_content(std::string &&s,
                                  const std::string &content_type) {
  body = std::move(s);

  auto rng = headers.equal_range("Content-Type");
  headers.erase(rng.first, rng.second);
  set_header("Content-Type", content_type);
}

void Response::set_content_provider(
    size_t in_length, const std::string &content_type, ContentProvider provider,
    ContentProviderResourceReleaser resource_releaser) {
  set_header("Content-Type", content_type);
  content_length_ = in_length;
  if (in_length > 0) { content_provider_ = std::move(provider); }
  content_provider_resource_releaser_ = std::move(resource_releaser);
  is_chunked_content_provider_ = false;
}

void Response::set_content_provider(
    const std::string &content_type, ContentProviderWithoutLength provider,
    ContentProviderResourceReleaser resource_releaser) {
  set_header("Content-Type", content_type);
  content_length_ = 0;
  content_provider_ = detail::ContentProviderAdapter(std::move(provider));
  content_provider_resource_releaser_ = std::move(resource_releaser);
  is_chunked_content_provider_ = false;
}

void Response::set_chunked_content_provider(
    const std::string &content_type, ContentProviderWithoutLength provider,
    ContentProviderResourceReleaser resource_releaser) {
  set_header("Content-Type", content_type);
  content_length_ = 0;
  content_provider_ = detail::ContentProviderAdapter(std::move(provider));
  content_provider_resource_releaser_ = std::move(resource_releaser);
  is_chunked_content_provider_ = true;
}

// Result implementation
bool Result::has_request_header(const std::string &key) const {
  return request_headers_.find(key) != request_headers_.end();
}

std::string Result::get_request_header_value(const std::string &key,
                                                    size_t id) const {
  return detail::get_header_value(request_headers_, key, id, "");
}

size_t
Result::get_request_header_value_count(const std::string &key) const {
  auto r = request_headers_.equal_range(key);
  return static_cast<size_t>(std::distance(r.first, r.second));
}

// Stream implementation
ssize_t Stream::write(const char *ptr) {
  return write(ptr, strlen(ptr));
}

ssize_t Stream::write(const std::string &s) {
  return write(s.data(), s.size());
}

namespace detail {

// Socket stream implementation
SocketStream::SocketStream(socket_t sock, time_t read_timeout_sec,
                                  time_t read_timeout_usec,
                                  time_t write_timeout_sec,
                                  time_t write_timeout_usec)
    : sock_(sock), read_timeout_sec_(read_timeout_sec),
      read_timeout_usec_(read_timeout_usec),
      write_timeout_sec_(write_timeout_sec),
      write_timeout_usec_(write_timeout_usec), read_buff_(read_buff_size_, 0) {}

SocketStream::~SocketStream() = default;

bool SocketStream::is_readable() const {
  return select_read(sock_, read_timeout_sec_, read_timeout_usec_) > 0;
}

bool SocketStream::is_writable() const {
  return select_write(sock_, write_timeout_sec_, write_timeout_usec_) > 0 &&
         is_socket_alive(sock_);
}

ssize_t SocketStream::read(char *ptr, size_t size) {
#ifdef _WIN32
  size =
      (std::min)(size, static_cast<size_t>((std::numeric_limits<int>::max)()));
#else
  size = (std::min)(size,
                    static_cast<size_t>((std::numeric_limits<ssize_t>::max)()));
#endif

  if (read_buff_off_ < read_buff_content_size_) {
    auto remaining_size = read_buff_content_size_ - read_buff_off_;
    if (size <= remaining_size) {
      memcpy(ptr, read_buff_.data() + read_buff_off_, size);
      read_buff_off_ += size;
      return static_cast<ssize_t>(size);
    } else {
      memcpy(ptr, read_buff_.data() + read_buff_off_, remaining_size);
      read_buff_off_ += remaining_size;
      return static_cast<ssize_t>(remaining_size);
    }
  }

  if (!is_readable()) { return -1; }

  read_buff_off_ = 0;
  read_buff_content_size_ = 0;

  if (size < read_buff_size_) {
    auto n = read_socket(sock_, read_buff_.data(), read_buff_size_,
                         CPPHTTPLIB_RECV_FLAGS);
    if (n <= 0) {
      return n;
    } else if (n <= static_cast<ssize_t>(size)) {
      memcpy(ptr, read_buff_.data(), static_cast<size_t>(n));
      return n;
    } else {
      memcpy(ptr, read_buff_.data(), size);
      read_buff_off_ = size;
      read_buff_content_size_ = static_cast<size_t>(n);
      return static_cast<ssize_t>(size);
    }
  } else {
    return read_socket(sock_, ptr, size, CPPHTTPLIB_RECV_FLAGS);
  }
}

ssize_t SocketStream::write(const char *ptr, size_t size) {
  if (!is_writable()) { return -1; }

#if defined(_WIN32) && !defined(_WIN64)
  size =
      (std::min)(size, static_cast<size_t>((std::numeric_limits<int>::max)()));
#endif

  return send_socket(sock_, ptr, size, CPPHTTPLIB_SEND_FLAGS);
}

void SocketStream::get_remote_ip_and_port(std::string &ip,
                                                 int &port) const {
  return detail::get_remote_ip_and_port(sock_, ip, port);
}

void SocketStream::get_local_ip_and_port(std::string &ip,
                                                int &port) const {
  return detail::get_local_ip_and_port(sock_, ip, port);
}

socket_t SocketStream::socket() const { return sock_; }

// Buffer stream implementation
bool BufferStream::is_readable() const { return true; }

bool BufferStream::is_writable() const { return true; }

ssize_t BufferStream::read(char *ptr, size_t size) {
#if defined(_MSC_VER) && _MSC_VER < 1910
  auto len_read = buffer._Copy_s(ptr, size, size, position);
#else
  auto len_read = buffer.copy(ptr, size, position);
#endif
  position += static_cast<size_t>(len_read);
  return static_cast<ssize_t>(len_read);
}

ssize_t BufferStream::write(const char *ptr, size_t size) {
  buffer.append(ptr, size);
  return static_cast<ssize_t>(size);
}

void BufferStream::get_remote_ip_and_port(std::string & /*ip*/,
                                                 int & /*port*/) const {}

void BufferStream::get_local_ip_and_port(std::string & /*ip*/,
                                                int & /*port*/) const {}

socket_t BufferStream::socket() const { return 0; }

const std::string &BufferStream::get_buffer() const { return buffer; }

PathParamsMatcher::PathParamsMatcher(const std::string &pattern) {
  // One past the last ending position of a path param substring
  std::size_t last_param_end = 0;

#ifndef CPPHTTPLIB_NO_EXCEPTIONS
  // Needed to ensure that parameter names are unique during matcher
  // construction
  // If exceptions are disabled, only last duplicate path
  // parameter will be set
  std::unordered_set<std::string> param_name_set;
#endif

  while (true) {
    const auto marker_pos = pattern.find(marker, last_param_end);
    if (marker_pos == std::string::npos) { break; }

    static_fragments_.push_back(
        pattern.substr(last_param_end, marker_pos - last_param_end));

    const auto param_name_start = marker_pos + 1;

    auto sep_pos = pattern.find(separator, param_name_start);
    if (sep_pos == std::string::npos) { sep_pos = pattern.length(); }

    auto param_name =
        pattern.substr(param_name_start, sep_pos - param_name_start);

#ifndef CPPHTTPLIB_NO_EXCEPTIONS
    if (param_name_set.find(param_name) != param_name_set.cend()) {
      std::string msg = "Encountered path parameter '" + param_name +
                        "' multiple times in route pattern '" + pattern + "'.";
      throw std::invalid_argument(msg);
    }
#endif

    param_names_.push_back(std::move(param_name));

    last_param_end = sep_pos + 1;
  }

  if (last_param_end < pattern.length()) {
    static_fragments_.push_back(pattern.substr(last_param_end));
  }
}

bool PathParamsMatcher::match(Request &request) const {
  request.matches = std::smatch();
  request.path_params.clear();
  request.path_params.reserve(param_names_.size());

  // One past the position at which the path matched the pattern last time
  std::size_t starting_pos = 0;
  for (size_t i = 0; i < static_fragments_.size(); ++i) {
    const auto &fragment = static_fragments_[i];

    if (starting_pos + fragment.length() > request.path.length()) {
      return false;
    }

    // Avoid unnecessary allocation by using strncmp instead of substr +
    // comparison
    if (std::strncmp(request.path.c_str() + starting_pos, fragment.c_str(),
                     fragment.length()) != 0) {
      return false;
    }

    starting_pos += fragment.length();

    // Should only happen when we have a static fragment after a param
    // Example: '/users/:id/subscriptions'
    // The 'subscriptions' fragment here does not have a corresponding param
    if (i >= param_names_.size()) { continue; }

    auto sep_pos = request.path.find(separator, starting_pos);
    if (sep_pos == std::string::npos) { sep_pos = request.path.length(); }

    const auto &param_name = param_names_[i];

    request.path_params.emplace(
        param_name, request.path.substr(starting_pos, sep_pos - starting_pos));

    // Mark everythin up to '/' as matched
    starting_pos = sep_pos + 1;
  }
  // Returns false if the path is longer than the pattern
  return starting_pos >= request.path.length();
}

bool RegexMatcher::match(Request &request) const {
  request.path_params.clear();
  return std::regex_match(request.path, request.matches, regex_);
}

} // namespace detail


/*
 * SSL Implementation
 */
#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
namespace detail {

template <typename U, typename V>
SSL *ssl_new(socket_t sock, SSL_CTX *ctx, std::mutex &ctx_mutex,
                    U SSL_connect_or_accept, V setup) {
  SSL *ssl = nullptr;
  {
    std::lock_guard<std::mutex> guard(ctx_mutex);
    ssl = SSL_new(ctx);
  }

  if (ssl) {
    set_nonblocking(sock, true);
    auto bio = BIO_new_socket(static_cast<int>(sock), BIO_NOCLOSE);
    BIO_set_nbio(bio, 1);
    SSL_set_bio(ssl, bio, bio);

    if (!setup(ssl) || SSL_connect_or_accept(ssl) != 1) {
      SSL_shutdown(ssl);
      {
        std::lock_guard<std::mutex> guard(ctx_mutex);
        SSL_free(ssl);
      }
      set_nonblocking(sock, false);
      return nullptr;
    }
    BIO_set_nbio(bio, 0);
    set_nonblocking(sock, false);
  }

  return ssl;
}

void ssl_delete(std::mutex &ctx_mutex, SSL *ssl,
                       bool shutdown_gracefully) {
  // sometimes we may want to skip this to try to avoid SIGPIPE if we know
  // the remote has closed the network connection
  // Note that it is not always possible to avoid SIGPIPE, this is merely a
  // best-efforts.
  if (shutdown_gracefully) { SSL_shutdown(ssl); }

  std::lock_guard<std::mutex> guard(ctx_mutex);
  SSL_free(ssl);
}

template <typename U>
bool ssl_connect_or_accept_nonblocking(socket_t sock, SSL *ssl,
                                       U ssl_connect_or_accept,
                                       time_t timeout_sec,
                                       time_t timeout_usec) {
  auto res = 0;
  while ((res = ssl_connect_or_accept(ssl)) != 1) {
    auto err = SSL_get_error(ssl, res);
    switch (err) {
    case SSL_ERROR_WANT_READ:
      if (select_read(sock, timeout_sec, timeout_usec) > 0) { continue; }
      break;
    case SSL_ERROR_WANT_WRITE:
      if (select_write(sock, timeout_sec, timeout_usec) > 0) { continue; }
      break;
    default: break;
    }
    return false;
  }
  return true;
}

template <typename T>
bool process_server_socket_ssl(
    const std::atomic<socket_t> &svr_sock, SSL *ssl, socket_t sock,
    size_t keep_alive_max_count, time_t keep_alive_timeout_sec,
    time_t read_timeout_sec, time_t read_timeout_usec, time_t write_timeout_sec,
    time_t write_timeout_usec, T callback) {
  return process_server_socket_core(
      svr_sock, sock, keep_alive_max_count, keep_alive_timeout_sec,
      [&](bool close_connection, bool &connection_closed) {
        SSLSocketStream strm(sock, ssl, read_timeout_sec, read_timeout_usec,
                             write_timeout_sec, write_timeout_usec);
        return callback(strm, close_connection, connection_closed);
      });
}

template <typename T>
bool
process_client_socket_ssl(SSL *ssl, socket_t sock, time_t read_timeout_sec,
                          time_t read_timeout_usec, time_t write_timeout_sec,
                          time_t write_timeout_usec, T callback) {
  SSLSocketStream strm(sock, ssl, read_timeout_sec, read_timeout_usec,
                       write_timeout_sec, write_timeout_usec);
  return callback(strm);
}

class SSLInit {
public:
  SSLInit() {
    OPENSSL_init_ssl(
        OPENSSL_INIT_LOAD_SSL_STRINGS | OPENSSL_INIT_LOAD_CRYPTO_STRINGS, NULL);
  }
};

// SSL socket stream implementation
SSLSocketStream::SSLSocketStream(socket_t sock, SSL *ssl,
                                        time_t read_timeout_sec,
                                        time_t read_timeout_usec,
                                        time_t write_timeout_sec,
                                        time_t write_timeout_usec)
    : sock_(sock), ssl_(ssl), read_timeout_sec_(read_timeout_sec),
      read_timeout_usec_(read_timeout_usec),
      write_timeout_sec_(write_timeout_sec),
      write_timeout_usec_(write_timeout_usec) {
  SSL_clear_mode(ssl, SSL_MODE_AUTO_RETRY);
}

SSLSocketStream::~SSLSocketStream() = default;

bool SSLSocketStream::is_readable() const {
  return detail::select_read(sock_, read_timeout_sec_, read_timeout_usec_) > 0;
}

bool SSLSocketStream::is_writable() const {
  return select_write(sock_, write_timeout_sec_, write_timeout_usec_) > 0 &&
         is_socket_alive(sock_);
}

ssize_t SSLSocketStream::read(char *ptr, size_t size) {
  if (SSL_pending(ssl_) > 0) {
    return SSL_read(ssl_, ptr, static_cast<int>(size));
  } else if (is_readable()) {
    auto ret = SSL_read(ssl_, ptr, static_cast<int>(size));
    if (ret < 0) {
      auto err = SSL_get_error(ssl_, ret);
      auto n = 1000;
#ifdef _WIN32
      while (--n >= 0 && (err == SSL_ERROR_WANT_READ ||
                          (err == SSL_ERROR_SYSCALL &&
                           WSAGetLastError() == WSAETIMEDOUT))) {
#else
      while (--n >= 0 && err == SSL_ERROR_WANT_READ) {
#endif
        if (SSL_pending(ssl_) > 0) {
          return SSL_read(ssl_, ptr, static_cast<int>(size));
        } else if (is_readable()) {
          std::this_thread::sleep_for(std::chrono::milliseconds(1));
          ret = SSL_read(ssl_, ptr, static_cast<int>(size));
          if (ret >= 0) { return ret; }
          err = SSL_get_error(ssl_, ret);
        } else {
          return -1;
        }
      }
    }
    return ret;
  }
  return -1;
}

ssize_t SSLSocketStream::write(const char *ptr, size_t size) {
  if (is_writable()) {
    auto handle_size = static_cast<int>(
        std::min<size_t>(size, (std::numeric_limits<int>::max)()));

    auto ret = SSL_write(ssl_, ptr, static_cast<int>(handle_size));
    if (ret < 0) {
      auto err = SSL_get_error(ssl_, ret);
      auto n = 1000;
#ifdef _WIN32
      while (--n >= 0 && (err == SSL_ERROR_WANT_WRITE ||
                          (err == SSL_ERROR_SYSCALL &&
                           WSAGetLastError() == WSAETIMEDOUT))) {
#else
      while (--n >= 0 && err == SSL_ERROR_WANT_WRITE) {
#endif
        if (is_writable()) {
          std::this_thread::sleep_for(std::chrono::milliseconds(1));
          ret = SSL_write(ssl_, ptr, static_cast<int>(handle_size));
          if (ret >= 0) { return ret; }
          err = SSL_get_error(ssl_, ret);
        } else {
          return -1;
        }
      }
    }
    return ret;
  }
  return -1;
}

void SSLSocketStream::get_remote_ip_and_port(std::string &ip,
                                                    int &port) const {
  detail::get_remote_ip_and_port(sock_, ip, port);
}

void SSLSocketStream::get_local_ip_and_port(std::string &ip,
                                                   int &port) const {
  detail::get_local_ip_and_port(sock_, ip, port);
}

socket_t SSLSocketStream::socket() const { return sock_; }

static SSLInit sslinit_;

} // namespace detail

// SSL HTTP server implementation
SSLServer::SSLServer(const char *cert_path, const char *private_key_path,
                            const char *client_ca_cert_file_path,
                            const char *client_ca_cert_dir_path,
                            const char *private_key_password) {
  ctx_ = SSL_CTX_new(TLS_server_method());

  if (ctx_) {
    SSL_CTX_set_options(ctx_,
                        SSL_OP_NO_COMPRESSION |
                            SSL_OP_NO_SESSION_RESUMPTION_ON_RENEGOTIATION);

    SSL_CTX_set_min_proto_version(ctx_, TLS1_1_VERSION);

    if (private_key_password != nullptr && (private_key_password[0] != '\0')) {
      SSL_CTX_set_default_passwd_cb_userdata(
          ctx_,
          reinterpret_cast<void *>(const_cast<char *>(private_key_password)));
    }

    if (SSL_CTX_use_certificate_chain_file(ctx_, cert_path) != 1 ||
        SSL_CTX_use_PrivateKey_file(ctx_, private_key_path, SSL_FILETYPE_PEM) !=
            1) {
      SSL_CTX_free(ctx_);
      ctx_ = nullptr;
    } else if (client_ca_cert_file_path || client_ca_cert_dir_path) {
      SSL_CTX_load_verify_locations(ctx_, client_ca_cert_file_path,
                                    client_ca_cert_dir_path);

      SSL_CTX_set_verify(
          ctx_, SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT, nullptr);
    }
  }
}

SSLServer::SSLServer(X509 *cert, EVP_PKEY *private_key,
                            X509_STORE *client_ca_cert_store) {
  ctx_ = SSL_CTX_new(TLS_server_method());

  if (ctx_) {
    SSL_CTX_set_options(ctx_,
                        SSL_OP_NO_COMPRESSION |
                            SSL_OP_NO_SESSION_RESUMPTION_ON_RENEGOTIATION);

    SSL_CTX_set_min_proto_version(ctx_, TLS1_1_VERSION);

    if (SSL_CTX_use_certificate(ctx_, cert) != 1 ||
        SSL_CTX_use_PrivateKey(ctx_, private_key) != 1) {
      SSL_CTX_free(ctx_);
      ctx_ = nullptr;
    } else if (client_ca_cert_store) {
      SSL_CTX_set_cert_store(ctx_, client_ca_cert_store);

      SSL_CTX_set_verify(
          ctx_, SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT, nullptr);
    }
  }
}

SSLServer::SSLServer(
    const std::function<bool(SSL_CTX &ssl_ctx)> &setup_ssl_ctx_callback) {
  ctx_ = SSL_CTX_new(TLS_method());
  if (ctx_) {
    if (!setup_ssl_ctx_callback(*ctx_)) {
      SSL_CTX_free(ctx_);
      ctx_ = nullptr;
    }
  }
}

SSLServer::~SSLServer() {
  if (ctx_) { SSL_CTX_free(ctx_); }
}

bool SSLServer::is_valid() const { return ctx_; }

SSL_CTX *SSLServer::ssl_context() const { return ctx_; }

bool SSLServer::process_and_close_socket(socket_t sock) {
  auto ssl = detail::ssl_new(
      sock, ctx_, ctx_mutex_,
      [&](SSL *ssl2) {
        return detail::ssl_connect_or_accept_nonblocking(
            sock, ssl2, SSL_accept, read_timeout_sec_, read_timeout_usec_);
      },
      [](SSL * /*ssl2*/) { return true; });

  auto ret = false;
  if (ssl) {
    ret = detail::process_server_socket_ssl(
        svr_sock_, ssl, sock, keep_alive_max_count_, keep_alive_timeout_sec_,
        read_timeout_sec_, read_timeout_usec_, write_timeout_sec_,
        write_timeout_usec_,
        [this, ssl](Stream &strm, bool close_connection,
                    bool &connection_closed) {
          return process_request(strm, close_connection, connection_closed,
                                 [&](Request &req) { req.ssl = ssl; });
        });

    // Shutdown gracefully if the result seemed successful, non-gracefully if
    // the connection appeared to be closed.
    const bool shutdown_gracefully = ret;
    detail::ssl_delete(ctx_mutex_, ssl, shutdown_gracefully);
  }

  detail::shutdown_socket(sock);
  detail::close_socket(sock);
  return ret;
}


} // namespace httplib
