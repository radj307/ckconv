<h1 align="center">Bethesda Engine Unit Converter</h1>
<br />
<a href="https://github.com/radj307/ckconv/tags"><p align="center"><img alt="GitHub tag (latest by date)" src="https://img.shields.io/github/v/tag/radj307/ckconv?color=ffffff&label=Current%20Version&logo=github&style=for-the-badge"></a>&nbsp&nbsp<a href="https://github.com/radj307/ckconv/releases"><img alt="GitHub all releases" src="https://img.shields.io/github/downloads/radj307/ckconv/total?color=ffffff&logo=github&style=for-the-badge"></p></a>
  

  
<p align="center">This is a commandline utility that converts between common length measurement units & Bethesda's eponymous <a href="https://www.creationkit.com/index.php?title=Unit"><i>Unit</i></a></p>
  
## Features
- Cross-Platform:
  - Windows
  - Linux
  - macOS
- Supports Metric, Imperial, and Creation Kit measurement units
- Can be embedded in other programs


If you're new to using CLI programs, see the [beginners guide](https://github.com/radj307/ckconv/wiki#detailed-usage-guide-for-less-experienced-terminal-users).

## Installation
  No formal installation is required, simply place the executable somewhere and call it from your preferred shell.
  
  If you want to be able to use the program no matter your shell's current working directory, you'll need to place the executable in a location found in your `PATH` environment variable.  
  
  The program is small enough to embed within another executable, for an example see [this synthesis patcher](https://github.com/radj307/Metric-Units-Patcher).

### Adding to your PATH variable

  #### Linux
  Place the executable in `/usr/local/bin` to add to your path, or add `export PATH='$PATH;<PATH_TO_DIRECTORY>'` to the end of `~/.bashrc`
  
  #### Windows
  [See this stackoverflow answer](https://stackoverflow.com/a/9546345/8705305)
  
  #### macOS
  I genuinely have no idea as I have never owned or used a mac, but I assume it's similar to Linux.
  
## Usage  
  Use `ckconv -h` or `ckconv --help` to see a usage guide, and documentation on all of the available options.
  
  Use `ckconv --units` to see a list of all recognized units.  
  When specifying units, you can use the values in the `Symbol` or `Name` columns.
  
  ### Examples
  Simple conversions, and pipe operators:  
  ![](https://i.imgur.com/eoeCV8t.png)
  ![](https://i.imgur.com/djxJO0t.png)
  ![](https://i.imgur.com/AH01PU6.png)
  ![](https://i.imgur.com/djxJO0t.png)
  ![](https://i.imgur.com/02IlBID.png)  
  
  Multiple conversions at once:  
  ![](https://i.imgur.com/TPmpR1W.png)
  ![](https://i.imgur.com/djxJO0t.png)
  ![](https://i.imgur.com/WvhAz51.png)
  
  Converting 5 picometers to units, printed in standard notation:  
  ![](https://i.imgur.com/HoykBM7.png)
  ![](https://i.imgur.com/fLj1yvU.png)  
  
  With [v6.0.0](https://github.com/radj307/ckconv/releases/6.0.0) comes less-strict syntax support. That means you can use expressions like `260m u` or `260 m u` instead of the more strict `<UNIT> <VALUE> <OUTPUT_UNIT>` syntax required by previous versions.  
