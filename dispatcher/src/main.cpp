#include <getopt.h>

#include <iostream>
#include <string>

#include "service.h"

constexpr int SERVICE_PORT = 8080;

int
main(int argc, char** argv)
{
  std::string url("localhost:8000");
  int patch_size = 512;
  int stride_size = 256;
  int scaling_factor = 6;
  bool verbose = true;

  int opt;
  // Use getopt to parse command-line arguments
  while ((opt = getopt(argc, argv, "u:p:s:n:v")) != -1) {
    switch (opt) {
      case 'u':
        url = optarg;  // Triton server URL
        break;
      case 'p':
        patch_size = std::stoi(optarg);  // patch_size
        break;
      case 's':
        stride_size = std::stoi(optarg);  // stride_size
        break;
      case 'n':
        scaling_factor = std::stoi(optarg);  // scaling_factor
        break;
      case 'v':
        verbose = true;  // verbose flag
        break;
      default:
        std::cerr << "Unknown option: " << opt << std::endl;
        return -1;
    }
  }
  if (verbose) {
    std::cout << "Triton server URL: " << url << std::endl;
    std::cout << "Patch size: " << patch_size << std::endl;
    std::cout << "Stride size: " << stride_size << std::endl;
    std::cout << "Scale factor: " << scaling_factor << std::endl;
    std::cout << "Verbose: " << (verbose ? "true" : "false") << std::endl;
  }

  // Initialize the service with Triton server URL
  service::InferenceService inference_service(
      url, patch_size, stride_size, scaling_factor, verbose);

  // Start the service on the specified port
  inference_service.start(SERVICE_PORT);

  return 0;
}
