#
# Useful functions that can be used by the cluster test scripts
#

# Print error message and exit.
error()
{
  local STATUS=$?
  echo >&2 "ERROR: $@"
  exit $STATUS
}

# Signal handler function. Prints exit status and returns it.
print_status()
{
  local STATUS=$?
  case $STATUS in
    0)
      echo >&2 "OK" ;;
    124)
      echo >&2 "TIMEOUT" ;;
    129|13[0-9]|14[0-3])
      echo >&2 "SIGNALLED ($STATUS)" ;;
    *)
      echo >&2 "ERROR     ($STATUS)" ;;
  esac
  return $STATUS
}

# Run a command with a timeout. The command is run in the background; so that
# the shell can react on signals (e.g., a SIGINT send from the keyboard).
#
# Usage: run_command [options] "command" [timeout]
#
# Valid options:
#   -q (quiet), redirect stdout to /dev/null
#
# The timeout period is given in seconds and defaults to 20.
run_command()
{
  while getopts "q" opt
  do
    case $opt in 
      q) exec 1>/dev/null ;;
    esac
  done
  shift $((OPTIND-1))
  COMMAND="$1"
  TIMEOUT="${2:-20}"  # default timeout: 20 seconds
  echo -n "$COMMAND:    " >&2
  timeout -k1 $TIMEOUT $COMMAND 2> /dev/null &
  wait $! 2> /dev/null
  print_status
}