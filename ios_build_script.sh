#!/bin/bash
#
# iOS 정적 라이브러리 빌드 후 exercise_segment_flutter 패키지로 복사
#
# 사용:
#   chmod +x ios_build_script.sh
#   ./ios_build_script.sh
#
# 환경 변수 (선택):
#   FLUTTER_PACKAGE_DIR  복사 대상 Flutter 패키지 경로 (기본: ../exercise_segment_flutter)
#   IOS_DEPLOYMENT_TARGET  최소 iOS 버전 (기본: 13.0)
#   CMAKE_BIN            cmake 실행 파일 (기본: cmake)

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

FLUTTER_PACKAGE_DIR="${FLUTTER_PACKAGE_DIR:-$SCRIPT_DIR/../exercise_segment_flutter}"
IOS_DEPLOYMENT_TARGET="${IOS_DEPLOYMENT_TARGET:-13.0}"

resolve_cmake() {
  if [[ -n "${CMAKE_BIN:-}" ]] && command -v "$CMAKE_BIN" >/dev/null 2>&1; then
    return 0
  fi
  local candidate
  for candidate in cmake \
    /opt/homebrew/bin/cmake \
    /usr/local/bin/cmake \
    "$HOME/Library/Android/sdk/cmake/3.22.1/bin/cmake"; do
    if [[ -x "$candidate" ]]; then
      CMAKE_BIN="$candidate"
      return 0
    fi
  done
  return 1
}

if ! resolve_cmake; then
  echo "❌ cmake를 찾을 수 없습니다. Xcode CLT, Homebrew(brew install cmake), 또는 CMAKE_BIN= 경로를 지정하세요."
  exit 1
fi

IOS_NATIVE_DIR="$FLUTTER_PACKAGE_DIR/ios/Native"
IOS_LIB_DIR="$IOS_NATIVE_DIR/lib"
IOS_INCLUDE_DIR="$IOS_NATIVE_DIR/include"
STATIC_LIB_NAME="libexercise_segment_static.a"

echo "=== Exercise Segment API — iOS build ==="
echo "  Source:     $SCRIPT_DIR"
echo "  Flutter:    $FLUTTER_PACKAGE_DIR"
echo "  Deployment: iOS $IOS_DEPLOYMENT_TARGET (iphoneos / arm64)"

if ! xcrun --sdk iphoneos --show-sdk-path >/dev/null 2>&1; then
  echo "❌ iphoneos SDK를 찾을 수 없습니다. Xcode가 설치되어 있는지 확인하세요."
  exit 1
fi

IPHONEOS_SDK="$(xcrun --sdk iphoneos --show-sdk-path)"
echo "  CMake:      $CMAKE_BIN"
echo "  SDK:        $IPHONEOS_SDK"

# 1. 빌드 디렉토리
rm -rf build-ios
mkdir -p build-ios
cd build-ios

# 2. CMake 설정 (기기용 arm64 정적 라이브러리)
"$CMAKE_BIN" \
  -DCMAKE_SYSTEM_NAME=iOS \
  -DCMAKE_OSX_DEPLOYMENT_TARGET="$IOS_DEPLOYMENT_TARGET" \
  -DCMAKE_OSX_ARCHITECTURES=arm64 \
  -DCMAKE_OSX_SYSROOT=iphoneos \
  -DCMAKE_BUILD_TYPE=Release \
  -DEXERCISE_SEGMENT_BUILD_SHARED=OFF \
  -DEXERCISE_SEGMENT_BUILD_STATIC=ON \
  -DEXERCISE_SEGMENT_BUILD_EXAMPLES=OFF \
  ..

# 3. 빌드
"$CMAKE_BIN" --build . -j "$(sysctl -n hw.ncpu 2>/dev/null || echo 4)"

BUILT_LIB="libexercise_segment_static.a"
if [[ ! -f "$BUILT_LIB" ]]; then
  # 일부 CMake/제너레이터는 하위 폴더에 산출
  BUILT_LIB="$(find . -name 'libexercise_segment_static.a' -type f | head -1)"
fi

if [[ -z "$BUILT_LIB" || ! -f "$BUILT_LIB" ]]; then
  echo "❌ $STATIC_LIB_NAME 빌드 산출물을 찾을 수 없습니다."
  exit 1
fi

echo "✓ 빌드 완료: $SCRIPT_DIR/build-ios/$BUILT_LIB"

# 4. Flutter 패키지로 복사
mkdir -p "$IOS_LIB_DIR" "$IOS_INCLUDE_DIR"

cp "$BUILT_LIB" "$IOS_LIB_DIR/$STATIC_LIB_NAME"
cp "$SCRIPT_DIR/include/"*.h "$IOS_INCLUDE_DIR/"

echo "✓ 복사: $IOS_LIB_DIR/$STATIC_LIB_NAME"
echo "✓ 복사: $IOS_INCLUDE_DIR/*.h"

# 5. 심볼 확인 (FFI에서 사용하는 주요 API)
echo ""
echo "--- Exported symbols (FFI) ---"
if nm "$IOS_LIB_DIR/$STATIC_LIB_NAME" 2>/dev/null | grep -E 'segment_api_init|segment_calibrate_user|segment_load_all_segments_from_json' >/dev/null; then
  nm "$IOS_LIB_DIR/$STATIC_LIB_NAME" | grep -E ' T _segment_(api_init|calibrate_user|load_all_segments|set_current_segment|analyze_smart)' || true
  echo "✓ 주요 segment_* 심볼 확인됨"
else
  echo "⚠ segment_* 심볼을 찾지 못했습니다. force_load 설정을 확인하세요."
fi

# 6. 아키텍처 확인
echo ""
echo "--- Architecture ---"
lipo -info "$IOS_LIB_DIR/$STATIC_LIB_NAME"

echo ""
echo "=== 완료 ==="
echo "  다음: Flutter 앱에서 iOS 빌드 (flutter clean && flutter run)"
echo "  참고: 이 .a는 실기기(iphoneos) arm64 전용입니다."
echo "        시뮬레이터에서도 쓰려면 xcframework(iphoneos + iphonesimulator)가 필요합니다."
