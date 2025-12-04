add_executable(ncomix makemix.cpp mixnamedb.cpp mixnamedb.h mixnamedb_data.cpp crc32.cpp crc32.h fastini.cpp fastini.h)
target_link_libraries(ncomix PUBLIC common miniposix ${STATIC_LIBS})

if (WIN32)
    target_compile_definitions(ncomix PRIVATE -DNOMINMAX)
endif()