/*
 *
 * Copyright 2021-2023 Software Radio Systems Limited
 *
 * By using this file, you agree to the terms and conditions set
 * forth in the LICENSE file which can be found at the top level of
 * the distribution.
 *
 */

#pragma once

#include "srsran/du_high/du_high_configuration.h"
#include "srsran/du_manager/du_manager.h"
#include "srsran/f1ap/du/f1ap_du.h"
#include "srsran/mac/mac.h"
#include "srsran/scheduler/scheduler_metrics.h"
#include <memory>

namespace srsran {
namespace srs_du {

class du_high
{
public:
  explicit du_high(const du_high_configuration& cfg_);
  ~du_high();

  void start();
  void stop();

  f1ap_message_handler& get_f1ap_message_handler();

  mac_pdu_handler& get_pdu_handler();

  mac_cell_slot_handler& get_slot_handler(du_cell_index_t cell_index);

  mac_cell_rach_handler& get_rach_handler(du_cell_index_t cell_index);

  mac_cell_control_information_handler& get_control_information_handler(du_cell_index_t cell_index);

private:
  class layer_connector;

  du_high_configuration cfg;

  timer_manager& timers;

  // Connection between DU-high layers.
  std::unique_ptr<layer_connector> adapters;

  std::unique_ptr<scheduler_ue_metrics_notifier> metrics_notifier;

  // DU-high Layers.
  std::unique_ptr<du_manager_interface> du_manager;
  std::unique_ptr<f1ap_du>              f1ap;
  std::unique_ptr<mac_interface>        mac;

  std::unique_ptr<mac_cell_slot_handler> main_cell_slot_handler;
};

} // namespace srs_du
} // namespace srsran
