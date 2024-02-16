/*
 *
 * Copyright 2021-2024 Software Radio Systems Limited
 *
 * By using this file, you agree to the terms and conditions set
 * forth in the LICENSE file which can be found at the top level of
 * the distribution.
 *
 */

#include "scheduler_policy_factory.h"
#include "scheduler_time_rr.h"

using namespace srsran;

std::unique_ptr<scheduler_policy> srsran::create_scheduler_strategy(const scheduler_strategy_params&  params,
                                                                    const scheduler_ue_expert_config& expert_cfg_)
{
  return std::make_unique<scheduler_time_rr>(expert_cfg_);
}
