ARG PYTORCH_IMAGE_TAG=latest

FROM pytorch/pytorch:${PYTORCH_IMAGE_TAG}

# Set working directory
WORKDIR /opt/app

# Install system dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    libgl1-mesa-glx \
    libglib2.0-0 \
    libpython3.10 \
    && apt-get clean

# Upgrade pip to the latest version
RUN pip install --upgrade pip

# Copy the entire project directory into the container
COPY . /opt/app

# Install the core Python package
RUN pip install /opt/app
RUN pip install -U nvidia-pytriton

ENTRYPOINT ["python", "-m", "segtriton.server"]
