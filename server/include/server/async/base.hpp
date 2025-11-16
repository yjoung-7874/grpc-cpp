#ifndef GRPC_ASYNC_SERVER_BASE_HPP_
#define GRPC_ASYNC_SERVER_BASE_HPP_

#include <grpcpp/grpcpp.h>
#include <grpcpp/impl/service_type.h>

#include <atomic>
#include <condition_variable>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

class GrpcAsyncServerBase {
 public:
  explicit GrpcAsyncServerBase(const std::string& address, int worker_threads = 0);

  virtual ~GrpcAsyncServerBase() { Shutdown(); }

  void Init();
  bool BuildAndStartServer();
  void NonBlockRun();
  void Run();
  void Shutdown();

  class CallDataBase {
   public:
    virtual ~CallDataBase() = default;
    virtual void Proceed(bool ok) = 0;
  };

  template <typename RequestType, typename ResponseType, typename ResponderType = grpc::ServerAsyncResponseWriter<ResponseType>>
  class CallData : public CallDataBase {
   public:
    using RequestFn = std::function<void(grpc::ServerContext*, RequestType*, ResponderType*, grpc::ServerCompletionQueue*,
                                         grpc::ServerCompletionQueue*, void*)>;
    using HandlerFn = std::function<void(const RequestType&, ResponseType&)>;

    CallData(grpc::ServerCompletionQueue* cq, RequestFn request_fn, HandlerFn handler_fn)
        : cq_(cq), request_fn_(std::move(request_fn)), handler_fn_(std::move(handler_fn)), state_(CREATE), responder_(&ctx_) {
      Proceed(true);
    }

    void Proceed(bool ok) override {
      if (!ok) {
        delete this;
        return;
      }

      switch (state_) {
        case CREATE:
          state_ = PROCESS;
          request_fn_(&ctx_, &request_, &responder_, cq_, cq_, this);
          break;

        case PROCESS:
          new CallData(cq_, request_fn_, handler_fn_);
          handler_fn_(request_, response_);
          state_ = FINISH;
          responder_.Finish(response_, grpc::Status::OK, this);
          break;

        case FINISH:
          delete this;
          break;
      }
    }

   private:
    enum State { CREATE, PROCESS, FINISH };
    State state_;

    grpc::ServerCompletionQueue* cq_;
    grpc::ServerContext ctx_;

    RequestType request_;
    ResponseType response_;
    ResponderType responder_;

    RequestFn request_fn_;
    HandlerFn handler_fn_;
  };

  template <typename RequestType, typename ResponseType>
  class ServerStreamCallData : public CallDataBase {
   public:
    using RequestFn = std::function<void(grpc::ServerContext*, RequestType*, grpc::ServerAsyncWriter<ResponseType>*,
                                         grpc::ServerCompletionQueue*, grpc::ServerCompletionQueue*, void*)>;
    using HandlerFn = std::function<void(const RequestType&, grpc::ServerAsyncWriter<ResponseType>*, ServerStreamCallData*)>;

    ServerStreamCallData(grpc::ServerCompletionQueue* cq, RequestFn request_fn, HandlerFn handler_fn)
        : cq_(cq),
          request_fn_(std::move(request_fn)),
          handler_fn_(std::move(handler_fn)),
          responder_(&ctx_),
          state_(CREATE),
          accepted_(false) {
      Proceed(true);
    }

    void Proceed(bool ok) override {
      if (!ok) {
        delete this;
        return;
      }

      switch (state_) {
        case CREATE:
          state_ = PROCESS;
          request_fn_(&ctx_, &request_, &responder_, cq_, cq_, this);
          break;

        case PROCESS:
          if (!accepted_) {
            accepted_ = true;
            new ServerStreamCallData(cq_, request_fn_, handler_fn_);
          }
          handler_fn_(request_, &responder_, this);
          break;

        case FINISH:
          delete this;
          break;
      }
    }

    void WriteNext(const ResponseType& msg) { responder_.Write(msg, this); }

    void Finish() {
      responder_.Finish(grpc::Status::OK, this);
      state_ = FINISH;
    }

   private:
    enum State { CREATE, PROCESS, FINISH };
    grpc::ServerCompletionQueue* cq_;
    grpc::ServerContext ctx_;
    RequestType request_;
    grpc::ServerAsyncWriter<ResponseType> responder_;
    RequestFn request_fn_;
    HandlerFn handler_fn_;
    State state_;
    bool accepted_;
  };

