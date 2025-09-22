cmake_minimum_required(VERSION 3.16)
project(ExerciseSegmentAPI VERSION 1.0.0 LANGUAGES C)

# C 표준 설정
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# 컴파일러 플래그 설정
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -O3")
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -g -DDEBUG")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -DNDEBUG")

# 플랫폼별 최적화
if(CMAKE_SYSTEM_PROCESSOR MATCHES "arm64|aarch64")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -march=armv8-a")
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|amd64")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -march=native")
endif()

# 라이브러리 생성 (정적 + 동적)
add_library(exercise_segment_static STATIC
    src/segment_core.c
    src/calibration.c
    src/pose_analysis.c
    src/math_utils.c
)

add_library(exercise_segment SHARED
    src/segment_core.c
    src/calibration.c
    src/pose_analysis.c
    src/math_utils.c
)

# 헤더 파일 경로 설정
target_include_directories(exercise_segment_static PUBLIC include)
target_include_directories(exercise_segment PUBLIC include)

# 라이브러리 링크 (수학 라이브러리)
target_link_libraries(exercise_segment_static m)
target_link_libraries(exercise_segment m)

# 설치 설정
install(TARGETS exercise_segment exercise_segment_static
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
)

install(DIRECTORY include/
    DESTINATION include
    FILES_MATCHING PATTERN "*.h"
)

# 예제 실행 파일 생성
add_executable(example_basic examples/basic_example.c)
target_link_libraries(example_basic exercise_segment_static)

add_executable(realtime_demo examples/realtime_demo.c)
target_link_libraries(realtime_demo exercise_segment_static)

add_executable(smart_demo examples/smart_demo.c)
target_link_libraries(smart_demo exercise_segment_static)

add_executable(test_foot_center test_foot_center.c)
target_link_libraries(test_foot_center exercise_segment_static)

add_executable(test_realtime_feedback test_realtime_feedback.c)
target_link_libraries(test_realtime_feedback exercise_segment_static)

add_executable(test_simple_feedback test_simple_feedback.c)
target_link_libraries(test_simple_feedback exercise_segment_static)

add_executable(test_joint_calibration test_joint_calibration.c)
target_link_libraries(test_joint_calibration exercise_segment_static)

# 테스트 실행 파일 생성 (옵션)
option(BUILD_TESTS "Build test executables" OFF)
if(BUILD_TESTS)
    add_executable(test_math_utils tests/test_math_utils.c)
    target_link_libraries(test_math_utils exercise_segment_static)
    
    add_executable(test_calibration tests/test_calibration.c)
    target_link_libraries(test_calibration exercise_segment_static)
    
    add_executable(test_pose_analysis tests/test_pose_analysis.c)
    target_link_libraries(test_pose_analysis exercise_segment_static)
endif()

# 패키지 설정
set(CPACK_PACKAGE_NAME "ExerciseSegmentAPI")
set(CPACK_PACKAGE_VERSION "1.0.0")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Exercise Segment Analysis API")
set(CPACK_PACKAGE_VENDOR "Exercise Segment API Team")

include(CPack)
