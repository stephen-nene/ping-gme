#!/bin/bash

# Function to display usage information
usage() {
    echo "Usage: ./run.sh [-i] [-e]"
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
gcc Game.c -o Game  2> log/compile_error.log

# Check if compilation was successful
if [ $? -ne 0 ]; then
    echo "Error: Compilation failed"
    if [ "$enable_error_logging" = true ]; then
        log_file="log/error_log_$(date +"%Y-%m-%d_%H-%M-%S").txt"
        echo "Compilation failed at $(date)" > "$log_file"
        echo "Compilation error message:" >> "$log_file"
        cat log/compile_error.log >> "$log_file"
    fi
    exit 1
fi
# Function to handle cleanup before exiting
cleanup() {
    # Terminate the child terminal process
    kill -9 "$child_terminal_pid" >/dev/null 2>&1
    echo "Closed child terminal."
    exit 0
}

# Trap Ctrl+C and call the cleanup function
trap cleanup INT

# Open a new terminal window and run the program in the background
gnome-terminal --working-directory=$(pwd) -- bash -c './Game; exec bash' &
child_terminal_pid=$!

echo "To close game, press Ctrl + C...."


# Parent terminal continues to run until terminated manually
# while :
# do
#     sleep 1
# done