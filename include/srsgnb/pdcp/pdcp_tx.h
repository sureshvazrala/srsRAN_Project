/*
 *
 * Copyright 2013-2022 Software Radio Systems Limited
 *
 * By using this file, you agree to the terms and conditions set
 * forth in the LICENSE file which can be found at the top level of
 * the distribution.
 *
 */

#pragma once

#include "srsgnb/adt/byte_buffer.h"
#include "srsgnb/pdcp/pdcp_config.h"
#include "srsgnb/security/security.h"

/*
 * This file will hold the interfaces and notifiers for the PDCP entity.
 * They follow the following nomenclature:
 *
 *   pdcp_{tx/rx}_{lower/upper}_{[control/data]}_{interface/notifier}
 *
 * 1. TX/RX indicates whether the interface is intended for the
 *    TX or RX side of the entity
 * 2. Lower/Upper indicates whether the interface/notifier interacts
 *    with the upper or lower layers.
 * 3. Control/Data: indicates whether this interface is necessary for "control"
 *    purposes (e.g., notifying the RRC of an integrity failure, or that we are
 *    near max HFN) or "data" purposes (e.g. handling SDUs).
 *    This distinction is only necessary when interfacing with the upper layers,
 *    and as such, we omit it in the interfaces with the lower layers.
 * 4. Interface/Notifier: whether this is an interface the PDCP entity will
 *    inherit or if a notifier that the PDCP will keep as a member.
 *
 */
namespace srsgnb {

/// This interface represents the data exit point of the transmitting side of a PDCP entity.
/// The PDCP will push PDUs to the lower layers using this interface.
/// The PDCP will also use this interface to order the lower layer to discard PDUs if necessary.
class pdcp_tx_lower_notifier
{
public:
  pdcp_tx_lower_notifier()                                          = default;
  virtual ~pdcp_tx_lower_notifier()                                 = default;
  pdcp_tx_lower_notifier(const pdcp_tx_lower_notifier&)             = delete;
  pdcp_tx_lower_notifier& operator=(const pdcp_tx_lower_notifier&)  = delete;
  pdcp_tx_lower_notifier(const pdcp_tx_lower_notifier&&)            = delete;
  pdcp_tx_lower_notifier& operator=(const pdcp_tx_lower_notifier&&) = delete;

  virtual void on_new_pdu(byte_buffer pdu)    = 0; ///< Pass PDU to the lower layers.
  virtual void on_discard_pdu(uint32_t count) = 0; ///< Order lower layers to discard PDU
};

/// This interface represents the interface through which
/// the lower layers will notify the PDCP transmitting entity
/// of relevant events, namely that it can stop the discard timer.
/// On RLC AM, this is done when a PDCP SDU is ACK'ed, on UM
/// it is when transmission of a PDCP SDU begins.
class pdcp_tx_lower_interface
{
public:
  pdcp_tx_lower_interface()                                           = default;
  virtual ~pdcp_tx_lower_interface()                                  = default;
  pdcp_tx_lower_interface(const pdcp_tx_lower_interface&)             = delete;
  pdcp_tx_lower_interface& operator=(const pdcp_tx_lower_interface&)  = delete;
  pdcp_tx_lower_interface(const pdcp_tx_lower_interface&&)            = delete;
  pdcp_tx_lower_interface& operator=(const pdcp_tx_lower_interface&&) = delete;

  virtual void stop_discard_timer(uint32_t count) = 0;
};

/// This interface represents the data entry point of the transmitting side of a PDCP entity.
/// The upper-layers will use this call to pass PDUs into the TX entity.
class pdcp_tx_upper_data_interface
{
public:
  pdcp_tx_upper_data_interface()                                                = default;
  virtual ~pdcp_tx_upper_data_interface()                                       = default;
  pdcp_tx_upper_data_interface(const pdcp_tx_upper_data_interface&)             = delete;
  pdcp_tx_upper_data_interface& operator=(const pdcp_tx_upper_data_interface&)  = delete;
  pdcp_tx_upper_data_interface(const pdcp_tx_upper_data_interface&&)            = delete;
  pdcp_tx_upper_data_interface& operator=(const pdcp_tx_upper_data_interface&&) = delete;

