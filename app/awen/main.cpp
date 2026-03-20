#include <cstdlib>
#include <exception>
#include <print>

auto main() noexcept -> int
try
{
    std::println("Hello, Awen!");
    return EXIT_SUCCESS;
}
catch (const std::exception& e)
{
    return EXIT_FAILURE;
}