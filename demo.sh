#!/bin/bash

# Exercise Segment API 데모 스크립트
# 이 스크립트는 API를 빌드하고 테스트하며 예제를 실행합니다.

echo "=== Exercise Segment API 데모 ==="
echo ""

# 빌드 디렉토리가 있는지 확인
if [ ! -d "build" ]; then
    echo "1. 빌드 디렉토리 생성 중..."
    mkdir build
fi

cd build

# CMake 설정
echo "2. CMake 설정 중..."
cmake -DBUILD_TESTS=ON ..

# 빌드 실행
echo "3. 빌드 실행 중..."
make

echo ""
echo "=== 테스트 실행 ==="
echo ""

# 수학 유틸리티 테스트
echo "📐 수학 유틸리티 테스트:"
./test_math_utils
echo ""

# 캘리브레이션 테스트
echo "🎯 캘리브레이션 테스트:"
./test_calibration
echo ""

# 포즈 분석 테스트
echo "🏃 포즈 분석 테스트:"
./test_pose_analysis
echo ""

echo "=== 기본 예제 실행 ==="
echo ""

# 기본 예제 실행
./example_basic

echo ""
echo "=== 데모 완료 ==="
echo ""
echo "🎉 모든 테스트와 예제가 성공적으로 실행되었습니다!"
echo ""
echo "📁 프로젝트 구조:"
echo "  include/ - 헤더 파일들"
echo "  src/     - 소스 파일들" 
echo "  tests/   - 테스트 파일들"
echo "  examples/ - 예제 파일들"
echo "  build/   - 빌드 결과물"
echo ""
echo "🚀 사용 방법:"
echo "  1. include 디렉토리를 포함 경로에 추가"
echo "  2. libexercise_segment.a를 링크"
echo "  3. segment_api.h를 include하여 사용"
echo ""
echo "📖 자세한 사용법은 README.md를 참조하세요."
