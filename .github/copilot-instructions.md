# Awen – Copilot Instructions

## C++ Style

### Naming (from `.clang-tidy` `readability-identifier-naming`)
- Types, structs, enums, concepts: `CamelCase`
- Functions, methods, variables, constants, parameters, namespaces: `lower_case` (snake_case)
- Private and protected member variables: `lower_case_` (trailing `_` suffix)
- Public member variables: `lower_case` (no suffix)

### Types & Declarations
- Prefer `auto` everywhere a type can be deduced — local variables, `constexpr` constants, loop variables, etc. (e.g., `constexpr auto init_width = 1280;`, `const auto dt = GetFrameTime();`)
- Use trailing return types on function definitions (e.g., `auto foo() -> int`)
- Use `const` wherever a variable is not mutated

### Comments
- Never place a comment on the same line as code — always put comments on their own line above the code they describe
- Always add a blank line before a comment block when it is preceded by a non-blank line of code
- Comments must be complete sentences (start with a capital letter, end with a period)
- Use a paragraph comment above a block of related statements rather than commenting each line individually
- Document all classes, structs, and public functions with Doxygen `///` comments using `@brief`, `@param`, `@return`, `@note`, etc.

### Formatting (from `.clang-format`)
- Brace style: Allman — opening braces on their own line
- Indent width: 4 spaces, no tabs
- Line length limit: 150 columns
- Pointer aligned left: `int* ptr`, not `int *ptr`
- Always use braces `{}` on `if`/`else`/`for`/`while` bodies, even single-line — no short statements on a single line
- Sort `#include` directives; separate definition blocks with a blank line
- Always add a blank line after a closing `}` before the next line of code, unless that next line is itself a closing `}` or `}`-else chain
- Always add a blank line before `if`, `for`, and `while` statements when they are preceded by a non-blank line of code

### Structs & Initialization
- Use designated initializers for aggregate initialization, including inline struct literals passed as arguments (e.g., `Vector2{.x = 1.0F, .y = 2.0F}`)
- Zero-initialize POD members (numeric types, enums, pointers, bools) with `{}` in struct definitions (e.g., `float x{};`)
- Do **not** add `{}` to members of non-trivial class types (e.g., `std::string text;`, `std::vector<T> items;`) — they have their own default constructors and an empty initializer is redundant (`readability-redundant-member-init`)
- Multi-line initializer lists must have a trailing comma after the last element; single-line lists must not

### Operators
- Prefer `auto operator<=>(const T&) const noexcept = default` over `operator==` when equality or ordering is needed — the spaceship operator synthesises all six comparison operators including `==`
- Always `#include <compare>` in the global module fragment when using `operator<=>` in a C++ module (`.ixx`) file

### Literals
- Float literals use the `F` suffix (e.g., `0.0F`, `1.05F`)
- Prefer `static_cast<float>(…)` over C-style casts

### Standard Library Usage
- Prefer non-member free functions over container member functions where equivalents exist: `std::size()`, `std::empty()`, `std::begin()`, `std::end()`, `std::data()`, etc.
- Always use `std::visit` with `awn::Overloaded` (from `awen.overloaded`) when dispatching `std::variant` alternatives — never use a single generic lambda with `if constexpr` chains.

### Functions
- Keep functions small and focused on a single responsibility
- Pass large objects by reference or const-reference; avoid unnecessary copies
- Prefer early returns and guard clauses over deeply nested branches
- For template callable parameters (`F`): use `const F&` when the callable is invoked more than once (including across recursive calls); only use `F&&` when the callable will be forwarded exactly once with `std::forward<F>(f)` (`cppcoreguidelines-missing-std-forward`)
- Prefer iterative tree traversal over recursive helpers to avoid `misc-no-recursion` warnings and unbounded call-stack growth

## Build & Project
- Build system: CMake with Ninja, presets defined in `CMakePresets.json`
- Package manager: vcpkg (manifest mode, `vcpkg.json`)
- Primary graphics library: raylib

## CMake Style
- Place `find_package()` calls immediately before the `target_link_libraries()` / `target_include_directories()` / `target_compile_definitions()` calls that consume them — not at the top of the file
