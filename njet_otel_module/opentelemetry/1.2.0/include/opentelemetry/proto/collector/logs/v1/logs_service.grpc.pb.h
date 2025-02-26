// Generated by the gRPC C++ plugin.
// If you make any local change, they will be lost.
// source: opentelemetry/proto/collector/logs/v1/logs_service.proto
// Original file comments:
// Copyright 2020, OpenTelemetry Authors
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
#ifndef GRPC_opentelemetry_2fproto_2fcollector_2flogs_2fv1_2flogs_5fservice_2eproto__INCLUDED
#define GRPC_opentelemetry_2fproto_2fcollector_2flogs_2fv1_2flogs_5fservice_2eproto__INCLUDED

#include "opentelemetry/proto/collector/logs/v1/logs_service.pb.h"

#include <functional>
#include <grpc/impl/codegen/port_platform.h>
#include <grpcpp/impl/codegen/async_generic_service.h>
#include <grpcpp/impl/codegen/async_stream.h>
#include <grpcpp/impl/codegen/async_unary_call.h>
#include <grpcpp/impl/codegen/client_callback.h>
#include <grpcpp/impl/codegen/client_context.h>
#include <grpcpp/impl/codegen/completion_queue.h>
#include <grpcpp/impl/codegen/message_allocator.h>
#include <grpcpp/impl/codegen/method_handler.h>
#include <grpcpp/impl/codegen/proto_utils.h>
#include <grpcpp/impl/codegen/rpc_method.h>
#include <grpcpp/impl/codegen/server_callback.h>
#include <grpcpp/impl/codegen/server_callback_handlers.h>
#include <grpcpp/impl/codegen/server_context.h>
#include <grpcpp/impl/codegen/service_type.h>
#include <grpcpp/impl/codegen/status.h>
#include <grpcpp/impl/codegen/stub_options.h>
#include <grpcpp/impl/codegen/sync_stream.h>

