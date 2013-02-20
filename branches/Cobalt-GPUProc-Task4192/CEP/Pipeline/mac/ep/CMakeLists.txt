include(PythonInstall)

python_install(
  __init__.py
  control/__init__.py
  echo/__init__.py
  interface/__init__.py
  DESTINATION ep)
