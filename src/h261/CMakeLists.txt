cmake_minimum_required(VERSION 3.1)
project(liby262)

find_package(Threads)
find_program(YASM_EXE NAMES yasm)

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
	set(ARCH "_x64")
	if(WIN32)
		set(YASM_ARGS -f win32 -m amd64 -DARCH_X86_64 -DPIC)
	elseif(APPLE)
		set(YASM_ARGS -f macho64 -m amd64 -DARCH_X86_64 -DPIC --prefix=_)
	else()
		set(YASM_ARGS -f elf64 -m amd64 -DARCH_X86_64 -DPIC)
	endif()
else()
	set(ARCH "_x86")
	if(WIN32)
		set(YASM_ARGS -f win32 --prefix=_)
	elseif(APPLE)
		set(YASM_ARGS -f macho32 --prefix=_)
	else()
		set(YASM_ARGS -f elf32)
	endif()
endif()

add_custom_command(OUTPUT h261_pixelop_x86.o COMMAND ${YASM_EXE}
	ARGS ${YASM_ARGS} -o ${CMAKE_CURRENT_BINARY_DIR}/h261_pixelop_x86.o ${CMAKE_CURRENT_SOURCE_DIR}/h261_pixelop_x86.asm)
add_custom_command(OUTPUT h261_transform_x86.o COMMAND ${YASM_EXE}
	ARGS ${YASM_ARGS} -o ${CMAKE_CURRENT_BINARY_DIR}/h261_transform_x86.o ${CMAKE_CURRENT_SOURCE_DIR}/h261_transform_x86.asm)

add_executable(h261enc
    ${CMAKE_CURRENT_SOURCE_DIR}/common.h
    ${CMAKE_CURRENT_SOURCE_DIR}/h261_bitstream.c
    ${CMAKE_CURRENT_SOURCE_DIR}/h261_bitstream.h
    ${CMAKE_CURRENT_SOURCE_DIR}/h261_decl.h
    ${CMAKE_CURRENT_SOURCE_DIR}/h261_gob.c
    ${CMAKE_CURRENT_SOURCE_DIR}/h261_gob.h
    ${CMAKE_CURRENT_SOURCE_DIR}/h261_macroblock.c
    ${CMAKE_CURRENT_SOURCE_DIR}/h261_macroblock.h
    ${CMAKE_CURRENT_SOURCE_DIR}/h261_main.c
    ${CMAKE_CURRENT_SOURCE_DIR}/h261_mbcoder.c
    ${CMAKE_CURRENT_SOURCE_DIR}/h261_mbcoder.h
    ${CMAKE_CURRENT_SOURCE_DIR}/h261_me.c
    ${CMAKE_CURRENT_SOURCE_DIR}/h261_me.h
    ${CMAKE_CURRENT_SOURCE_DIR}/h261_moded.c
    ${CMAKE_CURRENT_SOURCE_DIR}/h261_moded.h
    ${CMAKE_CURRENT_SOURCE_DIR}/h261_picture.c
    ${CMAKE_CURRENT_SOURCE_DIR}/h261_picture.h
    ${CMAKE_CURRENT_SOURCE_DIR}/h261_pixelop.c
    ${CMAKE_CURRENT_SOURCE_DIR}/h261_pixelop.h
    ${CMAKE_CURRENT_SOURCE_DIR}/h261_pixelop_x86.h
    ${CMAKE_CURRENT_SOURCE_DIR}/h261_ratectrl.c
    ${CMAKE_CURRENT_SOURCE_DIR}/h261_ratectrl.h
    ${CMAKE_CURRENT_SOURCE_DIR}/h261_tables.c
    ${CMAKE_CURRENT_SOURCE_DIR}/h261_tables.h
    ${CMAKE_CURRENT_SOURCE_DIR}/h261_transform.c
    ${CMAKE_CURRENT_SOURCE_DIR}/h261_transform.h
    ${CMAKE_CURRENT_SOURCE_DIR}/h261_transform_x86.h
    ${CMAKE_CURRENT_SOURCE_DIR}/h261_types.h
    ${CMAKE_CURRENT_SOURCE_DIR}/sys_win32.c
    ${CMAKE_CURRENT_SOURCE_DIR}/sys_win32.h

	${CMAKE_CURRENT_BINARY_DIR}/h261_pixelop_x86.o
	${CMAKE_CURRENT_BINARY_DIR}/h261_transform_x86.o
)

	
set_target_properties(h261enc PROPERTIES 
	OUTPUT_NAME "h261enc$<$<CONFIG:Debug>:d>${ARCH}"
	RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

