# Copyright (C) 2008-2016 TrinityCore <http://www.trinitycore.org/>
#
# This file is free software; as a special exception the author gives
# unlimited permission to copy and/or distribute it, with or without
# modifications, as long as this notice is preserved.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY, to the extent permitted by law; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

# default linkage of module for SCRIPTS=0 (basic scripts only)
set(PREFER_BASIC_SCRIPTS   "static")
# default linkage of module for SCRIPTS=1 (prefer static linking)
set(PREFER_STATIC_SCRIPTS  "static")
# default linkage of module for SCRIPTS=2 (prefer dynamic linking)
set(PREFER_DYNAMIC_SCRIPTS "dynamic")
