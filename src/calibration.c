/**
 * @file calibration.c
 * @brief 캘리브레이션 구현 (ML Kit 33개 랜드마크 지원)
 * @author Exercise Segment API Team
 * @version 2.0.0
 */

#include "calibration.h"
#include "math_utils.h"
#include "segment_api.h"
#include "segment_types.h"
#include <math.h>
#include <stdbool.h>
#include <string.h>

// segment_core.c에서 정의된 전역 변수들 extern 선언
extern CalibrationData g_recorder_calibration;
extern bool g_recorder_calibrated;

// 최소 신뢰도 임계값
#define MIN_CONFIDENCE_THRESHOLD 0.5f

// segment_calibrate_recorder는 segment_core.c에서 구현됨

int segment_calibrate_user(const PoseData *base_pose) {
  // 사용자 캘리브레이션은 기록자 캘리브레이션과 동일
  return segment_calibrate_recorder(base_pose);
}

CalibrationData *get_calibration_data(void) {
  if (!g_recorder_calibrated) {
    return NULL;
  }
  return &g_recorder_calibration;
}

bool is_calibrated(void) {
  return g_recorder_calibrated && g_recorder_calibration.is_calibrated;
}

void reset_calibration(void) {
  memset(&g_recorder_calibration, 0, sizeof(CalibrationData));
  g_recorder_calibrated = false;
}

int segment_calibrate(const PoseData *base_pose,
                      CalibrationData *out_calibration) {
  // 기본 캘리브레이션 함수 (기록자용)
  int result = segment_calibrate_recorder(base_pose);
  if (result == SEGMENT_OK && out_calibration) {
    *out_calibration = g_recorder_calibration;
  }
  return result;
}

bool segment_validate_calibration(const CalibrationData *calibration) {
  if (!calibration) {
    return false;
  }

  // 캘리브레이션 완료 여부 확인
  if (!calibration->is_calibrated) {
    return false;
  }

  // 스케일 팩터 유효성 검사
  if (calibration->scale_factor <= 0.1f || calibration->scale_factor >= 10.0f) {
    return false;
  }

  // 캘리브레이션 품질 검사
  if (calibration->calibration_quality < 0.5f) {
    return false;
  }

  return true;
}

float calculate_pose_scale_factor(const PoseData *pose) {
  if (!pose) {
    return 1.0f;
  }

  // 어깨 너비를 기준으로 스케일 팩터 계산
  float shoulder_width =
      distance_3d(&pose->landmarks[POSE_LANDMARK_LEFT_SHOULDER].position,
                  &pose->landmarks[POSE_LANDMARK_RIGHT_SHOULDER].position);

  if (shoulder_width <= 0.0f) {
    return 1.0f;
  }

  float standard_shoulder_width = 40.0f;
  return standard_shoulder_width / shoulder_width;
}

Point3D calculate_pose_center(const PoseData *pose) {
  Point3D center = {0.0f, 0.0f, 0.0f};

  if (!pose) {
    return center;
  }

  // 모든 랜드마크의 평균 위치 계산
  for (int i = 0; i < POSE_LANDMARK_COUNT; i++) {
    center.x += pose->landmarks[i].position.x;
    center.y += pose->landmarks[i].position.y;
    center.z += pose->landmarks[i].position.z;
  }

  center.x /= POSE_LANDMARK_COUNT;
  center.y /= POSE_LANDMARK_COUNT;
  center.z /= POSE_LANDMARK_COUNT;

  return center;
}
