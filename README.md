# Exercise Segment Analysis API v2.1.0

Google ML Kit으로 추출한 포즈 데이터를 기반으로 운동 동작의 시작 키포즈에서 종료 키포즈까지의 진행도를 분석하고, 각 관절별로 어떻게 교정해야 하는지 실시간 피드백을 제공하는 C API입니다.

## ✨ 주요 기능

### 🎯 핵심 기능
- **사용자 역할 분리**: A(기록자)와 B(사용자) 역할로 API 분리
- **JSON 기반 워크아웃**: 포즈 데이터를 JSON 파일로 관리
- **실시간 분석**: 60fps 목표 성능으로 실시간 포즈 분석
- **개인화**: 사용자 체형에 맞게 자동으로 포즈 변환
- **관절별 교정**: 33개 관절별로 3D 교정 벡터 제공

### 🆕 v2.1.0 새로운 기능
- **향상된 세그먼트 관리**: 전체 세그먼트 미리 로드로 분석 속도 향상
- **메모리 JSON 로드**: `segment_load_all_segments_from_json()` — Flutter asset 등 파일 경로 없이 로드
- **사용자 위치 기준 분석**: `segment_analyze_smart()`로 사용자 위치에 맞춘 목표 포즈 반환
- **효율적인 메모리 관리**: 세그먼트 캐싱 시스템으로 성능 최적화
- **확장된 테스트 도구**: 다양한 체형과 상황을 테스트할 수 있는 프로그램들

## 🚀 빠른 시작

### 빌드 및 실행

```bash
# 빌드
mkdir build && cd build
cmake .. && make

# 테스트 실행
./smart_demo                    # 사용자 위치 기준 분석 데모
./test_joint_calibration        # 관절별 캘리브레이션 테스트
./example_basic                 # 기본 사용법 데모
```

### 기본 사용법

#### A 이용자 (기록자) - 워크아웃 생성
```c
#include "segment_api.h"

int main() {
    // API 초기화
    segment_api_init();
    
    // A의 기본 포즈로 캘리브레이션
    PoseData myBasePose = { /* A가 자연스럽게 서있는 자세 */ };
    segment_calibrate_recorder(&myBasePose);
    
    // 포즈 기록
    PoseData standingPose = { /* 서기 자세 */ };
    segment_record_pose(&standingPose, "standing", "squat_workout.json");
    
    // 워크아웃 완성
    segment_finalize_workout_json("squat", "squat_workout.json");
    
    segment_api_cleanup();
    return 0;
}
```

#### B 이용자 (사용자) - 워크아웃 사용 (v2.1.0 권장)

**권장 호출 순서**

1. `segment_api_init()` — 초기화  
2. `segment_calibrate_user(PoseData*)` — 사용자 캘리브레이션  
3. workout JSON 로드 — 환경에 따라 아래 중 하나  
   - 네이티브(C/iOS 파일 경로): `segment_load_all_segments(path)`  
   - **Flutter asset(권장)**: `segment_load_all_segments_from_json(json, len)`  
4. `segment_set_current_segment(start, end)` — 사용할 세그먼트 선택  
5. `segment_analyze_smart(...)` — 실시간 분석  

**네이티브 (JSON 파일 경로)**

```c
#include "segment_api.h"

int main() {
    segment_api_init();
    
    // B의 기본 포즈로 캘리브레이션
    PoseData myBasePose = { /* B가 자연스럽게 서있는 자세 */ };
    segment_calibrate_user(&myBasePose);
    
    // 전체 세그먼트 미리 로드 (성능 향상)
    segment_load_all_segments("squat_workout.json");
    segment_set_current_segment(0, 1);
    
    // 실시간 분석
    while (/* 운동 진행 중 */) {
        PoseData currentPose = { /* 현재 포즈 데이터 */ };
        float progress, similarity;
        bool is_complete;
        Point3D corrections[POSE_LANDMARK_COUNT];
        PoseData targetPose;

        segment_analyze_smart(&currentPose, SCALE_MODE_EXERCISE,
                              1080.0f, 1920.0f,
                              &progress, &similarity, &is_complete,
                              corrections, &targetPose);

        printf("진행도: %.2f, 완료: %s, 유사도: %.2f\n",
               progress, is_complete ? "예" : "아니오", similarity);
    }

    segment_api_cleanup();
    return 0;
}
```

**메모리 JSON (Flutter asset / 임베디드 버퍼)**

```c
// json: UTF-8 바이트 버퍼, json_len: 길이 (null 종료 불필요)
int rc = segment_load_all_segments_from_json(json_bytes, json_len);
if (rc != SEGMENT_OK) { /* 오류 처리 */ }

segment_set_current_segment(0, 1);
// 이후 segment_analyze_smart() 동일
```

