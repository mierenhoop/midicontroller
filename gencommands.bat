@echo off

chcp 65001 > nul
ninja -t clean
ninja -t compdb > compile_commands.json
ninja