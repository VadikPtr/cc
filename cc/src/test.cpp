#include "cc/test.hpp"
#include "cc/all.hpp"
#include <cstdarg>

namespace {
  struct TestCaseContainer {
    size_t    size  = 0;
    TestCase* cases = nullptr;
  };

  TestCaseContainer& get_test_case_container() {
    static TestCaseContainer container;
    return container;
  }
}  // namespace

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wformat-nonliteral"

TestCaseFail::TestCaseFail(const char* fmt, ...) : message(nullptr) {
  va_list args;

  va_start(args, fmt);
  int char_count = vsnprintf(nullptr, 0, fmt, args);
  va_end(args);

  if (char_count >= 0) {
    size_t buffer_size = (size_t)char_count + 1;
    message            = new char[buffer_size];
    memset(message, 0, buffer_size);
    va_start(args, fmt);
    vsnprintf(message, buffer_size, fmt, args);
    va_end(args);
  }
}

#pragma clang diagnostic pop

TestCaseFail::~TestCaseFail() {
  delete[] message;
}

const char* TestCaseFail::what() const noexcept {
  if (message) {
    return message;
  }
  return "<NO-MESSAGE>";
}

TestCase register_test_case(TestCase test_case) {
  auto& [size, cases] = get_test_case_container();
  if (cases == nullptr) {
    cases = (TestCase*)malloc(sizeof(TestCase));
  } else {
    cases = (TestCase*)realloc(cases, (size + 1) * sizeof(TestCase));
  }
  if (cases == nullptr) {
    fprintf(stdout, "Cannot allocate memory for test cases\n");
    fflush(stdout);
    abort();
  }
  cases[size++] = test_case;
  return test_case;
}

int tests_main(int argc, const char** argv) {
  Str filter;
  ProgOpts::add(ProgOpts::ArgumentStr{
      .long_name     = "filter",
      .short_name    = 'f',
      .help_argument = "Filter tests.",
      .value         = filter,
      .flags         = ProgOpts::Optional,
  });
  ProgOpts::parse(argc, argv);

  const auto& [size, cases] = get_test_case_container();
  fprintf(stdout, "Running tests. Cases: %" PRIu64 "\n", (u64)size);
  fflush(stdout);
  int passed = 0;
  int failed = 0;

  for (size_t i = 0; i < size; ++i) {
    const auto& [name, func] = cases[i];

    if (!filter.empty() && StrView(name).find(filter) == StrView::npos) {
      continue;
    }

    fprintf(stdout, "[%3" PRIu64 "/%3" PRIu64 "] %s... \n", (u64)i + 1, (u64)size, name);
    fflush(stdout);

    try {
      func();
      fprintf(stdout, "  Ok!\n");
      passed++;
    } catch (const Err& e) {
      fprintf(stdout, "  Fail on exception: %.*s\n", (int)e.message().size(),
              e.message().data());
      failed++;
    } catch (const std::exception& e) {
      fprintf(stdout, "  Fail on exception: %s\n", e.what());
      failed++;
    }

    fflush(stdout);
  }

  fprintf(stdout, "---\n");
  fprintf(stdout, "Passed: %d / Failed: %d\n", passed, failed);
  fprintf(stdout, "---\n");
  free(cases);
  return failed ? EXIT_FAILURE : EXIT_SUCCESS;
}
