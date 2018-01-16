cmake_minimum_required(VERSION 2.8)
#project name
project(quant_ctp_trade)

SET(CMAKE_C_COMPILER "/usr/local/bin/gcc")
SET(CMAKE_CXX_COMPILER "/usr/local/bin/g++")

#set compiler for c++ language
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x -liconv -D_GLIBCXX_USE_NANOSLEEP")

#source directory
set(SOURCE_FILES main.cpp USTPFtdcMduserApi.h USTPFtdcTraderApi.h USTPFtdcUserApiDataType.h USTPFtdcUserApiStruct.h FemasTdSpi.cpp FemasTdSpi.h QdpFtdcMdApi.h QdpFtdcTraderApi.h QdpFtdcUserApiDataType.h QdpFtdcUserApiStruct.h QdpTdSpi.cpp QdpTdSpi.h Utils.cpp Utils.h Timer.h Timer.cpp)

#set extern libraries
set(LIBRARIES libUSTPtraderapiAF.so libqdptraderapi.so)

#add execute file
add_executable(quant_ctp_trade ${SOURCE_FILES})

#add link library
target_link_libraries(quant_ctp_trade ${LIBRARIES})
