#include "server/async/base.hpp"

GrpcAsyncServerBase::GrpcAsyncServerBase(const std::string& address, int worker_threads)
    : server_address_(address), requested_worker_threads_(worker_threads) {}

void GrpcAsyncServerBase::Init() {
  InitServices();
  if (service_list_.empty()) {
    std::cerr << "No service registered." << std::endl;
  }
}

bool GrpcAsyncServerBase::BuildAndStartServer() {
  if (server_address_.empty()) {
    std::cerr << "Empty server address." << std::endl;
    return false;
  }

  grpc::ServerBuilder builder;
  builder.AddListeningPort(server_address_, grpc::InsecureServerCredentials());

  for (auto& svc : service_list_) {
    builder.RegisterService(svc.get());
  }

  for (auto& svc : service_list_) {
    int cq_count = std::max(1, GetServiceCQCount(svc.get()));
    auto& vec = service_cqs_[svc.get()];
    for (int i = 0; i < cq_count; ++i) {
      vec.push_back(builder.AddCompletionQueue());
    }
  }

  server_ = builder.BuildAndStart();
  if (!server_) {
    std::cerr << "Failed to start server." << std::endl;
    return false;
  }

  for (auto& [svc, cqs] : service_cqs_) {
    cq_rr_index_[svc] = 0;
    for (auto& u : cqs) all_cqs_.push_back(u.get());
  }

  if (requested_worker_threads_ <= 0) {
    worker_thread_count_ = worker_count_hint();
  } else {
    worker_thread_count_ = static_cast<unsigned int>(requested_worker_threads_);
  }

  running_.store(true);

  for (unsigned int i = 0; i < worker_thread_count_; ++i) {
    workers_.emplace_back([this, i] { WorkerLoop(i); });
  }

  std::cout << "Server listening on " << server_address_ << " (CQs=" << all_cqs_.size() << ", workers=" << worker_thread_count_
            << ")\n";

  return true;
}

void GrpcAsyncServerBase::NonBlockRun() {
  Init();
  if (!BuildAndStartServer()) return;
  InitRpcs();
}

void GrpcAsyncServerBase::Run() {
  NonBlockRun();

  for (auto& t : workers_) {
    if (t.joinable()) t.join();
  }
}

void GrpcAsyncServerBase::Shutdown() {
  if (!running_.load(std::memory_order_acquire) && !server_) return;
  running_.store(false, std::memory_order_release);
  if (server_) server_->Shutdown();

  for (auto& [svc, cqs] : service_cqs_) {
    for (auto& cq : cqs) cq->Shutdown();
  }

  for (auto& t : workers_) {
    if (t.joinable()) t.join();
  }

  workers_.clear();
  all_cqs_.clear();
  service_cqs_.clear();
  cq_rr_index_.clear();
  server_.reset();
}

grpc::ServerCompletionQueue* GrpcAsyncServerBase::PickCQForService(grpc::Service* svc) {
  auto it = service_cqs_.find(svc);
  if (it == service_cqs_.end() || it->second.empty()) {
    if (!all_cqs_.empty()) return all_cqs_[0];
    return nullptr;
  }
  size_t idx = cq_rr_index_[svc].fetch_add(1) % it->second.size();
  return it->second[idx].get();
}

unsigned int GrpcAsyncServerBase::worker_count_hint() const {
  return requested_worker_threads_ > 0 ? requested_worker_threads_ : std::thread::hardware_concurrency() * 2;
}

void GrpcAsyncServerBase::WorkerLoop(unsigned int worker_index) {
  size_t idx = worker_index % (all_cqs_.empty() ? 1 : all_cqs_.size());
  while (running_.load(std::memory_order_acquire)) {
    if (all_cqs_.empty()) {
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
      continue;
    }
    grpc::ServerCompletionQueue* cq = all_cqs_[idx++ % all_cqs_.size()];
    void* tag = nullptr;
    bool ok = false;

    if (!cq->Next(&tag, &ok)) {
      continue;
    }
    if (tag) {
      try {
        static_cast<CallDataBase*>(tag)->Proceed(ok);
      } catch (const std::exception& e) {
        std::cerr << "Exception in Proceed(): " << e.what() << std::endl;
      }
    }
  }
}
