TEMPLATE    = app
TARGET      = raintk

# sdl - required to use ks_platform
# PATH_SDL_LIB = ...
# PATH_SDL_INCLUDE = ...

# ks - specify include path for ks
# PATH_KS = ...
include($${PATH_KS}/ks_test/ks.pri)

# raintk
PATH_RAINTK = $${_PRO_FILE_PWD_}
include($${_PRO_FILE_PWD_}/raintk.pri)
