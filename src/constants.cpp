// Copyright (c) 2025, Joe Dinius, Ph.D.
// SPDX-License-Identifier: Apache-2.0
#include "taktile/constants.hpp"

#include <boost/asio.hpp>
#include <string>

namespace taktile {

// NOLINTBEGIN[runtime/string]
const std::string DEFAULT_HOST_ID = "taktile@" + boost::asio::ip::host_name();
// NOLINTEND[runtime/string]

}  // namespace taktile
