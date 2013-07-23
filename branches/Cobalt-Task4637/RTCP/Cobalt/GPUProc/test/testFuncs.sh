# Bash functions used by the different GPU tests.
#
# This file must be source'd, not executed!

# Check if our system has a GPU installed.
haveGPU()
{
  if ! lspci | grep -E "VGA|3D" | grep -E "ATI|NVIDIA" > /dev/null
  then
    echo "No ATI/NVIDIA GPU card detected."
    return 1
  fi
  return 0
}
