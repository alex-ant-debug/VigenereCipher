cmake_minimum_required(VERSION 3.1)	 # Checking the CMake version.
	

project(VigenereCipher)			# Project name

set(SOURCE_EXE src/VigenereCipher.c)		# Setting a variable with a list of sources for an executable file

set(SOURCE_LIB src/language.c)			# Same thing but for the library

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/inc)  # Connecting a file header

add_library(language STATIC ${SOURCE_LIB})	       # Create a static library with a language name

add_executable(VigenereCipher ${SOURCE_EXE})	       # Creates an executable named VigenereCipher

target_link_libraries(VigenereCipher language)       # Linking a program with a library
