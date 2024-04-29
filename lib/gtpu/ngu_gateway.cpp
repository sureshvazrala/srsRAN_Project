/*
 *
 * Copyright 2021-2024 Software Radio Systems Limited
 *
 * By using this file, you agree to the terms and conditions set
 * forth in the LICENSE file which can be found at the top level of
 * the distribution.
 *
 */

#include "srsran/gtpu/ngu_gateway.h"
#include "srsran/gateways/udp_network_gateway_factory.h"
#include "srsran/support/io/io_broker.h"

using namespace srsran;
using namespace srs_cu_up;

namespace {

/// Implementation of a NG-U TNL PDU session that uses a UDP connection.
class udp_ngu_tnl_session final : public ngu_tnl_pdu_session
{
  // private ctor.
  udp_ngu_tnl_session(network_gateway_data_notifier_with_src_addr& data_notifier_, io_broker& io_brk_) :
    io_brk(io_brk_), data_notifier(data_notifier_), logger(srslog::fetch_basic_logger("GTPU"))
  {
  }

public:
  udp_ngu_tnl_session(udp_ngu_tnl_session&& other) noexcept                 = default;
  udp_ngu_tnl_session(const udp_ngu_tnl_session& other) noexcept            = delete;
  udp_ngu_tnl_session& operator=(const udp_ngu_tnl_session& other) noexcept = delete;
  udp_ngu_tnl_session& operator=(udp_ngu_tnl_session&& other) noexcept      = delete;
  ~udp_ngu_tnl_session() override
  {
    if (udp_gw != nullptr and udp_gw->get_socket_fd() >= 0) {
      // Deregister UDP gateway from IO broker.
      if (not io_brk.unregister_fd(udp_gw->get_socket_fd())) {
        logger.warning("Failed to stop NG-U gateway socket");
      }

      // Closes socket.
      udp_gw.reset();
    }
  }

  static std::unique_ptr<udp_ngu_tnl_session> create(const udp_network_gateway_config&            cfg,
                                                     network_gateway_data_notifier_with_src_addr& data_notifier,
                                                     io_broker&                                   io_brk,
                                                     task_executor&                               io_tx_executor)
  {
    std::unique_ptr<udp_ngu_tnl_session> conn(new udp_ngu_tnl_session(data_notifier, io_brk));

    // Create a new UDP network gateway instance.
    conn->udp_gw = create_udp_network_gateway(udp_network_gateway_creation_message{cfg, *conn, io_tx_executor});

    // Bind/open the gateway, start handling of incoming traffic from UPF, e.g. echo
    if (not conn->udp_gw->create_and_bind()) {
      conn->logger.error("Failed to create and connect NG-U gateway");
    }
    bool success = io_brk.register_fd(
        conn->udp_gw->get_socket_fd(),
        [udp_gw_ptr = conn->udp_gw.get()]() { udp_gw_ptr->receive(); },
        []() { /* TODO */ });
    if (!success) {
      conn->logger.error("Failed to register NG-U (GTP-U) network gateway at IO broker. socket_fd={}",
                         conn->udp_gw->get_socket_fd());
      return nullptr;
    }

    return conn;
  }

  void handle_pdu(byte_buffer pdu, const sockaddr_storage& dest_addr) override
  {
    // Forward PDU to UDP interface.
    udp_gw->handle_pdu(std::move(pdu), dest_addr);
  }

  void on_new_pdu(byte_buffer pdu, const sockaddr_storage& src_addr) override
  {
    // Forward PDU to data notifier.
    data_notifier.on_new_pdu(std::move(pdu), src_addr);
  }

  optional<uint16_t> get_bind_port() override { return udp_gw->get_bind_port(); }

private:
  io_broker&                                   io_brk;
  network_gateway_data_notifier_with_src_addr& data_notifier;
  srslog::basic_logger&                        logger = srslog::fetch_basic_logger("CU-UP");

  std::unique_ptr<udp_network_gateway> udp_gw;
};

/// Implementation of the NG-U gateway for the case a UDP connection is used to a remote UPF.
class udp_ngu_gateway final : public ngu_gateway
{
public:
  udp_ngu_gateway(const udp_network_gateway_config& cfg_, io_broker& io_brk_, task_executor& io_tx_executor_) :
    cfg(cfg_), io_brk(io_brk_), io_tx_executor(io_tx_executor_)
  {
  }

  std::unique_ptr<ngu_tnl_pdu_session> create(network_gateway_data_notifier_with_src_addr& data_notifier) override
  {
    return udp_ngu_tnl_session::create(cfg, data_notifier, io_brk, io_tx_executor);
  }

private:
  const udp_network_gateway_config cfg;
  io_broker&                       io_brk;
  task_executor&                   io_tx_executor;
};

} // namespace

std::unique_ptr<ngu_gateway> srsran::srs_cu_up::create_udp_ngu_gateway(const udp_network_gateway_config& config,
                                                                       io_broker&                        io_brk,
                                                                       task_executor&                    io_tx_executor)
{
  return std::make_unique<udp_ngu_gateway>(config, io_brk, io_tx_executor);
}

/* ---- No Core version ---- */

namespace {

/// Implementation of an NG-U TNL PDU session when a local UPF stub is used.
class no_core_ngu_tnl_pdu_session final : public ngu_tnl_pdu_session
{
public:
  void handle_pdu(byte_buffer pdu, const sockaddr_storage& dest_addr) override
  {
    // Do nothing.
  }

  void on_new_pdu(byte_buffer pdu, const sockaddr_storage& src_addr) override
  {
    // Do nothing.
  }

  optional<uint16_t> get_bind_port() override { return nullopt; }
};

/// Implementation of the NG-U gateway for the case a local UPF stub is used.
class no_core_ngu_gateway : public ngu_gateway
{
public:
  std::unique_ptr<ngu_tnl_pdu_session> create(network_gateway_data_notifier_with_src_addr& data_notifier) override
  {
    return std::make_unique<no_core_ngu_tnl_pdu_session>();
  }
};

} // namespace

std::unique_ptr<ngu_gateway> srsran::srs_cu_up::create_no_core_ngu_gateway()
{
  return std::make_unique<no_core_ngu_gateway>();
}
