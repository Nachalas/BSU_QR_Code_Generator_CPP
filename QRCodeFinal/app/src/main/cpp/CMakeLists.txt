cmake_minimum_required(VERSION 3.4.1)

add_library(
             main

             SHARED

                Main.cpp
                BitStream.h
                BitStream.cpp
                Constants.h
                Mode.h
                QRCode.h
                QRCodeSegment.h
                QRGenerator.h
        )

find_library(
              log-lib

              log )
