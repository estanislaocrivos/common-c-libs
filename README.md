# Shared C Libraries

Shared C libraries for embedded firmware projects.

## Libraries

| Library        | Description                                   |
| -------------- | --------------------------------------------- |
| `buffer`       | Linear byte buffer with index tracking        |
| `ring-buffer`  | Circular buffer for streaming data            |
| `crc`          | CRC-8 calculator with configurable polynomial |
| `framing`      | Frame parser/builder for serial protocols     |
| `embedded-hal` | Hardware abstraction layer interfaces         |
| `logging`      | Lightweight logging utility                   |
| `drivers/`     | Hardware-specific drivers                     |

## Libraries Repository Structure

```text
libraries/c/
├── inc/                  # Shared headers (errno.h)
├── buffer/
│   ├── inc/              # Public headers
│   ├── src/              # Implementation
│   └── test/             # Unit tests
├── ring-buffer/
├── crc/
├── framing/
└── embedded-hal/
...
```

Each public header file must have all its public API documented using Doxygen comments.

## Contributing

## Prerequisites

| Tool                                                             | Version | Purpose                |
| ---------------------------------------------------------------- | ------- | ---------------------- |
| [Ruby](https://www.ruby-lang.org/en/documentation/installation/) | ≥ 3.0.0 | Ceedling runtime       |
| [Python](https://www.python.org/downloads/)                      | ≥ 3.8   | Pre-commit hooks       |
| [GCC](https://gcc.gnu.org/install/)                              | ≥ 9.0   | C compiler for testing |

### Coding Style

Based on Linux kernel, Barr Group Embedded C, and MISRA-C guidelines.

**Formatting** (enforced by `.clang-format`):

- 4-space indentation, no tabs
- Allman brace style
- 100-column limit
- Pointer alignment left: `int* ptr`

**Naming**:

| Element       | Style               | Example                                |
| ------------- | ------------------- | -------------------------------------- |
| Functions     | `module_action`     | `buffer_push`, `framing_init`          |
| Variables     | `snake_case`        | `payload_size`, `was_initialized`      |
| Structs/Enums | `struct snake_case` | `struct buffer`, `enum gpio_direction` |
| Enum values   | `UPPER_SNAKE`       | `FRAMING_START_STATE`, `GPIO_INPUT`    |
| Macros        | `UPPER_SNAKE`       | `SERIAL_DEFINE`, `EFAULT`              |

**Error handling**:

- Return `int8_t`: `0` = success, `-ERRNO` = failure
- Validation order: NULL check (`-EFAULT`) → initialized check (`-EPERM`) → value check (`-EINVAL`)

**Struct design**:

```c
struct module
{
    /* public: set before init */
    uint8_t* const buffer;
    const size_t   size;

    /* private: do not access directly */
    bool was_initialized;
};
```

**Key principles**:

- Explicit `_init()` required before use.
- No dynamic allocation — user provides all buffers.
- Dependency injection via struct fields, not globals.
- `const` for immutable config fields.

### Development Setup

#### Create a Python Virtual Environment

```bash
cd libraries/c/
python -m venv .venv
source .venv/bin/activate
```

Install required Python packages:

```bash
pip install -r requirements.txt
```

#### Install Ceedling

Unit testing framework for C.

- **Docs:** <https://www.throwtheswitch.org/ceedling>

```bash
gem install ceedling
```

#### Install Pre-commit Hooks

Git hooks for code quality checks.

- **Docs:** <https://pre-commit.com>

```bash
cd libraries/c/
source .venv/bin/activate
pre-commit install --install-hooks -t pre-commit -t commit-msg
```

#### Install Doxygen (optional)

Documentation generator.

- **Docs:** <https://www.doxygen.nl/manual/install.html>

```bash
sudo apt install doxygen        # Debian/Ubuntu
sudo dnf install doxygen        # Fedora
sudo pacman -S doxygen          # Arch
```

## Testing

Each library has its own Ceedling project. Navigate to the library directory first:

```bash
cd <library>/   # e.g., cd buffer/
```

| Command                | Description              |
| ---------------------- | ------------------------ |
| `ceedling test:all`    | Run all tests            |
| `ceedling test:<name>` | Run specific test        |
| `ceedling gcov:all`    | Generate coverage report |

## Generating Documentation

Generate documentation using Doxygen:

```bash
cd libraries/c/
doxygen Doxyfile
```