  /// Handle the incoming SDU.
  virtual void handle_sdu(byte_buffer sdu) = 0;
};

/// This interface represents the control SAP of the transmitting side of a PDCP entity.
/// The RRC will use this interface to configure security keys and enable/disable
/// integrity and ciphering.
class pdcp_tx_upper_control_interface
{
public:
  pdcp_tx_upper_control_interface()                                                   = default;
  virtual ~pdcp_tx_upper_control_interface()                                          = default;
  pdcp_tx_upper_control_interface(const pdcp_tx_upper_control_interface&)             = delete;
  pdcp_tx_upper_control_interface& operator=(const pdcp_tx_upper_control_interface&)  = delete;
  pdcp_tx_upper_control_interface(const pdcp_tx_upper_control_interface&&)            = delete;
  pdcp_tx_upper_control_interface& operator=(const pdcp_tx_upper_control_interface&&) = delete;

  /// Handle the incoming SDU.
  virtual void set_as_security_config(sec_128_as_config sec_cfg)                                       = 0;
  virtual void enable_or_disable_security(pdcp_integrity_enabled integ, pdcp_ciphering_enabled cipher) = 0;
};

/// This interface represents the control upper layer that the
/// TX PDCP bearer must notify in case of reaching max HFN,
/// so that keys can be re-negotiated. Other protocol failures
/// will also be notified through this interface.
class pdcp_tx_upper_control_notifier
{
public:
  pdcp_tx_upper_control_notifier()                                                  = default;
  virtual ~pdcp_tx_upper_control_notifier()                                         = default;
  pdcp_tx_upper_control_notifier(const pdcp_tx_upper_control_notifier&)             = delete;
  pdcp_tx_upper_control_notifier& operator=(const pdcp_tx_upper_control_notifier&)  = delete;
  pdcp_tx_upper_control_notifier(const pdcp_tx_upper_control_notifier&&)            = delete;
  pdcp_tx_upper_control_notifier& operator=(const pdcp_tx_upper_control_notifier&&) = delete;

  virtual void on_protocol_failure() = 0;
  virtual void on_max_hfn_reached()  = 0;
};

/*
 * PDCP TX metrics interface and structures.
 */
/// This struct will hold relevant metrics for the PDCP TX
struct pdcp_tx_metrics_container {
  uint32_t num_sdus;
  uint32_t num_sdu_bytes;
  uint32_t num_pdus;
  uint32_t num_pdu_bytes;
  uint32_t num_discard_timeouts;
};

/// This interface will allow the relevant entities to query PDCP RX
/// metrics (and possibly reset them).
class pdcp_tx_metrics_interface
{
public:
  pdcp_tx_metrics_interface()                                                   = default;
  virtual ~pdcp_tx_metrics_interface()                                          = default;
  pdcp_tx_metrics_interface(const pdcp_tx_upper_control_interface&)             = delete;
  pdcp_tx_metrics_interface& operator=(const pdcp_tx_upper_control_interface&)  = delete;
  pdcp_tx_metrics_interface(const pdcp_tx_upper_control_interface&&)            = delete;
  pdcp_tx_metrics_interface& operator=(const pdcp_tx_upper_control_interface&&) = delete;

  /// Metrics interfaces.
  virtual pdcp_tx_metrics_container get_metrics()           = 0;
  virtual pdcp_tx_metrics_container get_metrics_and_reset() = 0;
  virtual void                      reset_metrics()         = 0;
};

} // namespace srsgnb

namespace fmt {
// PDCP RX metrics formatter
template <>
struct formatter<srsgnb::pdcp_tx_metrics_container> {
  template <typename ParseContext>
  auto parse(ParseContext& ctx) -> decltype(ctx.begin())
  {
    return ctx.begin();
  }

  template <typename FormatContext>
  auto format(srsgnb::pdcp_tx_metrics_container m, FormatContext& ctx) -> decltype(std::declval<FormatContext>().out())
  {
    return format_to(ctx.out(),
                     "num_sdus={}, num_sdu_bytes={}, num_pdus={}, num_pdu_bytes={}, num_discard_timeouts={}",
                     m.num_sdus,
                     m.num_sdu_bytes,
                     m.num_pdus,
                     m.num_pdu_bytes,
                     m.num_discard_timeouts);
  }
};
} // namespace fmt

