## Testing
To run the automated unit tests, simply:
```bash
$ applyenv --systemTest
```
(This feature is not documented in `--help`.)

## Issues
With a codebase this small and specific, I actually don't anticipate anyone will create issues or pull requests, but if anyone does use it and wants to, great! Thanks in advance! If you wish to submit issues, feature requests or bug reports, please feel free to open an issue on github.

## Code Quality
I'm sorry. This is my first C project and I was learning as I went. There are nonsensical comments in places, stuff which could be done or at least formatted better, stuff not in use, and stuff doubled. I must say writing a parser in a new language was a challenge, but it works and the tests pass. Hopefully I'll clean it up before anyone actually reads this, or the code.

Known specific things:
- The --parserV2 flag is not used. It was used to toggle the advanced .env format parser as opposed to the simple parser.
- The simple_parser() function is never called from anywhere. My intention is to slightly or more drastically revamp v2 so it can be used without a whole file, and then revamp simple_parser to function more interactively when used with a tty attached rather than piping in data from another program or a file.

- Piping data from a file is problematic, not only because that leaves env traces in the file system, but also because if a command also needs data piped via stdin, we have a problem because my app doesn't know the data is finished if you pipe two files. Doing so via another program, that program can send the CTRL+D itself and the next data will go to the next program, but via the shell it just doesn't work. My thought is to add a flag with a value like `--stop-on [end_of_file_identifier]` so that a `// EOF` comment or the like can tell it to leave the rest for the next app.

## Licensing
[![CLA assistant](https://cla-assistant.io/readme/badge/alt-jero/ApplyEnv)](https://cla-assistant.io/alt-jero/ApplyEnv)  
In looking a bit into the licensing aspect of open source projects and contributions, I've found out that there's quite a bit of legal stuff to consider. As such, I've adapted a contributor license agreement based on those used by Goole, The Apache Foundation, and Okta, found here: [Individual Contributor License Agreement](CLA.INDIVIDUAL.md) and summarized as follows:

1. Your contribution remains yours, but you grant the right to use, modify, redistribute, [...], and sublicense it.

2. You grant right to use any of your patents implemented in the code of your contribution.

3. The project as it stands is released as open source under the GNU General Public License. That may change, but any contribution retains the license of the commit on which it is based as well as all subsequent commits in which it exists.

4. In the unlikely event that some megacorporation somewhere wants to offer a bajillion dollars to use this privately (one can dream) the intention is that the bajillion dollars would be fairly distributed among contributors.

5. The license, though containing lots of legalese, is discussable.