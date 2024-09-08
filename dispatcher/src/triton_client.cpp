#include "triton_client.h"

#include <cstring>
#include <stdexcept>

namespace client {

TritonClient::TritonClient(
    const std::string& model_name, const std::string& model_version,
    const std::string& server_url, bool verbose, int max_retries,
    int retry_interval)
    : model_name_(model_name), model_version_(model_version),
      server_url_(server_url), verbose_(verbose), max_retries_(max_retries),
      retry_interval_(retry_interval)
{
  tc::Error err =
      tc::InferenceServerHttpClient::Create(&client_, server_url_, verbose_);
  if (!err.IsOk()) {
    throw std::runtime_error(
        "Failed to create Triton client: " + err.Message());
  }

  // Check if the server is alive
  bool is_server_live;
  err = client_->IsServerLive(&is_server_live);
  if (!err.IsOk()) {
    std::cerr << "Warning: Failed to check server liveness: " << err.Message()
              << std::endl;
  } else {
    std::cout << "Server liveness check: "
              << (is_server_live ? "Server is live." : "Server is not live.")
              << std::endl;
  }
}

cv::Mat
TritonClient::run_inference(const cv::Mat& image)
{
  if (image.empty()) {
    throw std::runtime_error("Error: Image is empty");
  }

  // Prepare input tensor for image
  std::vector<int64_t> input_shape = {
      1, image.rows, image.cols, 3};  // NHWC format
  std::vector<uint8_t> input_data(
      image.data, image.data + (image.total() * image.elemSize()));

  // Create input tensor
  std::shared_ptr<tc::InferInput> input_ptr;
  {
    tc::InferInput* input;
    tc::InferInput::Create(&input, "images", input_shape, "UINT8");
    input_ptr.reset(input);
    input_ptr->AppendRaw(
        reinterpret_cast<uint8_t*>(input_data.data()), input_data.size());
  }

  // Prepare inference options
  tc::InferOptions options(model_name_);
  options.model_version_ = model_version_;

  // Make inference request
  std::shared_ptr<tc::InferResult> result_ptr;
  tc::Error err;
  int attempts = 0;
  do {
    tc::InferResult* result;
    std::vector<const tc::InferRequestedOutput*> outputs;
    err = client_->Infer(&result, options, {input_ptr.get()}, outputs, {});
    if (err.IsOk()) {
      result_ptr.reset(result);
      break;
    }
    std::cerr << "Error: " << err << std::endl;
    std::cerr << "Sleeping for " << retry_interval_
              << " seconds and retrying. [Attempt: " << attempts + 1 << "/"
              << max_retries_ << "]" << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(retry_interval_));
  } while (++attempts < max_retries_);
  if (!err.IsOk()) {
    throw std::runtime_error("Inference failed: " + err.Message());
  }

  return get_mask(result_ptr, image.rows, image.cols);
}

cv::Mat
TritonClient::get_mask(
    const std::shared_ptr<tc::InferResult>& result, int rows, int cols) const
{
  size_t output_byte_size;
  const uint8_t* mask_data;

  // Retrieve raw mask data from Triton
  tc::Error err = result->RawData("masks", &mask_data, &output_byte_size);
  if (!err.IsOk()) {
    throw std::runtime_error("Failed to retrieve masks: " + err.Message());
  }

  // Validate the size of the mask data
  if (output_byte_size != static_cast<size_t>(rows * cols)) {
    throw std::runtime_error("Mask size does not match expected dimensions.");
  }

  // Create a cv::Mat from the raw mask data
  cv::Mat mask(rows, cols, CV_8UC1);
  std::memcpy(mask.data, mask_data, output_byte_size);

  return mask;
}

}  // namespace client
