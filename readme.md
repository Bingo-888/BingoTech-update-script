# BingoTech Mod Update Script

## Project Overview
This is a custom update script for the BingoTech modpack in Minecraft, created by the author. It can be used to update various aspects of the modpack, including mods, quests, configuration files, and even various modified codes.

## Project Features
- **Fully AI-Coded**: The entire project was developed using AI coding tools.
- **Version Updates**: Can be used to update major versions of the modpack.
- **Extension Packs**: Supports installing extension packs for the modpack.
- **MIT License**: Licensed under the MIT License.

## What is an Extension Pack
The core modpack includes only the main mods, quests, and modifications. Extension packs allow players with different needs to add extra content freely and based on their interests.

## Usage
1. Place the executable in a directory adjacent to the `mods_update` folder.
2. Ensure the Minecraft directory structure exists: `../.minecraft/versions/BingoTech/mods`
3. Run the script (`mod.exe`) to perform verification and copying.
4. The script will pause at the end to allow viewing results.

## Requirements
- Windows Operating System
- Proper Minecraft directory structure
- `mods_update` folder containing mod files

## Algorithm
1. Starts from the program's directory, moves to parent directory.
2. Checks for `.minecraft` folder; switches to it if found.
3. Checks for `versions` folder; switches to it if found.
4. Checks for `BingoTech` folder; switches to it if found.
5. Checks for `mods` folder; displays "Verification complete." if found.
6. Returns to program directory, checks for `mods_update` folder; switches to it if found.
7. Copies all files from `mods_update` to `mods`, with MD5 checks.

## Compilation
Compile using GCC with required libraries:
```
gcc mod.c -o mod.exe -lshlwapi -lcrypt32
```

## Notes
- If a file is in use during copying, the operation will fail and report an error.
- The script uses English output only.
- Results are displayed and the program pauses before exiting.

## License
This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.