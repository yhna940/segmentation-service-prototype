#include "service.h"

#include "scene_inferencer.h"

namespace service {

void
InferenceService::start(int port)
{
  // Set up the route for /segment
  server_->Post(
      "/segment", [this](const httplib::Request& req, httplib::Response& res) {
        handle_inference_request(req, res);
      });

  server_->set_read_timeout(0, 0);
  server_->set_write_timeout(0, 0);

  std::cout << "Starting server on port " << port << std::endl;
  server_->listen("0.0.0.0", port);
}

void
InferenceService::handle_inference_request(
    const httplib::Request& req, httplib::Response& res)
{
  {
    std::unique_lock<std::mutex> lock(inference_mutex_);
    cv_.wait(
        lock, [this] { return active_requests_ < max_concurrent_requests_; });
    active_requests_++;
  }

  // Parse the request
  auto image_path = req.get_param_value("image_path");
  auto output_path = req.get_param_value("output_path");

  if (verbose_) {
    std::cout << "Received inference request with image path: " << image_path
              << " and output path: " << output_path << std::endl;
  }

  try {
    inferencer_.run_inference(image_path, output_path);
  }
  catch (const std::exception& e) {
    res.set_content(e.what(), "text/plain");
    {
      std::unique_lock<std::mutex> lock(inference_mutex_);
      active_requests_--;
    }
    cv_.notify_one();
    return;
  }

  // Respond to the request
  res.set_content("Inference completed successfully.", "text/plain");
  {
    std::unique_lock<std::mutex> lock(inference_mutex_);
    active_requests_--;
  }
  cv_.notify_one();
}

}  // namespace service
