#include <cstdlib>
#include <exception>
#include <print>

auto main() noexcept -> int
try
{
    std::println("Hello, Awen!");
    return EXIT_SUCCESS;
}
catch (const std::exception& /*unused*/)
{
    // Can't print since it may throw, but we can log it to a file or something if needed.
    return EXIT_FAILURE;
}