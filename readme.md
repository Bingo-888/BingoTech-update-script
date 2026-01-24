# BingoTech Update Script

## Project Overview
This is a custom update script for the BingoTech modpack in Minecraft, created by the author. It can be used to update various aspects of the modpack, including mods, quests, configuration files, and even various modified codes.

## Project Features
- **Fully AI-Coded**: The entire project was developed using AI coding tools.
- **Cross-Platform**: Compatible with both Windows and Linux operating systems.
- **Version Updates**: Can be used to update major versions of the modpack or install extension packs for the modpack.
- **MIT License**: Licensed under the MIT License.

## What is an Extension Pack
The core modpack includes only the main mods, quests, and modifications. Extension packs allow players with different needs to add extra content freely and based on their interests.

## Usage
1. Place the executable in a directory adjacent to the `resources` folder.
2. Ensure the Minecraft directory structure exists: `../.minecraft/versions/BingoTech`
3. Run the script to perform verification and copying.
4. The script will pause at the end to allow viewing results.

## Requirements
- Windows or Linux Operating System
- Proper Minecraft directory structure
- `resources` folder containing update files and directories
- On Linux, OpenSSL library for MD5 calculation
## Compilation
### Windows
Compile using GCC with required libraries:
```
gcc mod.c -o mod.exe -lshlwapi -lcrypt32
```

### Linux
Compile using GCC with OpenSSL:
```
gcc mod.c -o mod -lssl -lcrypto
```

## License
This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.
