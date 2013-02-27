# Join lines that contain a backslash.
:join
/\\.*$/{N
s/\\.*\n//
b join
}

# Strip comments
s/#.*$//

# Strip all spaces
s/[ \t]//g

# Delete blank lines
/^$/d
