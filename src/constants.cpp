#include <boost/asio.hpp>

#include "taktile/constants.hpp"

namespace taktile {

  const std::string DEFAULT_HOST_ID = "taktile@" + boost::asio::ip::host_name();

} // namespace taktile
