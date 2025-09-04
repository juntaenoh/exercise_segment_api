/**
 * @file segment_types.h
 * @brief 운동 세그먼트 분석 API의 기본 데이터 타입 정의
 * @author Exercise Segment API Team
 * @version 1.0.0
 */

#ifndef SEGMENT_TYPES_H
#define SEGMENT_TYPES_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 3D 좌표점 구조체
 * Google ML Kit 좌표계 기준:
 * - x: 양수(오른쪽), 음수(왼쪽)
 * - y: 양수(아래), 음수(위)  
 * - z: 양수(카메라에서 멀리), 음수(카메라로 가까이)
 */
typedef struct {
    float x, y, z;
} Point3D;

/**
 * @brief 관절 타입 열거형
 * Google ML Kit의 13개 주요 관절을 정의
 */
typedef enum {
    JOINT_NOSE = 0,           // 코 (머리 중심점)
    JOINT_LEFT_SHOULDER,      // 왼쪽 어깨
    JOINT_RIGHT_SHOULDER,     // 오른쪽 어깨
    JOINT_LEFT_ELBOW,         // 왼쪽 팔꿈치
    JOINT_RIGHT_ELBOW,        // 오른쪽 팔꿈치
    JOINT_LEFT_WRIST,         // 왼쪽 손목
    JOINT_RIGHT_WRIST,        // 오른쪽 손목
    JOINT_LEFT_HIP,           // 왼쪽 골반
    JOINT_RIGHT_HIP,          // 오른쪽 골반
    JOINT_LEFT_KNEE,          // 왼쪽 무릎
    JOINT_RIGHT_KNEE,         // 오른쪽 무릎
    JOINT_LEFT_ANKLE,         // 왼쪽 발목
    JOINT_RIGHT_ANKLE,        // 오른쪽 발목
    JOINT_COUNT               // 총 13개 (배열 크기용)
} JointType;

/**
 * @brief 포즈 데이터 구조체
 * Google ML Kit에서 추출한 포즈 정보를 담는 구조체
 */
typedef struct {
    Point3D joints[JOINT_COUNT];       // 13개 관절의 3D 좌표
    float confidence[JOINT_COUNT];     // 각 관절의 감지 신뢰도 (0.0~1.0)
    uint64_t timestamp;                // 포즈 캡처 시간 (밀리초)
} PoseData;

/**
 * @brief 캘리브레이션 데이터 구조체
 * 사용자 개인화를 위한 캘리브레이션 정보
 */
typedef struct {
    float scale_factor;                // 사용자/표준 크기 비율
    Point3D center_offset;             // 중심점 보정 오프셋
    bool is_calibrated;                // 캘리브레이션 완료 플래그
    float calibration_quality;         // 캘리브레이션 품질 점수 (0.0~1.0)
} CalibrationData;

/**
 * @brief 세그먼트 입력 구조체
 * 실시간 분석을 위한 입력 데이터
 */
typedef struct {
    PoseData raw_pose;                 // Google ML Kit에서 받은 원본 포즈
} SegmentInput;

/**
 * @brief 세그먼트 출력 구조체
 * 분석 결과 및 피드백 정보
 */
typedef struct {
    float progress;                    // 진행도 (0.0=시작, 1.0=완료)
    bool completed;                    // 세그먼트 완료 여부
    float similarity;                  // 현재 목표 키포즈와 유사도 (0.0~1.0)
    Point3D corrections[JOINT_COUNT];  // 각 관절별 교정 방향 벡터
    uint64_t timestamp;                // 분석 시점
} SegmentOutput;

/**
 * @brief 에러 코드 열거형
 * API 함수들의 반환값으로 사용
 */
typedef enum {
    SEGMENT_OK = 0,                    // 성공
    SEGMENT_ERROR_NOT_INITIALIZED = -1, // 시스템 초기화 안됨
    SEGMENT_ERROR_INVALID_POSE = -2,    // 잘못된 포즈 데이터
    SEGMENT_ERROR_CALIBRATION_FAILED = -3, // 캘리브레이션 실패
    SEGMENT_ERROR_SEGMENT_NOT_CREATED = -4, // 세그먼트 생성 안됨
    SEGMENT_ERROR_INVALID_PARAMETER = -5,   // 잘못된 매개변수
    SEGMENT_ERROR_MEMORY_ALLOCATION = -6    // 메모리 할당 실패
} SegmentErrorCode;

#ifdef __cplusplus
}
#endif

#endif // SEGMENT_TYPES_H
