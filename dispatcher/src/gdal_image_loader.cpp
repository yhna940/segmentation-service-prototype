#include "gdal_image_loader.h"

#include <filesystem>
#include <stdexcept>

namespace fs = std::filesystem;

namespace scene {

GdalImageLoader::GdalImageLoader(
    const std::string& image_path, int patch_size, int stride_size)
    : image_path_(image_path), patch_size_(patch_size),
      stride_size_(stride_size), dataset_(nullptr)
{
  // Check if the image path exists
  if (!fs::exists(image_path)) {
    throw std::runtime_error("Image path does not exist: " + image_path);
  }

  GDALAllRegister();
  dataset_ =
      static_cast<GDALDataset*>(GDALOpen(image_path.c_str(), GA_ReadOnly));
  if (dataset_ == nullptr) {
    throw std::runtime_error("Failed to open the image with GDAL.");
  }

  // Get the raster bands for the RGB channels
  red_band_ = dataset_->GetRasterBand(1);
  green_band_ = dataset_->GetRasterBand(2);
  blue_band_ = dataset_->GetRasterBand(3);

  if (!red_band_ || !green_band_ || !blue_band_) {
    throw std::runtime_error("Failed to retrieve the required raster bands.");
  }
}

GdalImageLoader::~GdalImageLoader()
{
  clean_up();
}

std::vector<cv::Rect>
GdalImageLoader::get_patch_coordinates() const
{
  std::vector<cv::Rect> coordinates;
  int image_width = dataset_->GetRasterXSize();
  int image_height = dataset_->GetRasterYSize();

  // Calculate the coordinates of each patch
  for (int y = 0; y < image_height; y += stride_size_) {
    for (int x = 0; x < image_width; x += stride_size_) {
      int calib_x = std::min(x, image_width - patch_size_);
      int calib_y = std::min(y, image_height - patch_size_);
      coordinates.push_back(
          cv::Rect(calib_x, calib_y, patch_size_, patch_size_));
    }
  }

  return coordinates;
}

ImagePatch
GdalImageLoader::read_patch_from_coordinates(const cv::Rect& coords) const
{
  try {
    cv::Mat patch(coords.height, coords.width, CV_8UC3);  // 3 channels (RGB)

    // Read the RGB channels
    red_band_->RasterIO(
        GF_Read, coords.x, coords.y, coords.width, coords.height, patch.data,
        coords.width, coords.height, GDT_Byte, 3, patch.step);
    green_band_->RasterIO(
        GF_Read, coords.x, coords.y, coords.width, coords.height,
        patch.data + 1, coords.width, coords.height, GDT_Byte, 3, patch.step);
    blue_band_->RasterIO(
        GF_Read, coords.x, coords.y, coords.width, coords.height,
        patch.data + 2, coords.width, coords.height, GDT_Byte, 3, patch.step);

    return {patch, coords};  // Return the patch and its coordinates
  }
  catch (const std::exception& e) {
    throw std::runtime_error("Error reading patch: " + std::string(e.what()));
  }
}

int
GdalImageLoader::get_image_width() const
{
  return dataset_->GetRasterXSize();
}

int
GdalImageLoader::get_image_height() const
{
  return dataset_->GetRasterYSize();
}

void
GdalImageLoader::clean_up()
{
  if (dataset_) {
    GDALClose(dataset_);
    dataset_ = nullptr;
  }
}

}  // namespace scene
