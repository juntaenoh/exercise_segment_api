# Exercise Segment Analysis API v2.0.0

Google ML Kit으로 추출한 포즈 데이터를 기반으로 운동 동작의 시작 키포즈에서 종료 키포즈까지의 진행도를 분석하고, 각 관절별로 어떻게 교정해야 하는지 실시간 피드백을 제공하는 C API입니다.

## 🆕 v2.0.0 새로운 기능

- **사용자 역할 분리**: A(기록자)와 B(사용자) 역할로 API 분리
- **JSON 기반 워크아웃**: 포즈 데이터를 JSON 파일로 관리
- **이상적 표준 포즈**: API 내부에 완벽한 비율의 표준 포즈 저장
- **인덱스 기반 세그먼트**: 두 개의 인덱스로 구분 동작 정의
- **체형 자동 변환**: 사용자 체형에 맞게 자동으로 포즈 변환

## 주요 기능

### A 이용자 (기록자)
- **포즈 기록**: 운동 동작을 이상적 비율로 변환하여 JSON 파일로 저장
- **워크아웃 생성**: 여러 포즈를 하나의 워크아웃으로 묶어서 완성

### B 이용자 (사용자)
- **개인화**: 사용자 기본 포즈로 캘리브레이션하여 이상적 포즈를 개인 체형에 맞게 조정
- **진행도 분석**: 현재 포즈가 시작에서 끝까지 어느 정도 진행되었는지 0.0~1.0으로 계산
- **관절별 교정**: 13개 주요 관절별로 어느 방향으로 움직여야 하는지 3D 벡터로 제공
- **실시간 피드백**: 로드된 세그먼트에 맞춰 실시간 분석 및 피드백

## 설계 원칙

- **범용성**: 특정 운동에 종속되지 않는 최소 단위 기능
- **단순성**: 복잡한 조건 체크 없이 키포즈 비교 중심
- **확장성**: 다양한 운동과 플랫폼에서 활용 가능
- **실시간 처리**: 60fps 목표 성능

## 빌드 방법

### 요구사항
- CMake 3.16 이상
- C99 호환 컴파일러 (GCC, Clang, MSVC)

### 빌드 명령어

```bash
# 빌드 디렉토리 생성
mkdir build
cd build

# CMake 설정
cmake ..

# 빌드 실행
make

# 테스트 포함 빌드
cmake -DBUILD_TESTS=ON ..
make
```

### 플랫폼별 빌드

#### iOS
```bash
cmake -DCMAKE_SYSTEM_NAME=iOS -DCMAKE_OSX_DEPLOYMENT_TARGET=12.0 ..
make
```

#### Android (NDK)
```bash
cmake -DCMAKE_TOOLCHAIN_FILE=$ANDROID_NDK/build/cmake/android.toolchain.cmake \
      -DANDROID_ABI=arm64-v8a \
      -DANDROID_PLATFORM=android-21 ..
make
```

#### Windows (Visual Studio)
```cmd
cmake -G "Visual Studio 16 2019" -A x64 ..
cmake --build . --config Release
```

## 사용 예제

### A 이용자 (기록자) - 워크아웃 생성

```c
#include "segment_api.h"

int main() {
    // API 초기화
    int result = segment_api_init();
    if (result != SEGMENT_OK) {
        printf("초기화 실패: %s\n", segment_get_error_message(result));
        return -1;
    }
    
    // A의 기본 포즈로 캘리브레이션
    PoseData myBasePose = { /* A가 자연스럽게 서있는 자세 */ };
    result = segment_calibrate_recorder(&myBasePose);
    if (result != SEGMENT_OK) {
        printf("캘리브레이션 실패: %s\n", segment_get_error_message(result));
        segment_api_cleanup();
        return -1;
    }
    
    // 스쿼트 워크아웃 기록
    PoseData standingPose = { /* 서기 자세 */ };
    PoseData squatDownPose = { /* 스쿼트 내려가기 자세 */ };
    PoseData squatUpPose = { /* 스쿼트 올라가기 자세 */ };
    
    segment_record_pose(&standingPose, "standing", "squat_workout.json");
    segment_record_pose(&squatDownPose, "squat_down", "squat_workout.json");
    segment_record_pose(&squatUpPose, "squat_up", "squat_workout.json");
    
    // 워크아웃 완성
    segment_finalize_workout_json("squat", "squat_workout.json");
    
    // 정리
    segment_api_cleanup();
    
    return 0;
}
```