Flutter에서는 `exercise_segment_flutter` 패키지의 FFI(`loadAllSegmentsFromJsonBytes`)로 위 API를 호출합니다. 자세한 예시는 아래 [Flutter 사용법](#-flutter-사용법)을 참고하세요.

## 🧪 테스트 프로그램들

| 프로그램 | 설명 | 실행 명령 |
|---------|------|----------|
| `smart_demo` | 사용자 위치 기준 분석 데모 | `./smart_demo` |
| `test_joint_calibration` | 관절별 캘리브레이션 테스트 | `./test_joint_calibration` |
| `test_simple_feedback` | 간단한 피드백 테스트 | `./test_simple_feedback` |
| `test_realtime_feedback` | 실시간 피드백 테스트 | `./test_realtime_feedback` |
| `example_basic` | 기본 사용법 데모 | `./example_basic` |
| `realtime_demo` | 실시간 분석 데모 | `./realtime_demo` |

## 📚 API 참조

### 주요 함수

#### 기본 API
- `segment_api_init()`: API 초기화
- `segment_calibrate_user()`: 사용자 캘리브레이션
- `segment_analyze()`: 실시간 포즈 분석
- `segment_api_cleanup()`: API 정리

#### 향상된 세그먼트 관리 API (v2.1.0)
- `segment_load_all_segments(const char *json_file_path)`: JSON **파일 경로**에서 모든 세그먼트 미리 로드
- `segment_load_all_segments_from_json(const char *json, size_t json_len)`: JSON **메모리 버퍼**에서 로드 (Flutter asset 권장, `segment_calibrate_user()` 선행 필요)
- `segment_set_current_segment(int start_index, int end_index)`: `segment_load_all_segments*` 로드 후 세그먼트 선택
- `segment_analyze_smart(...)`: 사용자 위치 기준 목표 포즈 반환
- `segment_get_segment_info()`: 세그먼트 정보 조회

#### A 이용자 (기록자) API
- `segment_calibrate_recorder()`: 기록자 캘리브레이션
- `segment_record_pose()`: 포즈 기록 및 JSON 저장
- `segment_finalize_workout_json()`: 워크아웃 JSON 파일 완성

### 데이터 구조
- `Point3D`: 3D 좌표점
- `PoseData`: 포즈 데이터 (33개 관절 + 신뢰도)
- `CalibrationData`: 캘리브레이션 데이터

## 🏗️ 빌드 방법

### 요구사항
- CMake 3.16 이상
- C99 호환 컴파일러 (GCC, Clang, MSVC)

### 기본 빌드
```bash
mkdir build && cd build
cmake .. && make
```

### 플랫폼별 빌드

#### Android (NDK)

프로젝트 루트의 스크립트로 빌드·Flutter 패키지 복사까지 한 번에 수행합니다 (16KB 페이지 정렬 포함).

```bash
chmod +x android_build_script.sh
./android_build_script.sh
```

산출물: `build-android/libexercise_segment.so` → `exercise_segment_flutter/android/src/main/jniLibs/arm64-v8a/`

수동 빌드 예시:

```bash
cmake -DCMAKE_TOOLCHAIN_FILE=$ANDROID_NDK/build/cmake/android.toolchain.cmake \
      -DANDROID_ABI=arm64-v8a -DANDROID_PLATFORM=android-35 ..
cmake --build . -j
```

#### iOS (정적 라이브러리)

```bash
chmod +x ios_build_script.sh
./ios_build_script.sh
```

산출물을 `exercise_segment_flutter/ios/Native/`에 반영한 뒤 iOS 앱을 다시 빌드하세요.

#### Windows (Visual Studio)
```cmd
cmake -G "Visual Studio 16 2019" -A x64 ..
cmake --build . --config Release
```

## 📱 Flutter 사용법

Flutter asset은 앱 번들 안에 있어 **파일 시스템 경로로 `segment_load_all_segments()`를 쓰기 어렵습니다.**  
`rootBundle`으로 읽은 바이트를 `segment_load_all_segments_from_json()`에 넘기는 방식을 권장합니다.

### 권장 호출 순서

1. `segment_api_init()` (Dart: FFI 래퍼 초기화 시 자동/명시)  
2. `segment_calibrate_user(pose)`  
3. `segment_load_all_segments_from_json(json, len)` — asset 바이트  
4. `segment_set_current_segment(start, end)`  
5. `segment_analyze_smart(...)` 등 분석 API  

### Dart 예시 (`exercise_segment_flutter`)

```dart
import 'package:flutter/services.dart' show rootBundle;
import 'dart:typed_data';

Future<void> loadWorkoutFromAsset(ExerciseSegmentFfi api, String assetPath) async {
  final ByteData bd = await rootBundle.load(assetPath);
  final Uint8List bytes = bd.buffer.asUint8List(bd.offsetInBytes, bd.lengthInBytes);

  final code = api.loadAllSegmentsFromJsonBytes(bytes);
  if (code != 0) {
    throw Exception('loadAllSegmentsFromJsonBytes failed: $code');
  }

  api.setCurrentSegment(0, 1);
}
```

네이티브 `.so` / 정적 라이브러리를 수정한 뒤에는 **앱 완전 재빌드**가 필요합니다 (hot reload만으로는 FFI 심볼이 갱신되지 않을 수 있음).

- Android: `./android_build_script.sh` 실행 후 Flutter 앱 재빌드  
- iOS: `./ios_build_script.sh` 실행 후 `exercise_segment_flutter/ios/Native/` 갱신 확인  

## 📱 Swift 사용법

### ⚠️ Swift 사용 시 주의사항

Swift에서 C API를 사용할 때 다음과 같은 타입 변환 문제가 발생할 수 있습니다:

#### 문제 1: 타입 변환 오류
```
Cannot convert value of type 'Swift.UnsafePointer<FisicaWorkout.PoseData>' 
to expected argument type 'Swift.UnsafePointer<__ObjC.PoseData>'
```

#### 문제 2: __ObjC 타입 인식 오류
```
Cannot find type '__ObjC' in scope
```

#### 해결 방법

이러한 문제들은 **Bridging Header**에서 타입 별칭을 정의하여 해결되었습니다:

```objc
// FisicaWorkout-Bridging-Header.h
#import "segment_api.h"
#import "segment_types.h"

// Swift에서 타입 충돌을 방지하기 위한 별칭
typedef PoseData CAPoseData;
typedef PoseLandmark CAPoseLandmark;
typedef Point3D CAPoint3D;
typedef SegmentOutput CASegmentOutput;
typedef PoseLandmarkType CAPoseLandmarkType;
```

**핵심 해결책:**
1. **타입 별칭 사용**: `PoseData` → `CAPoseData`로 별칭을 만들어 Swift에서 명확하게 구분
2. **Bridging Header 활용**: Objective-C 브릿지를 통해 C 타입을 Swift에서 안전하게 사용
3. **withUnsafePointer 사용**: Swift에서 C 함수에 포인터를 안전하게 전달

### CocoaPods 설치
```ruby
# Podfile
pod 'ExerciseSegmentAPI', '~> 2.1'
pod 'GoogleMLKit/PoseDetection', '~> 4.0'
```

### 기본 사용법
```swift
import Foundation
import GoogleMLKit

class ExerciseSegmentUser: ObservableObject {
    func setupAsUser() -> Bool {
        let result = segment_api_init()
        return result == SEGMENT_OK
    }
    
    func calibrateUser(with pose: Pose) -> Bool {
        let poseData = convertMLKitPoseToPoseData(pose)
        let result = withUnsafePointer(to: poseData) { cPoseData in
            segment_calibrate_user(cPoseData)
        }
        return result == Int32(SEGMENT_OK.rawValue)
    }
    
    func analyzeCurrentPose(_ pose: Pose) -> (progress: Float, completed: Bool, similarity: Float)? {
        let poseData = convertMLKitPoseToPoseData(pose)
        var progress: Float = 0.0
        var completed: Bool = false
        var similarity: Float = 0.0
        
        let result = withUnsafePointer(to: poseData) { cPoseData in
            segment_analyze_simple(cPoseData, &progress, &completed, &similarity, nil)
        }
        
        guard result == Int32(SEGMENT_OK.rawValue) else { return nil }
        return (progress: progress, completed: completed, similarity: similarity)
    }
    
    private func convertMLKitPoseToPoseData(_ pose: Pose) -> CAPoseData {
        // Google ML Kit Pose를 CAPoseData로 변환
        // (자세한 구현은 소스 코드 참조)
    }
}
```

## ⚡ 성능 요구사항

- **실시간 처리**: 60fps 유지 (최대 16ms per frame)
- **메모리 사용량**: 최대 10MB
- **정확도**: 진행도 ±5% 오차, 교정 벡터 ±10% 오차

## 📄 라이선스

MIT License

## 🤝 기여하기

1. Fork the repository
2. Create your feature branch (`git checkout -b feature/AmazingFeature`)
3. Commit your changes (`git commit -m 'Add some AmazingFeature'`)
4. Push to the branch (`git push origin feature/AmazingFeature`)
5. Open a Pull Request

## 📞 문의사항

프로젝트에 대한 문의사항이나 버그 리포트는 GitHub Issues를 통해 제출해 주세요.