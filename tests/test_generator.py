import argparse
import logging
from abc import ABC, abstractmethod
from pathlib import Path
import random
import string

logger = logging.getLogger(__name__)

# --- CONFIG ---
DEFAULT_OUTPUT_DIR = Path.cwd() / "test_files"
DEFAULT_SIZE_CATEGORIES = {
    "small": (1 * 1024, 8 * 1024),        # 1-8 KB
    "medium": (64 * 1024, 512 * 1024),    # 64-512 KB
    "large": (1 * 1024 * 1024, 16 * 1024 * 1024),  # 1-16 MB
}
DEFAULT_WORD_DICTIONARY_PATH = Path("/usr/share/dict/words")
DEFAULT_LINE_LEN = 80
DEFAULT_MIN_WORD_LEN = 3
DEFAULT_FILE_COUNT = "5,3,1"
SUPPORTED_MODES = ["all", "random", "words"]
DEFAULT_MODE = "all"


class FileGenerator(ABC):
    _registry: dict[str, type["FileGenerator"]] = {}

    def __init_subclass__(cls, *, name: str | None = None, **kwargs):
        super().__init_subclass__(**kwargs)
        if name is not None:
            FileGenerator._registry[name] = cls

    def __init__(
        self,
        output_dir: Path = DEFAULT_OUTPUT_DIR,
        file_count: str = DEFAULT_FILE_COUNT,
        line_len: int = DEFAULT_LINE_LEN,
        mode: str = DEFAULT_MODE,
        **kwargs,
    ):
        self.output_dir = output_dir
        self.line_len = line_len
        self._mode = self._parse_mode(mode)
        self._file_count = self._parse_file_count(file_count)
        self._size_range = DEFAULT_SIZE_CATEGORIES

    @abstractmethod
    def generate(self, path: Path, size: int) -> None:
        pass

    def _parse_mode(self, new_mode: str) -> str:
        if new_mode not in SUPPORTED_MODES and new_mode != "all":
            logger.warning("Invalid mode given, default value will be used!")
            return DEFAULT_MODE
        return new_mode

    def _parse_file_count(self, file_count: str) -> tuple[int, int, int]:
        try:
            return tuple(map(int, file_count.split(",")))
        except (ValueError, AttributeError):
            logger.warning(
                f"Invalid file count format '{file_count}', default {DEFAULT_FILE_COUNT} will be used!"
            )
            return tuple(map(int, DEFAULT_FILE_COUNT.split(",")))

    @staticmethod
    def _file_generator_static(
        output_dir: Path,
        generators: dict[str, "FileGenerator"],
        file_count: str,
    ) -> None:
        size_range = DEFAULT_SIZE_CATEGORIES
        try:
            s, m, l = map(int, file_count.split(","))
        except Exception:
            s, m, l = map(int, DEFAULT_FILE_COUNT.split(","))
        file_counts = (s, m, l)

        for gen_name, gen in generators.items():
            logger.info(f"{gen_name.capitalize()} generation started!")

            for idx, (size, size_range_tuple) in enumerate(size_range.items()):
                for i in range(file_counts[idx]):
                    f_path = output_dir / f"{gen_name}/{size}/{i}"
                    f_size = random.randint(*size_range_tuple)

                    f_path.parent.mkdir(parents=True, exist_ok=True)
                    gen.generate(f_path, f_size)

                logger.info(f"{gen_name.capitalize()} {size.capitalize()} generation finished!")

    @classmethod
    def run(cls, output_dir, file_count, line_len, mode, **kwargs):
        if mode == "all":
            selected = cls._registry
        else:
            if mode not in cls._registry:
                raise ValueError(f"No generator registered for mode '{mode}'")
            selected = {mode: cls._registry[mode]}

        generators = {
            name: gen_cls(output_dir=output_dir, line_len=line_len, **kwargs)
            for name, gen_cls in selected.items()
        }

        cls._file_generator_static(output_dir, generators, file_count)



class RandomGenerator(FileGenerator, name="random"):
    def generate(self, path: Path, size: int) -> None:
        path = path.with_suffix(".txt")
        pfile_path = path.with_name(path.stem + "_patterns.txt")

        patterns_len = int(size ** (2 / 3) / self.line_len) + 3
        count = max(1, patterns_len // 3)

        extra_lines = {self._generate_line(self.line_len) for _ in range(count)}
        patterns = set(extra_lines)

        with open(path, "w") as f:
            written = 0
            for i in range(size // self.line_len):
                line = self._generate_line(min(self.line_len, size - written))
                if line not in extra_lines:
                    f.write(line + "\n")
                    written += len(line) + 1

                if i % patterns_len == 0 and len(line) > 1:
                    start = random.randint(0, len(line) - 1)           
                    end = random.randint(start + 1, len(line))         
                    patterns.add(line[start:end])

        with open(pfile_path, "w") as f:
            f.writelines(p + "\n" for p in patterns)

    def _generate_line(self, length: int) -> str:
        return "".join(
            random.choices(
                string.ascii_letters + string.digits + string.punctuation + " ",
                k=length,
            )
        )



class WordGenerator(FileGenerator, name="words"):
    def __init__(
        self,
        *,
        output_dir: Path,
        line_len: int,
        word_dict_path: Path = DEFAULT_WORD_DICTIONARY_PATH,
        min_word_len: int = DEFAULT_MIN_WORD_LEN,
    ):
        super().__init__(output_dir=output_dir, line_len=line_len)
        self._word_dict_path = word_dict_path
        self._min_word_len = min_word_len
        self._words: list[str] | None = None

    def generate(self, path: Path, size: int) -> None:
        if self._words is None:
            self._words = self._load_word_dict()

        path = path.with_suffix(".txt")
        pfile_path = path.with_name(path.stem + "_patterns.txt")

        patterns_len = int(size ** (2 / 3) / self.line_len) + 3
        count = max(1, patterns_len // 3)

        excluded_words = set(random.choices(self._words, k=count))
        patterns = set(excluded_words)

        with open(path, "w") as f:
            for i in range(size // self.line_len):
                line = self._generate_line(self.line_len, excluded_words)
                f.write(line + "\n")

                if i % patterns_len == 0 and len(line) > 1:
                    start = random.randint(0, len(line) - 1)           
                    end = random.randint(start + 1, len(line))         
                    patterns.add(line[start:end])

        with open(pfile_path, "w") as f:
            f.writelines(p + "\n" for p in patterns)

    def _load_word_dict(self) -> list[str]:
        try:
            with open(self._word_dict_path, "r") as f:
                return [
                    line.strip()
                    for line in f
                    if line.strip().isalpha()
                    and len(line.strip()) >= self._min_word_len
                ]
        except FileNotFoundError:
            logger.error(f"Word Dictionary not found at {self._word_dict_path}")
            raise

    def _generate_line(self, max_len: int, excluded_words: set[str]) -> str:
        current_len = 0
        parts: list[str] = []

        while True:
            word = random.choice(self._words)
            if word in excluded_words:
                continue

            sep = 1 if parts else 0
            if current_len + len(word) + sep > max_len:
                break

            parts.append(word)
            current_len += len(word) + sep

        return " ".join(parts)



def arg_parser() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        prog="File Generator",
        description="Generates random character and word test files",
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
                words - Generates random text from word dictionary""",
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

    FileGenerator.run(
        output_dir=args.output,
        file_count=args.count,
        line_len=args.line_len,
        mode=args.mode,
        word_dict_path=args.dictionary_path,
        min_word_len=args.word_len,
    )