### B 이용자 (사용자) - 워크아웃 사용

```c
#include "segment_api.h"

int main() {
    // API 초기화
    int result = segment_api_init();
    if (result != SEGMENT_OK) {
        printf("초기화 실패: %s\n", segment_get_error_message(result));
        return -1;
    }
    
    // B의 기본 포즈로 캘리브레이션
    PoseData myBasePose = { /* B가 자연스럽게 서있는 자세 */ };
    result = segment_calibrate_user(&myBasePose);
    if (result != SEGMENT_OK) {
        printf("캘리브레이션 실패: %s\n", segment_get_error_message(result));
        segment_api_cleanup();
        return -1;
    }
    
    // 세그먼트 1: 서기 → 스쿼트 내려가기 (인덱스 0→1)
    result = segment_load_segment("squat_workout.json", 0, 1);
    if (result != SEGMENT_OK) {
        printf("세그먼트 로드 실패: %s\n", segment_get_error_message(result));
        segment_api_cleanup();
        return -1;
    }
    
    // 실시간 분석 루프
    while (/* 운동 진행 중 */) {
        PoseData currentPose = { /* 현재 포즈 데이터 */ };
        SegmentOutput output = segment_analyze(&currentPose);
        
        printf("진행도: %.2f, 완료: %s, 유사도: %.2f\n", 
               output.progress, 
               output.completed ? "예" : "아니오",
               output.similarity);
        
        // 목표 포즈 확인
        PoseData targetPose;
        segment_get_transformed_end_pose(&targetPose);
    }
    
    // 정리
    segment_api_cleanup();
    return 0;
}
```

## Swift 사용법 (v2.0.0)

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
pod 'ExerciseSegmentAPI', '~> 2.0'
pod 'GoogleMLKit/PoseDetection', '~> 4.0'
```

### A 이용자 (기록자) - 워크아웃 생성

```swift
import Foundation
import GoogleMLKit

class WorkoutRecorder: ObservableObject {
    private var isInitialized = false
    
    func setupAsRecorder() -> Bool {
        // API 초기화
        let result = segment_api_init()
        isInitialized = (result == SEGMENT_OK)
        return isInitialized
    }
    
    func calibrateRecorder(with pose: Pose) -> Bool {
        guard isInitialized else { return false }
        
        let poseData = convertMLKitPoseToPoseData(pose)
        
        let result = withUnsafePointer(to: poseData) { cPoseData in
            segment_calibrate_recorder(cPoseData)
        }
        
        return result == Int32(SEGMENT_OK.rawValue)
    }
    
    func recordPose(_ pose: Pose, name: String, workoutFile: String) -> Bool {
        guard isInitialized else { return false }
        
        let poseData = convertMLKitPoseToPoseData(pose)
        
        let result = withUnsafePointer(to: poseData) { cPoseData in
            segment_record_pose(cPoseData, name, workoutFile)
        }
        
        return result == Int32(SEGMENT_OK.rawValue)
    }
    
    func finalizeWorkout(name: String, workoutFile: String) -> Bool {
        guard isInitialized else { return false }
        
        let result = segment_finalize_workout_json(name, workoutFile)
        return result == Int32(SEGMENT_OK.rawValue)
    }
    
