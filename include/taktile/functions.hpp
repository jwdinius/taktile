#pragma once

#include <string>
#include <cstdint>

#include "taktile/constants.hpp"

namespace taktile {
  /// @brief A URL structure - see RFC-1808
  ///        https://datatracker.ietf.org/doc/html/rfc1808.html
  /// @note The default constructor initializes the URL object as DEFAULT_COT_URL.
  class URL {
  public:
    Scheme scheme;
    std::string net_loc;
    uint16_t port;

    URL();
    ~URL() = default;
    URL(URL const &) = default;
    URL(URL &&) = default;
    URL & operator=(URL const &) = default;
    URL & operator=(URL &&) = default;    

    /// @brief Construct a URL object with the given URL string
    /// @param url
    /// @throws std::invalid_argument if the scheme is not in SCHEME
    explicit URL(std::string const & url);
    
    /// @brief Construct a URL object with the given scheme, net_loc, and port 
    /// @param _scheme 
    /// @param _net_loc 
    /// @param _port
    URL(Scheme const & _scheme, std::string const & _net_loc, uint16_t _port);
  
  private:
    /// @brief Parse a string into a URL structure
    /// @param inp 
    /// @return url
    /// @throws std::invalid_argument if the scheme is not in SCHEME 
    URL parse_url(std::string const & inp);
  };

  /// @brief Cursor-on-Target (CoT) message structure
  struct CotMessage {
    double lat {0.0};
    double lon {0.0};
    double ce {DEFAULT_COT_VAL};
    double hae {DEFAULT_COT_VAL};
    double le {DEFAULT_COT_VAL};
    std::string uid {DEFAULT_HOST_ID};
    uint32_t stale {DEFAULT_COT_STALE};
    std::string cot_type {DEFAULT_COT_TYPE};

    CotMessage() = default;
    ~CotMessage() = default;

    CotMessage(CotMessage const &) = default;
    CotMessage(CotMessage &&) = default;
    CotMessage & operator=(CotMessage const &) = default;
    CotMessage & operator=(CotMessage &&) = default;
    
    /// @brief Construct a CoT message with the given parameters
    /// @param _lat latitude (-90 to 90 degrees)
    /// @param _lon longitude (-180 to 180 degrees)
    /// @param _ce circular error (>= 0)
    /// @param _hae height above ellipsoid (>= 0)
    /// @param _le linear error (>= 0)
    /// @param _uid unique identifier (non-empty)
    /// @param _stale time in seconds before the message is considered stale
    /// @param _cot_type CoT type (non-empty string)
    /// @throws std::invalid_argument if any parameter is out of range
    explicit CotMessage(double _lat, double _lon, double _ce, double _hae, double _le, std::string _uid, uint32_t _stale, std::string _cot_type);

    explicit CotMessage(std::string _uid);

    /// @brief Convert the CoT message to an XML string
    /// @return XML string 
    std::string as_xml_string() const;

    /// @brief Convert the CoT message to a byte array
    /// @return byte array
    std::vector<uint8_t> as_byte_array() const;

    /// @brief Get the current time in W3C XML datetime format
    /// @details Comparable to cot_time
    /// @param stale_time time in seconds before the message is considered stale (optional)
    /// @return 
    static std::string get_time(std::optional<int32_t> stale_time = std::nullopt);
  };

  CotMessage hello_event(std::optional<std::string> uid);

} // namespace taktile