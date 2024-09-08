#ifndef CLIENT_TRITON_CLIENT_H
#define CLIENT_TRITON_CLIENT_H

#include <memory>
#include <opencv2/opencv.hpp>
#include <string>

#include "http_client.h"

namespace tc = triton::client;

namespace client {

class TritonClient {
 public:
  // Constructor that initializes the Triton client with model details and
  // server URL
  TritonClient(
      const std::string& model_name, const std::string& model_version,
      const std::string& server_url, bool verbose, int max_retries = 32,
      int retry_interval = 4);

  // Runs inference on an input image and returns the resulting mask
  cv::Mat run_inference(const cv::Mat& image);

 private:
  std::string model_name_;
  std::string model_version_;
  std::string server_url_;
  bool verbose_;
  int max_retries_;
  int retry_interval_;

  std::unique_ptr<tc::InferenceServerHttpClient> client_;

  // Helper function to extract mask from Triton inference result
  cv::Mat get_mask(
      const std::shared_ptr<tc::InferResult>& result, int rows, int cols) const;
};

}  // namespace client

#endif  // CLIENT_TRITON_CLIENT_H
