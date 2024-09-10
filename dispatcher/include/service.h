#ifndef SERVICE_INFERENCE_SERVICE_H
#define SERVICE_INFERENCE_SERVICE_H

#include <iostream>
#include <string>

#include "httplib.h"
#include "scene_inferencer.h"

namespace service {

class InferenceService {
 public:
  // Constructor that accepts Triton server address
  InferenceService(
      const std::string& triton_server_url, int patch_size, int stride_size,
      int scaling_factor, bool verbose, int num_classes = 3,
      const std::string& model_name = "Segmentor",
      const std::string& model_version = "", int max_concurrent_requests = 8)
      : triton_server_url_(triton_server_url), patch_size_(patch_size),
        stride_size_(stride_size), scaling_factor_(scaling_factor),
        verbose_(verbose),
        inferencer_(
            num_classes, model_name, model_version, triton_server_url,
            patch_size, stride_size, verbose, scaling_factor),
        server_(std::make_unique<httplib::Server>()), active_requests_(0),
        max_concurrent_requests_(max_concurrent_requests)
  {
  }

  // Start the HTTP server
  void start(int port);

 private:
  std::string triton_server_url_;
  int patch_size_;
  int stride_size_;
  int scaling_factor_;
  bool verbose_;
  int active_requests_;
  int max_concurrent_requests_;
  std::mutex inference_mutex_;
  std::condition_variable cv_;

  // HTTP server instance
  std::unique_ptr<httplib::Server> server_;

  // Inferencer instance
  inference::SceneInferencer inferencer_;

  std::mutex inferencer_mutex_;

  // Handle inference requests
  void handle_inference_request(
      const httplib::Request& req, httplib::Response& res);
};

}  // namespace service

#endif  // SERVICE_INFERENCE_SERVICE_H
