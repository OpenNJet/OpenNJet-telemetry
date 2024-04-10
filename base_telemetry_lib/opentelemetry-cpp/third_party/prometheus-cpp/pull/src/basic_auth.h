#pragma once

#include <functional>
#include <string>
#include <unordered_map>

#include "CivetServer.h"
#include "prometheus/detail/pull_export.h"

namespace prometheus {

/**
 * Handler for HTTP Basic authentication for Endpoints.
 */
class PROMETHEUS_CPP_PULL_EXPORT BasicAuthHandler : public CivetAuthHandler {
 public:
  using AuthFunc = std::function<bool(const std::string&, const std::string&)>;
  explicit BasicAuthHandler(AuthFunc callback, std::string realm);

  /**
   * Implements civetweb authorization interface.
   *
   * Attempts to extract a username and password from the Authorization header
   * to pass to the owning AuthHandler, `this->handler`.
   * If handler returns true, permits the request to proceed.
   * If handler returns false, or the Auth header is absent,
   * rejects the request with 401 Unauthorized.
   */
  bool authorize(CivetServer* server, mg_connection* conn) override;

 private:
  bool AuthorizeInner(CivetServer* server, mg_connection* conn);
  void WriteUnauthorizedResponse(mg_connection* conn);

  AuthFunc callback_;
  std::string realm_;
};

}  // namespace prometheus
