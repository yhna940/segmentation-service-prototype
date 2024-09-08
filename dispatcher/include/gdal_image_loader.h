#ifndef SCENE_GDAL_IMAGE_LOADER_H
#define SCENE_GDAL_IMAGE_LOADER_H

#include <gdal_priv.h>

#include <opencv2/opencv.hpp>
#include <string>
#include <vector>

namespace scene {

struct ImagePatch {
  cv::Mat image;
  cv::Rect roi;
};

class GdalImageLoader {
 public:
  // Constructor that loads the image using GDAL and sets the patch size and
  // stride
  GdalImageLoader(
      const std::string& image_path, int patch_size, int stride_size);

  ~GdalImageLoader();

  // Gets the coordinates of patches to process
  std::vector<cv::Rect> get_patch_coordinates() const;

  // Reads a patch of the image based on the given coordinates
  ImagePatch read_patch_from_coordinates(const cv::Rect& coords) const;

  // Get the image width
  int get_image_width() const;

  // Get the image height
  int get_image_height() const;

 private:
  std::string image_path_;
  int patch_size_;
  int stride_size_;
  GDALDataset* dataset_;
  GDALRasterBand* red_band_;
  GDALRasterBand* green_band_;
  GDALRasterBand* blue_band_;

  void clean_up();
};

}  // namespace scene

#endif  // SCENE_GDAL_IMAGE_LOADER_H
