// Renders every page with sample data to build/preview/<page>.pbm so layouts can
// be inspected without flashing: pio test -e native -f test_preview
// then: uv run --with pillow tools/pbm2png.py build/preview/clock.pbm build/preview/clock.png
#include <unity.h>
#include <filesystem>
#include <fstream>
#include "ClockPage.h"

void setUp() {}
void tearDown() {}

static bool writePbm(const char *name, Canvas &c) {
  std::filesystem::create_directories("build/preview");
  std::ofstream f(std::string("build/preview/") + name + ".pbm", std::ios::binary);
  f << "P4\n" << c.width() << " " << c.height() << "\n";
  f.write(reinterpret_cast<const char *>(c.getBuffer()), ((c.width() + 7) / 8) * c.height());
  return f.good();
}

void test_preview_clock() {
  Canvas c(kW, kH);
  c.fillScreen(0);
  renderClock(c, {true, 14, 41, 4, 30, 10});
  TEST_ASSERT_TRUE(writePbm("clock", c));
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_preview_clock);
  return UNITY_END();
}
