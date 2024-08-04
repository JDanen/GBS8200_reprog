cmake_minimum_required(VERSION 3.18.0)

project(mtv230manipulator
	VERSION 0.1
	DESCRIPTION "MTV230M firmware read and write utilities"
	LANGUAGES C)


################	Dump	###################

add_executable(mtv230dump
	source/i2cdriver/i2cdriver.h
	source/i2cdriver/i2cdriver.c
	source/mtv230m/mtv230m.h
	source/mtv230m/mtv230m.c
	source/read/main.h
	source/read/main.c)

target_include_directories(mtv230dump PUBLIC
	source/i2cdriver
	source/mtv230m)	
# target_link_libraries(mtv230dump PUBLIC i2cdriver mtv230m)
target_compile_features(mtv230dump PRIVATE c_std_11)

################	Flash	###################

add_executable(mtv230flash
	source/i2cdriver/i2cdriver.h
	source/i2cdriver/i2cdriver.c
	source/mtv230m/mtv230m.h
	source/mtv230m/mtv230m.c
	source/write/main.h
	source/write/main.c)

target_include_directories(mtv230flash PUBLIC
	source/i2cdriver
	source/mtv230m)	
# target_link_libraries(mtv230dump PUBLIC i2cdriver mtv230m)
target_compile_features(mtv230flash PRIVATE c_std_11)
