import argparse
import logging
import os
from pathlib import Path
import random
from typing import Callable

logger = logging.getLogger(__name__)

# --- CONFIG ---

DEFAULT_OUTPUT_PATH = Path.cwd() / "patterns"
DEFAULT_PATTERN_FREQ = 5
DEFAULT_MAX_PATTERN_LEN = 16
DEFAULT_MIN_PATTERN_LEN = 3
DEFAULT_REC = False


class PaternGenerator:
    def __init__(
        self,
        path: Path,
        rec: bool = DEFAULT_REC,
        output_path: Path = DEFAULT_OUTPUT_PATH,
        pattern_count: int = DEFAULT_PATTERN_FREQ,
        pattern_max_len: int = DEFAULT_MAX_PATTERN_LEN,
        pattern_min_len: int = DEFAULT_MIN_PATTERN_LEN,
    ) -> None:
        self.path = path
        self.rec = (rec,)
        self.output_path = output_path
        self.pattern_count = pattern_count
        self.pattern_max_len = pattern_max_len
        self.pattern_min_len = pattern_min_len

    def _generate_all(self):
        pass

    def _generate_patterns(self, path: Path) -> list[str]:
        with open(path, "r") as f:
            lines = [
                line.strip() for line in f if len(line.strip()) >= self.pattern_min_len
            ]
            patterns = []

            for _ in range(self.pattern_count):
                line = random.choice(
                    [l for l in lines if len(l) >= self.pattern_min_len]
                )
                length = random.randint(
                    self.pattern_min_len, min(self.pattern_max_len, len(line))
                )
                start = random.randint(0, len(line) - length)
                patterns.append(line[start : start + length])

        return patterns

    def _pattern_generator(self):
        logger.info("Generation started!")
        with open(self.output_path, "w") as f:
            for file_path in Path(self.path).rglob("*"):
                if file_path.is_file():
                    f.write("\n".join(self._generate_patterns(file_path)))
                    logger.info(f"Finished: {file_path}")

    def run(self) -> None:
        self._pattern_generator()


def arg_parser() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        prog="Pattern generator",
        description="""Generates random patterns from files in given directory""",
    )
    parser.add_argument(
        "--output",
        "-o",
        type=Path,
        default=DEFAULT_OUTPUT_PATH,
        help='Overwrites default output path (default: "./patterns")',
    )
    parser.add_argument(
        "--min-len",
        type=int,
        default=DEFAULT_MIN_PATTERN_LEN,
        help="Sets minimum pattern length (default: 3)",
    )
    parser.add_argument(
        "--max-len",
        type=int,
        default=DEFAULT_MAX_PATTERN_LEN,
        help="Sets maximum pattern length (default: 80)",
    )
    parser.add_argument(
        "-c",
        "--count",
        type=int,
        default=DEFAULT_PATTERN_FREQ,
        help="Generated patterns count (default: 5)",
    )
    parser.add_argument(
        "-d",
        "--directory",
        type=Path,
        default=DEFAULT_REC,
        help="Recursivly generates from given directory",
    )

    return parser.parse_args()


if __name__ == "__main__":
    logging.basicConfig(level=logging.INFO, format="%(levelname)s: %(message)s")
    args = arg_parser()
    generator = PaternGenerator(
        path=args.directory,
        output_path=args.output,
        pattern_count=args.count,
        pattern_max_len=args.max_len,
        pattern_min_len=args.min_len,
    )
    generator.run()
