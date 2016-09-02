cmake_minimum_required(VERSION 3.3)
project(hpals)

set(CMAKE_INSTALL_PREFIX "/usr")
set(DBUS_CONFIG config/org.hpals.als.conf)
set(SYSTEMD_SERVICE config/org.hpals.als.service)

set(dir ${CMAKE_CURRENT_SOURCE_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${dir}/bin")

#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Werror")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")

###################
# <<< Service >>> #
###################
set(SOURCE_FILES_D src/hpals_service.c)
add_executable(hpals_service ${SOURCE_FILES_D})

# Dependencies
find_package(Eldbus)
if(ELDBUS_FOUND)
    include_directories(${ELDBUS_INCLUDE_DIRS})
    target_link_libraries(hpals_service ${ELDBUS_LIBRARIES})
endif(ELDBUS_FOUND)

find_package(Ecore)
if(ECORE_FOUND)
    include_directories(${ECORE_INCLUDE_DIRS})
    target_link_libraries(hpals_service ${ECORE_LIBRARIES})
endif(ECORE_FOUND)

find_package(Eeze)
if(EEZE_FOUND)
    include_directories(${EEZE_INCLUDE_DIRS})
    target_link_libraries(hpals_service ${EEZE_LIBRARIES})
endif(EEZE_FOUND)

###################
# <<< Client >>> #
###################
set(SOURCE_FILES_C src/hpals_client.c src/hpals_client.h src/hpals_service.h)
add_executable(hpals_client ${SOURCE_FILES_C})

# Dependencies
find_package(Eldbus)
if(ELDBUS_FOUND)
    include_directories(${ELDBUS_INCLUDE_DIRS})
    target_link_libraries(hpals_client ${ELDBUS_LIBRARIES})
endif(ELDBUS_FOUND)
find_package(Ecore)
if(ECORE_FOUND)
    include_directories(${ECORE_INCLUDE_DIRS})
    target_link_libraries(hpals_client ${ECORE_LIBRARIES})
endif(ECORE_FOUND)
find_package(Eet)
if(EET_FOUND)
    include_directories(${EET_INCLUDE_DIRS})
    target_link_libraries(hpals_client ${EET_LIBRARIES})
endif(EET_FOUND)
find_package(Eina)
if(EINA_FOUND)
    include_directories(${EINA_INCLUDE_DIRS})
    target_link_libraries(hpals_client ${EINA_LIBRARIES})
endif(EINA_FOUND)

###################
# <<< INSTALL >>> #
###################
# add the install targets
install(TARGETS hpals_service DESTINATION bin)
install(TARGETS hpals_client DESTINATION bin)

install(FILES ${DBUS_CONFIG} DESTINATION /etc/dbus-1/system.d)
install(FILES ${SYSTEMD_SERVICE} DESTINATION /usr/share/dbus-1/system-services)

