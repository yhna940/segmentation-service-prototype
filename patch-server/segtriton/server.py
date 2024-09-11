import argparse
import logging
from typing import Dict, List, Optional

import numpy as np
import pytriton.decorators as decorators
import torch
from PIL import Image
from pytriton.model_config import DynamicBatcher, ModelConfig, Tensor
from pytriton.triton import Triton
from segtriton.segmenter import Segmenter


class _InferCallable:
    def __init__(
        self,
        device: str,
        model_id: str,
        cache_dir: str,
    ):
        self.model = Segmenter(model_id, device, cache_dir)
        logging.info(f"Model loaded on {device}.")
        logging.info(f"Model ID: {model_id}")

    @decorators.batch
    def __call__(self, images: np.ndarray) -> Dict[str, np.ndarray]:
        processed_images = [Image.fromarray(img) for img in images]

        results = []
        for processed_image in processed_images:
            _, mask = self.model.predict(processed_image)
            results.append(mask.cpu().numpy())

        return {
            "masks": np.array(results, dtype=np.uint8),
        }


def multi_device_factory(
    detector_id: str = "ratnaonline1/segFormer-b4-city-satellite-segmentation-1024x1024",
    cache_dir: Optional[str] = None,
) -> List[_InferCallable]:
    if not torch.cuda.is_available():
        raise ValueError("CUDA is not available. Please run on a GPU device.")
    logging.info(f"Using {torch.cuda.device_count()} GPUs for inference.")
    return [
        _InferCallable(
            device=f"cuda:{i}",
            model_id=detector_id,
            cache_dir=cache_dir,
        )
        for i in range(torch.cuda.device_count())
    ]


def main():
    # Argument parser for user-defined configurations
    parser = argparse.ArgumentParser(description="GroundedSam Triton Server")
    parser.add_argument(
        "--max-batch-size", type=int, default=4, help="Maximum batch size."
    )
    parser.add_argument(
        "--verbose", action="store_true", default=False, help="Enable verbose logging."
    )
    parser.add_argument(
        "--model-id",
        type=str,
        default="ratnaonline1/segFormer-b4-city-satellite-segmentation-1024x1024",
        help="Model ID.",
    )
    parser.add_argument(
        "--cache-dir",
        type=str,
        default=None,
        help="Directory to cache model weights.",
    )
    args = parser.parse_args()

    # Configure logging
    log_level = logging.DEBUG if args.verbose else logging.INFO
    logging.basicConfig(
        level=log_level, format="%(asctime)s - %(levelname)s - %(message)s"
    )

    # Start Triton Inference Server
    with Triton() as triton:
        logging.info("Loading models...")
        triton.bind(
            model_name="Segmenter",
            infer_func=multi_device_factory(
                detector_id=args.model_id,
                cache_dir=args.cache_dir,
            ),
            inputs=[Tensor(name="images", dtype=np.uint8, shape=(-1, -1, 3))],
            outputs=[
                Tensor(name="masks", dtype=np.uint8, shape=(-1, -1))  # Masks (H, W)
            ],
            config=ModelConfig(
                max_batch_size=args.max_batch_size,
                batcher=DynamicBatcher(max_queue_delay_microseconds=100),
            ),
            strict=True,
        )
        triton.serve()


if __name__ == "__main__":
    main()
