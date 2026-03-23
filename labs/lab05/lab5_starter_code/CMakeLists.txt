# SafePulse Lab 4 - Zephyr RTOS Build Configuration
# Week 5: System Software, RTOS, Scheduling & Real-Time Properties

cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(safepulse_rtos)

target_sources(app PRIVATE main.c)
