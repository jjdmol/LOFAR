#
# This file contains functions that can be used by unit test programs that
# use PVSS.
#

# Environment variable PVSS_II must be exported, otherwise the PVSS services
# will not startup properly.
export PVSS_II

# Full path to the global PVSS configuration file.
PVSSINST_CONF=/etc/opt/pvss/pvssInst.conf

# Variable that records if we started PVSS services. If we did not, we should
# not shut them down.
PVSS_STARTED_BY_ME=0

# Simple error function. Print error message and exit with status 1.
error()
{
  echo "ERROR: $@" >&2
  exit 1
}

# Initialize the environment
init()
{
  # Check if the global PVSS configuration file exists.
  [ -f $PVSSINST_CONF ] || \
    error "Global PVSS configuration file $PVSSINST_CONF does not exist."

  # Get PVSS installation directory from the global PVSS configuration file.
  pvsshome=$(
    sed -n '/^\[Software\\ETM\\PVSS II\\[0-9]\+\.[0-9]\+\]$/,/^\[/ { 
           s,^InstallationDir *= *"\([^"]*\)"$,\1,p }' $PVSSINST_CONF
  )

  # Check if the install directory exists.
  [ -d $pvsshome ] || \
    error "PVSS installation directory $pvsshome does not exist."

  # Add the directory $pvsshome/bin to PATH
  PATH=$pvsshome/bin:$PATH

  # Set the PVSS_II variable to full path to the project's configuration file.
  PVSS_II=$(pvss_project_config)

  # Unset DISPLAY to avoid that PVSS litters the screen with windows.
  unset DISPLAY
}

# Determine the PVSS command prefix. PVSS versions <= 3.10 use "PVSS00" as
# prefix for most commands; PVSS versions > 3.10 use "WCCOA" as prefix.
pvss_cmdprefix()
{
  pvss_version=$(
    sed -n 's,^\[Software\\ETM\\PVSS II\\\([0-9]\+\.[0-9]\+\)\]$,\1,p' \
      $PVSSINST_CONF)
  pvss_version_major=$(echo $pvss_version | cut -d'.' -f1)
  pvss_version_minor=$(echo $pvss_version | cut -d'.' -f2)

  # We don't support PVSS versions other than 3.x
  [ $pvss_version_major -eq 3 ] || \
    error "Unsupported PVSS version $pvss_version"

  [ $pvss_version_minor -le 10 ] && { echo "PVSS00"; return; }
  [ $pvss_version_minor -gt 10 ] && { echo "WCCOA"; return; }
}

# Import the given data-point-list file into the PVSS database
pvss_import_dplist()
{
  [ $# -eq 1 ] || error "Usage: pvss_import_dplist <filename>"
  cmd="$(pvss_cmdprefix)ascii -yes -in $1"
  $cmd || error "Error executing command: $cmd"
}

# Get the full path to the project configuration file of the current PVSS
# project. Use some sed-magic to extract this information from the global PVSS
# configuration file. First get the name of the current project; then get the
# location of the projection configuration file for the current project.
pvss_project_config()
{
  currentProj=$(
    sed -n 's,^currentProject *= *"\([^"]*\)"$,\1,p' $PVSSINST_CONF
  )
  pvss_ii=$(
    sed -n '/^\[.*\\'$currentProj'\]$/,/^\[/s,^PVSS_II *= *"\([^"]*\)"$,\1,p' \
      $PVSSINST_CONF
  )
  [ -f $pvss_ii ] && echo "$pvss_ii" || \
    error "PVSS project configuration file $pvss_ii does not exist."
}

setup()
{
  init
  start_pvss2 || error "Failed to start PVSS services"
  PVSS_STARTED_BY_ME=1
  sleep 5    # wait for the PVSS services to start
}

teardown()
{
  [ $PVSS_STARTED_BY_ME -eq 1 ] && \
    { kill_pvss2 || error "Failed to stop PVSS services"; }
}
