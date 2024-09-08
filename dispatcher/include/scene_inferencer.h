#ifndef INFERENCE_SCENE_INFERENCER_H
#define INFERENCE_SCENE_INFERENCER_H

#include <memory>
#include <string>
#include <vector>

#include "gdal_image_loader.h"
#include "gdal_image_saver.h"
#include "triton_client.h"
#include "worker_thread.h"

namespace inference {

class SceneInferencer {
 public:
  // Constructor
  SceneInferencer(
      int num_classes, const std::string& model_name,
      const std::string& model_version, const std::string& url, int patch_size,
      int stride_size, bool verbose = true, int scaling_factor = 6)
      : num_classes_(num_classes), model_name_(model_name),
        model_version_(model_version), url_(url), patch_size_(patch_size),
        stride_size_(stride_size), verbose_(verbose),
        scaling_factor_(scaling_factor)
  {
  }

  // Method to perform the inference process
  void run_inference(
      const std::string& image_path, const std::string& output_path);

 private:
  int num_classes_;
  std::string model_name_;
  std::string model_version_;
  std::string url_;
  int patch_size_;
  int stride_size_;
  bool verbose_;
  int scaling_factor_;

  void append_resources(
      std::vector<std::unique_ptr<utility::WorkerThread>>& worker_threads,
      std::vector<std::unique_ptr<client::TritonClient>>& clients,
      std::vector<std::unique_ptr<scene::GdalImageLoader>>& loaders,
      const std::string& image_path);
};

}  // namespace inference

#endif  // INFERENCE_SCENE_INFERENCER_H
