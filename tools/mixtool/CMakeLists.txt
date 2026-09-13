add_executable(nco-mix makemix.cpp mixnamedb.cpp mixnamedb.h mixnamedb_data.cpp crc32.cpp crc32.h fastini.cpp fastini.h ../../common/logger.cpp)
target_link_libraries(nco-mix PUBLIC common miniposix ${STATIC_LIBS})

if (WIN32)
    target_compile_definitions(nco-mix PRIVATE -DNOMINMAX)
endif()