namespace opentelemetry {
namespace proto {
namespace collector {
namespace logs {
namespace v1 {

// Service that can be used to push logs between one Application instrumented with
// OpenTelemetry and an collector, or between an collector and a central collector (in this
// case logs are sent/received to/from multiple Applications).
class LogsService final {
 public:
  static constexpr char const* service_full_name() {
    return "opentelemetry.proto.collector.logs.v1.LogsService";
  }
  class StubInterface {
   public:
    virtual ~StubInterface() {}
    // For performance reasons, it is recommended to keep this RPC
    // alive for the entire life of the application.
    virtual ::grpc::Status Export(::grpc::ClientContext* context, const ::opentelemetry::proto::collector::logs::v1::ExportLogsServiceRequest& request, ::opentelemetry::proto::collector::logs::v1::ExportLogsServiceResponse* response) = 0;
    std::unique_ptr< ::grpc::ClientAsyncResponseReaderInterface< ::opentelemetry::proto::collector::logs::v1::ExportLogsServiceResponse>> AsyncExport(::grpc::ClientContext* context, const ::opentelemetry::proto::collector::logs::v1::ExportLogsServiceRequest& request, ::grpc::CompletionQueue* cq) {
      return std::unique_ptr< ::grpc::ClientAsyncResponseReaderInterface< ::opentelemetry::proto::collector::logs::v1::ExportLogsServiceResponse>>(AsyncExportRaw(context, request, cq));
    }
    std::unique_ptr< ::grpc::ClientAsyncResponseReaderInterface< ::opentelemetry::proto::collector::logs::v1::ExportLogsServiceResponse>> PrepareAsyncExport(::grpc::ClientContext* context, const ::opentelemetry::proto::collector::logs::v1::ExportLogsServiceRequest& request, ::grpc::CompletionQueue* cq) {
      return std::unique_ptr< ::grpc::ClientAsyncResponseReaderInterface< ::opentelemetry::proto::collector::logs::v1::ExportLogsServiceResponse>>(PrepareAsyncExportRaw(context, request, cq));
    }
    class experimental_async_interface {
     public:
      virtual ~experimental_async_interface() {}
      // For performance reasons, it is recommended to keep this RPC
      // alive for the entire life of the application.
      virtual void Export(::grpc::ClientContext* context, const ::opentelemetry::proto::collector::logs::v1::ExportLogsServiceRequest* request, ::opentelemetry::proto::collector::logs::v1::ExportLogsServiceResponse* response, std::function<void(::grpc::Status)>) = 0;
      #ifdef GRPC_CALLBACK_API_NONEXPERIMENTAL
      virtual void Export(::grpc::ClientContext* context, const ::opentelemetry::proto::collector::logs::v1::ExportLogsServiceRequest* request, ::opentelemetry::proto::collector::logs::v1::ExportLogsServiceResponse* response, ::grpc::ClientUnaryReactor* reactor) = 0;
      #else
      virtual void Export(::grpc::ClientContext* context, const ::opentelemetry::proto::collector::logs::v1::ExportLogsServiceRequest* request, ::opentelemetry::proto::collector::logs::v1::ExportLogsServiceResponse* response, ::grpc::experimental::ClientUnaryReactor* reactor) = 0;
      #endif
    };
    #ifdef GRPC_CALLBACK_API_NONEXPERIMENTAL
    typedef class experimental_async_interface async_interface;
    #endif
    #ifdef GRPC_CALLBACK_API_NONEXPERIMENTAL
    async_interface* async() { return experimental_async(); }
    #endif
    virtual class experimental_async_interface* experimental_async() { return nullptr; }
  private:
    virtual ::grpc::ClientAsyncResponseReaderInterface< ::opentelemetry::proto::collector::logs::v1::ExportLogsServiceResponse>* AsyncExportRaw(::grpc::ClientContext* context, const ::opentelemetry::proto::collector::logs::v1::ExportLogsServiceRequest& request, ::grpc::CompletionQueue* cq) = 0;
    virtual ::grpc::ClientAsyncResponseReaderInterface< ::opentelemetry::proto::collector::logs::v1::ExportLogsServiceResponse>* PrepareAsyncExportRaw(::grpc::ClientContext* context, const ::opentelemetry::proto::collector::logs::v1::ExportLogsServiceRequest& request, ::grpc::CompletionQueue* cq) = 0;
  };
  class Stub final : public StubInterface {
   public:
    Stub(const std::shared_ptr< ::grpc::ChannelInterface>& channel);
    ::grpc::Status Export(::grpc::ClientContext* context, const ::opentelemetry::proto::collector::logs::v1::ExportLogsServiceRequest& request, ::opentelemetry::proto::collector::logs::v1::ExportLogsServiceResponse* response) override;
    std::unique_ptr< ::grpc::ClientAsyncResponseReader< ::opentelemetry::proto::collector::logs::v1::ExportLogsServiceResponse>> AsyncExport(::grpc::ClientContext* context, const ::opentelemetry::proto::collector::logs::v1::ExportLogsServiceRequest& request, ::grpc::CompletionQueue* cq) {
      return std::unique_ptr< ::grpc::ClientAsyncResponseReader< ::opentelemetry::proto::collector::logs::v1::ExportLogsServiceResponse>>(AsyncExportRaw(context, request, cq));
    }
    std::unique_ptr< ::grpc::ClientAsyncResponseReader< ::opentelemetry::proto::collector::logs::v1::ExportLogsServiceResponse>> PrepareAsyncExport(::grpc::ClientContext* context, const ::opentelemetry::proto::collector::logs::v1::ExportLogsServiceRequest& request, ::grpc::CompletionQueue* cq) {
      return std::unique_ptr< ::grpc::ClientAsyncResponseReader< ::opentelemetry::proto::collector::logs::v1::ExportLogsServiceResponse>>(PrepareAsyncExportRaw(context, request, cq));
    }
    class experimental_async final :
      public StubInterface::experimental_async_interface {
     public:
      void Export(::grpc::ClientContext* context, const ::opentelemetry::proto::collector::logs::v1::ExportLogsServiceRequest* request, ::opentelemetry::proto::collector::logs::v1::ExportLogsServiceResponse* response, std::function<void(::grpc::Status)>) override;
      #ifdef GRPC_CALLBACK_API_NONEXPERIMENTAL
      void Export(::grpc::ClientContext* context, const ::opentelemetry::proto::collector::logs::v1::ExportLogsServiceRequest* request, ::opentelemetry::proto::collector::logs::v1::ExportLogsServiceResponse* response, ::grpc::ClientUnaryReactor* reactor) override;
      #else
      void Export(::grpc::ClientContext* context, const ::opentelemetry::proto::collector::logs::v1::ExportLogsServiceRequest* request, ::opentelemetry::proto::collector::logs::v1::ExportLogsServiceResponse* response, ::grpc::experimental::ClientUnaryReactor* reactor) override;
      #endif
     private:
      friend class Stub;
      explicit experimental_async(Stub* stub): stub_(stub) { }
      Stub* stub() { return stub_; }
      Stub* stub_;
    };
    class experimental_async_interface* experimental_async() override { return &async_stub_; }