    // Google ML Kit Pose를 CAPoseData로 변환
    private func convertMLKitPoseToPoseData(_ pose: Pose) -> CAPoseData {
        var poseData = CAPoseData()
        
        // MLKit의 33개 랜드마크를 C API의 33개 랜드마크로 매핑
        let landmarkMapping: [(MLKit.PoseLandmarkType, Int32)] = [
            (.nose, 0), (.leftEyeInner, 1), (.leftEye, 2), (.leftEyeOuter, 3),
            (.rightEyeInner, 4), (.rightEye, 5), (.rightEyeOuter, 6),
            (.leftEar, 7), (.rightEar, 8), (.mouthLeft, 9), (.mouthRight, 10),
            (.leftShoulder, 11), (.rightShoulder, 12), (.leftElbow, 13), (.rightElbow, 14),
            (.leftWrist, 15), (.rightWrist, 16), (.leftPinkyFinger, 17), (.rightPinkyFinger, 18),
            (.leftIndexFinger, 19), (.rightIndexFinger, 20), (.leftThumb, 21), (.rightThumb, 22),
            (.leftHip, 23), (.rightHip, 24), (.leftKnee, 25), (.rightKnee, 26),
            (.leftAnkle, 27), (.rightAnkle, 28), (.leftHeel, 29), (.rightHeel, 30),
            (.leftAnkle, 31), (.rightAnkle, 32)
        ]
        
        for (mlKitType, cType) in landmarkMapping {
            let landmark = pose.landmark(ofType: mlKitType)
            poseData.landmarks[Int(cType)].position.x = Float(landmark.position.x)
            poseData.landmarks[Int(cType)].position.y = Float(landmark.position.y)
            poseData.landmarks[Int(cType)].position.z = 0.0
            poseData.landmarks[Int(cType)].inFrameLikelihood = landmark.inFrameLikelihood
        }
        
        poseData.timestamp = UInt64(Date().timeIntervalSince1970 * 1000)
        return poseData
    }
}
```

### B 이용자 (사용자) - 워크아웃 사용

```swift
import Foundation
import GoogleMLKit

class ExerciseSegmentUser: ObservableObject {
    private var isInitialized = false
    private var isCalibrated = false
    private var isSegmentLoaded = false
    
    func setupAsUser() -> Bool {
        // API 초기화
        let result = segment_api_init()
        isInitialized = (result == SEGMENT_OK)
        return isInitialized
    }
    
    func calibrateUser(with pose: Pose) -> Bool {
        guard isInitialized else { return false }
        
        let poseData = convertMLKitPoseToPoseData(pose)
        
        let result = withUnsafePointer(to: poseData) { cPoseData in
            segment_calibrate_user(cPoseData)
        }
        
        isCalibrated = (result == Int32(SEGMENT_OK.rawValue))
        return isCalibrated
    }
    
    func loadSegment(workoutFile: String, startIndex: Int, endIndex: Int) -> Bool {
        guard isInitialized && isCalibrated else { return false }
        
        let result = segment_load_segment(workoutFile, Int32(startIndex), Int32(endIndex))
        isSegmentLoaded = (result == Int32(SEGMENT_OK.rawValue))
        return isSegmentLoaded
    }
    
