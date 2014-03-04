# Bash functions used by the different GPU tests.
#
# This file must be source'd, not executed!

# Generic function to report and handle errors.
error()
{
  echo "ERROR: $@"
  exit 1
}

# Make sure LOFARROOT is set, otherwise bail out.
[ -n "$LOFARROOT" ] || error "Environment variable LOFARROOT is not set!"

# Check if runtime output directories $LOFARROOT/var/{log,run} exist,
# otherwise bail out.
for dir in $LOFARROOT/var/{log,run}
do
  [ -d "$dir" ] || error "Directory $dir does not exist!"
done

# Set all locales to "C" to avoid problems with, e.g., perl.
export LC_ALL="C"
