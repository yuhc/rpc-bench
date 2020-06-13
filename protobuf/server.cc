#include <chrono>
#include <iostream>
#include <grpcpp/grpcpp.h>
#include "bench.grpc.pb.h"
#include "server.h"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;

std::chrono::time_point<std::chrono::system_clock> endTime, start;

class ServerServiceImpl final : public bench::ServerService::Service {
  Status GetMessage(ServerContext *context,
                    bench::Message *reply) {
    // process reply
    return Status::OK;
  }
};

class BenchServer: RpcBenchServer {
public:
  BenchServer() = default;

  void StartServer() override {
    std::string server_addr("0.0.0.0:50051");
    ServerServiceImpl service;
    ServerBuilder builder;

    builder.AddListeningPort(server_addr, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);
    auto server = builder.BuildAndStart();
    std::cout << "Server listening on " << server_addr << std::endl;
    server->Wait();
  }
};

int main() {
  auto server = new BenchServer();
  server->StartServer();
  google::protobuf::ShutdownProtobufLibrary();
  return 0;
}
