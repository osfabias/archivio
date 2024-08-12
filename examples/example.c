#include <arch/arch.h>

int main() {
  const ArchLoggerCreateInfoEx info = {
    .name = "logger",
    .pathFormat = "./my-logs/",
    .fileNameFormat = "#d.#M.#y_#h:#m:#s.txt",
    .messageFormats = {
      "\e[36m ~ #{\e[3m#t#}\n",
      "#{ ℹ#} #t\n",
      "#{ ✓#} #t\n",
      "\n\e[37m[#h:#m:#s]#} #{ WARNING #} #t\n\n",
      "\e[31m ✗#} #t\n",
      "\n\e[37m[#h:#m:#s]#} #{  FATAL  #} #t\n\n",
    },
    .level = ARCH_LOG_LEVEL_TRACE,
  };

  ArchLogger *logger = archLoggerCreateEx(&info);
  if (!logger) { return 1; }

  for (int i = 0; i < 10; ++i) {
    archLog(logger, i % ARCH_LOG_LEVEL_MAX_ENUM, "message %d", i);
  }

  archLoggerDestroy(logger);

  return 0;
}
