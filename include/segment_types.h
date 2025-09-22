/**
 * @file segment_types.h
 * @brief 운동 세그먼트 분석 API의 기본 데이터 타입 정의
 * @author Exercise Segment API Team
 * @version 1.0.0
 */

#ifndef SEGMENT_TYPES_H
#define SEGMENT_TYPES_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 2D 좌표점 구조체 (Google ML Kit PoseLandmark.position과 동일)
 * Google ML Kit 좌표계 기준:
 * - x: 양수(오른쪽), 음수(왼쪽)
 * - y: 양수(아래), 음수(위)
 */
typedef struct {
  float x, y;
} Point2D;

/**
 * @brief 3D 좌표점 구조체
 * 3차원 좌표계 기준:
 * - x: 양수(오른쪽), 음수(왼쪽)
 * - y: 양수(아래), 음수(위)
 * - z: 양수(앞), 음수(뒤)
 */
typedef struct {
  float x, y, z;
} Point3D;

/**
 * @brief 관절 타입 열거형
 * Google ML Kit의 33개 랜드마크를 정의 (ML Kit Pose Detection API 기준)
 */
typedef enum {
  /* 얼굴 (11개) */
  POSE_LANDMARK_NOSE = 0,
  POSE_LANDMARK_LEFT_EYE_INNER,
  POSE_LANDMARK_LEFT_EYE,
  POSE_LANDMARK_LEFT_EYE_OUTER,
  POSE_LANDMARK_RIGHT_EYE_INNER,
  POSE_LANDMARK_RIGHT_EYE,
  POSE_LANDMARK_RIGHT_EYE_OUTER,
  POSE_LANDMARK_LEFT_EAR,
  POSE_LANDMARK_RIGHT_EAR,
  POSE_LANDMARK_MOUTH_LEFT,
  POSE_LANDMARK_MOUTH_RIGHT,

  /* 상체 (12개) */
  POSE_LANDMARK_LEFT_SHOULDER,
  POSE_LANDMARK_RIGHT_SHOULDER,
  POSE_LANDMARK_LEFT_ELBOW,
  POSE_LANDMARK_RIGHT_ELBOW,
  POSE_LANDMARK_LEFT_WRIST,
  POSE_LANDMARK_RIGHT_WRIST,
  POSE_LANDMARK_LEFT_PINKY,
  POSE_LANDMARK_RIGHT_PINKY,
  POSE_LANDMARK_LEFT_INDEX,
  POSE_LANDMARK_RIGHT_INDEX,
  POSE_LANDMARK_LEFT_THUMB,
  POSE_LANDMARK_RIGHT_THUMB,

  /* 하체 (10개) */
  POSE_LANDMARK_LEFT_HIP,
  POSE_LANDMARK_RIGHT_HIP,
  POSE_LANDMARK_LEFT_KNEE,
  POSE_LANDMARK_RIGHT_KNEE,
  POSE_LANDMARK_LEFT_ANKLE,
  POSE_LANDMARK_RIGHT_ANKLE,
  POSE_LANDMARK_LEFT_HEEL,
  POSE_LANDMARK_RIGHT_HEEL,
  POSE_LANDMARK_LEFT_FOOT_INDEX,
  POSE_LANDMARK_RIGHT_FOOT_INDEX,

  POSE_LANDMARK_COUNT = 33 /* 총 33개 */
} PoseLandmarkType;

/* 기존 호환성을 위한 별칭 */
typedef PoseLandmarkType JointType;
#define JOINT_COUNT POSE_LANDMARK_COUNT

/**
 * @brief 포즈 랜드마크 구조체 (Google ML Kit PoseLandmark와 동일)
 * Google ML Kit에서 추출한 포즈 랜드마크 정보를 담는 구조체
 */
typedef struct {
  Point3D position; /* 3D 좌표 (x, y, z) - ML Kit의 position과 z 좌표 포함 */
  float inFrameLikelihood; /* 프레임 내 신뢰도 (0.0~1.0) - ML Kit의
                              inFrameLikelihood와 동일 */
} PoseLandmark;

/**
 * @brief 포즈 데이터 구조체
 * Google ML Kit Pose와 동일한 구조로 변경 (33개 랜드마크)
 */
