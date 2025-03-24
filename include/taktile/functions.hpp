// Copyright (c) 2025, Joe Dinius, Ph.D.
// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <cstdint>
#include <memory>
#include <optional>
#include <simpleio/message.hpp>
#include <simpleio/messages/xml.hpp>
#include <string>
#include <utility>
#include <vector>

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
  static URL parse_url(std::string const &inp);
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

  explicit CotType(std::string _uid);

  /// @brief Get the current time in W3C XML datetime format
  /// @details Comparable to cot_time
  /// @param cot_stale time in seconds before the message is considered stale
  /// (optional)
  /// @return
  static std::string get_time(std::optional<int32_t> cot_stale = std::nullopt);

  static void validate(CotType const &cot);
};

class Cot2Xml {
 public:
  /// @brief Convert a CoT message to an XML message
  /// @param cot
  /// @return xml
  static simpleio::messages::XmlMessageType convert(CotType const &cot);

  static CotType convert(simpleio::messages::XmlMessageType const &xml);
};

class CotSerializer : public simpleio::SerializationStrategy<CotType> {
 public:
  std::vector<std::byte> serialize(CotType const &cot) override = 0;

  CotType deserialize(std::vector<std::byte> const &xml) override = 0;
};

class CotXmlSerializer : public CotSerializer {
 public:
  explicit CotXmlSerializer(
      std::shared_ptr<simpleio::messages::XmlSerializer> strategy);

  std::vector<std::byte> serialize(CotType const &entity) override;

  CotType deserialize(std::vector<std::byte> const &_blog) override;

 private:
  std::shared_ptr<simpleio::messages::XmlSerializer> xml_serializer_;
};

template <size_t N>
class CotMessage : public simpleio::Message<CotType, N> {
 public:
  CotMessage() = delete;

  ~CotMessage() override = default;

  explicit CotMessage(CotType &&cot_type,
                      std::shared_ptr<CotSerializer> strategy)
      : simpleio::Message<CotType, N>(std::move(cot_type), strategy) {}

  explicit CotMessage(std::vector<std::byte> &&_blob,
                      std::shared_ptr<CotSerializer> strategy)
      : simpleio::Message<CotType, N>(std::move(_blob), strategy) {}
};

CotType hello_event(std::optional<std::string> const &uid);

}  // namespace taktile
