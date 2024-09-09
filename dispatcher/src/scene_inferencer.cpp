#include "scene_inferencer.h"

#include <iostream>
#include <thread>
#include <vector>

#include "gdal_image_loader.h"
#include "gdal_image_saver.h"
#include "triton_client.h"

namespace inference {

const int MAX_HARDWARE_THREADS = std::thread::hardware_concurrency();

// Method to perform the inference process
void
SceneInferencer::run_inference(
    const std::string& image_path, const std::string& output_path)
{
  // Initialize image saver
  scene::GdalImageSaver saver(output_path, num_classes_);

  // Initialize separate loaders for each worker
  std::vector<std::unique_ptr<utility::WorkerThread>> worker_threads;
  std::vector<std::unique_ptr<client::TritonClient>> clients;
  std::vector<std::unique_ptr<scene::GdalImageLoader>> loaders;

  // Initialize the first worker
  append_resources(worker_threads, clients, loaders, image_path);

  // Get patch coordinates from one of the loaders
  std::vector<cv::Rect> coordinates = loaders[0]->get_patch_coordinates();
  int total_patches = coordinates.size();

  // Pre-initialize the image saver
  int width = loaders[0]->get_image_width();
  int height = loaders[0]->get_image_height();
  saver.init_gdal(width, height);

  int num_workers = std::min(
      static_cast<int>(std::sqrt(total_patches) / scaling_factor_),
      MAX_HARDWARE_THREADS);
  if (verbose_) {
    std::cout << "Total number of patches: " << total_patches << std::endl;
    std::cout << "Image dimensions: " << width << " x " << height << std::endl;
    std::cout << "Number of workers: " << num_workers << std::endl;
  }

  for (int worker_id = 1; worker_id < num_workers; ++worker_id) {
    append_resources(worker_threads, clients, loaders, image_path);
  }

  // Distribute tasks to workers
  std::vector<std::future<void>> futures;
  for (int i = 0; i < total_patches; ++i) {
    int worker_id = i % num_workers;  // Round-robin distribution of tasks

    futures.push_back(worker_threads[worker_id]->add_task([=, &saver, &clients,
                                                           &loaders]() mutable {
      const auto& coord = coordinates[i];
      if (verbose_) {
        std::cout << "Worker " << worker_id
                  << " processing patch at coordinates: " << coord << std::endl;
      }
      scene::ImagePatch patch =
          loaders[worker_id]->read_patch_from_coordinates(coord);
      cv::Mat mask = clients[worker_id]->request_inference(patch.image);
      saver.save_patch(coord, mask);
    }));
  }

  // Wait for all futures to complete
  for (auto& future : futures) {
    future.get();  // Blocking call to ensure all tasks are done
  }
}

void
SceneInferencer::append_resources(
    std::vector<std::unique_ptr<utility::WorkerThread>>& worker_threads,
    std::vector<std::unique_ptr<client::TritonClient>>& clients,
    std::vector<std::unique_ptr<scene::GdalImageLoader>>& loaders,
    const std::string& image_path)
{
  loaders.push_back(std::make_unique<scene::GdalImageLoader>(
      image_path, patch_size_, stride_size_));
  worker_threads.push_back(std::make_unique<utility::WorkerThread>());
  clients.push_back(std::make_unique<client::TritonClient>(
      model_name_, model_version_, url_, verbose_));
}

}  // namespace inference
