//
//  httplib.h
//
//  Copyright (c) 2024 Yuji Hirose. All rights reserved.
//  MIT License
//


class SSLServer : public Server {
public:
  SSLServer(const char *cert_path, const char *private_key_path,
            const char *client_ca_cert_file_path = nullptr,
            const char *client_ca_cert_dir_path = nullptr,
            const char *private_key_password = nullptr);

  SSLServer(X509 *cert, EVP_PKEY *private_key,
            X509_STORE *client_ca_cert_store = nullptr);

  SSLServer(
      const std::function<bool(SSL_CTX &ssl_ctx)> &setup_ssl_ctx_callback);

  ~SSLServer() override;

  bool is_valid() const override;

  SSL_CTX *ssl_context() const;

private:
  bool process_and_close_socket(socket_t sock) override;

  SSL_CTX *ctx_;
  std::mutex ctx_mutex_;
};


#endif // CPPHTTPLIB_HTTPLIB_H
