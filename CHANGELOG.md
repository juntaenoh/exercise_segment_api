# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [1.1.0] - 2024-12-19

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
