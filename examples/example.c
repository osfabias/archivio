#include <stdio.h>
#include <stdlib.h>

#include <arch.h>

int main(void) {
  const arch_logger_create_info_t logger_info = {
    .path_fmt = "./mylogs/",
    .filename_fmt = "logs.txt",
    .msg_fmts = {
      "\e[36m ~ \e[37;3m#t#}\n",
      "\e[34m ℹ#} #t\n",
      "\e[32m ✓#} #t\n",
      "\n\e[37m[#h:#m:#s]#} \e[1;30;43m WARNING #} #t\n\n",
      "\e[31m ✗#} #t\n",
      "\n\e[37m[#h:#m:#s]#} \e[1;97;41m  FATAL  #} #t\n\n",
    },
    .file_msg_fmts = {
      "#h:#m:#s | TRACE | #t\n",
      "#h:#m:#s | DEBUG | #t\n",
      "#h:#m:#s | INFO  | #t\n",
      "#h:#m:#s | WARN  | #t\n",
      "#h:#m:#s | ERROR | #t\n",
      "#h:#m:#s | FATAL | #t\n",
    },
    .level = ARCH_LOG_LEVEL_TRACE,
  };

  // printing messages
  arch_logger_t logger, logger2;
  if (!(
    (logger  = arch_logger_create(&logger_info)) &&
    (logger2 = arch_logger_create(&logger_info))
  )) {
    puts("Failed to create loggers.");
    return EXIT_FAILURE;
  }

  for (int i = 0; i < 10; ++i) {
    arch_log(logger, i % ARCH_LOG_LEVEL_MAX_ENUM, "message %d", i);
    arch_log(logger2, i % ARCH_LOG_LEVEL_MAX_ENUM, "message %d", i + 100);
  }

  arch_logger_destroy(logger);
  arch_logger_destroy(logger2);

  return EXIT_SUCCESS;
}