  template <typename RequestType, typename ResponseType>
  class BidiCallData : public GrpcAsyncServerBase::CallDataBase {
   public:
    using RequestFn = std::function<void(grpc::ServerContext*, grpc::ServerAsyncReaderWriter<ResponseType, RequestType>*,
                                         grpc::ServerCompletionQueue*, grpc::ServerCompletionQueue*, void*)>;
    using HandlerFn = std::function<void(RequestType&&, ResponseType&)>;

    BidiCallData(grpc::ServerCompletionQueue* cq, RequestFn request_fn, HandlerFn handler_fn)
        : cq_(cq), request_fn_(std::move(request_fn)), handler_fn_(std::move(handler_fn)), responder_(&ctx_), state_(CREATE) {}

    void Proceed(bool ok) override {
      if (!ok) {
        delete this;
        return;
      }

      switch (state_) {
        case CREATE:
          state_ = READ;
          responder_.Read(&request_, this);
          break;

        case READ:
          state_ = WRITE;
          handler_fn_(std::move(request_), response_);
          responder_.Write(response_, this);
          break;

        case WRITE:
          state_ = READ;
          responder_.Read(&request_, this);
          break;

        case FINISH:
          delete this;
          break;
      }
    }

   private:
    grpc::ServerCompletionQueue* cq_;
    grpc::ServerContext ctx_;
    RequestType request_;
    ResponseType response_;
    grpc::ServerAsyncReaderWriter<ResponseType, RequestType> responder_;
    RequestFn request_fn_;
    HandlerFn handler_fn_;
    enum State { CREATE, READ, WRITE, FINISH } state_;
  };

 protected:
  std::vector<std::unique_ptr<grpc::Service>> service_list_;
  virtual void InitServices() = 0;
  virtual void InitRpcs() = 0;
  virtual int GetServiceCQCount(grpc::Service* /*svc*/) { return 1; }

  grpc::ServerCompletionQueue* PickCQForService(grpc::Service* svc);

  template <typename ServiceType, typename RequestType, typename ResponseType, typename ResponderType>
  void StartRpc(ServiceType* svc, typename CallData<RequestType, ResponseType, ResponderType>::RequestFn req_fn,
                typename CallData<RequestType, ResponseType, ResponderType>::HandlerFn handler_fn) {
    grpc::ServerCompletionQueue* cq = PickCQForService(svc);
    new CallData<RequestType, ResponseType, ResponderType>(cq, std::move(req_fn), std::move(handler_fn));
  }

  template <typename ServiceType, typename RequestType, typename ResponseType>
  void StartServerStream(ServiceType* svc, typename ServerStreamCallData<RequestType, ResponseType>::RequestFn req_fn,
                         typename ServerStreamCallData<RequestType, ResponseType>::HandlerFn handler_fn) {
    grpc::ServerCompletionQueue* cq = PickCQForService(svc);
    new ServerStreamCallData<RequestType, ResponseType>(cq, std::move(req_fn), std::move(handler_fn));
  }

  template <typename ServiceType, typename RequestType, typename ResponseType>
  void StartBidi(ServiceType* svc, typename BidiCallData<RequestType, ResponseType>::RequestFn req_fn,
                 typename BidiCallData<RequestType, ResponseType>::HandlerFn handler_fn) {
    grpc::ServerCompletionQueue* cq = PickCQForService(svc);
    new BidiCallData<RequestType, ResponseType>(cq, std::move(req_fn), std::move(handler_fn));
  }

 private:
  std::string server_address_;
  std::unique_ptr<grpc::Server> server_;
  std::unordered_map<grpc::Service*, std::vector<std::unique_ptr<grpc::ServerCompletionQueue>>> service_cqs_;
  std::vector<grpc::ServerCompletionQueue*> all_cqs_;
  std::unordered_map<grpc::Service*, std::atomic<size_t>> cq_rr_index_;
  std::vector<std::thread> workers_;
  std::atomic<bool> running_{false};
  unsigned int worker_thread_count_ = 0;
  int requested_worker_threads_ = 0;
  std::vector<std::thread::id> thread_ids_;

  unsigned int worker_count_hint() const;
  void WorkerLoop(unsigned int worker_index);
};

#endif  // GRPC_ASYNC_SERVER_BASE_HPP_