   private:
    std::shared_ptr< ::grpc::ChannelInterface> channel_;
    class experimental_async async_stub_{this};
    ::grpc::ClientAsyncResponseReader< ::opentelemetry::proto::collector::logs::v1::ExportLogsServiceResponse>* AsyncExportRaw(::grpc::ClientContext* context, const ::opentelemetry::proto::collector::logs::v1::ExportLogsServiceRequest& request, ::grpc::CompletionQueue* cq) override;
    ::grpc::ClientAsyncResponseReader< ::opentelemetry::proto::collector::logs::v1::ExportLogsServiceResponse>* PrepareAsyncExportRaw(::grpc::ClientContext* context, const ::opentelemetry::proto::collector::logs::v1::ExportLogsServiceRequest& request, ::grpc::CompletionQueue* cq) override;
    const ::grpc::internal::RpcMethod rpcmethod_Export_;
  };
  static std::unique_ptr<Stub> NewStub(const std::shared_ptr< ::grpc::ChannelInterface>& channel, const ::grpc::StubOptions& options = ::grpc::StubOptions());

  class Service : public ::grpc::Service {
   public:
    Service();
    virtual ~Service();
    // For performance reasons, it is recommended to keep this RPC
    // alive for the entire life of the application.
    virtual ::grpc::Status Export(::grpc::ServerContext* context, const ::opentelemetry::proto::collector::logs::v1::ExportLogsServiceRequest* request, ::opentelemetry::proto::collector::logs::v1::ExportLogsServiceResponse* response);
  };
  template <class BaseClass>
  class WithAsyncMethod_Export : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service* /*service*/) {}
   public:
    WithAsyncMethod_Export() {
      ::grpc::Service::MarkMethodAsync(0);
    }
    ~WithAsyncMethod_Export() override {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable synchronous version of this method
    ::grpc::Status Export(::grpc::ServerContext* /*context*/, const ::opentelemetry::proto::collector::logs::v1::ExportLogsServiceRequest* /*request*/, ::opentelemetry::proto::collector::logs::v1::ExportLogsServiceResponse* /*response*/) override {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
    void RequestExport(::grpc::ServerContext* context, ::opentelemetry::proto::collector::logs::v1::ExportLogsServiceRequest* request, ::grpc::ServerAsyncResponseWriter< ::opentelemetry::proto::collector::logs::v1::ExportLogsServiceResponse>* response, ::grpc::CompletionQueue* new_call_cq, ::grpc::ServerCompletionQueue* notification_cq, void *tag) {
      ::grpc::Service::RequestAsyncUnary(0, context, request, response, new_call_cq, notification_cq, tag);
    }
  };
  typedef WithAsyncMethod_Export<Service > AsyncService;
  template <class BaseClass>
  class ExperimentalWithCallbackMethod_Export : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service* /*service*/) {}
   public:
    ExperimentalWithCallbackMethod_Export() {
    #ifdef GRPC_CALLBACK_API_NONEXPERIMENTAL
      ::grpc::Service::
    #else
      ::grpc::Service::experimental().
    #endif
        MarkMethodCallback(0,
          new ::grpc::internal::CallbackUnaryHandler< ::opentelemetry::proto::collector::logs::v1::ExportLogsServiceRequest, ::opentelemetry::proto::collector::logs::v1::ExportLogsServiceResponse>(
            [this](
    #ifdef GRPC_CALLBACK_API_NONEXPERIMENTAL
                   ::grpc::CallbackServerContext*
    #else
                   ::grpc::experimental::CallbackServerContext*
    #endif
                     context, const ::opentelemetry::proto::collector::logs::v1::ExportLogsServiceRequest* request, ::opentelemetry::proto::collector::logs::v1::ExportLogsServiceResponse* response) { return this->Export(context, request, response); }));}
    void SetMessageAllocatorFor_Export(
        ::grpc::experimental::MessageAllocator< ::opentelemetry::proto::collector::logs::v1::ExportLogsServiceRequest, ::opentelemetry::proto::collector::logs::v1::ExportLogsServiceResponse>* allocator) {
    #ifdef GRPC_CALLBACK_API_NONEXPERIMENTAL
      ::grpc::internal::MethodHandler* const handler = ::grpc::Service::GetHandler(0);
    #else
      ::grpc::internal::MethodHandler* const handler = ::grpc::Service::experimental().GetHandler(0);
    #endif
      static_cast<::grpc::internal::CallbackUnaryHandler< ::opentelemetry::proto::collector::logs::v1::ExportLogsServiceRequest, ::opentelemetry::proto::collector::logs::v1::ExportLogsServiceResponse>*>(handler)
              ->SetMessageAllocator(allocator);
    }
    ~ExperimentalWithCallbackMethod_Export() override {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable synchronous version of this method
    ::grpc::Status Export(::grpc::ServerContext* /*context*/, const ::opentelemetry::proto::collector::logs::v1::ExportLogsServiceRequest* /*request*/, ::opentelemetry::proto::collector::logs::v1::ExportLogsServiceResponse* /*response*/) override {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
    #ifdef GRPC_CALLBACK_API_NONEXPERIMENTAL
    virtual ::grpc::ServerUnaryReactor* Export(
      ::grpc::CallbackServerContext* /*context*/, const ::opentelemetry::proto::collector::logs::v1::ExportLogsServiceRequest* /*request*/, ::opentelemetry::proto::collector::logs::v1::ExportLogsServiceResponse* /*response*/)
    #else
    virtual ::grpc::experimental::ServerUnaryReactor* Export(
      ::grpc::experimental::CallbackServerContext* /*context*/, const ::opentelemetry::proto::collector::logs::v1::ExportLogsServiceRequest* /*request*/, ::opentelemetry::proto::collector::logs::v1::ExportLogsServiceResponse* /*response*/)
    #endif
      { return nullptr; }
  };
  #ifdef GRPC_CALLBACK_API_NONEXPERIMENTAL
  typedef ExperimentalWithCallbackMethod_Export<Service > CallbackService;
  #endif

  typedef ExperimentalWithCallbackMethod_Export<Service > ExperimentalCallbackService;
  template <class BaseClass>
  class WithGenericMethod_Export : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service* /*service*/) {}
   public:
    WithGenericMethod_Export() {
      ::grpc::Service::MarkMethodGeneric(0);
    }
    ~WithGenericMethod_Export() override {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable synchronous version of this method
    ::grpc::Status Export(::grpc::ServerContext* /*context*/, const ::opentelemetry::proto::collector::logs::v1::ExportLogsServiceRequest* /*request*/, ::opentelemetry::proto::collector::logs::v1::ExportLogsServiceResponse* /*response*/) override {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
  };
  template <class BaseClass>
  class WithRawMethod_Export : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service* /*service*/) {}
   public:
    WithRawMethod_Export() {
      ::grpc::Service::MarkMethodRaw(0);
    }
    ~WithRawMethod_Export() override {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable synchronous version of this method
    ::grpc::Status Export(::grpc::ServerContext* /*context*/, const ::opentelemetry::proto::collector::logs::v1::ExportLogsServiceRequest* /*request*/, ::opentelemetry::proto::collector::logs::v1::ExportLogsServiceResponse* /*response*/) override {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
    void RequestExport(::grpc::ServerContext* context, ::grpc::ByteBuffer* request, ::grpc::ServerAsyncResponseWriter< ::grpc::ByteBuffer>* response, ::grpc::CompletionQueue* new_call_cq, ::grpc::ServerCompletionQueue* notification_cq, void *tag) {
      ::grpc::Service::RequestAsyncUnary(0, context, request, response, new_call_cq, notification_cq, tag);
    }
  };
  template <class BaseClass>
  class ExperimentalWithRawCallbackMethod_Export : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service* /*service*/) {}
   public:
    ExperimentalWithRawCallbackMethod_Export() {
    #ifdef GRPC_CALLBACK_API_NONEXPERIMENTAL
      ::grpc::Service::
    #else
      ::grpc::Service::experimental().
    #endif
        MarkMethodRawCallback(0,
          new ::grpc::internal::CallbackUnaryHandler< ::grpc::ByteBuffer, ::grpc::ByteBuffer>(
            [this](
    #ifdef GRPC_CALLBACK_API_NONEXPERIMENTAL
                   ::grpc::CallbackServerContext*
    #else
                   ::grpc::experimental::CallbackServerContext*
    #endif
                     context, const ::grpc::ByteBuffer* request, ::grpc::ByteBuffer* response) { return this->Export(context, request, response); }));
    }
    ~ExperimentalWithRawCallbackMethod_Export() override {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable synchronous version of this method
    ::grpc::Status Export(::grpc::ServerContext* /*context*/, const ::opentelemetry::proto::collector::logs::v1::ExportLogsServiceRequest* /*request*/, ::opentelemetry::proto::collector::logs::v1::ExportLogsServiceResponse* /*response*/) override {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
    #ifdef GRPC_CALLBACK_API_NONEXPERIMENTAL
    virtual ::grpc::ServerUnaryReactor* Export(
      ::grpc::CallbackServerContext* /*context*/, const ::grpc::ByteBuffer* /*request*/, ::grpc::ByteBuffer* /*response*/)
    #else
    virtual ::grpc::experimental::ServerUnaryReactor* Export(
      ::grpc::experimental::CallbackServerContext* /*context*/, const ::grpc::ByteBuffer* /*request*/, ::grpc::ByteBuffer* /*response*/)
    #endif
      { return nullptr; }
  };
  template <class BaseClass>
  class WithStreamedUnaryMethod_Export : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service* /*service*/) {}
   public:
    WithStreamedUnaryMethod_Export() {
      ::grpc::Service::MarkMethodStreamed(0,
        new ::grpc::internal::StreamedUnaryHandler<
          ::opentelemetry::proto::collector::logs::v1::ExportLogsServiceRequest, ::opentelemetry::proto::collector::logs::v1::ExportLogsServiceResponse>(
            [this](::grpc::ServerContext* context,
                   ::grpc::ServerUnaryStreamer<
                     ::opentelemetry::proto::collector::logs::v1::ExportLogsServiceRequest, ::opentelemetry::proto::collector::logs::v1::ExportLogsServiceResponse>* streamer) {
                       return this->StreamedExport(context,
                         streamer);
                  }));
    }
    ~WithStreamedUnaryMethod_Export() override {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable regular version of this method
    ::grpc::Status Export(::grpc::ServerContext* /*context*/, const ::opentelemetry::proto::collector::logs::v1::ExportLogsServiceRequest* /*request*/, ::opentelemetry::proto::collector::logs::v1::ExportLogsServiceResponse* /*response*/) override {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
    // replace default version of method with streamed unary
    virtual ::grpc::Status StreamedExport(::grpc::ServerContext* context, ::grpc::ServerUnaryStreamer< ::opentelemetry::proto::collector::logs::v1::ExportLogsServiceRequest,::opentelemetry::proto::collector::logs::v1::ExportLogsServiceResponse>* server_unary_streamer) = 0;
  };
  typedef WithStreamedUnaryMethod_Export<Service > StreamedUnaryService;
  typedef Service SplitStreamedService;
  typedef WithStreamedUnaryMethod_Export<Service > StreamedService;
};

}  // namespace v1
}  // namespace logs
}  // namespace collector
}  // namespace proto
}  // namespace opentelemetry


#endif  // GRPC_opentelemetry_2fproto_2fcollector_2flogs_2fv1_2flogs_5fservice_2eproto__INCLUDED
