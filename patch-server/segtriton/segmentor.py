from typing import Optional, Tuple

import torch
from PIL import Image
from transformers import SegformerForSemanticSegmentation, SegformerImageProcessor


class Segmenter:
    def __init__(
        self,
        model_id: str = "ratnaonline1/segFormer-b4-city-satellite-segmentation-1024x1024",
        device: str = "cuda" if torch.cuda.is_available() else "cpu",
        cache_dir: Optional[str] = None,
    ):
        self.model = SegformerForSemanticSegmentation.from_pretrained(
            model_id, cache_dir=cache_dir
        ).to(device)
        self.processor = SegformerImageProcessor.from_pretrained(
            model_id, cache_dir=cache_dir
        )
        self.device = device

    @torch.no_grad()
    def predict(self, image: Image) -> Tuple[torch.Tensor]:
        """
        Generate masks and return masks.

        Args:
            image: Input image

        Returns:
            logits: Logits tensor (height/4, width/4, num_labels)
            masks: Mask tensor (height, width)
        """

        inputs = self.processor(images=image, return_tensors="pt").to(self.device)
        outputs = self.model(**inputs)
        return (
            outputs.logits[0],
            self.processor.post_process_semantic_segmentation(
                outputs, target_sizes=[image.size[::-1]]
            )[0],
        )
