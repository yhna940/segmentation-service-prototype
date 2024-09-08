#ifndef SCENE_GDAL_IMAGE_SAVER_H
#define SCENE_GDAL_IMAGE_SAVER_H

#include <gdal_priv.h>

#include <mutex>
#include <opencv2/opencv.hpp>
#include <string>

namespace scene {

class GdalImageSaver {
 public:
  // Constructor initializes output path and number of classes, and applies the
  // palette
  GdalImageSaver(const std::string& output_path, int num_classes);
  ~GdalImageSaver();

  // Initialize GDAL datasets
  void init_gdal(int width, int height);

  // Save a patch of the image and update class counts
  void save_patch(const cv::Rect& roi, const cv::Mat& patch);

 private:
  // Clean up GDAL datasets
  void clean_up();

  // Apply grayscale palette to the output image (now called in constructor)
  void apply_palette();

  // Generate a unique filename for temporary count map
  std::string generate_uuid();

  // GDAL datasets and parameters
  GDALDataset* image_dataset_;
  GDALDataset* count_dataset_;
  std::string output_path_;
  std::string count_filename_;

  int width_, height_;
  int num_classes_;
  bool is_initialized_;

  // Mutex for thread safety
  std::mutex init_mutex_;
};

}  // namespace scene

#endif  // SCENE_GDAL_IMAGE_SAVER_H
