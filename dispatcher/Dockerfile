# Use the official NVIDIA Triton SDK image as the base image
FROM nvcr.io/nvidia/tritonserver:23.08-py3-sdk

# Install additional dependencies including GDAL and git for cloning repositories
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    libcurl4-openssl-dev \
    rapidjson-dev \
    gdal-bin \
    libgdal-dev \
    gdb \
    git \
    && rm -rf /var/lib/apt/lists/*

# Set environment variables for the Triton client build directory
ENV TRITON_CLIENT_BUILD_DIR /workspace/install

# Clone cpp-httplib from GitHub
RUN git clone https://github.com/yhirose/cpp-httplib.git /app/include/cpp-httplib

# Copy the source code and CMakeLists.txt into the container
COPY . /app

# Set the working directory to the application directory
WORKDIR /app

# Build the C++ application using CMake
RUN rm -rf build && mkdir build && cd build && \
    cmake -DCMAKE_BUILD_TYPE=Release .. && \
    cmake --build .

# Set the entry point to run the built executable
ENTRYPOINT ["/app/build/remote-segmentor-dispatcher"]