typedef struct {
  PoseLandmark landmarks[POSE_LANDMARK_COUNT]; /* 33개 랜드마크 (Google ML Kit
                                                  Pose.landmarks와 동일) */
  uint64_t timestamp; /* 포즈 캡처 시간 (밀리초) */
} PoseData;

/* Swift 호환성을 위한 타입 별칭 */
#ifdef __cplusplus
extern "C" {
#endif

/* Swift에서 사용할 수 있도록 타입을 명시적으로 노출 */
typedef PoseData SegmentPoseData;
typedef Point2D SegmentPoint2D;
typedef Point3D SegmentPoint3D;
typedef PoseLandmark SegmentPoseLandmark;

/* Swift에서 __ObjC 접두사 없이 사용할 수 있도록 매크로 정의 */
#ifndef __OBJC__
#define __ObjC
#endif

#ifdef __cplusplus
}
#endif

/**
 * @brief 관절 간 연결 관계 정의
 * 관절별 길이 켈리브레이션을 위한 연결 정보
 */
typedef struct {
  PoseLandmarkType from_joint; /* 시작 관절 */
  PoseLandmarkType to_joint;   /* 끝 관절 */
  const char *name;            /* 연결 이름 (예: "상완", "대퇴") */
} JointConnection;

/**
 * @brief 관절별 길이 정보
 * 각 관절 간의 길이와 스케일 팩터
 */
typedef struct {
  float ideal_length; /* 이상적 길이 (표준 포즈 기준) */
  float user_length;  /* 사용자 실제 길이 */
  float scale_factor; /* 사용자/이상적 비율 */
  bool is_valid;      /* 유효한 측정인지 여부 */
} JointLength;

/**
 * @brief 관절별 길이 켈리브레이션 데이터
 * 모든 관절 간 연결의 길이 정보
 */
typedef struct {
  JointLength lengths[20]; /* 주요 관절 연결들 (최대 20개) */
  int count;               /* 실제 연결 개수 */
} JointLengthCalibration;

/**
 * @brief 캘리브레이션 데이터 구조체
 * 사용자 개인화를 위한 캘리브레이션 정보
 */
typedef struct {
  float scale_factor;    /* 사용자/표준 크기 비율 (전체 스케일) */
  Point3D center_offset; /* 중심점 보정 오프셋 (z는 0으로 설정) */
  bool is_calibrated;    /* 캘리브레이션 완료 플래그 */
  float calibration_quality; /* 캘리브레이션 품질 점수 (0.0~1.0) */
  JointLengthCalibration joint_lengths; /* 관절별 길이 켈리브레이션 */
} CalibrationData;

/**
 * @brief 세그먼트 입력 구조체
 * 실시간 분석을 위한 입력 데이터
 */
typedef struct {
  PoseData raw_pose; /* Google ML Kit에서 받은 원본 포즈 */
} SegmentInput;

/**
 * @brief 세그먼트 출력 구조체
 * 분석 결과 및 피드백 정보
 */
typedef struct {
  float progress;   /* 진행도 (0.0=시작, 1.0=완료) */
  bool completed;   /* 세그먼트 완료 여부 */
  float similarity; /* 현재 목표 키포즈와 유사도 (0.0~1.0) */
  Point3D corrections[JOINT_COUNT]; /* 각 관절별 교정 방향 벡터 */
  uint64_t timestamp;               /* 분석 시점 */
} SegmentOutput;

/**
 * @brief 에러 코드 열거형
 * API 함수들의 반환값으로 사용
 */
typedef enum {
  SEGMENT_OK = 0,                         /* 성공 */
  SEGMENT_ERROR_NOT_INITIALIZED = -1,     /* 시스템 초기화 안됨 */
  SEGMENT_ERROR_INVALID_POSE = -2,        /* 잘못된 포즈 데이터 */
  SEGMENT_ERROR_CALIBRATION_FAILED = -3,  /* 캘리브레이션 실패 */
  SEGMENT_ERROR_SEGMENT_NOT_CREATED = -4, /* 세그먼트 생성 안됨 */
  SEGMENT_ERROR_INVALID_PARAMETER = -5,   /* 잘못된 매개변수 */
  SEGMENT_ERROR_MEMORY_ALLOCATION = -6    /* 메모리 할당 실패 */
} SegmentErrorCode;

#ifdef __cplusplus
}
#endif

#endif /* SEGMENT_TYPES_H */
