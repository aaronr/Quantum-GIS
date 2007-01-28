
# macro that checks for grass installation in specified directory

MACRO (CHECK_GRASS G_PREFIX)

  FIND_PATH (GRASS_INCLUDE_DIR grass/gis.h ${G_PREFIX}/include)

  SET (GRASS_LIB_NAMES gis vect dig2 dbmiclient dbmibase shape dgl rtree datetime linkm form gproj)

  SET (GRASS_LIBRARIES "")

  FOREACH (LIB ${GRASS_LIB_NAMES})
    SET(LIB_PATH NOTFOUND)
    FIND_LIBRARY(LIB_PATH grass_${LIB} PATHS ${G_PREFIX}/lib)

    IF (LIB_PATH)
      IF (NOT GRASS_LIBRARIES STREQUAL NOTFOUND)
        SET (GRASS_LIBRARIES ${GRASS_LIBRARIES} ${LIB_PATH})
      ENDIF (NOT GRASS_LIBRARIES STREQUAL NOTFOUND)
    ELSE (LIB_PATH)
      SET (GRASS_LIBRARIES NOTFOUND)
    ENDIF (LIB_PATH)

  ENDFOREACH (LIB)

  IF (GRASS_INCLUDE_DIR AND GRASS_LIBRARIES)
    SET (GRASS_FOUND TRUE)
    SET (GRASS_PREFIX ${G_PREFIX})
  ENDIF (GRASS_INCLUDE_DIR AND GRASS_LIBRARIES)
    
  MARK_AS_ADVANCED (
    GRASS_INCLUDE_DIR
    GRASS_LIBRARIES
  )

ENDMACRO (CHECK_GRASS)

###################################
# search for grass installations

# list of paths which to search - user's choice as first
SET (GRASS_PATHS ${GRASS_PREFIX} /usr/lib/grass)

IF (WITH_GRASS)

  FOREACH (G_PREFIX ${GRASS_PATHS})
    IF (NOT GRASS_FOUND)
      CHECK_GRASS(${G_PREFIX})
    ENDIF (NOT GRASS_FOUND)
  ENDFOREACH (G_PREFIX)

ENDIF (WITH_GRASS)

###################################

IF (GRASS_FOUND)

   # read grass version number and remove trailing newline
   FILE (READ ${GRASS_PREFIX}/etc/VERSIONNUMBER GRASS_VERSION)
   STRING(REPLACE "\n" "" GRASS_VERSION ${GRASS_VERSION})

   IF (NOT GRASS_FIND_QUIETLY)
      MESSAGE(STATUS "Found GRASS: ${GRASS_PREFIX} (${GRASS_VERSION})")
   ENDIF (NOT GRASS_FIND_QUIETLY)

  # openpty is currently needed for GRASS shell
  INCLUDE(CheckFunctionExists)
  SET (CMAKE_REQUIRED_INCLUDES pty.h)
  SET (CMAKE_REQUIRED_LIBRARIES util)
  CHECK_FUNCTION_EXISTS(openpty HAVE_OPENPTY)

  # add 'util' library to the dependencies
  IF (HAVE_OPENPTY)
    FIND_LIBRARY(OPENPTY_LIBRARY NAMES util PATH /usr/local/lib /usr/lib)
    SET (GRASS_LIBRARIES ${GRASS_LIBRARIES} ${OPENPTY_LIBRARY})
  ENDIF (HAVE_OPENPTY)

ELSE (GRASS_FOUND)

   IF (WITH_GRASS)

     IF (GRASS_FIND_REQUIRED)
        MESSAGE(FATAL_ERROR "Could not find GRASS")
     ELSE (GRASS_FIND_REQUIRED)
        MESSAGE(STATUS "Could not find GRASS")
     ENDIF (GRASS_FIND_REQUIRED)

   ENDIF (WITH_GRASS)

ENDIF (GRASS_FOUND)
