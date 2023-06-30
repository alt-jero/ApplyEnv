#include <stdio.h>
#include "help.h"
#include "version.h"

int print_help_info(char *arg0) {
    printf(CLI_BANNER "\n");
    printf("\n");
    printf("Usage: %s [options] <binary_path> [args_for_binary]\n", arg0);
    printf("\n");
    printf("Options:\n");
    printf("--help: Display this help message\n");
    printf("--hide: Hide typed input when entering env variables\n");
    printf("\n");
    printf("Explanation:\n");
    //       
    printf("ApplyEnv is a lightweight command-line utility designed to\n");
    printf("seamlessly integrate or replace environment variables with values\n");
    printf("passed through standard input (stdin) when launching a command.\n");
    printf("\n");
    printf("After launching the program, enter variables line-by-line in the normal\n");
    printf("KEY=VALUE format. Press [CTRL]+[D] when finished.\n");
    printf("\n");
    printf("- Leaving the value blank unsets the variable.\n");
    printf("- You can pass a .env file using the '<' operator.\n");
    printf("- You can also pass a shell script consisting solely of exports.\n");
    printf("\n");
    printf("  Example: \n");
    printf("  :: export KEY=VALUE\\n\n");
    printf("  :: export KEY2=VALUE\\n\n");
    printf("  :: KEY_WITHOUT_EXPORTS=VALUE\\n\n");
    printf("  :: UNSET_THIS_VARIABLE=\\n\n");
    printf("  :: THIS_ALSO_WORKS='''\\n\n");
    printf("  :: Value has multiple lines if you want...\\n\n");
    printf("  :: '''\\n\n");
    printf("  :: [Ctrl] + [D]\n");
    printf("\n");
    printf("  Your program starts here.\n");
    return -1;
}