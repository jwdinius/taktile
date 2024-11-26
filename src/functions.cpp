
//#include <boost/url.hpp>
#include <Poco/URI.h>

#include "taktile/functions.hpp"

namespace taktile {
url parse_url(std::string const & inp) {
  // Parse the URL
  Poco::URI uri{inp};
  return url{uri.getScheme(), uri.getHost(), uri.getPort()};
}
} // namespace taktile
