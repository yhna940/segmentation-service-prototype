#include "gdal_image_saver.h"

#include <gdal_priv.h>

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <filesystem>
#include <stdexcept>

namespace scene {

const std::string COUNT_FILENAME_PREFIX = "/tmp/.countmap_";

GdalImageSaver::GdalImageSaver(const std::string& output_path, int num_classes)
    : output_path_(output_path), num_classes_(num_classes),
      is_initialized_(false), image_dataset_(nullptr), count_dataset_(nullptr)
{
  GDALAllRegister();
}

GdalImageSaver::~GdalImageSaver()
{
  clean_up();
}

void
GdalImageSaver::clean_up()
{
  if (image_dataset_) {
    GDALClose(image_dataset_);
    image_dataset_ = nullptr;
  }

  if (count_dataset_) {
    GDALClose(count_dataset_);
    count_dataset_ = nullptr;
  }

  if (!count_filename_.empty() && std::filesystem::exists(count_filename_)) {
    std::filesystem::remove(count_filename_);
  }
}

std::string
GdalImageSaver::generate_uuid()
{
  boost::uuids::uuid uuid = boost::uuids::random_generator()();
  return boost::uuids::to_string(uuid);
}

void
GdalImageSaver::init_gdal(int width, int height)
{
  std::lock_guard<std::mutex> lock(init_mutex_);
  if (is_initialized_) {
    return;
  }

  width_ = width;
  height_ = height;

  if (width <= 0 || height <= 0) {
    throw std::runtime_error("Invalid image dimensions for init_gdal.");
  }

  GDALDriver* driver = GetGDALDriverManager()->GetDriverByName("GTiff");
  if (!driver) {
    throw std::runtime_error("GDAL GTiff driver not found.");
  }

  // Create image dataset at output_path_
  image_dataset_ = driver->Create(
      output_path_.c_str(), width_, height_, 1, GDT_Byte, nullptr);
  if (!image_dataset_) {
    throw std::runtime_error("Failed to create image dataset at output_path.");
  }

  // Apply the grayscale palette to the image dataset
  apply_palette();

  // Create count map dataset as a temporary file
  std::string uuid = generate_uuid();
  count_filename_ = COUNT_FILENAME_PREFIX + uuid + ".tif";
  count_dataset_ = driver->Create(
      count_filename_.c_str(), width_, height_, num_classes_, GDT_Int32,
      nullptr);
  if (!count_dataset_) {
    throw std::runtime_error("Failed to create count map dataset.");
  }

  is_initialized_ = true;
}

void
GdalImageSaver::save_patch(const cv::Rect& roi, const cv::Mat& patch)
{
  // Ensure GDAL is initialized
  {
    std::lock_guard<std::mutex> lock(init_mutex_);
    if (!is_initialized_) {
      throw std::runtime_error("GDAL is not initialized.");
    }
  }

  // Prepare buffers for count map and final class labels
  std::vector<int> count_buffer(roi.width * roi.height * num_classes_, 0);
  std::vector<uint8_t> final_class_buffer(roi.width * roi.height, 0);

  // Populate count buffer and track final class labels
  for (int y = 0; y < roi.height; ++y) {
    for (int x = 0; x < roi.width; ++x) {
      int local_idx = y * roi.width + x;
      int class_label = patch.at<uint8_t>(y, x);

      if (class_label < num_classes_) {
        count_buffer[local_idx * num_classes_ + class_label] += 1;
      }
    }
  }

  // Write count buffer and find final class labels
  for (int c = 0; c < num_classes_; ++c) {
    GDALRasterBand* count_band = count_dataset_->GetRasterBand(c + 1);
    std::vector<int> single_class_buffer(roi.width * roi.height);

    for (int i = 0; i < roi.width * roi.height; ++i) {
      single_class_buffer[i] = count_buffer[i * num_classes_ + c];

      // Determine final class for each pixel based on max count
      if (single_class_buffer[i] >
          count_buffer[i * num_classes_ + final_class_buffer[i]]) {
        final_class_buffer[i] = static_cast<uint8_t>(c);
      }
    }

    // Write count map to disk
    {
      std::lock_guard<std::mutex> lock(init_mutex_);
      count_band->RasterIO(
          GF_Write, roi.x, roi.y, roi.width, roi.height,
          single_class_buffer.data(), roi.width, roi.height, GDT_Int32, 0, 0);
    }
  }

  // Write final class labels directly to the image dataset
  GDALRasterBand* image_band = image_dataset_->GetRasterBand(1);
  {
    std::lock_guard<std::mutex> lock(init_mutex_);
    image_band->RasterIO(
        GF_Write, roi.x, roi.y, roi.width, roi.height,
        final_class_buffer.data(), roi.width, roi.height, GDT_Byte, 0, 0);
  }
}

void
GdalImageSaver::apply_palette()
{
  if (!image_dataset_) {
    throw std::runtime_error(
        "Dataset is not initialized for palette application.");
  }

  GDALRasterBand* band = image_dataset_->GetRasterBand(1);

  GDALColorTable color_table;
  GDALColorEntry color_entry;

  int grayscale_step = 255 / (num_classes_ - 1);

  // Create a grayscale palette based on the number of classes
  for (int class_label = 0; class_label < num_classes_; ++class_label) {
    int grayscale_value = grayscale_step * class_label;
    color_entry.c1 = grayscale_value;  // R
    color_entry.c2 = grayscale_value;  // G
    color_entry.c3 = grayscale_value;  // B
    color_entry.c4 = 255;              // Alpha
    color_table.SetColorEntry(class_label, &color_entry);
  }

  band->SetColorTable(&color_table);
}

}  // namespace scene