    func analyzeCurrentPose(_ pose: Pose) -> (progress: Float, completed: Bool, similarity: Float)? {
        guard isInitialized && isSegmentLoaded else { return nil }
        
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
    
    // Google ML Kit Pose를 CAPoseData로 변환 (동일한 변환 함수)
    private func convertMLKitPoseToPoseData(_ pose: Pose) -> CAPoseData {
        var poseData = CAPoseData()
        
        let landmarkMapping: [(MLKit.PoseLandmarkType, Int32)] = [
            (.nose, 0), (.leftEyeInner, 1), (.leftEye, 2), (.leftEyeOuter, 3),
            (.rightEyeInner, 4), (.rightEye, 5), (.rightEyeOuter, 6),
            (.leftEar, 7), (.rightEar, 8), (.mouthLeft, 9), (.mouthRight, 10),
            (.leftShoulder, 11), (.rightShoulder, 12), (.leftElbow, 13), (.rightElbow, 14),
            (.leftWrist, 15), (.rightWrist, 16), (.leftPinkyFinger, 17), (.rightPinkyFinger, 18),
            (.leftIndexFinger, 19), (.rightIndexFinger, 20), (.leftThumb, 21), (.rightThumb, 22),
            (.leftHip, 23), (.rightHip, 24), (.leftKnee, 25), (.rightKnee, 26),
            (.leftAnkle, 27), (.rightAnkle, 28), (.leftHeel, 29), (.rightHeel, 30),
            (.leftAnkle, 31), (.rightAnkle, 32)
        ]
        
        for (mlKitType, cType) in landmarkMapping {
            let landmark = pose.landmark(ofType: mlKitType)
            poseData.landmarks[Int(cType)].position.x = Float(landmark.position.x)
            poseData.landmarks[Int(cType)].position.y = Float(landmark.position.y)
            poseData.landmarks[Int(cType)].position.z = 0.0
            poseData.landmarks[Int(cType)].inFrameLikelihood = landmark.inFrameLikelihood
        }
        
        poseData.timestamp = UInt64(Date().timeIntervalSince1970 * 1000)
        return poseData
    }
}
```

### 고급 사용법

```swift
// 직접 C API 사용 (withUnsafePointer로 안전한 포인터 전달)
let startPoseData = convertMLKitPoseToPoseData(startPose)
let endPoseData = convertMLKitPoseToPoseData(endPose)
let calibrationData = getCalibrationData()

let result = withUnsafePointer(to: startPoseData) { startPtr in
    withUnsafePointer(to: endPoseData) { endPtr in
        withUnsafePointer(to: calibrationData) { calPtr in
            segment_create_with_indices(
                startPtr,
                endPtr,
                calPtr,
                jointIndices,
                Int32(jointIndices.count)
            )
        }
    }
}

// 단순화된 분석 (실제 해결된 방법)
var progress: Float = 0.0
var isComplete: Bool = false
var similarity: Float = 0.0
var corrections: [CAPoint3D] = Array(repeating: CAPoint3D(x: 0, y: 0, z: 0), count: 33)

let currentPoseData = convertMLKitPoseToPoseData(currentPose)

let result = withUnsafePointer(to: currentPoseData) { posePtr in
    segment_analyze_simple(
        posePtr,
        &progress,
        &isComplete,
        &similarity,
        &corrections
    )
}
```

자세한 Swift 사용법은 [SWIFT_USAGE.md](SWIFT_USAGE.md)를 참조하세요.

## API 참조

### 데이터 구조

- `Point3D`: 3D 좌표점
- `PoseData`: 포즈 데이터 (13개 관절 + 신뢰도)
- `CalibrationData`: 캘리브레이션 데이터
- `SegmentInput`: 분석 입력 데이터
- `SegmentOutput`: 분석 결과 데이터

### 주요 함수

#### 기본 API
- `segment_api_init()`: API 초기화
- `segment_calibrate()`: 사용자 캘리브레이션
- `segment_create()`: 운동 세그먼트 생성
- `segment_analyze()`: 실시간 포즈 분석
- `segment_destroy()`: 세그먼트 해제
- `segment_api_cleanup()`: API 정리

#### Swift 친화적인 함수 (v1.1.0)
- `segment_create_with_indices()`: 인덱스 배열로 세그먼트 생성
- `segment_analyze_simple()`: 단순화된 분석 결과 반환
- `segment_create_pose_data()`: 포즈 데이터 생성 헬퍼
- `segment_create_calibration_data()`: 캘리브레이션 데이터 생성 헬퍼

## 성능 요구사항

- **실시간 처리**: 60fps 유지 (최대 16ms per frame)
- **메모리 사용량**: 최대 10MB
- **정확도**: 진행도 ±5% 오차, 교정 벡터 ±10% 오차

## 라이선스

MIT License

## 기여하기

1. Fork the repository
2. Create your feature branch (`git checkout -b feature/AmazingFeature`)
3. Commit your changes (`git commit -m 'Add some AmazingFeature'`)
4. Push to the branch (`git push origin feature/AmazingFeature`)
5. Open a Pull Request

## 문의사항

프로젝트에 대한 문의사항이나 버그 리포트는 GitHub Issues를 통해 제출해 주세요.
