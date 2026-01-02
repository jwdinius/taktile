// Copyright (c) 2025, Joe Dinius, Ph.D.
// SPDX-License-Identifier: Apache-2.0
#include <filesystem>
#include <iostream>
#include <memory>
#include <simpleio/transports/ip/ip.hpp>
#include <string>

#include "certs_path.h"  // NOLINT [build/include_subdir]
#include "taktile/taktile.hpp"

/// Send CoT (i.e., TAK v0) messages over TCP or TLS
/// Usage: ./send {address} {port} {tcp|tls}
/// E.g., ./send 127.0.0.1 8087 tcp
///
/// This example is modeled after the pytak library's "send" example:
/// https://pytak.readthedocs.io/en/latest/examples/#send-tak-data
///
/// A "takPong" event is sent every 5 seconds. This requires a TAK server
/// listening on the specified address/port using the specified protocol.
/// You can run the example TAK server included with this library by running
/// the `run_taky` script in the `examples/taky` directory.
namespace siotrnsip = simpleio::transports::ip;

taktile::TakData tak_pong() {
  auto tak_msg = atakmap::commoncommo::protobuf::v1::TakMessage();
  auto* cot = tak_msg.mutable_cotevent();
  cot->set_type("t-x-d-d");
  cot->set_uid("takPong");
  cot->set_how(taktile::DEFAULT_COT_HOW);
  auto const time_msec = taktile::TimeProvider::get_time();
  cot->set_sendtime(time_msec);
  cot->set_starttime(time_msec);
  cot->set_staletime(time_msec + taktile::DEFAULT_COT_STALE);
  return taktile::TakData(tak_msg);
}

int main(int argc, char** argv) {
  if (argc < 4) {
    std::cerr << "Usage: ./send {address} {port} {tcp|tls}" << std::endl;
    return 1;
  }
  auto address = argv[1];
  auto port = static_cast<uint16_t>(std::stoi(argv[2]));
  auto protocol = std::string(argv[3]);

  auto serializer = std::make_shared<taktile::TakDataSerializerTcp>(true);
  auto framer =
      std::make_shared<taktile::TakDataFramer>(taktile::ProtocolVersion::V0);

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

  auto context = std::make_unique<siotrnsip::Context>();
  auto sender = context->create_sender<taktile::TakMessageTcp>(options);

  while (true) {
    auto tak_msg = taktile::TakMessageTcp{tak_pong(), serializer};
    BOOST_LOG_TRIVIAL(debug) << "Sending message: " << tak_msg.blob();
    sender->send(tak_msg);
    std::this_thread::sleep_for(std::chrono::seconds(5));
  }

  return 0;
}
