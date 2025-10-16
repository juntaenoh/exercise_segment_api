# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [2.2.1] - 2025-10-16
### Changed
- 진행도 계산 알고리즘 개선: 골반 중심 기준 상대 좌표 계산으로 변경
- 카메라 이동에 영향받지 않는 더 정확한 진행도 측정
- 가중 평균 방식으로 관절별 중요도 반영

### Added
- 테스트 유틸리티 추가: `check_pose_diff.c`, `test_mid_progress.c`, `test_real_json_poses.c`

### Fixed
- 세그먼트 진행도 계산 시 카메라 위치 변화에 민감하던 문제 해결

## [2.1.0] - 2025-09-17
### Added
- **향상된 세그먼트 관리 API**: 더 효율적인 워크아웃 관리
  - `segment_load_all_segments()`: JSON 파일에서 모든 세그먼트 미리 로드
  - `segment_set_current_segment()`: 미리 로드된 세그먼트 중 선택
  - `segment_analyze_smart()`: 사용자 위치 기준 목표 포즈 반환
  - `segment_get_segment_info()`: 세그먼트 정보 조회
- **새로운 테스트 프로그램들**:
  - `smart_demo.c`: 사용자 위치 기준 포즈 분석 데모
  - `test_simple_feedback.c`: 간단한 피드백 테스트
  - `test_joint_calibration.c`: 관절별 캘리브레이션 테스트
  - `test_realtime_feedback.c`: 실시간 피드백 테스트
- **Swift 호환성 강화**:
  - `segment_calibrate_recorder_swift()`: Swift 친화적 기록자 캘리브레이션
  - `segment_record_pose_swift()`: Swift 친화적 포즈 기록
- **JSON 워크아웃 파일 생성 도구**:
  - `create_test_json.c`: 테스트용 JSON 파일 생성
  - `test_json_load.c`: JSON 파일 로드 테스트

### Changed
- **성능 최적화**: 전체 세그먼트 미리 로드로 분석 속도 향상
- **메모리 관리 개선**: 효율적인 세그먼트 캐싱 시스템
- **API 사용성 개선**: 더 직관적인 함수명과 매개변수 구조

### Fixed
- 세그먼트 로드 시 메모리 누수 문제 해결
- 캘리브레이션 품질 검증 로직 개선
- 다양한 체형에서의 정확도 향상

## [2.0.0] - 2025-09-10
### Added
- **사용자 역할 분리**: A(기록자)와 B(사용자) 역할로 API 분리
  - `segment_calibrate_recorder()`: A 이용자 캘리브레이션
  - `segment_record_pose()`: 포즈 기록 및 JSON 저장
  - `segment_finalize_workout_json()`: 워크아웃 JSON 파일 완성
  - `segment_calibrate_user()`: B 이용자 캘리브레이션
  - `segment_load_segment()`: JSON 파일에서 세그먼트 로드
  - `segment_get_transformed_end_pose()`: 변환된 목표 포즈 반환
- **JSON 기반 워크아웃 관리**: 포즈 데이터를 JSON 파일로 저장 및 관리
- **이상적 표준 포즈**: API 내부에 완벽한 비율의 표준 포즈 저장
- **인덱스 기반 세그먼트**: 두 개의 인덱스로 구분 동작 정의
- **체형 자동 변환**: 사용자 체형에 맞게 자동으로 포즈 변환
- `segment_calibrate_between_poses()`: 두 포즈 간 캘리브레이션 데이터 계산

### Changed
- **API 구조 대폭 변경**: 기존 단일 사용자 모델에서 기록자/사용자 분리 모델로 전환
- `segment_analyze()`: `SegmentInput` 매개변수 제거, `PoseData` 직접 전달
- `SegmentInput` 구조체 제거: 더 이상 필요하지 않음
- 내부 데이터 구조 개선: 사용자별 캘리브레이션 데이터 분리 저장
- 예제 코드 업데이트: 새로운 API 구조에 맞게 수정

### Removed
- `segment_create()`: 기존 세그먼트 생성 함수 제거
- `segment_create_with_indices()`: Swift 친화적 함수 제거
- `segment_create_calibration_data()`: 불필요한 헬퍼 함수 제거
- `SegmentInput` 구조체: 더 이상 사용하지 않음

### Fixed
- 빌드 오류 수정: 새로운 API 구조에 맞게 컴파일 오류 해결
- 메모리 관리 개선: 사용자별 데이터 분리로 메모리 효율성 향상

## [1.1.0] - 2025-09-05

### Added
- Swift 친화적인 C 함수들 추가
  - `segment_create_with_indices()`: 인덱스 배열로 세그먼트 생성
  - `segment_analyze_simple()`: 단순화된 분석 결과 반환
  - `segment_create_pose_data()`: 포즈 데이터 생성 헬퍼
  - `segment_create_calibration_data()`: 캘리브레이션 데이터 생성 헬퍼
- Swift 브리징 헤더 추가 (`ExerciseSegmentAPI-Bridging-Header.h`)
- Swift 래퍼 클래스 (`ExerciseSegmentManager`) 추가
- CocoaPods 지원 강화
- GoogleMLKit/PoseDetection 의존성 추가

### Changed
- `PoseData` 구조체 개선: 튜플 → 배열로 변경하여 더 명확한 구조
- `CalibrationData` 구조체 필드명 개선:
  - `basePose` → `scaleFactor`, `centerOffset`로 분리
  - `is_calibrated` → `isCalibrated`로 명명 규칙 통일
- `SegmentOutput` 구조체 개선:
  - `isComplete` → `completed`로 변경
  - `corrections` 필드를 배열로 변경
- 에러 처리 개선: 더 상세한 에러 메시지 제공

### Fixed
- Swift-C 브리징 시 발생하는 타입 불일치 문제 해결
- `inout` 인수 전달 시 발생하는 컴파일 에러 해결
- GoogleMLKit import 문제 해결 (`import GoogleMLKit` → `import MLKit`)

### Documentation
- Swift 사용법 문서 추가 (`SWIFT_USAGE.md`)
- README.md에 Swift 사용 예제 추가
- API 문서 업데이트

## [1.0.0] - 2024-12-18

### Added
- 초기 릴리스
- 기본 C API 구현
- Google ML Kit 포즈 데이터 분석 기능
- 실시간 운동 세그먼트 분석
- 개인화 캘리브레이션 기능
- 13개 주요 관절 지원
- 크로스 플랫폼 빌드 지원 (iOS, Android, Windows, macOS, Linux)
