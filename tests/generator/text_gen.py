import argparse
import logging
import os
from pathlib import Path
import random
import string
from typing import Callable

logger = logging.getLogger(__name__)

# --- CONFIG ---

DEFAULT_OUTPUT_DIR = Path.cwd() / "test_files"
DEFAULT_SIZE_CATEGORIES = {
    "small": (1 * 1024, 8 * 1024),  # 1-8 KB
    "medium": (64 * 1024, 512 * 1024),  # 64-512 KB
    "large": (1 * 1024 * 1024, 16 * 1024 * 1024),  # 1-16 MB
}
DEFAULT_WORD_DICTIONARY_PATH = Path("/usr/share/dict/words")
DEFAULT_LINE_LEN = 80
DEFAULT_MIN_WORD_LEN = 3
DEFAULT_FILE_COUNT = "5,3,1"
SUPPORTED_MODES = ["all", "random", "words"]
DEFAULT_MODE = "all"


class FileGenerator:
    def __init__(
        self,
        output_dir: Path = DEFAULT_OUTPUT_DIR,
        word_dict_path: Path = DEFAULT_WORD_DICTIONARY_PATH,
        file_count: str = DEFAULT_FILE_COUNT,
        line_len: int = DEFAULT_LINE_LEN,
        min_word_len: int = DEFAULT_MIN_WORD_LEN,
        mode: str = DEFAULT_MODE,
    ) -> None:
        self.output_dir = output_dir
        self.line_len = line_len
        self._word_dict_path = word_dict_path
        self.min_word_len = min_word_len
        self._mode = self._parse_mode(mode)
        self._file_count = self._parse_file_count(file_count)
        self._size_range = DEFAULT_SIZE_CATEGORIES
        self._words = None

    def _load_word_dict(self) -> list[str]:
        if self.min_word_len <= 0:
            logger.warning("Minimum word len must be > 0, default value will be used!")
            self.min_word_len = DEFAULT_MIN_WORD_LEN

        try:
            with open(self._word_dict_path, "r") as f:
                return [
                    line.strip()
                    for line in f
                    if line.strip().isalpha() and len(line.strip()) >= self.min_word_len
                ]
        except FileNotFoundError:
            logger.error(f"Word Dictionary not found at {self._word_dict_path}")
            raise FileNotFoundError("Missing Dictionary")

    def change_mode(self, new_mode: str) -> None:
        self._mode = self._parse_mode(new_mode)

    def _parse_mode(self, new_mode: str) -> str:
        if new_mode not in SUPPORTED_MODES:
            logger.warning("Invalid mode given, default value will be used!")
            return DEFAULT_MODE

        return new_mode

    def change_word_dict(self, new_path: Path) -> None:
        self._word_dict_path = new_path
        self._words = None

    def change_file_count(self, new_count: str) -> None:
        self._file_count = self._parse_file_count(new_count)

    def _parse_file_count(self, file_count: str) -> tuple[int, int, int]:
        try:
            s, m, l = map(int, file_count.split(","))
        except (ValueError, AttributeError):
            logger.warning(
                "Invalid format for -c, expected: '5,3,1', default value will be used!"
            )
            s, m, l = map(int, DEFAULT_FILE_COUNT.split(","))

        return (s, m, l)

    def _random_generator(self, path: Path, size: int) -> None:
        path = path.with_suffix(".txt")
        with open(path, "w") as f:
            written = 0

            while written < size:
                line_len = min(self.line_len, size - written)
                line = "".join(
                    random.choices(
                        string.ascii_letters + string.digits + string.punctuation + " ",
                        k=line_len,
                    )
                )
                f.write(line + "\n")
                written += len(line) + 1

    def _word_generator(self, path: Path, size: int) -> None:
        path = path.with_suffix(".txt")
        if self._words is None:
            self._words = self._load_word_dict()

        with open(path, "w") as f:
            written = 0
            current_line = ""

            while written < size:
                assert self._words is not None
                word = random.choice(self._words)

                if len(current_line) + len(word) + 1 > self.line_len:
                    f.write(current_line.rstrip() + "\n")
                    written += len(current_line) + 1
                    current_line = ""
                current_line += word + " "

            if current_line and written < size:
                f.write(current_line.rstrip() + "\n")
                written += len(current_line) + 1

    def _file_generator(
        self, generators: dict[str, Callable[[Path, int], None]]
    ) -> None:
        for gen_name, gen in generators.items():
            logger.info(f"{gen_name.capitalize()} generation started!")
            for idx, (size, size_range) in enumerate(self._size_range.items()):
                for i in range(self._file_count[idx]):
                    f_path = Path(self.output_dir / f"{gen_name}/{size}/{i}")
                    f_size = random.randint(*size_range)
                    f_path.parent.mkdir(parents=True, exist_ok=True)
                    gen(f_path, f_size)
                logger.info(
                    f"{gen_name.capitalize()} {size.capitalize()} generation finished!"
                )

    def run(self) -> None:
        match self._mode:
            case "all":
                self._file_generator(
                    {
                        "random": self._random_generator,
                        "words": self._word_generator,
                    }
                )
            case "random":
                self._file_generator({"random": self._random_generator})
            case "words":
                self._file_generator({"words": self._word_generator})


def arg_parser() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        prog="File Generator",
        description="Generates random character and text test files",
    )
    parser.add_argument(
        "--mode",
        "-m",
        type=str.lower,
        default=DEFAULT_MODE,
        choices=SUPPORTED_MODES,
        help="""Text generation mode
                all - Generates all file formats
                random - Generates random text from utf-8 characters
                word - Generates random text from word dictionary""",
    )
    parser.add_argument(
        "--output",
        "-o",
        type=Path,
        default=DEFAULT_OUTPUT_DIR,
        help='Overwrites default output directory (default: "./test_files")',
    )
    parser.add_argument(
        "--dictionary-path",
        "-d",
        type=Path,
        default=DEFAULT_WORD_DICTIONARY_PATH,
        help='Specifies default word dictionary path (default: "/usr/share/dict/words")',
    )
    parser.add_argument(
        "--count",
        "-c",
        type=str,
        default=DEFAULT_FILE_COUNT,
        help='Comma separated file count (small,medium,large) (default: "5,3,1")',
    )
    parser.add_argument(
        "--line-len",
        "-l",
        type=int,
        default=DEFAULT_LINE_LEN,
        help="Maximum line length for text file generation (default: 80)",
    )
    parser.add_argument(
        "--word-len",
        type=int,
        default=DEFAULT_MIN_WORD_LEN,
        help="Minimum word length in word text generation (default: 3)",
    )

    return parser.parse_args()


if __name__ == "__main__":
    logging.basicConfig(level=logging.INFO, format="%(levelname)s: %(message)s")

    args = arg_parser()
    generator = FileGenerator(
        output_dir=args.output,
        word_dict_path=args.dictionary_path,
        file_count=args.count,
        line_len=args.line_len,
        min_word_len=args.word_len,
        mode=args.mode,
    )
    generator.run()
