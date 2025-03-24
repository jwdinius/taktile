// Copyright (c) 2025, Joe Dinius, Ph.D.
// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

#include <simpleio/message.hpp>
#include <simpleio/messages/xml.hpp>
#include "taktile/constants.hpp"

namespace taktile {

void init_logger();

/// @brief A URL structure - see RFC-1808
///        https://datatracker.ietf.org/doc/html/rfc1808.html
/// @note The default constructor initializes the URL object as DEFAULT_COT_URL.
class URL {
 public:
  Scheme scheme{Scheme::UDP_WRITE_ONLY};
  std::string net_loc{DEFAULT_IPV4_ADDRESS};
  uint16_t port{DEFAULT_BROADCAST_PORT};

  URL() = default; 
  ~URL() = default;
  URL(URL const &) = default;
  URL(URL &&) = default;
  URL &operator=(URL const &) = default;
  URL &operator=(URL &&) = default;

  /// @brief Construct a URL object with the given URL string
  /// @param url
  /// @throws std::invalid_argument if the scheme is not in SCHEME
  explicit URL(std::string const &url);

  /// @brief Construct a URL object with the given scheme, net_loc, and port
  /// @param _scheme
  /// @param _net_loc
  /// @param _port
  URL(Scheme const &_scheme, std::string const &_net_loc, uint16_t _port);

 private:
  /// @brief Parse a string into a URL structure
  /// @param inp
  /// @return url
  /// @throws std::invalid_argument if the scheme is not in SCHEME
  URL parse_url(std::string const &inp);
};

/// @brief Cursor-on-Target (CoT) message structure
struct CotType {
  double lat{0.0};
  double lon{0.0};
  double ce{DEFAULT_COT_VAL};
  double hae{DEFAULT_COT_VAL};
  double le{DEFAULT_COT_VAL};
  std::string uid{DEFAULT_HOST_ID};
  uint32_t stale{DEFAULT_COT_STALE};
  std::string cot_type{DEFAULT_COT_TYPE};

  CotType() = default;
  ~CotType() = default;

  CotType(CotType const &) = default;
  CotType(CotType &&) = default;
  CotType &operator=(CotType const &) = default;
  CotType &operator=(CotType &&) = default;

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
  explicit CotType(double _lat, double _lon, double _ce, double _hae,
                      double _le, std::string _uid, uint32_t _stale,
                      std::string _cot_type);

  explicit CotType(std::string _uid);

  /// @brief Get the current time in W3C XML datetime format
  /// @details Comparable to cot_time
  /// @param stale_time time in seconds before the message is considered stale
  /// (optional)
  /// @return
  static std::string get_time(std::optional<int32_t> stale_time = std::nullopt);
};

class CotXml : public CotType {
  CotXml() = default;
  ~CotXml() override = default;

  CotXml(CotXml const &) = default;
  CotXml(CotXml &&) = default;
  CotXml &operator=(CotXml const &) = default;
  CotXml &operator=(CotXml &&) = default;

  // @throws std::invalid_argument if the XML message is invalid
  explicit CotXml(simpleio::messages::XmlMessageType const& _xml);

  explicit CotXml(CotType const& cot);

private:
  simpleio::messages::XmlMessageType xml_;
};

class CotSerializer : public simpleio::SerializationStrategy<CotType> {
 public:
  ~CotSerializer() override = default;

  std::vector<std::byte> serialize(CotType const &entity) override = 0;

  CotType deserialize(std::vector<std::byte> const &_blob) override = 0;
};

class CotXmlSerializer : public CotSerializer {
 public:
  explicit CotXmlSerializer(std::shared_ptr<simpleio::messages::XmlSerializer> strategy)
      : strategy_(std::move(strategy)) {}
  std::vector<std::byte> serialize(CotXml const &entity) override;

  CotXml deserialize(std::vector<std::byte> const &_blob) override;

private:
  std::shared_ptr<simpleio::messages::XmlSerializer> strategy_;
};


/// @brief Represent CotType as an XML message
template <size_t N>
class CotMessage : public simpleio::Message<CotType, N> {
 public:

  CotMessage() = delete;

  ~CotMessage() override = default;

  explicit CotMessage(CotType&& cot_type,
                      std::shared_ptr<CotSerializer> strategy)
      : simpleio::Message<CotType, N>(std::move(cot_type), strategy) {}

  explicit CotMessage(std::vector<std::byte>&& _blob,
                      std::shared_ptr<CotSerializer> strategy)
      : simpleio::Message<CotType, N>(std::move(_blob), strategy) {}
};

CotType hello_event(std::optional<std::string> uid);

}  // namespace taktile
