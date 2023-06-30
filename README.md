# ApplyEnv
## Introduction
ApplyEnv is a lightweight command-line utility designed to seamlessly integrate or replace environment variables with values passed through standard input (stdin) when launching a command.

It empowers developers, server administrators, and DevOps engineers by providing a simple way to control the execution environment when launching commands or applications.

With ApplyEnv, you can effortlessly adapt and fine-tune settings without modifying scripts or resorting to temporary shell variables.

Whether you need to dynamically configure paths, override configuration options, or inject runtime parameters, ApplyEnv simplifies the process and enables you to achieve greater flexibility in managing your environment.

## Installation
Currently, ApplyEnv must be built from source. Clone the git repository and run `make`. The binary will appear in the root of the repository. 

## Usage
```
$ applyenv <your_command>
```
To run a command with ApplyEnv, simply prepend `applyenv` to your command.

```
$ applyenv --help
```
Running ApplyEnv with the `--help` flag will yield information about ApplyEnv and its arguments.

### Arguments
Any arguments after your command will be passed on verbatim.

ApplyEnv will attempt to parse anything starting with `-` following the `applyenv` invocation; the first item without a `-` will be considered to be your command. Everything after that will be passed to your command as args.

**Example**  
Imagine your command is called "baz".
```
$ applyenv --foo -b -a -r baz bux --duck --duck --goose
```
ApplyEnv will see:
```
$ applyenv --foo -b -a -r <your_command...>
```
Your app will only see:
```
$ baz bux --duck --duck --goose
```

### Input
The environment variables you wish to set or modify are passed via the STDIN after launching the command.

#### Format
`examples/specExample.env`
```bash
# # # # # # # # # # # # # # 
#                         #
##  STDENV Spec Example  ##
#                         #
# # # # # # # # # # # # # #

# The spec is based on the dotenv file format
# specified by the Dotenvy spec (Version: 0.8.0)
# https://hexdocs.pm/dotenvy/dotenv-file-format.html

# Key-Value pair
HELLO=WORLD

# Optional Export lead
export INCLUDED=This works!

# Key Format: [A-Za-z_]+[0-9A-Za-z_]*
th15_iS_vALiD= But key MUST NOT start with a digit.

# Values:
SIMPLE_VALUE= Trims spaces #and inline comments

SINGLE_VALUE= 'Is surrounded by single quotes. Escapes are processed, but no interpolation.'

DOUBLE_VALUE= "Is surrounded by double quotes. Escapes are processed. Interpolation: ${INCLUDED}"

MULTILINE_SINGLE='''
This is the first line.
This is the secont line.
Escapes are performed.   # This is not a comment; it will be included verbatim.
Interpolation ${APPEARS_VERBATIM},
Just like the single line variant.
''' # This ends the multiline. Only spaces and comments allowed after it.

MULTILINE_DOUBLE="""
This is the first line.
This is the second.
Like its single-line counterpart, interpolation: ${INCLUDED}
For single and double-quoted multiline values, the line ending on this line is trimmed.
"""

# Escape Sequences:
New_Line= \n
Carriage_Return= \r
Tab= \t
Form_Feed= \f
Backspace= \b
Double_Quote= \"
Single_Quote= \'
Backslash= \\
Unicode= \u0024 is a dollar sign
Others= Like its Dotenvy counterpart, stdenv treats all other escaped characters literally.
```
### Special Considerations:

- The total size of the input must not exceed available system memory.

- The max value size is not defined, however limits may be defined elsewhere in the kernel.

- The max size for a key is 256 bytes. Anything larger will be truncated.

- Escapes are processed before substitutions
    - `\u0024\u007BINCLUDED\u007D` will output `This works!` as it will be interpolated from the previously defined variable: `INCLUDED`.

- Substitutions are taken directly from the actual environment variables, for example:
    - `${USER}` >> Your Username
    - `${PWD}` >> Current working directory
    - (Run `env` to see others. Differs per machine.)

- Lines are processed immediately, so previous lines are available for substitution in subsequent ones.

- Whitespace is trimmed wherever possible. Technically, there should be no space between the variable name and the equals sign, but this parser accepts it. (Recommend doing it properly though.)

### Invalid but functional
```bash
   export   this  =  technically invalid, but still works. # => this=technically invalid, but still works.

Multiline_invalid='''This is also invalid, but will work.'''

THIS_VARNAME_IS_WAY_TOO_LONG_AND_WILL_BE_TRUNCATED_AT_256_CHARACTERS_AND_AS_SUCH_DEMONSTRATING_THIS_REQUIRES_A_LOT_OF_SCREEN_SPACE_BUT_ITS_THE_UNIX_SPEC_APPARENTLY_AND_WHO_KNOWS_WHY_ANYONE_WOULD_ACTUALLY_WANT_TO_USE_A_KEY_NAME_THIS_LONG_BUT_WE_SHOULD_NEVERTHELESS_CHECK_EVEN_THOUGH_WE_DONT_REALLY=use variables with names this long!

```

### Example
All of the above formatting information can be found in `examples/specExample.env` and can be tested with:
```bash
$ applyenv env < examples/specExample.env
```

## Backstory
While pondering the security implications of passing secret information to an application, it didn't seem all that easy to pass environment variables without them appearing somewhere on the system: In a shell script as exports; or in bash command history if prepended to the command. I wondered how cloud providers might accomplish it and thought they must have a way to inject them somehow.

Wanting to try to make something similar, if not for practical use then just to see if I could, I decided to do so. After figuring out that most interpreted languages require you to start a sub-process to pass environment variables, I decided to attempt doing it in C, which allows you to completely replace your program in memory with another, thus not wasting resources unnecessarily.

As for the security aspects for which I originally had the ides, it can be paired with a launcher of some kind which fetches secrets from HashiCorp's Vault software or other secrets management system and then pipes it in via ApplyEnv.

Environment variables in general can be read from a running process by typing `ps aux -e|grep <the_name_of_the_target_process_or_env_var>` on the command line, but should only be visible to the user who started the process or to Root. Likewise, on Linux, the information can be found in `/proc`, subject to the same restrictions. If you are developing an app, it may be a small extra measure to wipe any sensitive information from your ENV variables once you have read it, leaving only options such as `ptrace()` for reading them from your actual program. Also, secrets which are only needed once should likewise be unset/wiped so that they only live in your applications memory for the duration that they are actually needed.

## See Also
- [CONTRIBUTING](CONTRIBUTING.md)
- [AUTHORS](AUTHORS.md)
- [LICENSE](LICENSE)
