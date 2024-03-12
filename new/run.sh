#!/bin/bash

# Function to display usage information
usage() {
    echo "Usage: ./run_game.sh [-i] [-e]"
    echo "Options:"
    echo "  -i: Keep intermediate files (e.g., object files)"
    echo "  -e: Enable error logging"
    exit 1
}

# Check if the number of arguments is at least 1
if [ $# -eq 0 ]; then
    usage
fi

# Process command line options
keep_intermediate=false
enable_error_logging=false

while getopts ":ie" option; do
    case $option in
        i)
            keep_intermediate=true
            ;;
        e)
            enable_error_logging=true
            ;;
        \?)
            echo "Invalid option: -$OPTARG" >&2
            usage
            ;;
    esac
done

# Compile Game.c
gcc src/Game.c -o Game 2> src/log/compile_error.log

# Check if compilation was successful
if [ $? -ne 0 ]; then
    echo "Error: Compilation failed"
    if [ "$enable_error_logging" = true ]; then
        log_file="src/log/error_log_$(date +"%Y-%m-%d_%H-%M-%S").txt"
        echo "Compilation failed at $(date)" > "$log_file"
        echo "Compilation error message:" >> "$log_file"
        cat src/log/compile_error.log >> "$log_file"
    fi
    exit 1
fi

# Run the program
./src/Game

# Remove intermediate files if the -i flag is not used
if [ "$keep_intermediate" = false ]; then
    rm -f src/Game src/log/compile_error.log
fi
