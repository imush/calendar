cmake_minimum_required(VERSION 3.20)
project(hconverter C)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# ── library ──────────────────────────────────────────────────────────────────
# Parsha data (name table + year-type schedules) lives in the
# hebrewcalendar-data submodule so it can be shared with other consumers
# (Flutter app, Scala site, Java lib) without drift.
set(HC_DATA_DIR ${CMAKE_CURRENT_SOURCE_DIR}/hebrewcalendar-data/generated/c)
if(NOT EXISTS ${HC_DATA_DIR}/parshiot_data.c)
    message(FATAL_ERROR
        "hebrewcalendar-data submodule not initialized. "
        "Run: git submodule update --init --recursive")
endif()

add_library(hconverter STATIC
    src/common.c
    src/gregorian.c
    src/julian.c
    src/hebrew.c
    src/hconverter.c
    src/tekufot.c
    src/parshiot.c
    src/hc_jewish_dates.c
    src/hc_haftarah.c
    src/hc_torah_reading.c
    src/noaa.c
    src/zmanim.c
    ${HC_DATA_DIR}/parshiot_data.c
    ${HC_DATA_DIR}/haftarot_data.c
    ${HC_DATA_DIR}/torah_data.c
)
target_include_directories(hconverter PUBLIC src ${HC_DATA_DIR})
target_link_libraries(hconverter m)
target_compile_options(hconverter PRIVATE -Wall -Wextra)

# ── CLI tool ──────────────────────────────────────────────────────────────────
add_executable(hconverter_cli src/main.c)
target_link_libraries(hconverter_cli hconverter)

# ── tests ─────────────────────────────────────────────────────────────────────
add_executable(hconverter_tests
    test/test_main.c
    test/test_jewish_time.c
    test/test_hconverter.c
    test/test_calendar_sweep.c
    test/test_parshiot.c
    test/test_haftarah.c
    test/test_torah_reading.c
    test/test_tekufot.c
    test/test_jewish_dates.c
    test/test_year_scan.c
    test/test_zmanim.c
)
target_link_libraries(hconverter_tests hconverter)
# The readings oracle: opentorah's own answers, shared with the Java library
# through the data submodule rather than copied per consumer.
target_compile_definitions(hconverter_tests PRIVATE
    HC_READINGS_ORACLE="${CMAKE_CURRENT_SOURCE_DIR}/hebrewcalendar-data/oracle/opentorah-readings.tsv")

enable_testing()
add_test(NAME all_tests COMMAND hconverter_tests)
