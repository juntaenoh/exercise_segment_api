# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

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
