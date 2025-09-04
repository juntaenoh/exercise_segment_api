# Exercise Segment Analysis API

Google ML Kit으로 추출한 포즈 데이터를 기반으로 운동 동작의 시작 키포즈에서 종료 키포즈까지의 진행도를 분석하고, 각 관절별로 어떻게 교정해야 하는지 실시간 피드백을 제공하는 C API입니다.

## 주요 기능

- **개인화**: 사용자 기본 포즈로 캘리브레이션하여 표준 키포즈를 개인 체형에 맞게 조정
- **진행도 분석**: 현재 포즈가 시작에서 끝까지 어느 정도 진행되었는지 0.0~1.0으로 계산
- **관절별 교정**: 11개 주요 관절별로 어느 방향으로 움직여야 하는지 3D 벡터로 제공
- **관심 영역**: 운동별로 중요한 관절만 선별하여 피드백 (Care/Don't Care)

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

### 기본 사용법

```c
#include "segment_api.h"

int main() {
    // API 초기화
    int result = segment_api_init();
    if (result != SEGMENT_OK) {
        printf("초기화 실패: %s\n", segment_get_error_message(result));
        return -1;
    }
    
    // 사용자 기본 포즈로 캘리브레이션
    PoseData base_pose = { /* 사용자 기본 포즈 데이터 */ };
    CalibrationData calibration;
    result = segment_calibrate(&base_pose, &calibration);
    if (result != SEGMENT_OK) {
        printf("캘리브레이션 실패: %s\n", segment_get_error_message(result));
        segment_api_cleanup();
        return -1;
    }
    
    // 운동 세그먼트 생성
    PoseData start_keypose = { /* 시작 키포즈 */ };
    PoseData end_keypose = { /* 종료 키포즈 */ };
    JointType care_joints[] = {JOINT_LEFT_ELBOW, JOINT_RIGHT_ELBOW, JOINT_LEFT_KNEE, JOINT_RIGHT_KNEE};
    
    result = segment_create(&start_keypose, &end_keypose, &calibration, 
                           care_joints, 4);
    if (result != SEGMENT_OK) {
        printf("세그먼트 생성 실패: %s\n", segment_get_error_message(result));
        segment_api_cleanup();
        return -1;
    }
    
    // 실시간 분석 루프
    while (/* 운동 진행 중 */) {
        PoseData current_pose = { /* 현재 포즈 데이터 */ };
        SegmentInput input = {current_pose};
        
        SegmentOutput output = segment_analyze(&input);
        
        printf("진행도: %.2f, 완료: %s, 유사도: %.2f\n", 
               output.progress, 
               output.completed ? "예" : "아니오",
               output.similarity);
        
        // 관절별 교정 벡터 출력
        for (int i = 0; i < 4; i++) {
            JointType joint = care_joints[i];
            Point3D correction = output.corrections[joint];
            printf("관절 %d 교정: (%.2f, %.2f, %.2f)\n", 
                   joint, correction.x, correction.y, correction.z);
        }
    }
    
    // 정리
    segment_destroy();
    segment_api_cleanup();
    
    return 0;
}
```

## API 참조

### 데이터 구조

- `Point3D`: 3D 좌표점
- `PoseData`: 포즈 데이터 (13개 관절 + 신뢰도)
- `CalibrationData`: 캘리브레이션 데이터
- `SegmentInput`: 분석 입력 데이터
- `SegmentOutput`: 분석 결과 데이터

### 주요 함수

- `segment_api_init()`: API 초기화
- `segment_calibrate()`: 사용자 캘리브레이션
- `segment_create()`: 운동 세그먼트 생성
- `segment_analyze()`: 실시간 포즈 분석
- `segment_destroy()`: 세그먼트 해제
- `segment_api_cleanup()`: API 정리

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
