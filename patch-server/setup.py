from setuptools import find_packages, setup

setup(
    name="segtriton",
    version="0.1.0",
    author="yhna@lunit.io",
    description="A lightweight implementation of Segmentation Service using Triton Inference Server",
    packages=find_packages(),
    install_requires=[
        "torch>=2.1.0",
        "transformers>=4.21.0",
        "numpy>=1.19.0",
        "opencv-python>=4.5.3",
        "Pillow>=8.0.0",
        "matplotlib>=3.4.0",
    ],
    classifiers=[
        "License :: OSI Approved :: MIT License",
    ],
    python_requires=">=3.8",
)
