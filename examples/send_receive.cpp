// Copyright (c) 2025, Joe Dinius, Ph.D.
// SPDX-License-Identifier: Apache-2.0
#include <filesystem>
#include <iostream>
#include <memory>
#include <simpleio/message.hpp>
#include <simpleio/messages/xml.hpp>
#include <simpleio/transports/ip/ip.hpp>
#include <string>
#include <utility>

#include "certs_path.h"  // NOLINT [build/include_subdir]
#include "taktile/taktile.hpp"

/// Send and receive TAK messages (v0/CoT, v1_mesh, v1_stream) over TCP, TLS, or
/// UDP Usage: ./send_receive {address} {port} {tcp|tls|udp}
/// {v0|v1_mesh|v1_stream} E.g., ./send_receive 127.0.0.1 8087 tcp v0
///
/// This example is modeled after the pytak library's "send_receive" example:
/// https://pytak.readthedocs.io/en/latest/examples/#send-receive-tak-data
///
/// A generic TAK event is sent every 5 seconds. The receiver prints any
/// received messages to the console.
namespace sio = simpleio;
namespace siotrnsip = sio::transports::ip;
namespace siomsg = sio::messages;

using XmlSerializer = siomsg::XmlSerializer<>;

taktile::TakData gen_cot() {
  auto tak_msg = atakmap::commoncommo::protobuf::v1::TakMessage();
  auto* cot = tak_msg.mutable_cotevent();
  cot->set_type("a-h-A-M-A");  // insert your type of marker
  cot->set_uid("name_your_marker");
  cot->set_how(taktile::DEFAULT_COT_HOW);
  cot->set_sendtime(taktile::TimeProvider::get_time());
  cot->set_starttime(taktile::TimeProvider::get_time());
  cot->set_staletime(
      taktile::TimeProvider::get_time(taktile::DEFAULT_COT_STALE));
  cot->set_lat(40.781789);  // set your lat (this loc points to Central Park NY)
  cot->set_lon(
      -73.968698);  // set your long (this loc points to Central Park NY)
  cot->set_hae(0);
  cot->set_ce(10);
  cot->set_le(10);
  return taktile::TakData(tak_msg);
}

void receive_cb(taktile::TakData const& tak_data) {
  auto xml_msg =
      std::make_shared<sio::Message<XmlSerializer>>(std::move(tak_data.xml()));
  std::cout << "Received TAK message:" << std::endl;
  std::cout << xml_msg->blob() << std::endl;
}

int main(int argc, char** argv) {
  if (argc < 5) {
    std::cerr << "Usage: ./send_and_receive {address} {port} {tcp|tls|udp} "
                 "{v0|v1_mesh|v1_stream}"
              << std::endl;
    return 1;
  }
  auto address = argv[1];
  auto port = static_cast<uint16_t>(std::stoi(argv[2]));
  auto protocol = std::string(argv[3]);
  auto version = std::string(argv[4]);

  if (version.compare("v0") != 0 && version.compare("v1_mesh") != 0 &&
      version.compare("v1_stream") != 0) {
    std::cerr << "Unknown protocol version: " << version << std::endl;
    return 1;
  }

  auto context = std::make_unique<siotrnsip::Context>();
  auto use_v0_protocol = (version.compare("v0") == 0);

  if (protocol.compare("udp") == 0) {
    if (version.compare("v1_stream") == 0) {
      std::cerr << "V1 stream protocol is not supported for UDP transport."
                << std::endl;
      return 1;
    }
    auto serializer =
        std::make_shared<taktile::TakDataSerializerUdp>(use_v0_protocol);
    siotrnsip::Options options = siotrnsip::UdpOptions{
        .endpoint = siotrnsip::Endpoint{.ip = address, .port = port}};

    auto sender = context->create_sender<taktile::TakMessageUdp>(options);
    auto receiver = context->create_receiver<taktile::TakMessageUdp>(
        options, [](taktile::TakMessageUdp const& tak_msg) {
          receive_cb(tak_msg.entity());
        });

    while (true) {
      auto tak_msg = taktile::TakMessageUdp{gen_cot(), serializer};
      BOOST_LOG_TRIVIAL(debug) << "Sending message: " << tak_msg.blob();
      sender->send(tak_msg);
      std::this_thread::sleep_for(std::chrono::seconds(5));
    }
  }

  auto serializer =
      std::make_shared<taktile::TakDataSerializerTcp>(use_v0_protocol);
  std::shared_ptr<taktile::TakDataFramer> framer;

  if (version.compare("v0") == 0) {
    framer =
        std::make_shared<taktile::TakDataFramer>(taktile::ProtocolVersion::V0);
  } else if (version.compare("v1_mesh") == 0) {
    framer = std::make_shared<taktile::TakDataFramer>(
        taktile::ProtocolVersion::V1_MESH);
  } else {  // v1_stream
    framer = std::make_shared<taktile::TakDataFramer>(
        taktile::ProtocolVersion::V1_STREAM);
  }

  siotrnsip::Options options;
  if (protocol.compare("tcp") == 0) {
    options = siotrnsip::TcpOptions{
        .endpoint = siotrnsip::Endpoint{.ip = address, .port = port},
        .streaming = true,
        .framer = framer};
  } else if (protocol.compare("tls") == 0) {
    options = siotrnsip::TlsOptions{
        .tcp_options =
            siotrnsip::TcpOptions{
                .endpoint = siotrnsip::Endpoint{.ip = address, .port = port},
                .streaming = true,
                .framer = framer},
        .credentials = siotrnsip::TlsCredentials{
            .ca_file = std::filesystem::path(CERTS_PATH) / "ca.crt",
            .cert_file = std::filesystem::path(CERTS_PATH) / "client.crt",
            .key_file =
                std::filesystem::path(CERTS_PATH) / "private/client.key"}};
  } else {
    std::cerr << "Unknown protocol: " << protocol << std::endl;
    return 1;
  }

  auto sender = context->create_sender<taktile::TakMessageTcp>(options);
  auto receiver = context->create_receiver<taktile::TakMessageTcp>(
      options, [](taktile::TakMessageTcp const& tak_msg) {
        receive_cb(tak_msg.entity());
      });

  while (true) {
    auto tak_msg = taktile::TakMessageTcp{gen_cot(), serializer};
    BOOST_LOG_TRIVIAL(debug) << "Sending message: " << tak_msg.blob();
    sender->send(tak_msg);
    std::this_thread::sleep_for(std::chrono::seconds(5));
  }

  return 0;
}
